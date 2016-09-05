#include "layout_sample.h"
#include "generate.h"

#include <gtest/gtest.h>

TEST (LayoutSamplingTest, GenerateSameLayout) {
	for (uint32_t iter = 0; iter < 100; iter++) {
		GameState state = GenLayout();
		CardsProbabilities probs = { { { { 0.0f } } } };
		for (uint32_t bit = 0; bit < 32; bit++) {
			for (uint32_t i = 0; i < 4; i++) {
				if (i == 3) {
					probs[i][bit] = 1.0f;
				} else if (state.Hand(i).IsInSet(CardFromBit(bit))) {
					probs[i][bit] = 1.0f;
					break;
				}
			}
		}

		vector<GameState> res = SampleFromDistribution(probs, 20, 2, Spades);
		for (const auto& resState : res) {
			ASSERT_EQ(state.Hand(0), resState.Hand(0));
			ASSERT_EQ(state.Hand(1), resState.Hand(1));
			ASSERT_EQ(state.Hand(2), resState.Hand(2));
			ASSERT_EQ(resState.GetTrump(), Spades);
			ASSERT_EQ(resState.GetCurPlayer(), 2);
		}
	}
}
