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

#include <PrefEngineImpl.h>
#include <EngineData.h>
#include <PassoutPolicy.h>
#include <MinMaxSolver.h>
#include <MonteCarloSolver.h>
#include <MinMaxSolver.h>
#include <MiserePolicy.h>
#include <ContractGamePolicy.h>
#include <DealState.h>
#include <Bullet.h>
#include <PrefTools.h>
#include <RandomLayoutsGenerator.h>
#include <Errors.h>

PrefEngineImpl::PrefEngineImpl(const string& dataPath) :
	contractTrialsOutcomes(boost::extents[InitialCardsCount + 1][NumOfSuits + 1]),
	playerNum(0),
	callback(0)
{
	data.reset( new EngineData() );
	layoutsGenerator.reset( new RandomLayoutsGenerator() );
	ifstream in(dataPath.c_str());
	data->Load(in);
}

PrefEngineImpl::~PrefEngineImpl()
{
}

void PrefEngineImpl::SetEngineCallback(PrefEngineCallback* _callback)
{
	callback = _callback;
}

void PrefEngineImpl::processNewLayoutStart() 
{
	std::fill(contractTrialsOutcomes.data(), contractTrialsOutcomes.data() + contractTrialsOutcomes.num_elements(), 0);
	layoutsGenerator->Reset();
	playerNum = 0;
	for( int player = 0; player < NumOfPlayers; player++ ) {
		CardsSet hand = model->GetPlayerCards(player);
		if( hand == EmptyCardsSet ) {
			continue;
		}
		layoutsGenerator->SetPlayerCards(hand, player);
		for( int i = 0; i <= InitialCardsCount; i++ ) {
			for( int j = 0; j <= 4; j++ ) {
				contractTrialsOutcomes[i][j] = 0;
			}
		}
	}
}

void PrefEngineImpl::processDealStateChanged(DealStateType prevState, DealStateType newState) 
{
	GetLog() << "processDealStateChanged" << std::endl;
	if( (newState == GST_Misere || newState == GST_ContractGame)
		&& model->Dealer() != 0 ) 
	{
		GetLog() << "Dealer = " << model->Dealer() << std::endl;
		layoutsGenerator->ResetProbabilitiesForContract(model->Contract(), model->Dealer(), *data);
	}
}

void PrefEngineImpl::processPlayerBid(BidType bid) 
{
	PrefAssert(model != 0);
	updateRanksAfterBid(bid);
}

void PrefEngineImpl::processPlayerDrop(CardsSet drop, BidType deal) 
{
	PrefAssert( model != 0 );
	updateRanksAfterDrop(Drop(drop, deal));
}

void PrefEngineImpl::processPlayerMove(Card move)
{
	PrefAssert( model != 0 );
	updateRanksAfterMove(move);
}

void PrefEngineImpl::processHandOpening(CardsSet hand, int player)
{
	PrefAssert( model != 0 );
	layoutsGenerator->SetPlayerCards(hand, player);
}

void PrefEngineImpl::processPassoutWidowCardOpening(Card card)
{
	PrefAssert( model != 0 );
	updateRanksAfterPassoutWidowOpened(card);
}

void PrefEngineImpl::processWidowOpening(CardsSet widow)
{
	updateRanksAfterWidowOpened(widow);
}

void PrefEngineImpl::updateRanksAfterPassoutWidowOpened(Card card)
{
	layoutsGenerator->SetWidowCards(AddCardToSet(EmptyCardsSet, card));
}

void PrefEngineImpl::updateRanksAfterWidowOpened(CardsSet widow)
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( i != model->Dealer() ) {
			layoutsGenerator->ForbidCardsSet(widow, i);
		}
	}
}

bool PrefEngineImpl::AcceptOffer(int offer)
{
	// TODO: not implemented
	return false;
}

Card PrefEngineImpl::DoMove()
{
	callback->OnEngineCallback();
	switch( model->GetDealStateType() ) {
		case GST_Misere:
			return doMisereMove();
		case GST_Passout:
			return doPassoutMove();
		case GST_ContractGame:
			return doContractGameMove();
		default:
			// not a valid game state for move
			PrefAssert(false);
			return -1;
	}
}

BidType PrefEngineImpl::DoBid()
{
	callback->OnEngineCallback();
	switch( model->GetDealStateType() ) {
		case GST_Bidding:
			return doTradeBid();
		case GST_Whisting:
			return doWhistBid();
		case GST_OpenOrCloseWhist:
			return doOpenOrCloseWhistBid();
		default:
			PrefAssert(false);
			return Bid_Pass;
	}
}

Drop PrefEngineImpl::DoDrop()
{
	PrefAssert(model != 0);
	PrefAssert(model->GetDealStateType() == GST_Drop);
	callback->OnEngineCallback();
	return doDrop();
}

void PrefEngineImpl::updateRanksAfterMove(Card move)
{
	GetLog() << "Updating ranks after move" << endl;
	if( model->PrevPlayer() != playerNum ) {
		layoutsGenerator->SetPlayerCards(AddCardToSet(EmptyCardsSet, move), model->PrevPlayer());
		int player = model->PrevPlayer();
		if ( model->PrevMoveSuit() != SuitNoTrump && !model->HasCardsOfSuit( player, model->PrevMoveSuit() ) ) {
			layoutsGenerator->SetPlayerHasNoMoreCardsOfSuit( player, 
				model->PrevMoveSuit(), model->UtilizedCards() );
		}
	}
}

void PrefEngineImpl::updateRanksAfterBid(BidType bid)
{
}

void PrefEngineImpl::updateRanksAfterDrop(const Drop& drop)
{
}

Card PrefEngineImpl::doPassoutMove()
{
	vector<float> totalScores(InitialCardsCount, 0.0f);
	Card bestMove = 0;
	float bestScore = 0.0f;

	// If no match found in data generate random layout
	vector<Layout> layouts = layoutsGenerator->GenerateLayouts(engineSettings.NumOfPassoutLayouts);
	// For all possible moves calculating probability of number of tricks for current player if he starts from that move
	// Sum all probabilities for all layouts and calculating mean
	for( int i = 0; i < layouts.size(); i++ ) {
		PassoutPolicy policy(data.get());
		MonteCarloSolver<PassoutPolicy> solver(policy);
		Layout layout = layouts[i];
		prepareLayout(layout);
		for( int i = 0; i < NumOfPlayers; i++ ) {
			GetLog() << layout.Cards[i] << endl;
		}
		solver.Solve(layout, engineSettings.NumOfPassoutSimulations);
		callback->OnEngineCallback();
		const McTreeNode* root = solver.ResultTreeRoot();
		// +Infinity
		bestScore = 1e20f;
		// Assume all possible moves from given layout are ordered.
		int index = 0;
		for( const McTreeNode* child = root->FirstChild; child != 0; child = child->Next, index++ ) {	
			// Selecting move with lowest mean
			totalScores[index] += 1.0f * child->TotalScores[layout.CurrentPlayer] / child->PlayoutsCount;
			if( totalScores[index] < bestScore ) {
				bestScore = totalScores[index];
				bestMove = child->Move;
			}
		}
	}

	return bestMove;
}

Card PrefEngineImpl::doMisereMove()
{
	vector<Layout> candidates;
	candidates = layoutsGenerator->GenerateLayouts(engineSettings.NumOfSamplesPerMove);
	
	vector<MoveQuality> qualities;
	PrefAssert( candidates.size() != 0 );
	for( int i = 0; i < candidates.size(); i++ ) {
		MiserePolicy policy;
		policy.SetCheckingMode(false);
		MinMaxSolver<MiserePolicy, true> solver(policy);
		Layout layout = candidates[i];
		prepareMisereLayout(layout);
		for( int j = 0; j < 3; j++ ) {
			GetLog() << layout.Cards[j] << endl;
		}
		if( i == 0 ) {
			qualities = solver.Solve(layout, 0, 5);
			PrefAssert( qualities.size() > 0 );
		} else {
			vector<MoveQuality> tmp = solver.Solve(layout, 0, 5);
			PrefAssert( tmp.size() == qualities.size() );
			for( int j = 0; j < tmp.size(); j++ ) {
				if( model->Dealer() == playerNum ) {
					qualities[j].Score = std::max(tmp[j].Score, qualities[j].Score);
				} else {
					qualities[j].Score = std::min(tmp[j].Score, qualities[j].Score);
				}
			}
		}
		callback->OnEngineCallback();
	}

	vector<Card> bestMoves;
	selectBestMoves(qualities, bestMoves, playerNum != model->Dealer());
	int sample = RandomNextInt(bestMoves.size());
	return bestMoves[sample];
}

void PrefEngineImpl::prepareLayout( Layout& layout ) const
{
	layout.CurrentPlayer = model->CurrentPlayer();
	for( int i = 0; i < NumOfPlayers; i++ ) {
		layout.Desk[i] = model->GetCardOnDesk(i);
	}
	layout.Dealer = model->Dealer();
	layout.MoveSuit = model->CurrentMoveSuit();
	layout.MaxCard = model->MaxCardOnDesk();
	layout.MovesRemaining = model->DealMovesRemaining();
	layout.NumCardsOnDesk = model->NumCardsOnDesk();
	layout.Trump = GetContractTrump(model->Contract());
	if( playerNum == layout.Dealer && model->Contract() != Bid_Pass ) {
		layout.Cards[playerNum] = RemoveCardsFromSet( 
			AddCardsToSet( layout.Cards[playerNum], model->Widow() ), model->DroppedCards() );
	}
	for( int i = 0; i < NumOfPlayers; i++ ) {
		layout.Cards[i] = RemoveCardsFromSet( layout.Cards[i],
			CardsSetsIntersection( layout.Cards[i], model->UtilizedCards() ) );
	}
}

void PrefEngineImpl::prepareContractGameLayout(Layout& layout) const
{
	// saving initial layout parameters
	CardsSet initialCardsSet = layout.Cards[model->Dealer()];
	CardsSet widow = layout.Widow;
	// getting current game state layout
	prepareLayout(layout);
	// we already know dropped cards. Nothing to guess
	if( model->Dealer() == model->CurrentPlayer() ) {
		return;
	}
	// selecting possible drops taking into account move ordering
	vector< vector<Card> > drops = GetPossibleDrops(AddCardsToSet(layout.Cards[model->Dealer()], widow), false);
	filterDropsByEmptyHands(drops, AddCardsToSet(initialCardsSet, widow), model->UtilizedCards());
	// selecting best available drop
	Drop drop = doContractGameDrop(AddCardsToSet(initialCardsSet, widow), drops);
	cerr << "Drop: " << drop.Cards << endl;
	PrefAssert( drop.Cards != EmptyCardsSet );
	layout.Cards[model->Dealer()] = RemoveCardsFromSet(AddCardsToSet(layout.Cards[model->Dealer()], widow), drop.Cards);
}

void PrefEngineImpl::filterDropsByEmptyHands( vector< vector<Card> >& drops, CardsSet dealer, CardsSet utilizedCards) const
{
	vector< vector<Card> > newDrops;
	for( int i = 0; i < drops.size(); i++ ) {
		bool isGoodDrop = true;
		for( SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
			CardsSet cardsWithoutWidow = RemoveCardFromSet(
				RemoveCardFromSet(dealer, drops[i][1]), drops[i][0]);
			if( !model->HasCardsOfSuit(model->Dealer(), suitIt.GetObject()) 
				&& GetRanksOfSuit( RemoveCardsFromSet(cardsWithoutWidow, utilizedCards), suitIt.GetObject() ) != EmptyRanksSet ) 
			{
				isGoodDrop = false;
				break;
			}
		}
		if( isGoodDrop ) {
			newDrops.push_back(drops[i]);
		}
	}
	drops = newDrops;
}

void PrefEngineImpl::prepareMisereLayout(Layout& layout) const
{
	CardsSet initialCardsSet = layout.Cards[model->Dealer()];
	CardsSet widow = layout.Widow;
	prepareLayout(layout);
	if( model->Dealer() == model->CurrentPlayer() ) {
		return;
	}
	vector< vector<Card> > drops = GetPossibleDrops(AddCardsToSet(layout.Cards[model->Dealer()], widow), true);
	filterDropsByEmptyHands(drops, AddCardsToSet(initialCardsSet, widow), model->UtilizedCards());
	Drop drop = doMisereDrop(AddCardsToSet(initialCardsSet, widow), drops);
	PrefAssert( drop.Cards != EmptyCardsSet );
	layout.Cards[model->Dealer()] = RemoveCardsFromSet(AddCardsToSet(layout.Cards[model->Dealer()], widow), drop.Cards);
}

Card PrefEngineImpl::doContractGameMove()
{
	vector<Layout> candidates;
	if( model->Dealer() == model->CurrentPlayer() ) {
		candidates = layoutsGenerator->GenerateLayouts(engineSettings.NumOfSamplesPerMove);
	} else {
		candidates = layoutsGenerator->SelectLayoutsFromDatabase(model->CurrentPlayer(), engineSettings.NumOfSamplesPerMove);
		if( candidates.size() < engineSettings.NumOfSamplesPerMove ) {
			candidates = layoutsGenerator->GenerateLayouts(engineSettings.NumOfSamplesPerMove);
		} else {
			GetLog() << "Layouts were taken from database" << endl;
		}
	}
	PrefAssert( candidates.size() != 0 );
	
	vector<MoveQuality> qualities;
	bool isOpenWhist = false;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		isOpenWhist = isOpenWhist || model->GetPlayerBid(i) == Bid_OpenWhist;
	}
	GetLog() << (isOpenWhist ? "OpenWhist" : "CloseWhist") << endl;

	for( int i = 0; i < candidates.size(); i++ ) {
		ContractGamePolicy policy;
		policy.SetCheckingMode(false);
		MinMaxSolver<ContractGamePolicy, false> solver(policy);
		Layout layout = candidates[i];
		prepareContractGameLayout(layout);
		GetLog() << "P1: " << layout.Cards[0] << "\n" << "P2: " << layout.Cards[1] << "\n" << "P3: " << layout.Cards[2] << endl;
		if( i == 0 ) {
			qualities = solver.Solve(layout, 4, 10);
			PrefAssert( qualities.size() > 0 );
		} else {
			vector<MoveQuality> tmp = solver.Solve(layout, 4, 10);
			PrefAssert( tmp.size() == qualities.size() );
			for( int j = 0; j < tmp.size(); j++ ) {
				if( !isOpenWhist ) {
					qualities[j].Score += tmp[j].Score;
				} else if( model->Dealer() == playerNum ) {
					qualities[j].Score = std::min(tmp[j].Score, qualities[j].Score);
				} else {
					qualities[j].Score = std::max(tmp[j].Score, qualities[j].Score);
				}
			}
		}
		callback->OnEngineCallback();
	}

	vector<Card> bestMoves;
	selectBestMoves(qualities, bestMoves, playerNum == model->Dealer());
	int sample = RandomNextInt(bestMoves.size());
	return bestMoves[sample];
}

void PrefEngineImpl::selectBestMoves(const vector<MoveQuality>& qualities, vector<Card>& result, bool isMax) const
{
	result.clear();
	int currentBest = !isMax ? INT_MAX : -1;
	for( int i = 0; i < qualities.size(); i++ ) {
		GetLog() << qualities[i].Move << " " << qualities[i].Score << std::endl;
		if( qualities[i].Score == currentBest ) {
			result.push_back(qualities[i].Move);
		} else if( (isMax && qualities[i].Score > currentBest) 
			|| (!isMax && qualities[i].Score < currentBest) ) 
		{
			currentBest = qualities[i].Score;
			result.clear();
			result.push_back(qualities[i].Move);
		}
	}
}

BidType PrefEngineImpl::doWhistBid()
{
	// TODO: not implemented
	return Bid_Whist;
}

BidType PrefEngineImpl::doOpenOrCloseWhistBid()
{
	// TODO: not implemented
	return Bid_OpenWhist;
}

BidType PrefEngineImpl::doTradeBid()
{
	// Trying to play misere
	if( isPlayMisereGoodIdea() ) {
		return Bid_Misere;
	}
	
	BidType bid = getMaxAcceptableContractGame();
	// Trying to play contract game
	if( bid != Bid_Pass 
		&& (bid > model->MaxBid() || (bid == model->MaxBid() && model->IsValidBid(model->MaxBid())))
		&& model->IsValidBid(bid) ) 
	{
		if( model->MaxBid() == Bid_Pass ) {
			int deal = 6;
			// Taking into account raise of first bid after passouts
			while( deal <= 10 ) {
				BidType result = ConstructContractGameBid(deal, SuitSpades);
				if( model->IsValidBid( result ) ) {
					return result;
				}
				deal++;
			}
			PrefAssert( false );
		} else if( model->IsValidBid(model->MaxBid()) ) {
			return model->MaxBid();
		} else {
			return (BidType)(model->MaxBid() + 1);
		}
	}
	
	// If we're unlucky... Just passing
	return Bid_Pass;
}

// returns maximal acceptable contract game or pass if playing game is too risky
BidType PrefEngineImpl::getMaxAcceptableContractGame() const
{
	vector<Layout> layouts = layoutsGenerator->GenerateLayouts(engineSettings.ContractGameChecks);
	// Trying random trials to determine if it's possible to play contract game
	for( int i = 0; i < layouts.size(); i++ ) {
		ContractGamePolicy policy;
		policy.SetCheckingMode(true);
		MinMaxSolver<ContractGamePolicy, false> solver(policy);
		Layout layout = layouts[i];
		layout.Dealer = model->CurrentPlayer();
		layout.Cards[playerNum] = AddCardsToSet(layout.Cards[playerNum], layout.Widow);
		Drop drop = doContractGameDrop(layout.Cards[playerNum], GetPossibleDrops(layout.Cards[playerNum], true));
		layout.Cards[playerNum] = RemoveCardsFromSet(layout.Cards[playerNum], drop.Cards);
		layout.Trump = determineBestTrump(layout.Cards[playerNum]);
		int result = solver.GetBestResult(layout, 4, 10);
		contractTrialsOutcomes[result][layout.Trump.Value]++;
		callback->OnEngineCallback();
	}

	BidType result = Bid_Pass;
	// Evaluating results of random trials
	// Deal is acceptable if mean of total score of our sampling is positive
	// TODO: take into account game without trump
	for( int deal = 6; deal <= 10; deal++ ) {
		vector<float> evaluations = getGamesEvaluations(deal);
		vector<float> evaluationsNext = deal < 10 ? getGamesEvaluations(deal + 1) : evaluations;
		for( SuitForwardIterator itContractSuit; itContractSuit.HasNext(); itContractSuit.Next() ) { 
			float totalProfit = 0.0f;
			for( int i = 0; i < contractTrialsOutcomes.size(); i++ ) {
				for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
					if( itContractSuit.GetObject().Value > itSuit.GetObject().Value && deal < 10 ) {
						// we have to raise deal
						totalProfit += evaluationsNext[i] * contractTrialsOutcomes[i][itSuit.GetObject().Value];
					} else {
						// usual case
						totalProfit += evaluations[i] * contractTrialsOutcomes[i][itSuit.GetObject().Value];
					}
				}
			}
			// cutoff rule
			if( totalProfit < 0 ) {
				break;
			}
			// Updating result
			result = ConstructContractGameBid(deal, itContractSuit.GetObject());
		}
	}
	return result;
}

vector<float> PrefEngineImpl::getGamesEvaluations(int deal) const
{
	vector<float> result(InitialCardsCount + 1);
	for( int i = 0; i < contractTrialsOutcomes.size(); i++ ) {
		DealResult tmp;
		tmp.Bids[0] = ConstructContractGameBid(deal, SuitFirst);
		tmp.Bids[1] = Bid_Pass;
		tmp.Bids[2] = Bid_Pass;
		tmp.Tricks[0] = i;
		tmp.Tricks[1] = InitialCardsCount - i;
		tmp.Tricks[2] = 0;
		result[i] = model->EvaluateDealResult(tmp)[0];
	}
	return result;
}

// Trying to determine best trump for contract game
Suit PrefEngineImpl::determineBestTrump(CardsSet hand) const
{
	RanksSet resultSet = GetCardsOfSuit(hand, SuitFirst);
	Suit result = SuitFirst;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		if( SuitSetSize(hand, itSuit.GetObject()) > SuitSetSize(hand, result) ) {
			result = itSuit.GetObject();
		} else if( SuitSetSize(hand, itSuit.GetObject()) == SuitSetSize(hand, result) &&
			GetCardsOfSuit(hand, itSuit.GetObject()).Value > resultSet.Value ) 
		{
			result = itSuit.GetObject();
			resultSet = GetCardsOfSuit(hand, itSuit.GetObject());
		}
	}
	return result;
}

bool PrefEngineImpl::isPlayMisereGoodIdea() const
{
	if( !model->IsValidBid(Bid_Misere) ) {
		return false;
	}

	int numOfFailures = 0;
	int numOfWins = 0;
	vector<Layout> layouts = layoutsGenerator->GenerateLayouts(engineSettings.MaxMisereFailures + engineSettings.MinMisereWins);
	for( int index = 0; numOfFailures < engineSettings.MaxMisereFailures && numOfWins < engineSettings.MinMisereWins; index++ ) {
		MiserePolicy policy;
		policy.SetCheckingMode(true);
		MinMaxSolver<MiserePolicy, true> solver(policy);
		Layout layout = layouts[index];
		layout.Dealer = playerNum;
		layout.CurrentPlayer = 0;
		layout.Cards[playerNum] = AddCardsToSet(layout.Cards[playerNum], layout.Widow);
		Drop drop = doMisereDrop(layout.Cards[playerNum], GetPossibleDrops(layout.Cards[playerNum], false));
		layout.Cards[playerNum] = RemoveCardsFromSet(layout.Cards[playerNum], drop.Cards);
		if( solver.GetBestResult(layout, 0, 1) == 0 ) {
			numOfWins++;
		} else {
			numOfFailures++;
		}
		callback->OnEngineCallback();
	}
	return numOfWins == engineSettings.MinMisereWins;
}

Drop PrefEngineImpl::doMisereDrop(CardsSet hand, const vector<vector<Card> >& allDrops) const
{
	PrefAssert( CardsSetSize( hand ) == 12 );
	vector<CandidateDrop> candidates;
	for( int i = 0; i < allDrops.size(); i++ ) {
		const vector<Card>& drop = allDrops[i];
		array<float, NumOfSuits> weights1;
		data->GetMisereDropWeights(hand, EmptyCardsSet, weights1);
		array<float, NumOfSuits> weights2;
		CardsSet dealer = RemoveCardFromSet(hand, drop[0]);
		CardsSet utilized = AddCardToSet(EmptyCardsSet, drop[0]);
		data->GetMisereDropWeights(dealer, utilized, weights2);
		float weight = weights1[GetCardSuit(drop[0]).Value] * weights2[GetCardSuit(drop[1]).Value];
		candidates.push_back(CandidateDrop(AddCardToSet(utilized, drop[1]), weight));
	}

	Drop result = selectDropFromCandidates(candidates);
	result.Contract = Bid_Misere;
	return result;
}

Drop PrefEngineImpl::doContractGameDrop(CardsSet hand, const vector< vector<Card> >& allDrops ) const
{
	PrefAssert( CardsSetSize( hand ) == 12 );
	vector<CandidateDrop> candidates;
	for( int i = 0; i < allDrops.size(); i++ ) {
		const vector<Card>& drop = allDrops[i];
		array<float, NumOfSuits> weights1;
		data->GetContractDropWeights(hand, EmptyCardsSet, weights1);
		array<float, NumOfSuits> weights2;
		CardsSet _hand = RemoveCardFromSet(hand, drop[0]);
		CardsSet utilized = AddCardToSet(EmptyCardsSet, drop[0]);
		data->GetContractDropWeights(_hand, utilized, weights2);
		float weight = weights1[GetCardSuit(drop[0]).Value] * weights2[GetCardSuit(drop[1]).Value];
		candidates.push_back(CandidateDrop(AddCardToSet(utilized, drop[1]), weight));
	}

	Drop result = selectDropFromCandidates(candidates);
	return result;
}

BidType PrefEngineImpl::chooseContractDeal(CardsSet drop) const
{
	vector<Layout> layouts = layoutsGenerator->GenerateLayouts(engineSettings.ContractGameChecks);
	int player = model->CurrentPlayer();
	Suit trump = SuitInvalid;
	// Calculating number of tricks in each sample
	array<int, InitialCardsCount + 1> deals;
	deals.assign(0.0);
	for( int i = 0; i < layouts.size(); i++ ) {
		Layout& layout = layouts[i];
		layout.Cards[player] = AddCardsToSet(layout.Cards[player], layout.Widow);
		layout.Cards[player] = RemoveCardsFromSet(layout.Cards[player], drop);
		layout.Dealer = player;
		layout.Trump = determineBestTrump(layout.Cards[player]);
		trump = layout.Trump;
		ContractGamePolicy policy;
		policy.SetCheckingMode(true);
		MinMaxSolver<ContractGamePolicy, false> solver(policy);
		int count = solver.GetBestResult(layout, 4, 10);
		deals[count]++;
		callback->OnEngineCallback();
	}
	PrefAssert( trump != SuitInvalid );
	// Choosing the best game according to statistics
	BidType result = Bid_Pass;
	float maxValue = -1e20f;
	int bestDeal = 6;
	for( int deal = 6; deal <= 10; deal++ ) {
		BidType tmp = ConstructContractGameBid(deal, trump);
		if( !model->IsValidDrop(Drop(drop, tmp)) ) {
			continue;
		}
		vector<float> evaluations = getGamesEvaluations(deal);
		float mean = 0.0f;
		for( int i = 0; i < evaluations.size(); i++ ) {
			mean += evaluations[i] * deals[i];
		}
		if( mean > maxValue ) {
			maxValue = mean;
			bestDeal = deal;
			result = ConstructContractGameBid(deal, trump);
		}
	}
	if( result != Bid_Pass ) {
		return result;
	}
	// Emergency case: two players raised bid to 10's. Don't know what to do. 
	// Choosing simplest solution - terminate progam
	PrefAssert( false );
	return ConstructContractGameBid(10, SuitNoTrump);
}

Drop PrefEngineImpl::selectDropFromCandidates( const vector<CandidateDrop>& candidates ) const
{
	// TODO: add randomness
	float max = -0.01f;
	Drop result;
	for( int i = 0; i < candidates.size(); i++ ) {
		if( candidates[i].Weight > max ) {
			max = candidates[i].Weight;
			result.Cards = candidates[i].Cards;
		}
	}
	vector<CandidateDrop> tmp;
	for( int i = 0; i < candidates.size(); i++ ) {
		if( candidates[i].Weight * 2.3 > max ) {
			tmp.push_back(candidates[i]);
		}
	}
	result.Cards = tmp[rand()%tmp.size()].Cards;
	return result;
}

Drop PrefEngineImpl::doDrop() const
{
	CardsSet hand = AddCardsToSet(model->GetPlayerCards(model->CurrentPlayer()), model->Widow());
	if( model->GetPlayerBid(model->CurrentPlayer()) == Bid_Misere ) {
		Drop result = doMisereDrop(hand, GetPossibleDrops(hand, false));
		result.Contract = Bid_Misere;
		return result;
	} else {
		Drop result = doContractGameDrop(hand, GetPossibleDrops(hand, true));
		result.Contract = chooseContractDeal(result.Cards);
		return result;
	}
}

