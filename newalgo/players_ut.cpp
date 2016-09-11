#include "player.h"
#include "gamemgr.h"
#include "generate.h"

#include <gtest/gtest.h>

TEST (PlayersUT, AIPlayerTest) {
	for (uint32_t i = 0; i < 10000; i++) {
		vector<shared_ptr<IPlayer>> players;
		for (int i = 0; i < 3; i++) {
			players.push_back(shared_ptr<IPlayer>(CreatePlayer("random:random:random")));
		}
		GameState state = GenLayout();
		GameManager manager(players);
		manager.SetNewLayout(state);
		ASSERT_NO_THROW(manager.PlayToTheEnd());
		ASSERT_EQ(manager.GetState().GetMoveNumber(), 10);
	}
}

TEST(PlayersUT, ModelPlayerStrongerThanRandom) {
	array<uint32_t, 3> sum = { { 0 } };
	static vector<shared_ptr<IPlayer>> players;
	players.push_back(CreatePlayer("models:random:random"));
	players.push_back(CreatePlayer("random:random:random"));
	players.push_back(CreatePlayer("random:random:random"));
	for (uint32_t iter = 0; iter < 5000; iter++) {
		GameState state = GenLayout();
		GameManager manager(players);
		manager.SetNewLayout(state, /*open-cards=*/false);
		manager.PlayToTheEnd();
		ASSERT_EQ(manager.GetState().GetMoveNumber(), 10);
		for (uint32_t i = 0; i < 3; i++) {
			auto scores = manager.GetState().GetScores();
			sum[i] += scores[i];
		}
	}
	cerr << "Scores are: " << sum[0] << " " << sum[1] << " " << sum[2] << endl;
	ASSERT_TRUE(sum[0] < sum[1] && sum[0] < sum[2]);
}
