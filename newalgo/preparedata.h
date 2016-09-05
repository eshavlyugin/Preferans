#include "common.h"
#include "player.h"
#include "playout.h"
#include "train_model.h"

#include <bits/stdc++.h>

using TEvalFunction = std::function<float(const std::array<float, 3>, uint8_t player)>;

struct GameSamplerOptions {
	uint32_t SamplesPerLayout = 10;
	uint32_t PlayoutsPerMove = 20;
	bool SampleWhenPlaying = false;
	bool OpenCards = false;
	float MinDiffirence = 0.0f;
};

class GameSampler {
public:
	GameSampler(vector<shared_ptr<IPlayer>> players, TEvalFunction eval,
			const GameSamplerOptions& options) :
			players_(players), eval_(eval), options_(options) {
	}

	TrainModel BuildTrainModel(const vector<GameState>& states);

private:
	void SampleForLayout(const GameState& layout,
			vector<shared_ptr<IPlayer>> players, uint8_t playerToSample,
			TrainModel& model);
	void SampleForLayoutScores(const GameState& layout,
			vector<shared_ptr<IPlayer>> players, uint8_t playerToSample,
			TrainModel& model);
	void DoSampling(const vector<GameState>& states,
			vector<shared_ptr<IPlayer>> players, TrainModel& model,
			uint32_t threadId, uint32_t threadCount);

private:
	vector<shared_ptr<IPlayer>> players_;
	TEvalFunction eval_;
	GameSamplerOptions options_;
	PlayoutManager playoutMgr_;
	std::mutex lock_;
};
