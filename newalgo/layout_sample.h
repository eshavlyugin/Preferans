#pragma once

#include "common.h"

class LayoutSampler {
public:
	LayoutSampler(const GameState& state, uint32_t firstPlayer, uint32_t ourHero);

	vector<GameState> DoSample(uint32_t numOfSamples, bool playMoveHistory);

private:
	float dp_[11][11][11][3] = { 0.0f };
	CardsProbabilities probArray_ = {{0.0f}};
	GameState state_;
	uint32_t first_;
	Suit trump_;
};
