#include "layout_sample.h"

vector<GameState> SampleFromDistribution(const CardsProbabilities& probArray,
	uint32_t numOfSamples, uint32_t first, Suit trump)
{
	float dp[11][11][11][3] = { 0.0f };
	dp[0][0][0][0] = 1.0f;
	for (uint32_t p1 = 0; p1 <= 10; p1++) {
		for (uint32_t p2 = 0; p2 <= 10; p2++) {
			for (uint32_t p3 = 0; p3 <= 10; p3++) {
				for (uint32_t widow = 0; widow <= 2; widow++) {
					uint32_t bit = p1 + p2 + p3 + widow;
					if (p1 != 10) {
						dp[p1 + 1][p2][p3][widow] += dp[p1][p2][p3][widow]
								* probArray[0][bit];
					}
					if (p2 != 10) {
						dp[p1][p2 + 1][p3][widow] += dp[p1][p2][p3][widow]
								* probArray[1][bit];
					}
					if (p3 != 10) {
						dp[p1][p2][p3 + 1][widow] += dp[p1][p2][p3][widow]
								* probArray[2][bit];
					}
					if (widow != 2) {
						dp[p1][p2][p3][widow + 1] += dp[p1][p2][p3][widow]
								* probArray[3][bit];
					}
				}
			}
		}
	}

	PREF_ASSERT(dp[10][10][10][2] > 1e-11f);
	// sampling
	vector<GameState> result;
	for (uint32_t iter = 0; iter < numOfSamples; iter++) {
		array<CardsSet, 3> players;
		array<uint32_t, 3> rem = { 10, 10, 10 };
		uint32_t widow = 2;
		while (rem[0] > 0 || rem[1] > 0 || rem[2] > 0 || widow > 0) {
			array<float, 4> probs = {{0.0f}};
			uint32_t bit = rem[0] + rem[1] + rem[2] + widow - 1;
			probs[0] = rem[0] > 0 ? (dp[rem[0] - 1][rem[1]][rem[2]][widow] * probArray[0][bit]) : 0.0f;
			probs[1] = rem[1] > 0 ? (dp[rem[0]][rem[1] - 1][rem[2]][widow] * probArray[1][bit]) : 0.0f;
			probs[2] = rem[2] > 0 ? (dp[rem[0]][rem[1]][rem[2] - 1][widow] * probArray[2][bit]) : 0.0f;
			probs[3] = widow > 0 ? (dp[rem[0]][rem[1]][rem[2]][widow - 1] * probArray[3][bit]) : 0.0f;
			float sum = probs[0] + probs[1] + probs[2] + probs[3];
			PREF_ASSERT(sum > 1e-20f);
			float rnd = sum * rand() / RAND_MAX;
			rnd = std::min(rnd, sum * 0.999999f);
			rnd = std::max(rnd, 0.000001f);
			for (uint32_t idx = 0; idx < 4; idx++) {
				if (idx == 3) {
					PREF_ASSERT(widow != 0);
					widow--;
				} else if (rnd < probs[idx]) {
					players[idx].Add(CardFromBit(bit));
					PREF_ASSERT(rem[idx] != 0);
					rem[idx]--;
					break;
				}
				rnd -= probs[idx];
			}
		}
		result.push_back(GameState(players, first, trump));
	}
	return result;
}

vector<GameState> SimpleSampler(const GameState& state, uint32_t numOfSamples, uint32_t firstPlayer, uint32_t ourHero, bool playMoveHistory) {
	//state.Dump(cerr);
	//cerr << ourHero << " " << firstPlayer << endl;
	CardsProbabilities probs = {{0.0f}};
	CardsSet out;
	vector<CardsSet> hands(3);
	for (auto card : state.Hand(ourHero)) {
		out.Add(card);
		hands[ourHero].Add(card);
		probs[ourHero][GetCardBit(card)] = 1.0f;
	}
	for (const auto& moveData : state.GetMoveHistory()) {
		out.Add(moveData.card_);
		hands[moveData.player_].Add(moveData.card_);
		probs[moveData.player_][GetCardBit(moveData.card_)] = 1.0f;
	}
	for (uint32_t bit = 0; bit < 32; bit++) {
		auto card = CardFromBit(bit);
		if (out.IsInSet(card)) {
			continue;
		}
		auto suit = GetSuit(card);
		uint32_t total = 2;
		for (uint32_t player = 0; player < 3; player++) {
			if (!state.IsSuitOut(player, suit) && player != ourHero) {
				total += 10 - hands[player].Size();
			}
		}
		for (uint32_t player = 0; player < 3; player++) {
			if (!state.IsSuitOut(player, suit) && player != ourHero) {
				probs[player][bit] = 1.0f * (10 - hands[player].Size()) / total;
			}
		}
		probs[3][bit] = 2.0 / total;
	}
	vector<GameState> result = SampleFromDistribution(probs, numOfSamples, firstPlayer, NoSuit);
	if (playMoveHistory) {
		for (GameState& stateToReturn : result) {
			for (const auto& moveData : state.GetMoveHistory()) {
				stateToReturn.MakeMove(moveData.card_);
			}
		}
	}
	return result;
}

