#include "common.h"
#include "features.h"
#include "train_model.h"
#include "utils.h"

#include <gtest/gtest.h>


TEST(ModelPredictorUT, CheckModelFileIsGood) {
	FeaturesRegistry registry;

	ifstream correctScoreIst("model_error.txt");
	double squareError;
	correctScoreIst >> squareError;

	ModelPredictor predictor("expected_score.tsv");
	ifstream ist("model.tsv");
	string str;
	getline(ist, str);
	vector<string> tags = utils::splitString<string>(str);
	double total = 0.0f;
	uint32_t count = 0;
	while (getline(ist, str)) {
		vector<float> values = utils::splitString<float>(str);
		auto features = registry.CreateEmptySet();
		ASSERT_GE(values.size(), tags.size());
		float score = -1.0f;
		for (uint32_t i = 0; i < tags.size(); i++) {
			if (tags[i] == "open_cards" || tags[i] == "common_cards" || tags[i] == "close_cards" || tags[i] == "move") {
				if (fabs(values[i]) > 1e-5) {
					features.SetUT(i, values[i]);
				}
			} else if (tags[i] == "expected_score") {
				score = values[i];
			}
		}
		ASSERT_TRUE(score > -0.00001f);
		auto weights = predictor.CalcWeights(features);
		total += (weights[0] - score) * (weights[0] - score);
		count++;
	}
	total /= count;
	cerr << total << " " << squareError << endl;
	ASSERT_TRUE(fabs(total - squareError) < 1e-5);
}

