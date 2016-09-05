#include "common.h"
#include "generate.h"
#include "player.h"
#include "preparedata.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void PrepareData(const po::variables_map& opt) {
	string player1 = opt["play1"].as<string>();
	string player2 = opt["play2"].as<string>();
	string player3 = opt["play3"].as<string>();
	int numSamples = opt["num-samples"].as<int>();
	float minDiffirence = opt["min-diffirence"].as<float>();
	uint32_t simulationsPerMove = opt["simulations-per-move"].as<int>();

	vector<GameState> states;
	for (uint32_t i = 0; i < numSamples; i++) {
		states.push_back(GenLayout());
	}

	vector<shared_ptr<IPlayer>> players;
	players.push_back(CreatePlayer(player1));
	players.push_back(CreatePlayer(player2));
	players.push_back(CreatePlayer(player3));

	auto eval = [](const array<float, 3>& scores, uint8_t player) {
		return -scores[player];
	};
	GameSamplerOptions samplerOptions;
	samplerOptions.PlayoutsPerMove = simulationsPerMove;
	//samplerOptions.SampleWhenPlaying = PlayerUsesProbabilityPrediction(player1);
	samplerOptions.MinDiffirence = minDiffirence;
	samplerOptions.OpenCards = opt.count("open-cards");
	GameSampler sampler(players, eval, samplerOptions);
	auto model = sampler.BuildTrainModel(states);
	std::ofstream fst("model.tsv");
	model.WriteTsv(fst);
}

int main(int argc, char* argv[]) {
	srand(time(nullptr));
	po::options_description desc("Allowed options");
	const std::string formatDescr = "(format movePredictorsFolder:playingProbabilityPredFolder:trainingProbabilityPredFolder. In case we don't have prob folder for any of the components use random instead)";
	desc.add_options()
			("help", "produce help message")
			("open-cards", "train model when players are seeing cards of each other")
			("play1", po::value<string>()->default_value("random:random:random"), ("model folder for 1st player moves " + formatDescr).c_str())
			("play2", po::value<string>()->default_value("random:random:random"), ("model folder for 2nd player moves " + formatDescr).c_str())
			("play3", po::value<string>()->default_value("random:random:random"), ("model folder for 3rd player moves " + formatDescr).c_str())
			("num-samples", po::value<int>()->default_value(5000), "number of layouts to sample")
			("simulations-per-move", po::value<int>()->default_value(20), "number of layouts to sample")
			("min-diffirence", po::value<float>()->default_value(0.25f), "diffirence between best and worst move that allows position to be sampled")
			;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (vm.count("help")) {
		cerr << desc << endl;
		return 1;
	}
	PrepareData(vm);
	return 0;
}
