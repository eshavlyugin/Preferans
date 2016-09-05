#include "common.h"
#include "gamemgr.h"
#include "layout_sample.h"
#include "player.h"
#include "preparedata.h"
#include "train_model.h"

#include <bits/stdc++.h>

TrainModel GameSampler::BuildTrainModel(const std::vector<GameState>& states) {
	TrainModel result;
	vector<thread> pool;

	const uint32_t numThreads = 4;
	vector<vector<shared_ptr<IPlayer>>> players;
	for (int i = 0; i < numThreads; i++) {
		players.push_back(vector<shared_ptr<IPlayer>>());
		for (auto player : players_) {
			players.back().push_back(player->Clone());
		}
	}
	for (int i = 0; i < numThreads; i++) {
		pool.emplace_back(
				thread(
						[this, &result, &states, &players, i, numThreads] {this->DoSampling(states, players[i], result, i, numThreads);}));
	}

	for (auto& thread : pool) {
		thread.join();
	}

	return result;
}

void GameSampler::DoSampling(const vector<GameState>& states,
		vector<shared_ptr<IPlayer>> players, TrainModel& model,
		uint32_t threadId, uint32_t threadCount)
{
	for (auto i = threadId; i < states.size(); i += threadCount) {
		// SampleForLayout(states[i], players, /*playerToSample*/0, model);
		SampleForLayoutScores(states[i], players, /*playerToSample*/0, model);
	}
}

void GameSampler::SampleForLayoutScores(const GameState& layout,
	vector<shared_ptr<IPlayer>> players, uint8_t playerToSample, TrainModel& model)
{
	for (uint32_t i = 0; i < options_.SamplesPerLayout; i++) {
		GameManager manager(players);
		manager.SetNewLayout(layout, options_.OpenCards);
		uint8_t moveNumber = rand() % 10;
		uint8_t playerToMove = rand() % 3;
		const GameState* currentLayout = &manager.GetState();
		while (currentLayout->GetCurPlayer() != playerToMove
				|| currentLayout->GetMoveNumber() != moveNumber)
		{
			manager.PlayForNMoves(1);
			currentLayout = &manager.GetState();
		}
		array<float, 3> scores = {{ 0.0f }};
		for (uint32_t trial = 0; trial < options_.PlayoutsPerMove; trial++) {
			GameManager mgrCopy(manager);
			mgrCopy.PlayToTheEnd();
			for (uint32_t idx = 0; idx < 3; idx++) {
				scores[idx] += mgrCopy.GetState().GetScores()[idx] - manager.GetState().GetScores()[idx];
			}
		}
		for (uint32_t idx = 0; idx < 3; idx++) {
			scores[idx] /= options_.PlayoutsPerMove * (10 - moveNumber);
			assert(scores[idx] < 1.0001f && "Something wrong with the algorithm");
		}
		CardsProbabilities probabilities;
		players[playerToSample]->GetCardProbabilities(probabilities);

		StateContext ctx(players[playerToSample]->GetStateView(), manager.GetState(), probabilities, NoCard, playerToSample);
		ctx.SetPredictedScores(scores);
		lock_.lock();
		model.AddGameInfo(ctx);
		lock_.unlock();
	}
}

void GameSampler::SampleForLayout(const GameState& layout,
		vector<shared_ptr<IPlayer>> players, uint8_t playerToSample,
		TrainModel& model)
{
	for (uint32_t i = 0; i < options_.SamplesPerLayout; i++) {
		GameManager manager(players);
		manager.SetNewLayout(layout, options_.OpenCards);
		uint8_t moveNumber = rand() % 10;
		const GameState* currentLayout = &manager.GetState();
		while (currentLayout->GetCurPlayer() != playerToSample
				|| currentLayout->GetMoveNumber() != moveNumber)
		{
			manager.PlayForNMoves(1);
			currentLayout = &manager.GetState();
		}
		map<Card, array<float, 3>> result;
		if (options_.SampleWhenPlaying) {
			for (auto card : currentLayout->Hand(playerToSample)) {
				result[card] = array<float, 3>{{0.0f}};
			}
			CardsProbabilities probs;
			players[playerToSample]->GetCardProbabilities(probs);
			auto layouts = SampleFromDistribution(probs, options_.PlayoutsPerMove, layout.GetCurPlayer(), layout.GetTrump());
			for (const auto& layout : layouts) {
				assert(layout.Hand(playerToSample) == currentLayout->Hand(playerToSample) && "somehting wrong with the sampling algorithm");
				GameManager newMgr(players);
				newMgr.SetNewLayout(layout);
				const GameState* tmpLayout = &manager.GetState();
				while (tmpLayout->GetCurPlayer() != playerToSample
						|| currentLayout->GetMoveNumber() != moveNumber)
				{
					manager.PlayForNMoves(1);
					tmpLayout = &manager.GetState();
				}
				auto tmpRes = playoutMgr_.Play(manager, 1);
				for (auto pair : tmpRes) {
					for (uint32_t i = 0; i < pair.second.size(); i++) {
						result[pair.first][i] += pair.second[i];
					}
				}
			}
		} else {
			result = playoutMgr_.Play(manager, options_.PlayoutsPerMove);
		}
		auto evalTmp = [&](auto pair) {
			return eval_(pair.second, playerToSample);
		};
		map<Card, float> rewards;
		for (auto pair : result) {
			rewards[pair.first] = evalTmp(pair);
		}
		auto best = std::max_element(rewards.begin(), rewards.end(),
				[&](auto score1, auto score2) {return score1.second < score2.second;});
		auto bestCard = best->first;
		auto worst = std::min_element(rewards.begin(), rewards.end(),
			[&](auto score1, auto score2) {return score1.second < score2.second;});
		bool isGood = best->second - worst->second > options_.MinDiffirence;
		if (!isGood) {
			return;
		}
		float worstScore = worst->second;
		for (auto it = rewards.begin(); it != rewards.end(); ++it) {
			rewards[it->first] = it->second - worstScore;
		}
		CardsProbabilities probabilities;
		players[playerToSample]->GetCardProbabilities(probabilities);

		StateContext ctx(players[playerToSample]->GetStateView(), manager.GetState(), probabilities, bestCard, playerToSample);
		ctx.SetMovesReward(rewards);
		lock_.lock();
		model.AddGameInfo(ctx);
		lock_.unlock();
	}
}
