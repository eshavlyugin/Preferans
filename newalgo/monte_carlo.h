/*
 * monte_carlo.h
 *
 *  Created on: Sep 4, 2016
 *      Author: zhenyok
 */

#pragma once

#include "common.h"
#include "gamemgr.h"
#include "player.h"
#include "train_model.h"

struct MCNode {
	int n_ = 0;
	float sum_ = 0;
	Card move_;
	list<shared_ptr<MCNode>> childs_;
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
	}
	virtual void OnNewXRayLayout(const GameState& game) {
		xray_ = game;
	}
	virtual void OnMove(Card card) {
		stateView_.MakeMove(card);
		xray_.MakeMove(card);
	}
	virtual Card DoMove() {
		ourHero_ = stateView_.GetCurPlayer();
		auto result = RunForNSimulations(stateView_, xray_, 10000);
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

	Card RunForNSimulations(const GameState& state, const GameState& xray, uint32_t numOfSimulations) {
		auto root = shared_ptr<MCNode>(new MCNode());
		for (uint32_t i = 0; i < numOfSimulations; i++) {
			GameState copy = state;
			GameState xrayCopy = xray;
			DoSearch(root, xrayCopy, copy);
		}
		for (auto elem : root->childs_) {
			cerr << CardToString(elem->move_) << " " << elem->n_ << " " << elem->sum_ << endl;
		}
		xray.Dump(cerr);
		return (*(std::max_element(root->childs_.begin(), root->childs_.end(), [](auto child1, auto child2) {
			return child1->sum_ * child2->n_ < child2->sum_ * child1->n_;
		})))->move_;
	}

	void DoSearch(shared_ptr<MCNode> node, GameState& game, GameState& playerView) {
		if (game.GetMoveNumber() == 10) {
			return;
		}
		// leaf node
		bool leaf = node->childs_.size() == 0;
		if (leaf) {
			CardsProbabilities probs;
			for (auto move : game.GenValidMoves()) {
				node->childs_.push_back(shared_ptr<MCNode>(new MCNode()));
				auto child = node->childs_.back();
				child->n_ = 1;
				GameState copyXray = game;
				GameState copy = playerView;
				copyXray.MakeMove(move);
				copy.MakeMove(move);
				StateContext ctx(copy, copyXray, probs, NoCard, ourHero_);
				child->sum_ = -evaluator_.CalcWeights(ctx)[0];
				child->move_ = move;
			}
		}

		int total = 0;
		for (auto child : node->childs_) {
			total += child->n_;
		}
		static const float C = 3.8;
		auto eval = [](shared_ptr<MCNode> node, float logTotal) {
			return node->sum_ / node->n_ + C * sqrt(1e-6 + logTotal / node->n_);
		};
		auto child = *std::max_element(node->childs_.begin(), node->childs_.end(), [&](auto child1, auto child2) {
			return eval(child1, log(total)) < eval(child2, log(total));
		});
		uint32_t curPlayer = game.GetCurPlayer();
		game.MakeMove(child->move_);
		playerView.MakeMove(child->move_);
		if (leaf) {
			PlayToTheEnd(game);
		} else {
			DoSearch(child, game, playerView);
		}
		child->n_++;
		child->sum_ -= game.GetScores()[curPlayer];
	}

private:
	vector<shared_ptr<IPlayer>> players_;
	ModelPredictor evaluator_;
	GameState stateView_;
	GameState xray_;
	uint32_t ourHero_ = 0;
};
