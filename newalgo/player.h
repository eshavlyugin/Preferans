#pragma once

#include "common.h"
#include "train_model.h"

class IPlayer {
public:
	virtual ~IPlayer() {
	}

	virtual void OnNewLayout(const GameState& game) = 0;
	virtual void OnNewXRayLayout(const GameState& game) = 0;
	virtual void OnMove(Card card) = 0;
	virtual Card DoMove(float* moveValue = nullptr) = 0;
	virtual shared_ptr<IPlayer> Clone() = 0;

	// For training model
	virtual const GameState& GetStateView() const = 0;
};

std::shared_ptr<IPlayer> CreatePlayer(const std::string& descr, std::shared_ptr<IModelFactory> modelFactory);
