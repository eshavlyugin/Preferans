#pragma once

#include "common.h"
#include "features.h"

static const uint16_t InvalidIndex = 0xffff;

class StateContext {
public:
	StateContext(const GameState& playerView, Card move, uint32_t ourHero, FeatureTag tag);

	void SetMovesReward(const map<Card, float> movesReward) {
		movesReward_ = movesReward;
	}
	const FeaturesSet& GetFeatures() const;
	map<Card, float> GetMoveReward() const {
		return movesReward_;
	}
	const GameState& GetRealGame() const {
		return realGame_;
	}
	void SetPredictedScores(array<float, 3> scores) {
		scores_ = scores;
	}
	array<float, 3> GetPredictedScores() const {
		return scores_;
	}
	Card GetMove() const {
		return move_;
	}

private:
	GameState playerView_;
	GameState realGame_;
	array<float, 3> scores_ = {{0.0f}};
	map<Card, float> movesReward_;
	Card move_;
	FeaturesSet features_;
};

class IModel {
public:
	virtual ~IModel() {}

	virtual vector<float> Predict(const FeaturesSet& features) = 0;
	virtual vector<float> PredictSeq(const vector<FeaturesSet>& features) = 0;
};

class IModelFactory {
public:
	virtual ~IModelFactory() {}

	virtual shared_ptr<IModel> CreateModel(const std::string& modelName) = 0;
};

class ModelPredictor {
public:
	ModelPredictor(const string& weightsFilePath);

	uint32_t PredictLabel(StateContext& ctx);
	uint32_t PredictLabelUT(const FeaturesSet& features);
	vector<float> PredictProbabilities(StateContext& ctx);
	vector<float> CalcWeights(StateContext& ctx);
	vector<float> CalcWeights(FeaturesSet& features);

private:
	shared_ptr<IModel> model_;
};

