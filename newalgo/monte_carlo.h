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
	std::array<shared_ptr<MCNode>, 32> childs_ = {{nullptr}};
	shared_ptr<LayoutSampler> sampler_;
	array<shared_ptr<FeaturesSet>, 3> features_;
	MCNode* parent_ = nullptr;
	uint32_t curPlayer = 0;
};

class MonteCarloPlayer : public IPlayer {
public:
	MonteCarloPlayer(vector<shared_ptr<IPlayer>> simulationPlayers, shared_ptr<IModel> posGenerator, bool useTreeSearch)
		: players_(simulationPlayers)
		, useTreeSearch_(useTreeSearch)
		, posProbGenerator_(posGenerator)
	{
	}

	void OnNewLayout(const GameState& game) override {
		stateView_ = game;
		std::fill(features_.begin(), features_.end(), vector<FeaturesSet>());
		first_ = game.GetFirstPlayer();
	}

	void OnNewXRayLayout(const GameState& game) override {
		xray_.reset(new GameState(game));
	}

	void OnMove(Card card) override {
		features_[stateView_.GetCurPlayer()].push_back(CalcFeatures(stateView_, card, ourHero_, FT_PosPredict));
		stateView_.MakeMove(card);
		if (xray_.get()) {
			xray_->MakeMove(card);
		}
	}

	Card DoMove(float* moveValue) override {
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

	shared_ptr<IPlayer> Clone() override {
		new shared_ptr<IPlayer>(new MonteCarloPlayer(*this));
	}

	const GameState& GetStateView() const override {
		return stateView_;
	}

private:
	void PlayToTheEnd(GameState& game) {
		GameManager mgr(players_);
		mgr.SetNewLayout(game, /*isOpenCards*/true);
		mgr.PlayToTheEnd();
		game = mgr.GetState();
	}

	CardsProbabilities GenerateProbsArray(const array<vector<FeaturesSet>, 3>& features) {
		CardsProbabilities probs;
		for (uint32_t player = 0; player < 3; ++player) {
			if (player != ourHero_) {
				auto predicted = posProbGenerator_->PredictSeq(features[player]);
				std::copy(predicted.begin(), predicted.end(), probs[player].begin());
			}
		}
		return probs;
	}

	Card RunNoSearch(const GameState& state, uint32_t numOfSimulations, float* moveValue) {
		auto probs = GenerateProbsArray(features_);
		LayoutSampler sampler(state, first_, ourHero_, probs);
		map<Card, float> stats;
		for (uint32_t i = 0; i < numOfSimulations; ++i) {
			vector<GameState> state = sampler.DoSample(1, /*playMoveHistory*/true);
			for (Card move : state[0].GenValidMoves()) {
				GameState stateCopy = state[0];
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
		vector<shared_ptr<MCNode>> notNull;
		for (auto elem : root->childs_) {
			if (elem != nullptr) {
				cerr << CardToString(elem->move_) << " " << elem->n_ << " " << elem->sum_ << endl;
				notNull.push_back(elem);
			}
		}
		state.Dump(cerr);
		return (*(std::max_element(notNull.begin(), notNull.end(), [](auto child1, auto child2) {
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
		bool leaf = node->n_ == 1;
		if (node->sampler_.get() == nullptr) {
			array<vector<FeaturesSet>, 3> features = features_;
			MCNode* current = node.get();
			while (current != nullptr) {
				for (uint32_t player = 0; player < 3; ++player) {
					if (current->features_[player].get()) {
						features[player].push_back(*current->features_[player].get());
					}
				}
				current = current->parent_;
			}
			auto probs = GenerateProbsArray(features);
			node->sampler_.reset(new LayoutSampler(game, first_, ourHero_));
		}
		vector<GameState> sample = node->sampler_->DoSample(1, true);
		uint32_t total = 0;
		for (const auto card : sample[0].GenValidMoves()) {
			auto bit = GetCardBit(card);
			auto child = node->childs_[bit];
			if (child == nullptr) {
				child.reset(new MCNode());
				child->parent_ = node.get();
				node->childs_[bit] = child;
				child->move_ = card;
				child->n_ = 1;
				child->sum_ = -2;
			}
			total += child->n_;
		}
		static const float C = 3.8;
		auto logTotal = log(total);
		auto eval = [](shared_ptr<MCNode> node, float logTotal) {
			return node->sum_ / node->n_ + C * sqrt(1e-6 + logTotal / node->n_);
		};
		shared_ptr<MCNode> bestChild = nullptr;
		for (const auto card : sample[0].GenValidMoves()) {
			auto child = node->childs_[GetCardBit(card)];
			if (bestChild == nullptr || eval(bestChild, logTotal) < eval(child, logTotal)) {
				bestChild = child;
			}
		}
		uint32_t curPlayer = game.GetCurPlayer();
		if (leaf) {
			sample[0].MakeMove(bestChild->move_);
			PlayToTheEnd(sample[0]);
		} else {
			if (bestChild->features_[game.GetCurPlayer()] == nullptr) {
				bestChild->features_[game.GetCurPlayer()] = shared_ptr<FeaturesSet>(new FeaturesSet(CalcFeatures(game, bestChild->move_, ourHero_, FT_PosPredict)));
			}
			game.MakeMove(bestChild->move_);
			DoSearch(bestChild, game);
		}
		bestChild->n_++;
		bestChild->sum_ -= game.GetScores()[curPlayer];
	}

private:
	vector<shared_ptr<IPlayer>> players_;
	shared_ptr<IModel> posProbGenerator_;
	GameState stateView_;
	shared_ptr<GameState> xray_;
	uint32_t ourHero_ = 0;
	uint32_t first_ = 0;
	bool useTreeSearch_ = false;
	array<vector<FeaturesSet>, 3> features_;
};
