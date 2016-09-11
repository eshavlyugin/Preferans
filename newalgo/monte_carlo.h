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
	MonteCarloPlayer(vector<shared_ptr<IPlayer>> simulationPlayers, const string& evalModelFileName)
		: evaluator_(evalModelFileName)
		, players_(simulationPlayers)
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
	virtual Card DoMove() {
		ourHero_ = stateView_.GetCurPlayer();
		auto result = RunForNSimulations(IsCardsOpen() ? *xray_.get() : stateView_, 2000);
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

	vector<GameState> SampleLayouts(const GameState& prev, uint32_t count) {
		return SimpleSampler(prev, count, first_, ourHero_, /*playMoveHistory*/true);
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

	void DoSearch(shared_ptr<MCNode> node, GameState& game) {
		if (game.GetMoveNumber() == 10) {
			return;
		}
		// leaf node
		bool leaf = node->childs_.size() == 0;
		if (node->sampled_.size() == 0) {
			if (IsCardsOpen()) {
				node->sampled_ = {game};
			} else {
				// TODO: think of some dynamic sampling?
				node->sampled_ = SampleLayouts(game, 8);
			}
			for (const auto& sample : node->sampled_) {
				CardsProbabilities probs;
				array<MCNode*, 32> nodes = {{nullptr}};
				for (const auto child : node->childs_) {
					nodes[GetCardBit(child->move_)] = child.get();
				}
				for (auto move : sample.GenValidMoves()) {
					MCNode* child = nodes[GetCardBit(move)];
					if (!child) {
						node->childs_.push_back(shared_ptr<MCNode>(new MCNode()));
						child = node->childs_.back().get();
						child->move_ = move;
					}
					GameState copy = game;
					GameState sampleCopy = sample;
					copy.MakeMove(child->move_);
					sampleCopy.MakeMove(child->move_);
					StateContext ctx(copy, sampleCopy, probs, NoCard, ourHero_);
					child->n_ += 1;
					child->sum_ -= evaluator_.CalcWeights(ctx)[0];
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
			if (sample.IsValidMove(child1->move_) != sample.IsValidMove(child2->move_)) {
				return sample.IsValidMove(child1->move_) < sample.IsValidMove(child2->move_);
			}
			return eval(child1, log(total)) < eval(child2, log(total));
		});
		uint32_t curPlayer = game.GetCurPlayer();
		if (leaf) {
			game = sample;
			game.MakeMove(child->move_);
			PlayToTheEnd(game);
		} else {
			game.MakeMove(child->move_);
			DoSearch(child, game);
		}
		child->n_++;
		child->sum_ -= game.GetScores()[curPlayer];
	}

private:
	vector<shared_ptr<IPlayer>> players_;
	ModelPredictor evaluator_;
	GameState stateView_;
	shared_ptr<GameState> xray_;
	uint32_t ourHero_ = 0;
	uint32_t first_ = 0;
};
