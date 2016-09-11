#pragma once

#include "common.h"

vector<GameState> SampleFromDistribution(const CardsProbabilities& probArray,
		uint32_t numOfSamples, uint32_t first, Suit trump);
vector<GameState> SimpleSampler(const GameState& state, uint32_t numOfSamples, uint32_t firstPlayer, uint32_t ourHero, bool playMoveHistory);
