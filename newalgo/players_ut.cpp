#include "player.h"
#include "gamemgr.h"
#include "generate.h"

#include <gtest/gtest.h>

TEST (PlayersUT, AIPlayerTest) {
	for (uint32_t i = 0; i < 10000; i++) {
		vector<shared_ptr<IPlayer>> players;
		for (int i = 0; i < 3; i++) {
			players.push_back(shared_ptr<IPlayer>(CreatePlayer("random", nullptr)));
		}
		GameState state = GenLayout();
		GameManager manager(players);
		manager.SetNewLayout(state);
		ASSERT_NO_THROW(manager.PlayToTheEnd());
		ASSERT_EQ(manager.GetState().GetMoveNumber(), 10);
	}
}

