#include "common.h"
#include "features.h"
#include "gamemgr.h"
#include "generate.h"
#include "player.h"
#include "pymodels.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void PlayLoop(const vector<std::shared_ptr<IPlayer>>& players, array<int, 3>& scores, bool dump, bool openCards, shared_ptr<ostream> recordFile) {
	GameState state = GenLayout();
	GameManager manager(players);
	manager.SetNewLayout(state, openCards);
	manager.SetDumpGames(dump);
	manager.PlayToTheEnd();
	for (int i = 0; i < 3; i++) {
		scores[i] += manager.GetState().GetScores()[i];
	}
	if (recordFile.get()) {
	    *recordFile.get() << state;
	    for (auto& historyItem : manager.GetState().GetMoveHistory()) {
	        *recordFile.get() << CardToString(historyItem.card_) << " ";
	    }
	    *recordFile.get() << "\n";
	    for (float moveValue : manager.GetMoveValues()) {
	    	*recordFile.get() << moveValue << " ";
	    }
	    *recordFile.get() << "\n";
	    recordFile->flush();
	}
}

void PlayGames(const po::variables_map& opt) {
	string player1 = opt["play1"].as<string>();
	string player2 = opt["play2"].as<string>();
	string player3 = opt["play3"].as<string>();
	bool dump = player1 == "human" || player2 == "human" || player3 == "human";
	int numGames = opt["num-games"].as<int>();
	auto factory = shared_ptr<IModelFactory>(new PyModels::PyModelFactory());
	std::vector<std::shared_ptr<IPlayer>> players;
	players.push_back(CreatePlayer(player1, factory));
	players.push_back(CreatePlayer(player2, factory));
	players.push_back(CreatePlayer(player3, factory));
	array<int, 3> scores = {{0}};
	string recordFile = opt["record-games-to-file"].as<string>();
	shared_ptr<ostream> recordFileStream;
	if (recordFile.size() > 0) {
	    recordFileStream.reset(new ofstream(recordFile));
	}
	bool openCards = opt.count("open-cards");
	for (int i = 0; i < numGames; i++) {
		PlayLoop(players, scores, dump, openCards, recordFileStream);
	}
	cerr << "Simulation finished" << endl;
	cerr << "Scores: " << scores[0] << ", " << scores[1] << ", " << scores[2] << endl;
}

int main(int argc, char* argv[]) {
	srand(time(nullptr));
	PyModels::Init(argc, argv);
	auto factory = new PyModels::PyModelFactory();
	auto model = factory->CreateModel("model_lstm");
	model->PredictSeq(vector<FeaturesSet>(8, CalcFeatures(GameState(), NoCard, 0, FT_PosPredict)));
	po::options_description desc("Allowed options");
	const std::string formatDescr = "(format movePredictorsFolder:playingProbabilityPredFolder:trainingProbabilityPredFolder. In case we don't have prob folder for any of the components use random instead)";
	desc.add_options()
			("help", "produce help message")
			("play1", po::value<string>()->default_value("random"), ("model folder for 1st player moves " + formatDescr).c_str())
			("play2", po::value<string>()->default_value("random"), ("model folder for 2nd player moves " + formatDescr).c_str())
			("play3", po::value<string>()->default_value("random"), ("model folder for 3rd player moves " + formatDescr).c_str())
			("open-cards", "players are playing on open cards")
			("num-games", po::value<int>()->default_value(1), "number of games to play")
			("record-games-to-file", po::value<string>()->default_value(""), "Dump games to file")
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
