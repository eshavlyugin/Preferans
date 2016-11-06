#include "train_model.h"
#include "features.h"
#include "utils.h"

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

class SingleLayerPerceptron : public IModel {
public:
	SingleLayerPerceptron(const std::string& fileName) {
		ifstream ist(fileName);
		ist >> w_ >> h_;
		weights_.resize(w_ * h_);
		bias_.resize(h_);
		for (uint32_t i = 0; i < w_ * h_; i++) {
			ist >> weights_[i];
		}
		for (uint32_t i = 0; i < h_; i++) {
			ist >> bias_[i];
		}
	}

	vector<float> Predict(const FeaturesSet& features) override {
		vector<float> result = bias_;
		for (const auto pair : features.GetNonZeroFeatures()) {
			uint32_t beg = pair.first * h_;
			for (uint32_t i = 0; i < h_; i++) {
				result[i] += pair.second * weights_[beg + i];
			}
		}
		return result;
	}

	vector<float> PredictSeq(const vector<FeaturesSet>& features) override {
		PREF_ASSERT(false);
		return vector<float>();
	}

private:
	uint32_t w_ = 0;
	uint32_t h_ = 0;
	vector<float> bias_;
	vector<float> weights_;
};

class NativeModelFactory : public IModelFactory {
public:
	shared_ptr<IModel> CreateModel(const std::string& modelName) override {
		if (modelName == "model2") {
			return shared_ptr<IModel>(new SingleLayerPerceptron(modelName + ".txt"));
		}
		PREF_ASSERT(false);
		return nullptr;
	}
};

class PrefixModelFactory : public IModelFactory {
public:
	PrefixModelFactory(const vector<pair<string, shared_ptr<IModelFactory>>>& models) {
		for (auto pair : models) {
			map_[pair.first] = pair.second;
		}
	}

	shared_ptr<IModel> CreateModel(const std::string& modelName) override {
		auto parts = utils::split(modelName, ':');
		PREF_ASSERT(parts.size() > 1);
		return map_.at(parts[0])->CreateModel(modelName.substr(parts[0].size() + 1, modelName.size() - parts[0].size() - 1));
	}

private:
	map<string, shared_ptr<IModelFactory>> map_;
};

shared_ptr<IModelFactory> CreateNativeModelFactory() {
	return shared_ptr<IModelFactory>(new NativeModelFactory());
}

shared_ptr<IModelFactory> CreatePrefixModelFactory(vector<pair<string, shared_ptr<IModelFactory>>> models) {
	return shared_ptr<IModelFactory>(new PrefixModelFactory(models));
}


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

StateContext::StateContext(const GameState& playerView, Card move, uint32_t ourHero, FeatureTag tag)
	: playerView_(playerView)
	, move_(move)
	, features_(CalcFeatures(playerView, move, ourHero, tag))
{
}

