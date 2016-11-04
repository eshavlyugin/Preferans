#include "layout_sample.h"
#include "gamemgr.h"
#include "generate.h"
#include "player.h"

#include <gtest/gtest.h>

TEST (LayoutSamplingTest, GenerateSimpleLayout) {
	for (uint32_t iter = 0; iter < 100; iter++) {
		GameState gameState = GenLayout();
		vector<shared_ptr<IPlayer>> players = {
			CreatePlayer("random", nullptr),
			CreatePlayer("random", nullptr),
			CreatePlayer("random", nullptr)
		};
		GameManager manager(players);
		manager.SetNewLayout(gameState, /*openCards=*/true);
		manager.PlayForNMoves(15);
		auto sampler = LayoutSampler(manager.GetState(), gameState.GetFirstPlayer(), 0);
		auto newLayouts = sampler.DoSample(5, /*playMoveHistory=*/false);
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
