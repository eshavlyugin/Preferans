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

#include <DBConverter.h>
#include <DatabaseParser.h>
#include <EngineData.h>
#include <MinMaxSolver.h>
#include <MiserePolicy.h>
#include <ContractGamePolicy.h>
#include <Learning.h>
#include <Layout.h>
#include <Errors.h>
#include <PrefTools.h>

extern const int MaxDataLayouts;

DBConverter::DBConverter() :
	data(0),
	passoutDropData(0),
	misereDropData(0),
	contractDropData(0),
	passoutFirstMoveData(0)
{
	reset();
}

DBConverter::~DBConverter()
{
	delete data;
	delete passoutDropData;
	delete misereDropData;
	delete contractDropData;
	delete passoutFirstMoveData;
}

Suit DBConverter::determineMoveSuit(const vector<CardsSet>& hands, const vector<Card>& moves, Suit widowSuit) const
{
	Suit candidate = GetCardSuit(moves[0]);
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( GetCardSuit(moves[i]) != candidate 
			&& HasCardsOfSuit(hands[i], candidate) ) 
		{
			return widowSuit;
		}
	}
	return candidate;
}

void DBConverter::commitDBSet(const DBSet& set)
{
	// Adding statistics for move
	switch( set.MoveNumber ) {
		case 0: 
			data->AddPassoutMove1(set.Utilized, set.Hand, set.Move1);
			break;
		case 1:
			data->AddPassoutMove2(set.Utilized, set.Hand, set.Move1, set.Move2);
			break;
		case 2:
			data->AddPassoutMove3(set.Utilized, set.Hand, set.Move1, set.Move2, set.Move3);
			break;
		default:
			PrefAssert( false );
	}
	// Adding first move to set
	if( set.MoveNumber == 0 ) {
		vector<int> firstMove(1);
		firstMove[0] = -1;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			RanksSet ranks = GetCardsOfSuit(set.Hand, itSuit.GetObject());
			if( ranks == EmptyRanksSet ) {
				continue;
			}
			firstMove.push_back( GetRanksSetIndex(ranks, GetCardsOfSuit(set.Utilized, itSuit.GetObject())) );
			if( itSuit.GetObject() == GetCardSuit(set.Move1) ) {
				firstMove[0] = firstMove.back();
			}
		}
		assert( firstMove[0] >= 0 );
		passoutFirstMoveData->AddSet(firstMove);
	}
	// Determining move suit and player suit...
	Suit moveSuit = GetCardSuit(set.Move1);
	Suit playerSuit = moveSuit;
	Card move = set.Move1;
	if( set.MoveNumber == 1 ) {
		playerSuit = GetCardSuit(set.Move2);
		move = set.Move2;
	} else if( set.MoveNumber == 2 ) {
		playerSuit = GetCardSuit(set.Move3);
		move = set.Move3;
	}
	if( moveSuit != playerSuit ) {
		GetLog() << "Move suit != player suit" << endl;
		// Adding learning set data about passout drop if required
		vector<int> sample(1);
		sample[0] = -1;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			RanksSet ranksSet = GetCardsOfSuit(set.Hand, itSuit.GetObject());
			if( ranksSet == EmptyRanksSet ) {
				continue;
			}
			sample.push_back( GetRanksSetIndex(ranksSet, GetCardsOfSuit(set.Utilized, itSuit.GetObject())) ); 
			if( itSuit.GetObject() == playerSuit ) {
				sample[0] = sample.back();
			}
		}
		PrefAssert( sample[0] >= 0 );
		GetLog() << "Adding passout learning set: ";
		// dumping learning data
		for( int i = 0; i < sample.size(); i++ ) {
			GetLog() << sample[i] << " ";
		}
		GetLog() << endl;
		passoutDropData->AddSet(sample);
	}
}

bool DBConverter::isPossibleMoveSuit(const vector<CardsSet>& hands, const vector<Card>& moves, Suit suit) const
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( GetCardSuit(moves[i]) != suit && HasCardsOfSuit(hands[i], suit) ) {
			GetLog() << suit << " " << hands[i] << endl;
			return false;
		}
	}
	return true;
}

// returns index of player who takes trick
void DBConverter::addPassoutTurn(Suit moveSuit, vector<CardsSet>& hands, int& firstPlayer, const vector<Card>& moves, int turnNumber,
	vector<DBSet>& dbCandidates, CardsSet& utilizedCards) 
{
	int nextFirstPlayer = firstPlayer;
	Card currentMax = moves[firstPlayer];
	for( int i = 0; i < NumOfPlayers; i++ ) {
		// Hand index of move #i
		int handIndex = (firstPlayer + i) % NumOfPlayers;
		// Adding move information to set
		CheckError(IsSetContainsCard(hands[handIndex], moves[handIndex]), "Wrong layout");
		GetLog() << "Move suit: " << GetCardSuit(moves[firstPlayer+i]) << endl;
		dbCandidates.push_back(DBSet(utilizedCards, hands[handIndex], moves[firstPlayer],
			moves[(firstPlayer + 1) % NumOfPlayers], moves[(firstPlayer + 2) % NumOfPlayers], i));
		// Updating current game state.
		hands[handIndex] = RemoveCardFromSet(hands[handIndex], moves[handIndex]);
		utilizedCards = AddCardToSet(utilizedCards, moves[handIndex]);
		if( IsGreaterCard(moves[handIndex], currentMax, moveSuit, SuitNoTrump) ) {
			currentMax = moves[handIndex];
			nextFirstPlayer = handIndex;
		}
	}
	// According to Sochi and Leningrad rules first hand is first player until all widow cards opened
	if( turnNumber <= 2 ) {
		firstPlayer = nextFirstPlayer;
	}
}	

void DBConverter::calcPassoutProbabilities(const vector<CardsSet>& hands, const vector<Card>& widow, 
	const vector<Card>& moves, int _firstPlayer)
{
	int firstPlayer = _firstPlayer;
	int totalCards[NumOfSuits];
	int currentHand = 0;
	for( int i = 0; i < NumOfSuits; i++ ) {
		totalCards[i] = NumOfRanks;
	}
	vector<CardsSet> currentHands = hands;

	CardsSet utilizedCards = EmptyCardsSet;
	
	GetLog() << "ADDING... " << hands.size() << " " << widow.size() << " " << moves.size() << " " << firstPlayer << endl;
	vector<DBSet> dbCandidates;

	for( int firstCard = 0; firstCard + NumOfPlayers <= moves.size(); firstCard += NumOfPlayers ) {
		// Number of current turn
		int moveNumber = firstCard / NumOfPlayers;
		// Moves in current turn
		vector<Card> moveCards(NumOfPlayers);
		for( int i = 0; i < NumOfPlayers; i++ ) {
			moveCards[i] = moves[firstCard + i];
			GetLog() << "MOVE: " << moveCards[i] << endl;
		}
		// Move suit
		Suit moveSuit = GetCardSuit( moveCards[firstPlayer] );
	
		// Checking for errors. 
		GetLog() << "FirstPlayer: " << firstPlayer << endl;
		// Checking possible errors in data
		CheckError( IsSetContainsCard( currentHands[firstPlayer], moveCards[firstPlayer] ), "Bad game: card is not from set" );
		CheckError(	(isPossibleMoveSuit( currentHands, moveCards, moveSuit ) 
			|| (moveNumber < 2 && isPossibleMoveSuit( currentHands, moveCards, GetCardSuit(widow[moveNumber]) ) ) ), 
			"Impossible move suit. Probably it's Rostov passout" 
		);
		CheckError( moveNumber >= 2 || moveSuit == GetCardSuit(widow[moveNumber]), "Impossible move suit. Probably it's Rostov passout" );
		// Adding widow card
		if( moveNumber < 2 ) {
			utilizedCards = AddCardToSet(utilizedCards, widow[moveNumber]);
		}
		
		// Adding pending information about players moves in current turn.
		// This method updates almost all input parameters.
		addPassoutTurn(moveSuit, currentHands, firstPlayer, moveCards, moveNumber, dbCandidates, utilizedCards);
	}
	// Adding candidate layouts to data
	for( int i = 0; i < dbCandidates.size(); i++ ) {
		commitDBSet( dbCandidates[i] );
	}
	GetLog() << "Passout successfully processed" << endl;
}

void DBConverter::Reset()
{
	reset();
}

void DBConverter::reset() 
{
	delete data;
	delete passoutDropData;
	delete misereDropData;
	delete contractDropData;
	delete passoutFirstMoveData;

	data = new EngineData();
	passoutDropData = new LearningData(NumOfRanks * (1 << NumOfRanks));
	misereDropData = new LearningData(NumOfRanks * (1 << NumOfRanks));
	contractDropData = new LearningData(NumOfRanks * (1 << NumOfRanks));
	passoutFirstMoveData = new LearningData(NumOfRanks * (1 << NumOfRanks));
}

int DBConverter::determineDealer(int firstPlayer, const vector<BidType>& bidding)
{
	vector<bool> isPassed(NumOfPlayers, false);
	int currentPlayer = firstPlayer;
	for( int i = 0; i < bidding.size(); i++ ) {
		int nOfPasses = 0;
		while( isPassed[currentPlayer] ) {
			currentPlayer = (currentPlayer + 1) % NumOfPlayers;
			nOfPasses++;
			CheckError( nOfPasses < 3, "Error: wrong bidding history" );
		}
		isPassed[currentPlayer] = isPassed[currentPlayer] || (bidding[i] == Bid_Pass);
		currentPlayer = (currentPlayer + 1) % NumOfPlayers;
	}
	int dealer = -1;
	for( int i = 0; i < isPassed.size(); i++ ) {
		if( !isPassed[i] ) {
			CheckError( dealer < 0, "Error: wrong bidding history" );
			dealer = i;
		}
	}
	CheckError( dealer >= 0, "Error: wrong bidding history" );
	return dealer;
}	

void DBConverter::addMisereGame(const vector<CardsSet>& hands, 
	const vector<Card>& widow, const vector<BidType>& bidding, int firstPlayer, int dealer)
{
	if( data->GetLayouts(Bid_Misere).size() >= MaxDataLayouts ) {
		// Cutoff in layouts count
		return;
	}
	CardsSet dealerWithWidow = AddCardToSet(AddCardToSet(hands[dealer], widow[0]), widow[1]);
	vector< vector<Card> > drops = GetPossibleDrops(dealerWithWidow, true);
	vector< vector<Card> > bestDrops;
	int bestResult = 11;
	GetLog() << "Processing game... " << hands[0] << " " << hands[1] << " " << hands[2] << endl;
	for( int i = 0; i < drops.size(); i++ ) {
		MiserePolicy policy;
		policy.SetCheckingMode(true);
		MinMaxSolver<MiserePolicy, true> solver(policy);
		Layout layout;
		layout.Trump = SuitNoTrump;
		layout.Dealer = dealer;
		layout.CurrentPlayer = firstPlayer;
		for( int j = 0; j < NumOfPlayers; j++ ) {
			layout.Cards[j] = hands[j];
		}
		layout.Cards[dealer] = RemoveCardFromSet(RemoveCardFromSet(dealerWithWidow, drops[i][0]), drops[i][1]);
		int result = solver.GetBestResult(layout, 0, 2);
		GetLog() << "Deal: misere" << endl << "Result: " << result << endl;
		if( result < bestResult ) {
			bestDrops.clear();
			bestResult = result;
			bestDrops.push_back(drops[i]);
		} else if( result == bestResult ) {
			bestDrops.push_back(drops[i]);
		}
	}
	GetLog() << "Contract: misere" << " Result: " << bestResult << endl;
	if( bestResult >= 2 ) {
		// Too bad. Don't add this layout to learning set
		return;
	}
	addDropsDataToLearningSet(dealerWithWidow, Bid_Misere, bestDrops, misereDropData);
}

void DBConverter::addContractGame(const vector<CardsSet>& hands, 
	const vector<Card>& widow, const vector<BidType>& bidding, BidType contract, int firstPlayer, int dealer)
{
	if( data->GetLayouts(contract).size() >= MaxDataLayouts ) {
		return;
	}
	CardsSet dealerWithWidow = AddCardToSet(AddCardToSet(hands[dealer], widow[0]), widow[1]);
	vector< vector<Card> > drops = GetPossibleDrops(dealerWithWidow, false);
	vector< vector<Card> > bestDrops;
	Suit trump = GetContractTrump(contract);
	int deal = GetContractGameDeal(contract);
	int bestResult = -1;
	GetLog() << "Processing game... " << hands[0] << " " << hands[1] << " " << hands[2] << endl;
	GetLog() << "Dealer with widow: " << dealerWithWidow << endl;
	GetLog() << "Trump: " << trump << endl;
	for( int i = 0; i < drops.size(); i++ ) {
		ContractGamePolicy policy;
		policy.SetCheckingMode(true);
		MinMaxSolver<ContractGamePolicy, false> solver(policy);
		Layout layout;
		layout.Trump = trump;
		layout.Dealer = dealer;
		layout.CurrentPlayer = firstPlayer;
		for( int j = 0; j < NumOfPlayers; j++ ) {
			layout.Cards[j] = hands[j];
		}
		GetLog() << "Deal: " << deal << endl;
		layout.Cards[dealer] = RemoveCardFromSet(RemoveCardFromSet(dealerWithWidow, drops[i][0]), drops[i][1]);
		int result = solver.GetBestResult(layout, deal - 2, 10);
		GetLog() << "Result: " << result << endl;
		if( result > bestResult ) {
			bestDrops.clear();
			bestResult = result;
			bestDrops.push_back(drops[i]);
		} else if( result == bestResult ) {
			bestDrops.push_back(drops[i]);
		}
	}
	GetLog() << "Contract: " << contract << " Result: " << bestResult << endl;
	if( bestResult <= deal - 2 ) {
		// Too bad. Don't add layout to learning set
		return;
	}
	addDropsDataToLearningSet(dealerWithWidow, contract, bestDrops, contractDropData);
}

void DBConverter::addDropsDataToLearningSet(CardsSet dealerWithWidow, BidType contract,
	const vector< vector<Card> >& bestDrops, LearningData* learningData)
{
	PrefAssert( bestDrops.size() > 0 );
	// Determining drop index.
	int index = 0;
	int count = 17; // Infinity
	// From all drops leading to the same result select one with least cards count
	for( int i = 0; i < bestDrops.size(); i++ ) {
		RanksSet ranksSet1 = GetCardsOfSuit(dealerWithWidow, GetCardSuit(bestDrops[i][0]));
		RanksSet ranksSet2 = GetCardsOfSuit(dealerWithWidow, GetCardSuit(bestDrops[i][1]));
		if( RanksSetSize(ranksSet1) > RanksSetSize(ranksSet2) ) {
			continue;
		}
		int tmp = RanksSetSize(ranksSet1) + RanksSetSize(ranksSet2);
		if( GetCardSuit(bestDrops[i][0]) == GetCardSuit(bestDrops[i][1]) ) {
			tmp--;
		}
		if( tmp < count ) {
			count = tmp;
			index = i;
		}
	}

	const vector<Card>& drop = bestDrops[index];
	CardsSet utilized = EmptyCardsSet;
	CardsSet playerCards = dealerWithWidow;
	GetLog() << drop[0] << " " << drop[1] << endl;
	for( int i = 0; i < drop.size(); i++ ) {
		vector<int> dataSet(1);
		dataSet[0] = -1;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			RanksSet ranksSet = GetCardsOfSuit(playerCards, itSuit.GetObject());
			if( ranksSet == EmptyRanksSet ) {
				continue;
			}
			int value = GetRanksSetIndex(ranksSet, GetCardsOfSuit(utilized, itSuit.GetObject()));
			dataSet.push_back(value);
			if( itSuit.GetObject() == GetCardSuit(drop[i]) ) {
				dataSet[0] = dataSet.back();
			}
		}
		assert( dataSet[0] >= 0 );
		learningData->AddSet(dataSet);
		utilized = AddCardToSet(utilized, drop[i]);
		playerCards = RemoveCardFromSet(playerCards, drop[i]);
	}
	GetLog() << "Learning data was added" << endl;
	data->AddLayout(contract, RemoveCardFromSet(RemoveCardFromSet(dealerWithWidow, drop[0]), drop[1]));
}

void DBConverter::Convert(const string& src, const string& dest)
{
	reset();

	ifstream inFile;
	inFile.open(src.c_str());
	DatabaseParser parser(inFile);
	ParsedGame game;
	while( parser.ParseNextGame(game) ) {
		try {
			if( CardsSetSize( game.Hands[0] ) != InitialCardsCount
				|| CardsSetSize( game.Hands[1] ) != InitialCardsCount
				|| CardsSetSize( game.Hands[2] ) != InitialCardsCount )
			{
				// probably error while parsing database
				continue;
			}
			if( game.Contract == Bid_Pass ) {
				// processing passout deal
				calcPassoutProbabilities(game.Hands, game.Widow, 
					game.Moves, game.FirstPlayer);
			} else if( game.Contract == Bid_Misere ) {
				// processing misere deal
				for( int dealer = 0; dealer < 3; dealer++ ) {
					addMisereGame(game.Hands, game.Widow, game.Bidding, game.FirstPlayer, dealer);
				}
			} else if( GetContractGameDeal(game.Contract) >= 6 ) {
				// Processing contract game deal
				addContractGame(game.Hands, game.Widow, game.Bidding, game.Contract, game.FirstPlayer, 
					determineDealer(game.FirstPlayer, game.Bidding));
			}
		} catch( Exception& e ) {
			GetLog() << e.GetErrorText() << endl;
		}
	}

	data->DumpPassoutDB();
	vector<float> passoutWeights = Learn(*passoutDropData);
	vector<float> contractWeights = Learn(*contractDropData);
	vector<float> misereWeights = Learn(*misereDropData);
	vector<float> passoutFirstMoveWeights = Learn(*passoutFirstMoveData);
	PrintFloatArray(passoutWeights);
	PrintFloatArray(contractWeights);
	PrintFloatArray(misereWeights);
	PrintFloatArray(passoutFirstMoveWeights);
	data->SetPassoutDropWeights(passoutWeights);
	data->SetMisereDropWeights(misereWeights);
	data->SetContractDropWeights(contractWeights);
	data->SetPassoutFirstMoveWeights(passoutFirstMoveWeights);
	ofstream out("engine.dat");
	data->Save(out);
	ifstream in("engine.dat");
	data->Load(in);
	data->DumpPassoutDB();
}

