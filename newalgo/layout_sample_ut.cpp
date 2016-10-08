#include "layout_sample.h"
#include "gamemgr.h"
#include "generate.h"
#include "player.h"

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

TEST (LayoutSamplingTest, GenerateSimpleLayout) {
	for (uint32_t iter = 0; iter < 100; iter++) {
		GameState gameState = GenLayout();
		vector<shared_ptr<IPlayer>> players = {
			CreatePlayer("random:random:random"),
			CreatePlayer("random:random:random"),
			CreatePlayer("random:random:random")
		};
		GameManager manager(players);
		manager.SetNewLayout(gameState, /*openCards=*/true);
		manager.PlayForNMoves(15);
		auto newLayouts = SimpleSampler(manager.GetState(), 5, gameState.GetFirstPlayer(), 0, /*playMoveHistory=*/false, /*canBeInvalid*/false);
		for (const auto& newLayout : newLayouts) {
			// 2. If suit is out for some player, cards of suit in sampled layout must be the same
			// 3. Moves in history must be in the player hands
			ASSERT_EQ(newLayout.Hand(0).Size(), 10);
			ASSERT_EQ(newLayout.Hand(1).Size(), 10);
			ASSERT_EQ(newLayout.Hand(2).Size(), 10);
			ASSERT_EQ(newLayout.Hand(0).Add(newLayout.Hand(1).Add(newLayout.Hand(2))).Size(), 30);

			for (uint32_t player = 0; player < 3; player++) {
				for (Suit suit = (Suit)0; (uint8_t)suit < 4; suit = (Suit)((uint8_t)suit + 1)) {
					if (manager.GetState().IsSuitOut(player, suit)) {
						ASSERT_EQ(newLayout.Hand(player).GetSubsetOfSuit(suit), gameState.Hand(player).GetSubsetOfSuit(suit));
					}
				}
			}

			for (const auto moveData : manager.GetState().GetMoveHistory()) {
				ASSERT_TRUE(newLayout.Hand(moveData.player_).IsInSet(moveData.card_));
			}
		}
	}
}
