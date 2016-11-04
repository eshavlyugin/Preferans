/*
 * monte_carlo.h
 *
 *  Created on: Sep 4, 2016
 *      Author: zhenyok
 */

#pragma once

#include "common.h"
#include "gamemgr.h"
#include "layout_sample.h"
#include "player.h"
#include "train_model.h"

struct MCNode {
	int n_ = 0;
	float sum_ = 0;
	Card move_;
	list<shared_ptr<MCNode>> childs_;
	vector<GameState> sampled_;
};

class MonteCarloPlayer : public IPlayer {
public:
	MonteCarloPlayer(vector<shared_ptr<IPlayer>> simulationPlayers, const string& evalModelFileName, bool useTreeSearch)
		: players_(simulationPlayers)
		, useTreeSearch_(useTreeSearch)
	{
	}

	virtual void OnNewLayout(const GameState& game) {
		stateView_ = game;
		first_ = game.GetFirstPlayer();
	}

	virtual void OnNewXRayLayout(const GameState& game) {
		xray_.reset(new GameState(game));
	}

	virtual void OnMove(Card card) {
		stateView_.MakeMove(card);
		if (xray_.get()) {
			xray_->MakeMove(card);
		}
	}
	virtual Card DoMove(float* moveValue) {
		ourHero_ = stateView_.GetCurPlayer();
		Card result;
		if (!useTreeSearch_) {
			result = RunNoSearch(IsCardsOpen() ? *xray_.get() : stateView_, 2000, moveValue);
		} else {
			result = RunForNSimulations(IsCardsOpen() ? *xray_.get() : stateView_, 2000);
		}
		cerr << "OK!" << endl;
		return result;
	}
	virtual shared_ptr<IPlayer> Clone() {
		new shared_ptr<IPlayer>(new MonteCarloPlayer(*this));
	}
	virtual const GameState& GetStateView() const {
		return stateView_;
	}
	virtual void GetCardProbabilities(CardsProbabilities& res) {

	}

private:
	void PlayToTheEnd(GameState& game) {
		GameManager mgr(players_);
		mgr.SetNewLayout(game, /*isOpenCards*/true);
		mgr.PlayToTheEnd();
		game = mgr.GetState();
	}

	Card RunNoSearch(const GameState& state, uint32_t numOfSimulations, float* moveValue) {
		vector<GameState> states = SimpleSampler(state, numOfSimulations, first_, ourHero_, true);
		map<Card, float> stats;
		for (const auto& state : states) {
			for (Card move : state.GenValidMoves()) {
				GameState stateCopy = state;
				stateCopy.MakeMove(move);
				PlayToTheEnd(stateCopy);
				stats[move] -= stateCopy.GetScores()[ourHero_];
			}
		}
		for (auto it = stats.begin(); it != stats.end(); ++it) {
			cerr << CardToString(it->first) << " " << it->second << endl;
		}
		state.Dump(cerr);
		auto res = std::max_element(stats.begin(), stats.end(), [](auto pair1, auto pair2) {
			return pair1.second < pair2.second;
		});
		if (moveValue) {
			*moveValue = 1.0f;
			for (const auto& pair : stats) {
				if (pair.first != res->first) {
					*moveValue = std::min(*moveValue, 1.0f * (res->second - pair.second) / numOfSimulations);
				}
			}
		}
		return res->first;
	}

	Card RunForNSimulations(const GameState& state, uint32_t numOfSimulations) {
		auto root = shared_ptr<MCNode>(new MCNode());
		for (uint32_t i = 0; i < numOfSimulations; i++) {
			GameState copy = state;
			DoSearch(root, copy);
		}
		for (auto elem : root->childs_) {
			cerr << CardToString(elem->move_) << " " << elem->n_ << " " << elem->sum_ << endl;
		}
		state.Dump(cerr);
		return (*(std::max_element(root->childs_.begin(), root->childs_.end(), [](auto child1, auto child2) {
			return child1->sum_ * child2->n_ < child2->sum_ * child1->n_;
		})))->move_;
	}

	bool IsCardsOpen() const {
		return xray_.get();
	}

	bool DoSearch(shared_ptr<MCNode> node, GameState& game) {
		if (game.GetMoveNumber() == 10) {
			return true;
		}
		// leaf node
		bool leaf = node->childs_.size() == 0;
		if (node->sampled_.size() == 0) {
			if (!game.IsHandClosed(0) && !game.IsHandClosed(1) && !game.IsHandClosed(2)) {
				node->sampled_ = {game};
			} else {
				node->sampled_ = SimpleSampler(game, 32, first_, ourHero_, /*playMoveHistory*/true);
			}
			for (const auto& sample : node->sampled_) {
				CardsProbabilities probs;
				array<MCNode*, 32> nodes = {{nullptr}};
				for (const auto child : node->childs_) {
					nodes[GetCardBit(child->move_)] = child.get();
				}
				PREF_ASSERT(sample.GenValidMoves().Size() != 0 && "Something wrong with sampling");
				for (auto move : sample.GenValidMoves()) {
					MCNode* child = nodes[GetCardBit(move)];
					if (!child) {
						node->childs_.push_back(shared_ptr<MCNode>(new MCNode()));
						child = node->childs_.back().get();
						child->move_ = move;
					}
					child->n_ += 1;
					child->sum_ -= -2;//evaluator_.CalcWeights(ctx)[0];
				}
			}
		}
		const GameState& sample = node->sampled_[rand() % node->sampled_.size()];

		int total = 0;
		for (auto child : node->childs_) {
			if (sample.IsValidMove(child->move_)) {
				total += child->n_;
			}
		}
		static const float C = 3.8;
		auto eval = [](shared_ptr<MCNode> node, float logTotal) {
			return node->sum_ / node->n_ + C * sqrt(1e-6 + logTotal / node->n_);
		};
		auto child = *std::max_element(node->childs_.begin(), node->childs_.end(), [&](auto child1, auto child2) {
			bool valid1 = sample.IsValidMove(child1->move_);
			bool valid2 = sample.IsValidMove(child2->move_);
			if (valid1 != valid2) {
				return valid1 < valid2;
			}
			return eval(child1, log(total)) < eval(child2, log(total));
		});
		uint32_t curPlayer = game.GetCurPlayer();
		bool searched = true;
		if (leaf) {
			game = sample;
			game.MakeMove(child->move_);
			PlayToTheEnd(game);
		} else {
			game.MakeMove(child->move_);
			searched = DoSearch(child, game);
		}
		if (searched) {
			child->n_++;
			child->sum_ -= game.GetScores()[curPlayer];
		}
		return searched;
	}

private:
	vector<shared_ptr<IPlayer>> players_;
	//ModelPredictor evaluator_;
	GameState stateView_;
	shared_ptr<GameState> xray_;
	uint32_t ourHero_ = 0;
	uint32_t first_ = 0;
	bool useTreeSearch_ = false;
};
