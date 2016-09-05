#include "layout_sample.h"

vector<GameState> SampleFromDistribution(const CardsProbabilities& probArray,
		uint32_t numOfSamples, uint32_t first, Suit trump) {
	// filling probabilities array
	for (uint32_t i = 0; i < probArray.size(); i++) {
		for (uint32_t j = 0; j < probArray[i].size(); j++) {
			cerr << probArray[i][j] << " ";
		}
		cerr << endl;
	}
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

	assert(dp[10][10][10][2] > 0.0f);
	// sampling
	vector<GameState> result;
	for (uint32_t iter = 0; iter < numOfSamples; iter++) {
		array<CardsSet, 3> players;
		array<uint32_t, 3> rem = { 10, 10, 10 };
		uint32_t widow = 2;
		while (rem[0] > 0 || rem[1] > 0 || rem[2] > 0 || widow > 0) {
			array<float, 4> probs = { { 0.0f } };
			uint32_t bit = rem[0] + rem[1] + rem[2] + widow - 1;
			probs[0] =
					rem[0] > 0 ?
							dp[rem[0] - 1][rem[1]][rem[2]][widow]
									* probArray[0][bit] :
							0.0f;
			probs[1] =
					rem[1] > 0 ?
							dp[rem[0]][rem[1] - 1][rem[2]][widow]
									* probArray[1][bit] :
							0.0f;
			probs[2] =
					rem[2] > 0 ?
							dp[rem[0]][rem[1]][rem[2] - 1][widow]
									* probArray[2][bit] :
							0.0f;
			probs[3] =
					widow > 0 ?
							dp[rem[0]][rem[1]][rem[2]][widow - 1]
									* probArray[3][bit] :
							0.0f;
			float sum = probs[0] + probs[1] + probs[2] + probs[3];
			assert(sum > 0.0f);
			float rnd = min(0.999999f * sum, sum * rand() / RAND_MAX);
			for (uint32_t idx = 0; idx < 4; idx++) {
				if (idx == 3) {
					assert(widow != 0);
					widow--;
				} else if (rnd < probs[idx]) {
					players[idx].Add(CardFromBit(bit));
					assert(rem[idx] != 0);
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
