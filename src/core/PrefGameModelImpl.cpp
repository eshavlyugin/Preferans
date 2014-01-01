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

#include <PrefGameModelImpl.h>
#include <Layout.h>
#include <Errors.h>

PrefGameModelImpl::PrefGameModelImpl() : callback(0), firstHand(0), isInProgress(false)
{
}

PrefGameModelImpl::~PrefGameModelImpl() 
{
}

void PrefGameModelImpl::SetCallback(PrefModelCallback* _callback) 
{ 
	callback = _callback; 
}

void PrefGameModelImpl::SetSettings(const GameSettings& _settings) 
{
	settings = _settings; 
}

const GameSettings& PrefGameModelImpl::GetSettings() const 
{ 
	return settings; 
}

bool PrefGameModelImpl::IsBulletFinished() const 
{ 
	return !isInProgress; 
}

BidType PrefGameModelImpl::MaxBid() const
{
	return dealState.CurrentBid();
}

void PrefGameModelImpl::StartNewBullet()
{
	bullet.Initialize(settings);
	dealState.SetGameSettings(settings);
	isInProgress = true;
}

bool PrefGameModelImpl::ProcessBid(BidType bid)
{
	if( !dealState.IsValidBid(bid) ) {
		return false;
	}
	prevState = dealState;
	DealStateType oldState = dealState.CurrentStateType();
	dealState.UpdateOnBid(bid);
	DealStateType newState = dealState.CurrentStateType();
	callback->ProcessPlayerBid(bid);
	if( newState != oldState ) {
		callback->ProcessDealStateChanged( oldState, newState );
		processDealStateChanged( oldState, newState );
	}
	callback->ProcessModelChanged();
	return true;
}

bool PrefGameModelImpl::ProcessDrop(const Drop& drop)
{
	if( !dealState.IsValidDrop(drop) ) {
		return false;
	}
	prevState = dealState;
	DealStateType oldState = dealState.CurrentStateType();
	dealState.UpdateOnDrop(drop);
	DealStateType newState = dealState.CurrentStateType();
	callback->ProcessPlayerDrop(drop.Cards, drop.Contract);
	if( newState != oldState ) {
		processDealStateChanged( oldState, newState );
		callback->ProcessDealStateChanged(oldState, newState);
	}
	callback->ProcessModelChanged();
	return true;
}

void PrefGameModelImpl::HandOverCards(int playerFrom, int playerTo) 
{
	PrefAssert( handPlayers[playerTo] == playerTo && handPlayers[playerFrom] == playerFrom );
	PrefAssert( dealState.CurrentStateType() == GST_MisereCatcherVote );
	handPlayers[playerFrom] = playerTo;
	dealState.ProcessMisereCatcherVoteFinished();
	processDealStateChanged( GST_MisereCatcherVote, GST_Misere );
	callback->ProcessModelChanged();
}

int PrefGameModelImpl::PlayerCardsOwner( int player ) const
{
	return handPlayers[player];
}
	
bool PrefGameModelImpl::ProcessMisereCatcherChoosed(int catcher)
{
	PrefAssert( catcher != dealState.Dealer() );
	dealState.ProcessMisereCatcherVoteFinished();
}

bool PrefGameModelImpl::ProcessNewLayout(CardsSet hand1, CardsSet hand2, CardsSet hand3, int _firstHand)
{
	Layout layout;
	layout.Cards[0] = hand1;
	layout.Cards[1] = hand2;
	layout.Cards[2] = hand3;
	emptyHands = vector< vector<bool> >(NumOfPlayers, vector<bool>(NumOfSuits));
	for( int i = 0; i < NumOfPlayers; i++ ) {
		handPlayers[i] = i;
	}
	int count = 0;
	CardsSet widow = FullCardsSet;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( CardsSetSize(layout.Cards[i]) == InitialCardsCount ) {
			widow = RemoveCardsFromSet(widow, layout.Cards[i]);
			count++;
		} else if( CardsSetSize(layout.Cards[i]) != 0 ) {
			return false;
		}
	}
	if( count != 3 && count != 1 ) {
		return false;
	}
	firstHand = _firstHand;
	if( count == 3 ) {
		layout.Widow = widow;
	}
	hasFullInformation = count == 3;
	GetLog() << "HasFullInformation: " << (count == 3) << endl;
	GetLog() << "New layout was started" << endl;
	dealState.UpdateOnNewLayout(layout, firstHand);
	if( hasFullInformation ) {
		// determining widow cards order. Significant in passout games
		int widowCardIndex = RandomNextInt(2) == 0;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
				Card card = CreateCard(itSuit.GetObject(), itRank.GetObject());
				if( IsSetContainsCard(layout.Widow, card ) ) {
					if( widowCardIndex == 0 ) {
						dealState.SetFirstWidowCard(card);
					} else {
						dealState.SetSecondWidowCard(card);
					}
					widowCardIndex = (widowCardIndex + 1) % 2;
				}
			}
		}
	}
	callback->ProcessNewLayoutStart();
	callback->ProcessModelChanged();
	return true;
}

bool PrefGameModelImpl::HasCardsOfSuit(int player, Suit suit) const
{
	return !emptyHands[player][suit.Value];
}

Suit PrefGameModelImpl::PrevMoveSuit() const
{
	return prevState.MoveSuit();
}

bool PrefGameModelImpl::ProcessMove(Card move)
{
	if( !dealState.IsValidMove(move) ) {
		return false;
	}
	prevState = dealState;
	DealStateType oldState = dealState.CurrentStateType();
	dealState.UpdateOnMove(move);
	if( prevState.MoveSuit() != SuitNoTrump && prevState.MoveSuit() != GetCardSuit(move) ) {
		emptyHands[prevState.CurrentPlayer()][prevState.MoveSuit().Value] = true;
	}
	DealStateType newState = dealState.CurrentStateType();
	callback->ProcessPlayerMove(move);
	if( newState != oldState ) {
		processDealStateChanged( oldState, newState );
		callback->ProcessDealStateChanged( oldState, newState );
	}
	if( !hasFullInformation ) {
		callback->ProcessModelChanged();
		return true;
	}
	if( dealState.CurrentStateType() == GST_Passout 
		&& dealState.MovesRemaining() == 9
		&& dealState.NumCardsOnDesk() == 0 )
	{
		dealState.UpdateOnPassoutWidowCardOpen( dealState.SecondWidowCard() );
		callback->ProcessPassoutWidowCardOpening( dealState.SecondWidowCard() );
	}
	if( dealState.CurrentStateType() == GST_ContractGame
		&& settings.FirstMoveInDark 
		&& !dealState.IsDarkContractGame()
		&& dealState.NumCardsOnDesk() == 1
		&& dealState.MovesRemaining() == 10 )
	{
		for( int i = 0; i < NumOfPlayers; i++ ) {
			if( i != dealState.Dealer() ) {
				callback->ProcessHandOpening( dealState.Hands()[i], i );
			}
		}
	}
	callback->ProcessModelChanged();
	return true;
}

bool PrefGameModelImpl::ProcessWidowOpening(CardsSet widow)
{
	dealState.UpdateOnWidowOpen(widow);
	callback->ProcessWidowOpening(widow);
	return true;
}

int PrefGameModelImpl::PrevPlayer() const
{
	return prevState.CurrentPlayer();
}

int PrefGameModelImpl::GetPlayerTricks(int player) const
{
	return dealState.Tricks()[player];
}

bool PrefGameModelImpl::ProcessPassoutWidowCardOpening(Card card)
{
	dealState.UpdateOnPassoutWidowCardOpen(card);
	callback->ProcessPassoutWidowCardOpening(card);
	return true;
}

bool PrefGameModelImpl::ProcessHandOpening(CardsSet hand, int player)
{
	dealState.UpdateOnHandOpen(hand, player);
	callback->ProcessHandOpening(hand, player);
	return true;
}

int PrefGameModelImpl::GetPlayerCardsCount(int player) const
{
	int count = dealState.MovesRemaining();
	if( dealState.CardsOnDesk()[player] != UnknownCard ) {
		count--;
	}
	return count;
}

int PrefGameModelImpl::FirstHand() const
{
	return firstHand;
}

bool PrefGameModelImpl::IsValidBid(BidType bid) const
{
	return dealState.IsValidBid(bid);
}

bool PrefGameModelImpl::IsValidMove(Card move) const
{
	return dealState.IsValidMove(move);
}

bool PrefGameModelImpl::IsValidDrop(const Drop& drop) const
{
	return dealState.IsValidDrop(drop);
}

int PrefGameModelImpl::CurrentPlayer() const
{
	return dealState.CurrentPlayer();
}

int PrefGameModelImpl::Dealer() const
{
	return dealState.Dealer();
}

BidType PrefGameModelImpl::Contract() const
{
	return dealState.Contract();
}

CardsSet PrefGameModelImpl::Widow() const
{
	return dealState.Widow();
}

Card PrefGameModelImpl::FirstWidowCard() const
{
	return dealState.FirstWidowCard();
}

Card PrefGameModelImpl::SecondWidowCard() const
{
	return dealState.SecondWidowCard();
}

CardsSet PrefGameModelImpl::GetPlayerCards(int player) const
{
	return dealState.Hands()[player];
}

CardsSet PrefGameModelImpl::DroppedCards() const
{
	return dealState.DroppedCards();
}

Card PrefGameModelImpl::GetCardOnDesk(int player) const
{
	return dealState.CardsOnDesk()[player];
}

BidType PrefGameModelImpl::GetPlayerBid(int player) const
{
	return dealState.Bids()[player];
}

CardsSet PrefGameModelImpl::UtilizedCards() const
{
	return dealState.UtilizedCards();
}

DealStateType PrefGameModelImpl::GetDealStateType() const
{
	return dealState.CurrentStateType();
}

Suit PrefGameModelImpl::CurrentMoveSuit() const
{
	return dealState.MoveSuit();
}

Card PrefGameModelImpl::MaxCardOnDesk() const
{
	return dealState.MaxCardOnDesk();
}

int PrefGameModelImpl::DealMovesRemaining() const
{
	return dealState.MovesRemaining();
}

int PrefGameModelImpl::NumCardsOnDesk() const
{
	return dealState.NumCardsOnDesk();
}

void PrefGameModelImpl::handOverPasserHand()
{
	PrefAssert( dealState.CurrentStateType() == GST_ContractGame );
	int passer = -1;
	int whister = -1;
	for( int i = 0; i < 3; i++ ) {
		if( dealState.Bids()[i] == Bid_Pass ) {
			passer = i;
		} else if( dealState.Bids()[i] == Bid_Whist 
			|| dealState.Bids()[i] == Bid_OpenWhist
			|| dealState.Bids()[i] == Bid_CloseWhist ) 
		{
			whister = i;
		}
	}
	PrefAssert( whister != -1 && passer != -1 );
	handPlayers[passer] = whister;
}

void PrefGameModelImpl::processDealStateChanged(DealStateType oldState, DealStateType newState)
{
	if( !hasFullInformation && newState != GST_Finished ) {
		return;
	}
	switch( newState ) {
		case GST_ContractGame:
		{
			if( dealState.Bids()[prevState.CurrentPlayer()] == Bid_OpenWhist ) {
				handOverPasserHand();
			}
			if( !settings.FirstMoveInDark && !dealState.IsDarkContractGame() ) {
				for( int i = 0; i < NumOfPlayers; i++ ) {
					if( dealState.Dealer() != i ) {
						callback->ProcessHandOpening( dealState.Hands()[i], i );
					}
				}
			}
			break;
		}
		case GST_Misere:
		{
			for( int i = 0; i < NumOfPlayers; i++ ) {
				if( dealState.Dealer() != i ) {
					callback->ProcessHandOpening( dealState.Hands()[i], i );
				}
			}
			break;
		}
		case GST_Passout:
		{
			if( settings.Rules != RT_Rostov ) {
				dealState.UpdateOnPassoutWidowCardOpen( dealState.FirstWidowCard() );
				callback->ProcessPassoutWidowCardOpening( dealState.FirstWidowCard() );
			}
			break;
		}
		case GST_Drop:
		{
			dealState.UpdateOnWidowOpen( dealState.Widow() );
			callback->ProcessWidowOpening( dealState.Widow() );
			break;
		}
		case GST_Finished:
		{
			DealResult dealResult;
			for( int i = 0; i < NumOfPlayers; i++ ) {
				dealResult.Bids[i] = dealState.Bids()[i];
				dealResult.Tricks[i] = dealState.Tricks()[i];
			}
			bullet.UpdateScores(dealResult);
			break;
		}
	}
	if( bullet.IsFinished() ) {
		isInProgress = false;
	}
}

int PrefGameModelImpl::GetWhists(int playerFrom, int playerTo) const
{
	return bullet.GetScores().Whists[playerFrom][playerTo];
}

int PrefGameModelImpl::GetMountain(int player) const
{
	return bullet.GetScores().Mountain[player];
}

int PrefGameModelImpl::GetBullet(int player) const
{
	return bullet.GetScores().Bullet[player];
}


vector<float> PrefGameModelImpl::EvaluateDealResult(const DealResult& deal) const 
{
	return bullet.EvaluateDeal(deal);
}

