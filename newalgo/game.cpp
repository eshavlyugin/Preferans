#include "common.h"
#include "gamemgr.h"
#include "generate.h"
#include "player.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void PlayLoop(const vector<std::shared_ptr<IPlayer>>& players, array<int, 3>& scores, bool dump, bool openCards) {
	GameState state = GenLayout();
	GameManager manager(players);
	manager.SetNewLayout(state, openCards);
	manager.SetDumpGames(dump);
	manager.PlayToTheEnd();
	for (int i = 0; i < 3; i++) {
		scores[i] += manager.GetState().GetScores()[i];
	}
	state.Dump(cerr);
}

void PlayGames(const po::variables_map& opt) {
	string player1 = opt["play1"].as<string>();
	string player2 = opt["play2"].as<string>();
	string player3 = opt["play3"].as<string>();
	bool dump = player1 == "human" || player2 == "human" || player3 == "human";
	int numGames = opt["num-games"].as<int>();
	std::vector<std::shared_ptr<IPlayer>> players;
	players.push_back(CreatePlayer(player1));
	players.push_back(CreatePlayer(player2));
	players.push_back(CreatePlayer(player3));
	array<int, 3> scores = {{0}};
	bool openCards = opt.count("open-cards");
	for (int i = 0; i < numGames; i++) {
		PlayLoop(players, scores, dump, openCards);
	}
	cerr << "Simulation finished" << endl;
	cerr << "Scores: " << scores[0] << ", " << scores[1] << ", " << scores[2] << endl;
}

int main(int argc, char* argv[]) {
	srand(time(nullptr));
	po::options_description desc("Allowed options");
	const std::string formatDescr = "(format movePredictorsFolder:playingProbabilityPredFolder:trainingProbabilityPredFolder. In case we don't have prob folder for any of the components use random instead)";
	desc.add_options()
			("help", "produce help message")
			("play1", po::value<string>()->default_value("random:random:random"), ("model folder for 1st player moves " + formatDescr).c_str())
			("play2", po::value<string>()->default_value("random:random:random"), ("model folder for 2nd player moves " + formatDescr).c_str())
			("play3", po::value<string>()->default_value("random:random:random"), ("model folder for 3rd player moves " + formatDescr).c_str())
			("open-cards", "players are playing on open cards")
			("num-games", po::value<int>()->default_value(1), "number of games to play")
			;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (vm.count("help")) {
		cerr << desc << endl;
		return 1;
	}
	srand(time(nullptr));
	PlayGames(vm);
	return 0;
}