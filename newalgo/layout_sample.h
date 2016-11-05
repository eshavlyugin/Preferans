#pragma once

#include "common.h"

class LayoutSampler {
public:
	LayoutSampler(const GameState& state, uint32_t firstPlayer, uint32_t ourHero);
	LayoutSampler(const GameState& state, uint32_t firstPlayer, uint32_t ourHero, const CardsProbabilities& probsArray);

	vector<GameState> DoSample(uint32_t numOfSamples, bool playMoveHistory);

private:
	void InitProbsArray();
	void InitDpArray();

private:
	float dp_[11][11][11][3] = { 0.0f };
	CardsProbabilities probArray_ = {{0.0f}};
	GameState state_;
	uint32_t first_ = 0;
	uint32_t ourHero_ = 0;
	Suit trump_ = NoSuit;
};
