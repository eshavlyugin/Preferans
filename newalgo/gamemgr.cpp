#include "gamemgr.h"

GameManager::GameManager(std::vector<std::shared_ptr<IPlayer>> players) :
		players_(players) {
}

GameManager::GameManager(const GameManager& other) {
	for (auto player : other.players_) {
		players_.push_back(player->Clone());
	}

	gameState_ = other.gameState_;
}

void GameManager::SetNewLayout(const GameState& gameState, bool isOpenCards) {
	gameState_ = gameState;
	moveValues_.clear();
	for (int i = 0; i < players_.size(); i++) {
		std::vector<uint8_t> toClose;
		for (int j = 0; j < players_.size(); j++) {
			if (i != j) {
				toClose.push_back(j);
			}
		}
		GameState copy = gameState;
		copy.CloseHands(toClose);
		players_[i]->OnNewLayout(copy);
		if (isOpenCards) {
			players_[i]->OnNewXRayLayout(gameState);
		}
	}
}

void GameManager::PlayMove(Card move) {
	gameState_.MakeMove(move);
	for (auto player : players_) {
		player->OnMove(move);
	}
}

void GameManager::PlayToTheEnd() {
	PlayForNMoves(100);
}

void GameManager::PlayForNMoves(uint32_t nMoves) {
	while (!gameState_.IsFinished() && nMoves > 0) {
		if (dumpGames_) {
			gameState_.Dump(cerr);
		}
		float moveValue = 0.0f;
		auto move = players_[gameState_.GetCurPlayer()]->DoMove(&moveValue);
		moveValues_.push_back(moveValue);
		PlayMove(move);
		nMoves--;
	}
}

