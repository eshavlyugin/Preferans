/****************************************************************************
 Preferans: implementation of card-tricking game Preferans (or Preference).
 ****************************************************************************
 Copyright (c) 2010-2011  Eugene Shavlyugin <eshavlyugin@gmail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ****************************************************************************/

#include <precompiled.h>

#include <SolversTest.h>
#include <MinMaxSolver.h>
#include <MiserePolicy.h>
#include <ContractGamePolicy.h>
#include <Bullet.h>
#include <PrefTools.h>
#include <PrefEngine.h>
#include <EngineData.h>
#include <RandomLayoutsGenerator.h>
#include <DealState.h>
#include <Preference.h>
#include <PrefGameModel.h>
#include <MonteCarloSolver.h>
#include <PassoutPolicy.h>

void DoPassoutTest()
{
	int N;
	ifstream inp("../../tests/passout.tests");
	inp >> N;
	ifstream dataInp("../../data/engine.dat");
	EngineData data;
	data.Load(dataInp);
	for( int testN = 0; testN < N; testN++ ) {
		Layout layout;
		GetLog() << "Monte carlo solver..." << endl;
		for( int i = 0; i < NumOfPlayers; i++ ) {
			for( int j = 0; j < InitialCardsCount; j++ ) {
				string card;
				inp >> card;
				layout.Cards[i] = AddCardToSet(layout.Cards[i], CardFromString(card));
			}
		}
		layout.CurrentPlayer = 0;
		PassoutPolicy policy(&data);
		MonteCarloSolver<PassoutPolicy> solver(policy);
		solver.Solve(layout, 8000);
		const McTreeNode* root = solver.ResultTreeRoot();
		for( const McTreeNode* child = root->FirstChild; child != 0; child = child->Next ) {
			GetLog() << child->Move.Value << " " << child->TotalScores[0] << " " 
				<< child->TotalScores[1] << " " << child->TotalScores[2] << " " << child->PlayoutsCount << endl;
		}
		GetLog() << "Monte carlo solver has terminated" << endl;
	}
}

void DoSolversTest()
{
	int N;
	ifstream inp("../../tests/solvers.tests");
	inp >> N;
	for( int testN = 1; testN <= N; testN++ ) {
		Layout layout;
		for( int i = 0; i < NumOfPlayers; i++ ) {
			for( int j = 0; j < InitialCardsCount; j++ ) {
				string card;
				inp >> card;
				layout.Cards[i] = AddCardToSet(layout.Cards[i], CardFromString(card));
			}
		}
		int answer;
		string name;
		inp >> layout.Dealer;
		inp >> layout.CurrentPlayer;
		inp >> name;
		inp >> answer;
		vector<MoveQuality> qualities;
		bool isMin;
		int stateCount;
		if( name == "misere" ) {
			MiserePolicy policy;
			policy.SetCheckingMode(true);
			MinMaxSolver<MiserePolicy, true> solver(policy);
			qualities = solver.Solve(layout, 0, 1);
			//GetLog() << solver.GetBestResult(layout, 0, 10) << endl;
			isMin = layout.CurrentPlayer == layout.Dealer;
			stateCount = solver.StatesCount();
		} else {
			switch( name[name.length() - 1] ) {
				case 's': 
					layout.Trump = SuitSpades;
					break;
				case 'd':
					layout.Trump = SuitDiamonds;
					break;
				case 'h':
					layout.Trump = SuitHearts;
					break;
				case 'c':
					layout.Trump = SuitClubs;
					break;
				default:
					layout.Trump = SuitNoTrump;
			}
			ContractGamePolicy policy;
			policy.SetCheckingMode(true);
			MinMaxSolver<ContractGamePolicy, false> solver(policy);
			qualities = solver.Solve(layout, 4, 10);
			//GetLog() << solver.GetBestResult(layout, 0, 10) << endl;
			isMin = layout.CurrentPlayer != layout.Dealer;
			stateCount = solver.StatesCount();
		}
		int res;
		if( !isMin ) {
			res = 0;
			for( int i = 0; i < qualities.size(); i++ ) {
				res = std::max(qualities[i].Score, res);
			}
		} else {
			res = 10;
			for( int i = 0; i < qualities.size(); i++ ) {
				res = std::min(qualities[i].Score, res);
			}
		}
		GetLog() << "TEST #" << testN << ": states = " << stateCount << "; result = " << res << "; answer = " << answer << endl;
	}
}

static void printBullet(const PlayersScores& scores)
{
	GetLog() << "Bullet: ";
	for( int i = 0; i < 3; i++ ) {
		GetLog() << scores.Bullet[i] << " ";
	}
	GetLog() << endl;
	GetLog() << "Mountain: ";
	for( int i = 0; i < 3; i++ ) {
		GetLog() << scores.Mountain[i] << " ";
	}
	GetLog() << endl;
	GetLog() << "Whists:" << endl;
	for( int i = 0; i < 3; i++ ) {
		for( int j = 0; j < 3; j++ ) {
			GetLog() << scores.Whists[i][j] << " ";
		}
		GetLog() << endl;
	}
}

static void printLayouts(const vector<Layout>& layouts)
{
	multi_array<int, 2> totalCounts(boost::extents[NumOfPlayers][32]);
	for( int i = 0; i < layouts.size(); i++ ) {
		GetLog() << layouts[i].Cards[0].Value << " " << layouts[i].Cards[1].Value << " " << layouts[i].Cards[2].Value << endl;
		PrefAssert( CardsSetsIntersection( layouts[i].Cards[0], layouts[i].Cards[1] ) == EmptyCardsSet );
		PrefAssert( CardsSetsIntersection( layouts[i].Cards[0], layouts[i].Cards[2] ) == EmptyCardsSet );
		PrefAssert( CardsSetsIntersection( layouts[i].Cards[2], layouts[i].Cards[1] ) == EmptyCardsSet );
		PrefAssert( CardsSetSize(layouts[i].Cards[0]) == 10
			&& CardsSetSize(layouts[i].Cards[1]) == 10
			&& CardsSetSize(layouts[i].Cards[2]) == 10 );
		int count = 0;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
				Card card = CreateCard(itSuit.GetObject(), itRank.GetObject());
				for( int player = 0; player < NumOfPlayers; player++ ) {
					if( IsSetContainsCard(layouts[i].Cards[player], card ) ) {
						totalCounts[player][count]++;
					}
				}
				count++;
			}
		}
	}
	GetLog() << "Cards distribution: " << endl;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		GetLog() << "Player " << i << endl;
		for( int j = 0; j < 32; j++ ) {
			GetLog() << totalCounts[i][j] << " ";
		}
		GetLog() << endl;
	}
	
}

void DoEngineDataTest()
{
	srand(random());
	EngineData data;
	ifstream in("../../data/engine.dat");
	data.Load(in);
	RandomLayoutsGenerator generator;
	generator.Reset();
	generator.SetPlayerCards( AddCardToSet( EmptyCardsSet, CreateCard(SuitHearts, 4) ), 2 );
	vector<Layout> result = generator.GenerateLayouts(10000);
	generator.Reset();
	generator.SetWidowCards( AddCardToSet( AddCardToSet( EmptyCardsSet, 
		CreateCard(SuitDiamonds, 3) ), CreateCard(SuitHearts, 4) ) );
	result = generator.GenerateLayouts(10000);
	printLayouts(result);	
}

void DoBulletTest()
{
	GameSettings settings;
	settings.Rules == RT_Sochi;
	settings.BulletSize = 5;
	Bullet bullet;
	bullet.Initialize(settings);
	DealResult gr;
	gr.Bids[0] = Bid_Pass; gr.Bids[1] = Bid_HalfWhist; gr.Bids[2] = Bid_6s;
	gr.Tricks[0] = 0; gr.Tricks[1] = 2; gr.Tricks[2] = 8;
	bullet.UpdateScores(gr);
	printBullet(bullet.GetScores());
	gr.Bids[0] = Bid_Pass; gr.Bids[1] = Bid_Whist; gr.Bids[2] = Bid_6s;
	gr.Tricks[0] = 1; gr.Tricks[1] = 3; gr.Tricks[2] = 6;
	vector<float> evals = bullet.EvaluateDeal(gr);
	for( int i = 0; i < evals.size(); i++ ) {
		GetLog() << evals[i] << " ";
	}
	GetLog() << endl;
	bullet.UpdateScores(gr);
	printBullet(bullet.GetScores());
	gr.Bids[0] = Bid_Whist; gr.Bids[1] = Bid_6s; gr.Bids[2] = Bid_Whist;
	gr.Tricks[0] = 1; gr.Tricks[1] = 6; gr.Tricks[2] = 3;
	bullet.UpdateScores(gr);
	printBullet(bullet.GetScores());
	gr.Bids[0] = Bid_Pass; gr.Bids[1] = Bid_Pass; gr.Bids[2] = Bid_Misere;
	gr.Tricks[0] = 5; gr.Tricks[1] = 5; gr.Tricks[2] = 0;
	bullet.UpdateScores(gr);
	printBullet(bullet.GetScores());
	GetLog() << (bullet.IsFinished() ? "finished" : "not finished") << endl;
	gr.Bids[0] = Bid_Whist; gr.Bids[1] = Bid_Whist; gr.Bids[2] = Bid_6s;
	gr.Tricks[0] = 1; gr.Tricks[1] = 2; gr.Tricks[2] = 7;
	bullet.UpdateScores(gr);
	printBullet(bullet.GetScores());
	GetLog() << (bullet.IsFinished() ? "finished" : "not finished") << endl;
}	

static CardsSet readHand(istream& in)
{
	CardsSet cards = EmptyCardsSet;
	for( int j = 0; j < InitialCardsCount; j++ ) {
		string st;
		in >> st;
		if( st.empty() ) {
			break;
		}
		cards = AddCardToSet(cards, CardFromString(st));
	}
	return cards;
}

void DoEngineTest()
{
	GameSettings gameSettings;
	EngineSettings engineSettings;
	engineSettings.MaxMisereFailures = 2;
	engineSettings.MinMisereWins = 6;
	engineSettings.ContractGameChecks = 8;
	engineSettings.NumOfSamplesPerMove = 10;
	/*engine->SetEngineSettings(engineSettings);
	ifstream in("../../tests/engine.tests");
	// Testing bidding
	for( int count = 0; count < 2; count++ ) {
		CardsSet cards = readHand(in);
		if( CardsSetSize( cards ) == EmptyCardsSet ) {
			break;
		}
		engine->OnNewLayout(cards, 0);
		Bid bid = engine->Bid();
		GetLog() << BidToString(bid) << endl;
	}*/
	/*array<scoped_ptr<PrefEngine>, 3> engines;
	array<scoped_ptr<PrefGameModel>, 3> models;
	Layout layout;
	vector<CardsSet> hands(3, EmptyCardsSet);
	for( int i = 0; i < 3; i++ ) {
		engines[i].reset( new Engine("../../data/engine.dat") );
		models[i].reset( CreatePrefModel() );
		engines[i]->SetGameSettings(gameSettings);
		engines[i]->SetEngineSettings(engineSettings);
		CardsSet hand = readHand(in);
		layout.Cards[i] = hand;
		PrefAssert( CardsSetSize( hand ) == 10 );
		hands[i] = hand;
		engines[i]->SetModel(models[i].get());
		engines[i]->SetNumInModel(i);
		models[i]->SetCallback(engines[i]);
		models[i].ProcessNewLayout(hands[0], hands[1], hands[2], 0);
		hands[i] = EmptyCardsSet;
	}
	layout.Widow = RemoveCardsFromSet(FullCardsSet, 
		AddCardsToSet( layout.Cards[0], AddCardsToSet( layout.Cards[1], layout.Cards[2] ) ));
	while( models[0]->GetDealStateType() == GST_Bidding ) {
		int currentPlayer = models[0]->CurrentPlayer();
		Bid bid = engines[currentPlayer]->Bid();
		for( int i = 0; i < engines.size(); i++ ) {
			models[i]->UpdateOnBid(bid);
		}
		GetLog() << "Player " << currentPlayer << ": " << BidToString(bid) << endl;
	}
	PrefAssert( state.CurrentStateType() == GST_Drop );
	GetLog() << "Current player: " << state.CurrentPlayer() << endl;
	GetLog() << "Widow: " << state.Widow() << endl;
	for( int i = 0; i < engines.size(); i++ ) {
		engines[i]->UpdateOnWidowRevealed(state.Widow());
	}
	state.UpdateOnWidowOpen(state.Widow());
	Drop drop = engines[state.CurrentPlayer()]->Drop();
	GetLog() << drop.Drop << " " << BidToString(drop.Contract) << endl;
	for( int i = 0; i < engines.size(); i++ ) {
		engines[i]->UpdateOnDrop(drop);
	}
	state.UpdateOnDrop(drop);
	PrefAssert( state.CurrentStateType() == GST_Whisting );
	GetLog() << "Dealer: " << state.Dealer() << endl;
	while( state.CurrentStateType() == GST_Whisting 
		|| state.CurrentStateType() == GST_OpenOrCloseWhist ) 
	{
		Bid bid = engines[state.CurrentPlayer()]->Bid();
		GetLog() << "Player " << state.CurrentPlayer() << ": " << BidToString(bid) << endl;
		for( int i = 0; i < engines.size(); i++ ) {
			engines[i]->UpdateOnBid(bid);
		}
		state.UpdateOnBid(bid);
	}

	while( state.CurrentStateType() == GST_ContractGame ) {
		Card move = engines[state.CurrentPlayer()]->Move();
		for( int i = 0; i < engines.size(); i++ ) {
			engines[i]->UpdateOnMove(move);
		}
		GetLog() << "Player " << state.CurrentPlayer() << ": " << move << endl;
		state.UpdateOnMove(move);
		GetLog() << "Moves remaining: " << state.MovesRemaining() << endl;
	}
	GetLog() << "Tricks: ";
	for( int i = 0; i < NumOfPlayers; i++ ) {
		GetLog() << state.Tricks()[i] << " ";
	}
	GetLog() << endl;*/
}

void DoServerTest()
{
}

void DoGameStateTest()
{
}

