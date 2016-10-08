#include "train_model.h"
#include "features.h"

using namespace std;

static vector<float> readFloatVec(istream& ist, int elems) {
	vector<float> result(elems);
	for (auto& elem : result) {
		ist >> elem;
	}
	return result;
}

static float sigmoid(float x) {
	return 1.0 / (1.0 + exp(-x));
}

class NN3LayerModel : public IModel {
public:
	NN3LayerModel(istream& ist, vector<FeatureTag> validTags) {
		ist >> input_ >> hidden1_ >> hidden2_ >> output_;
		weights1_ = readFloatVec(ist, input_ * hidden1_);
		biases1_ = readFloatVec(ist, hidden1_);
		weights2_ = readFloatVec(ist, hidden1_ * hidden2_);
		biases2_ = readFloatVec(ist, hidden2_);
		weights3_ = readFloatVec(ist, hidden2_ * output_);

		FeaturesRegistry registry;
		array<bool, FeatureTag::FT_TypeCount> validTypes = { { false } };
		for (auto tag : validTags) {
			validTypes[tag] = true;
		}
		auto featureSet = registry.CreateEmptySet().GetFeatures();
		featureMap_.resize(featureSet.size());
		uint32_t counter = 0;
		for (uint32_t i = 0; i < featureSet.size(); ++i) {
			featureMap_[i] = validTypes[featureSet[i].tag_] ? counter++ : InvalidIndex;
		}
		PREF_ASSERT((input_ == counter) && "Invalid number of features in model");
	}

	virtual vector<float> Predict(const FeaturesSet& features) {
		vector<float> result(output_);
		vector<float> hidden1 = biases1_;
		vector<float> hidden2 = biases2_;
		for (const auto& feature : features.GetNonZeroFeatures()) {
			for (uint32_t j = 0; j < hidden1_; j++) {
				if (featureMap_[feature.first] != InvalidIndex) {
					hidden1[j] += weights1_[featureMap_[feature.first] * input_ + j] * feature.second;
				}
				hidden1[j] = sigmoid(hidden1[j]);
			}
		}

		for (uint32_t i = 0; i < hidden2_; i++) {
			for (uint32_t j = 0; j < hidden1_; j++) {
				hidden2[i] += weights2_[i * hidden2_ + j] * hidden1[j];
			}
			hidden2[i] = sigmoid(hidden2[i]);
		}

		for (uint32_t i = 0; i < hidden2_; i++) {
			for (uint32_t j = 0; j < output_; j++) {
				result[i] += weights3_[i * output_ + j] * hidden2[j];
			}
		}
		return result;
	}

private:
	vector<float> weights1_;
	vector<float> weights2_;
	vector<float> weights3_;
	vector<float> biases1_;
	vector<float> biases2_;
	vector<int> featureMap_;
	int input_ = 0;
	int hidden1_ = 0;
	int hidden2_ = 0;
	int output_ = 0;
};

class NN2LayerModel : public IModel {
public:
	NN2LayerModel(istream& ist, vector<FeatureTag> validTags) {
		ist >> input_ >> hidden1_ >> output_;
		biases1_ = readFloatVec(ist, hidden1_);
		weights1_ = readFloatVec(ist, input_ * hidden1_);
		weights2_ = readFloatVec(ist, hidden1_ * output_);

		FeaturesRegistry registry;
		array<bool, FeatureTag::FT_TypeCount> validTypes = { { false } };
		for (auto tag : validTags) {
			validTypes[tag] = true;
		}
		auto featureSet = registry.CreateEmptySet().GetFeatures();
		featureMap_.resize(featureSet.size());
		uint32_t counter = 0;
		for (uint32_t i = 0; i < featureSet.size(); ++i) {
			featureMap_[i] = validTypes[featureSet[i].tag_] ? counter++ : InvalidIndex;
		}
		PREF_ASSERT((input_ == counter) && "Invalid number of features in model");
	}

	virtual vector<float> Predict(const FeaturesSet& features) {
		vector<float> result(output_);
		vector<float> hidden1 = biases1_;
		for (const auto& feature : features.GetNonZeroFeatures()) {
			for (uint32_t j = 0; j < hidden1_; j++) {
				if (featureMap_[feature.first] != InvalidIndex) {
					hidden1[j] += weights1_[featureMap_[feature.first] * hidden1_ + j] * feature.second;
				}
			}
		}
		for (uint32_t  j = 0; j < hidden1_; j++) {
			hidden1[j] = sigmoid(hidden1[j]);
		}
		for (uint32_t i = 0; i < output_; i++) {
			for (uint32_t j = 0; j < hidden1_; j++) {
				result[i] += weights2_[i * output_ + j] * hidden1[j];
			}
		}

		return result;
	}

private:
	vector<float> weights1_;
	vector<float> weights2_;
	vector<float> biases1_;
	vector<int> featureMap_;
	int input_ = 0;
	int hidden1_ = 0;
	int output_ = 0;
};


class ModelFactory {
public:
	static shared_ptr<IModel> CreateModel(const std::string& model_name, istream& ist) {
		if (model_name == "3_layer_nn") {
			return shared_ptr<IModel>(new NN3LayerModel(ist, {FT_OpenCards, FT_CommonCards}));
		} else if (model_name == "2_layer_nn") {
			return shared_ptr<IModel>(new NN2LayerModel(ist, {FT_OpenCards, FT_CommonCards}));
		} else {
			PREF_ASSERT(false && "Unknown model name");
		}
		return shared_ptr<IModel>(nullptr);
	}
};

static vector<float> parseNumbersFromString(const string& str) {
	stringstream sstr(str);
	vector<float> result;
	int totalRead = 0;
	while (sstr.good()) {
		float value;
		sstr >> value;
		result.push_back(value);
	}
	return result;
}

const FeaturesSet& StateContext::GetFeatures() const {
	return features_;
}

StateContext::StateContext(const GameState& playerView, const GameState& realGame,
	const CardsProbabilities& probArray, Card move, uint32_t ourHero)
	: playerView_(playerView)
	, realGame_(realGame)
	, probArray_(probArray)
	, move_(move)
	, features_(CalcFeatures(playerView, realGame, probArray, move, ourHero))
{
}

ModelPredictor::ModelPredictor(const string& weightsFilePath)
{
	string str;
	ifstream ist(weightsFilePath);
	string modelName;
	ist >> modelName;
	model_ = ModelFactory::CreateModel(modelName, ist);
}

vector<float> ModelPredictor::CalcWeights(StateContext& ctx) {
	return model_->Predict(ctx.GetFeatures());
}

uint32_t ModelPredictor::PredictLabelUT(const FeaturesSet& features) {
	vector<float> weights = model_->Predict(features);
	auto it = std::max_element(weights.begin(), weights.end());
	return it - weights.begin();
}

vector<float> ModelPredictor::CalcWeights(FeaturesSet& features) {
	return model_->Predict(features);
}

uint32_t ModelPredictor::PredictLabel(StateContext& ctx) {
	auto weights = CalcWeights(ctx);
	auto it = std::max_element(weights.begin(), weights.end());
	return it - weights.begin();
}

vector<float> ModelPredictor::PredictProbabilities(StateContext& ctx) {
	auto weights = CalcWeights(ctx);
	float sum = 0.0f;
	for (float& w : weights) {
		w = exp(w);
		sum += w;
	}
	for (float& w : weights) {
		w /= sum;
	}
	return weights;
}

void TrainModel::AddFeatures(const FeaturesSet& features) {
	featuresArray_.push_back(vector<float>());
	auto featureSet = features.GetFeatures();
	PREF_ASSERT(tags_.size() == 0 || tags_.size() == featureSet.size());
	tags_.resize(featureSet.size());
	for (uint32_t i = 0; i < featureSet.size(); i++) {
		featuresArray_.back().push_back(featureSet[i].value_);
		tags_[i] = featureSet[i].tag_;
	}
}

void TrainModel::AddGameInfo(const StateContext& ctx) {
	auto features = ctx.GetFeatures();
	const auto& realGame = ctx.GetRealGame();
	AddFeatures(features);
	vector<float> actionsArray;
	auto movesReward = ctx.GetMoveReward();
	for (uint32_t i = 0; i < 32; ++i) {
		auto card = CardFromBit(i);
		auto it = movesReward.find(card);
		actionsArray.push_back(it == movesReward.end() ? 0.0f : it->second);
	}
	actionsArray.push_back(GetCardBit(ctx.GetMove()));
	for (uint8_t cardBit = 0; cardBit < 32; cardBit++) {
		auto card = CardFromBit(cardBit);
		for (uint8_t hand = 0; hand < 4; hand++) {
			if (hand == 3 || realGame.Hand(hand).IsInSet(card)) {
				actionsArray.push_back(hand);
				break;
			}
		}
	}
	actionsArray.push_back(ctx.GetPredictedScores()[0] < 1.0 / 2 ? 1 : 0);
	actionsArray.push_back(ctx.GetPredictedScores()[0] < 0.3 ? 1 : 0);
	actionsArray.push_back(ctx.GetPredictedScores()[0] < 1.0 / 6 ? 1 : 0);
	actionsArray.push_back(ctx.GetPredictedScores()[0]);
	actionsArray.push_back(ctx.GetPredictedScores()[0] * (10 - ctx.GetRealGame().GetMoveNumber()));
	actions_.push_back(actionsArray);
}

void TrainModel::WriteTsv(ostream& ost) {
	for (auto tag : tags_) {
		ost << TagToString(tag) << "\t";
	}

	for (uint32_t i = 0; i < 32; i++) {
		ost << "move_reward\t";
	}
	ost << "correct_move\t";
	for (uint32_t i = 0; i < 32; i++) {
		ost << "card" << i << "\t";
	}
	ost << "expected_score_2\texpected_score_3\texpected_score_6\texpected_score_norm\texpected_score\n";
	//ost << "expected_score\n";

	PREF_ASSERT(actions_[0].size() == 70 && "Wrong actions size");

	for (uint32_t i = 0; i < featuresArray_.size(); i++) {
		for (float value : featuresArray_[i]) {
			ost << value << "\t";
		}
		for (auto label : actions_[i]) {
			ost << label << "\t";
		}
		ost << '\n';
	}
}
