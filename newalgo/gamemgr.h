#pragma once

#include "common.h"
#include "player.h"

class GameManager {
public:
	GameManager(std::vector<std::shared_ptr<IPlayer>> players);
	GameManager(const GameManager& other);

	void SetNewLayout(const GameState& gameState, bool isOpenCards = false);

	void PlayToTheEnd();
	void PlayForNMoves(uint32_t nMoves);
	void PlayMove(Card move);
	void SetDumpGames(bool value) {
		dumpGames_ = value;
	}
	vector<float> GetMoveValues() const {
		return moveValues_;
	}

	const GameState& GetState() const {
		return gameState_;
	}

private:
	bool dumpGames_ = false;
	vector<shared_ptr<IPlayer>> players_;
	vector<float> moveValues_;
	GameState gameState_;
};
