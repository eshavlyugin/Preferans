#include "common.h"
#include "gamemgr.h"
#include "generate.h"
#include "player.h"
#include "train_model.h"

void TestLoop() {
	vector<shared_ptr<IPlayer>> players;
	for (int i = 0; i < 3; i++) {
		players.push_back(shared_ptr<IPlayer>(new AiPlayer()));
	}
	GameState state = GenLayout();
	GameManager manager(players);
	manager.SetNewLayout(state);
	manager.PlayToTheEnd();
	PREF_ASSERT(manager.GetState().GetMoveNumber() == 10);
}

void PrintRandomState() {
	vector<shared_ptr<IPlayer>> players;
	for (int i = 0; i < 3; i++) {
		players.push_back(shared_ptr<IPlayer>(new AiPlayer()));
	}
	GameState state = GenLayout();
	GameManager manager(players);
	manager.SetNewLayout(state);
	manager.PlayForNMoves(17);
	const auto& resState = manager.GetState();
	resState.Dump(cout);
	array<array<float, 32>, 4> probabilities;
	players[0]->GetCardProbabilities(probabilities);
	for (const auto x : probabilities) {
		for (const auto y : x) {
			cout << y << " ";
		}
		cout << "\n";
	}
}

int main() {
	CardsSet s;
	s.Add(MakeCard(Hearts, 3));
	s.Add(MakeCard(Spades, 4));

	cerr << MakeCard(Hearts, 3) << endl;
	cerr << MakeCard(Spades, 4) << endl;
	auto layout = GenLayout();
	layout.Dump(cout);
	auto x = 1352740880;
	cout << "Bit magic: " << __builtin_ctz(x) << endl;
	cout << s.Size() << endl;
	for (int i = 0; i < 3; i++) {
		cout << layout.Hand(i).Size() << endl;
	}
	cerr << layout.Hand(1) << endl;
	for (int hand = 0; hand < 3; hand++) {
		for (auto card : layout.Hand(hand)) {
			cout << card << endl;
			PREF_ASSERT(layout.Hand(hand).IsInSet(card));
		}
	}
	/*  for (int i = 0; i < 1000000; i++) {
	 TestLoop();
	 }*/
	PrintRandomState();
	return 0;
}
