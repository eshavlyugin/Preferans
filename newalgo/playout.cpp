#include "common.h"
#include "gamemgr.h"
#include "player.h"
#include "playout.h"

#include <bits/stdc++.h>

map<Card, array<float, 3>> PlayoutManager::Play(const GameManager& gameMgr,
		uint32_t trialsPerMove) {
	auto validMoves = gameMgr.GetState().GenValidMoves();
	map<Card, array<float, 3>> result;
	for (const auto move : validMoves) {
		array<float, 3> sumOfScores = { { 0.0f } };
		for (uint32_t trial = 0; trial < trialsPerMove; trial++) {
			GameManager mgrCopy(gameMgr);
			mgrCopy.PlayMove(move);
			mgrCopy.PlayToTheEnd();
			const auto& copy = mgrCopy.GetState();
			auto scores = copy.GetScores();
			PREF_ASSERT(copy.GetMoveNumber() == 10);
			PREF_ASSERT(scores[0] + scores[1] + scores[2] == 10);
			for (int i = 0; i < sumOfScores.size(); i++) {
				sumOfScores[i] += scores[i];
			}
		}
		for (auto& i : sumOfScores) {
			i /= trialsPerMove;
		}
		result[move] = sumOfScores;
	}
	return result;
}
