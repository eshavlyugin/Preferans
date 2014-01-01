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

#include <DealState.h>
#include <Errors.h>

DealState::DealState() {
	clean();
}

void DealState::Clean()
{
	clean();
}

DealStateType DealState::CurrentStateType() const
{
	return stateType;
}

void DealState::UpdateOnBid(BidType bid)
{
	CheckError( IsValidBid(bid), "Invalid bid" );
	switch( stateType ) {
		case GST_Bidding:
			processGameBidding(bid);
			break;
		case GST_Whisting:
			processWhistBidding(bid);
			break;
		case GST_OpenOrCloseWhist:
			processOpenOrCloseWhistBidding(bid);
			break;
		default:
			PrefAssert(false);
	}
}

void DealState::UpdateOnDrop(const Drop& drop)
{
	CheckError( isValidDrop(drop), "Invalid drop" );
	maxBid = drop.Contract;
	playersBids[layout.CurrentPlayer] = drop.Contract;

	droppedCards = drop.Cards;
	if( layout.Cards[layout.CurrentPlayer] != EmptyCardsSet ) {
		layout.Cards[layout.CurrentPlayer] = RemoveCardsFromSet( layout.Cards[layout.CurrentPlayer], drop.Cards );
	}
	// Misere case
	if( playersBids[layout.CurrentPlayer] == Bid_Misere ) {
		layout.CurrentPlayer = firstPlayer;
		stateType = GST_MisereCatcherVote;
		return;
	}
	
	layout.Trump = GetContractTrump(drop.Contract);
	layout.CurrentPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
	playersBids[layout.CurrentPlayer] = Bid_Unknown;
	playersBids[(layout.CurrentPlayer + 1) % NumOfPlayers] = Bid_Unknown;
	stateType = GST_Whisting;
}

bool DealState::IsValidBid(BidType bid) const 
{
	switch( stateType ) {
		case GST_Bidding:
			return isValidGameBid(bid);
		case GST_Whisting:
			return isValidWhistBid(bid);
		case GST_OpenOrCloseWhist:
			return bid == Bid_OpenWhist || bid == Bid_CloseWhist;
		default:
			return false;
	}
}

bool DealState::IsValidMove(Card move) const 
{
	return isValidMove(move);
}

bool DealState::IsValidDrop(const Drop& drop) const
{
	return isValidDrop(drop);
}

void DealState::UpdateOnNewLayout(const Layout& _layout, int player)
{
	clean();
	layout = _layout;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		layout.Desk[i] = UnknownCard;
	}
	layout.CurrentPlayer = player;
	layout.MovesRemaining = 10;
	stateType = GST_Bidding;
	layout.CurrentPlayer = player;
	firstPlayer = player;
}

void DealState::ProcessMisereCatcherVoteFinished()
{
	stateType = GST_Misere;
}

void DealState::UpdateOnMove(Card move)
{
	PrefAssert( stateType == GST_Misere || stateType == GST_Passout || stateType == GST_ContractGame );
	PrefAssert( move != 0 );
	CheckError(isValidMove(move), "Invalid move");
	layout.UpdateOnMove(move);
	if( layout.IsFinished() ) {
		stateType = GST_Finished;
	}
	if( layout.NumCardsOnDesk == 0 ) {
		for( int i = 0; i < NumOfPlayers; i++ ) {
			layout.Desk[i] = UnknownCard;
		}
		if( layout.MovesRemaining >= 8 
			&& stateType == GST_Passout
			&& settings.Rules != RT_Rostov ) 
		{
			layout.CurrentPlayer = firstPlayer;
		}
	}
}

bool DealState::IsDarkContractGame() const
{
	if( maxBid < Bid_6s || maxBid > Bid_10nt ) {
		return false;
	}
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( playersBids[i] == Bid_OpenWhist ) {
			return false;
		}
	}
	return true;
}

void DealState::UpdateOnHandOpen(CardsSet hand, int player)
{
	layout.Cards[player] = RemoveCardsFromSet(hand, CardsSetsIntersection(hand, layout.UtilizedCards));
}

void DealState::processOpenOrCloseWhistBidding(BidType bid) 
{
	CheckError(bid == Bid_OpenWhist || bid == Bid_CloseWhist, "Invalid bid");
	playersBids[layout.CurrentPlayer] = bid;
	stateType = GST_ContractGame;
	GetLog() << "Contract game started..." << std::endl;
	layout.CurrentPlayer = firstPlayer;
}

void DealState::processGameBidding(BidType bid)
{
	CheckError( isValidGameBid(bid), "Invalid bid" );
	playersBids[layout.CurrentPlayer] = bid;
	if( bid > maxBid ) {
		maxBid = bid;
	}
	int nOfPasses = 0;
	int nOfUnknowns = 0;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( playersBids[i] == Bid_Pass ) {
			nOfPasses++;
		} else if( playersBids[i] == Bid_Unknown ) {
			nOfUnknowns++;
		}
	}
	if( nOfPasses != NumOfPlayers ) {
		layout.CurrentPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
		while( playersBids[layout.CurrentPlayer] == Bid_Pass ) {
			layout.CurrentPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
		}
	}
	if( nOfUnknowns == 0 ) {
		if( nOfPasses == NumOfPlayers ) {
			// all players had passed
			layout.CurrentPlayer = firstPlayer;
			stateType = GST_Passout;
			return;
		} else if( nOfPasses == NumOfPlayers - 1 ) {
			dealer = layout.CurrentPlayer;
			stateType = GST_Drop;
		}
	} 
}

void DealState::processWhistBidding(BidType bid)
{
	CheckError(isValidWhistBid(bid), "Invalid bid");
	playersBids[layout.CurrentPlayer] = bid;
	
	int nOfWhists = 0;
	int nOfPasses = 0;
	int nOfHalfWhists = 0;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( playersBids[i] == Bid_Whist ) {
			nOfWhists++;
		} else if( playersBids[i] == Bid_Pass ) {
			nOfPasses++;
		} else if( playersBids[i] == Bid_HalfWhist ) {
			nOfHalfWhists++;
		}
	}

	int nextPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
	if( nextPlayer == dealer ) {
		nextPlayer = (nextPlayer + 1) % NumOfPlayers;
	}
	
	if( nOfPasses + nOfHalfWhists + nOfWhists < 2 ) {
		layout.CurrentPlayer = nextPlayer;
		return;
	} 
		
	// Pass - HalfWhist - Something
	if( nOfPasses == 0 && nOfHalfWhists == 1 ) {
		if( bid == Bid_HalfWhist ) {
			return;
		}
		if( bid == Bid_Whist ) {
			stateType = GST_OpenOrCloseWhist;
			while( playersBids[layout.CurrentPlayer] != Bid_Whist ) {
				layout.CurrentPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
			}
		} else {
			stateType = GST_Finished;
		}
		return;
	}
	
	// two passes
	if( nOfPasses == 2 ) {
		stateType = GST_Finished;
		return;
	}

	// two whists
	if( nOfWhists == 2 ) {
		layout.CurrentPlayer = firstPlayer;
		stateType = GST_ContractGame;
		return;
	}
	
	// one whist - one pass (or half-whist)
	if( nOfWhists == 1 ) {
		GetLog() << "Whist-pass" << std::endl;
		while( playersBids[layout.CurrentPlayer] != Bid_Whist ) {
			layout.CurrentPlayer = (layout.CurrentPlayer + 1) % NumOfPlayers;
		}
		stateType = GST_OpenOrCloseWhist;
		return;
	}
}

Suit DealState::MoveSuit() const
{
	return layout.MoveSuit;
}

static int distFromFirstPlayer(int player, int firstPlayer) 
{
	return (player - firstPlayer + NumOfPlayers) % NumOfPlayers;
}

bool DealState::isValidGameBid(BidType bid) const
{
	// index of max bid player != current player. If more than 1 player has maxBid choose the first from firstPlayer
	int maxBidIndex = -1;
	for( int i = firstPlayer, count = 0; 
		count < NumOfPlayers; 
		i = (i + 1) % NumOfPlayers, count++ ) 
	{
		if( i == layout.CurrentPlayer ) {
			continue;
		}
		if( playersBids[i] == maxBid ) {
			maxBidIndex = i;
		}
	}
	if( bid == Bid_HalfWhist || bid == Bid_OpenWhist || bid == Bid_CloseWhist || bid == Bid_Whist ) {
		return false;
	}
	if( playersBids[layout.CurrentPlayer] == Bid_Misere ) {
		return bid == Bid_Pass;
	}

	if( bid == maxBid ) {
		if( bid == Bid_Misere ) {
			return false;
		}
		if( bid != Bid_Pass 
			&& distFromFirstPlayer(layout.CurrentPlayer, firstPlayer) > distFromFirstPlayer(maxBidIndex, firstPlayer) 
			&& maxBidIndex != -1 ) 
		{
			return false;
		}
	}

	if( bid != Bid_Pass && bid < maxBid ) {
		return false;
	}
	
	if( bid == Bid_Misere && playersBids[layout.CurrentPlayer] != Bid_Unknown ) {
		return false;
	}
	return true;
}

bool DealState::isValidWhistBid(BidType bid) const 
{
	int nWhists = 0;
	int nPasses = 0;
	for( int player = 0; player < NumOfPlayers; player++ ) {
		if( playersBids[player] == Bid_Pass ) {
			nPasses++;
		} else if( playersBids[player] == Bid_Whist ) {
			nWhists++;
		}
	}
	if( GetContractGameDeal( maxBid ) < 8 && nPasses == 1 && nWhists == 0 ) {
		return bid == Bid_Whist || bid == Bid_HalfWhist;
	}
	return bid == Bid_Pass || bid == Bid_Whist;
}

void DealState::UpdateOnPassoutWidowCardOpen(Card widowCard)
{
	layout.MoveSuit = GetCardSuit(widowCard);
	layout.Widow = AddCardToSet(layout.Widow, widowCard);
	if( layout.MovesRemaining == 10 ) {
		firstWidowCard = widowCard;
	} else if( layout.MovesRemaining == 9 ) {
		secondWidowCard = widowCard;
	} else {
		PrefAssert( false );
	}
}

void DealState::UpdateOnWidowOpen(CardsSet _widow)
{
	if( layout.Widow == EmptyCardsSet ) {
		int widowCardsCount = 0;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) {
			for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) {
				Card card = CreateCard(itSuit.GetObject(), itRank.GetObject());
				if( IsSetContainsCard(_widow, card) ) {
					if( widowCardsCount == 0 ) {
						firstWidowCard = card;
					} else if( widowCardsCount == 1 ) {
						secondWidowCard = card;
					} else {
						// Widow conains more than 2 cards
						PrefAssert( false );
					}
					widowCardsCount++;
				}
			}
		}
	}
	layout.Widow = _widow;
	if( layout.Cards[dealer] != EmptyCardsSet ) {
		PrefAssert( CardsSetsIntersection(layout.Cards[dealer], layout.Widow) == EmptyCardsSet );
		layout.Cards[dealer] = AddCardsToSet(layout.Cards[dealer], layout.Widow );
	}
}

bool DealState::isValidMove(Card move) const
{
	if( stateType != GST_Passout
		&& stateType != GST_Misere
		&& stateType != GST_ContractGame )
	{
		return false;
	}
	// unknown hand. Assume opponent don't cheating
	if( layout.Cards[layout.CurrentPlayer] == EmptyCardsSet ) {
		return !IsSetContainsCard(layout.UtilizedCards, move);
	}
	if( !IsSetContainsCard(layout.Cards[layout.CurrentPlayer], move) ) {
		return false;
	}
	if( layout.MoveSuit != SuitNoTrump && layout.MoveSuit != GetCardSuit(move) ) {
		if( HasCardsOfSuit(layout.Cards[layout.CurrentPlayer], layout.MoveSuit) ) {
			return false;
		}
		if( layout.Trump != SuitNoTrump 
			&& HasCardsOfSuit(layout.Cards[layout.CurrentPlayer], layout.Trump)
			&& GetCardSuit(move) != layout.Trump )
		{
			return false;
		}
	}
	return true;
}

bool DealState::isValidDrop(const Drop& drop) const
{
	if( stateType != GST_Drop ) {
		return false;
	}
	if( layout.Cards[dealer] != EmptyCardsSet 
		&& CardsSetsIntersection(layout.Cards[dealer], drop.Cards ) != drop.Cards )
	{
		return false;
	}
	if( maxBid == Bid_Misere ) {
		return drop.Contract == Bid_Misere;
	}

	return drop.Contract >= maxBid 
		&& drop.Contract != Bid_Misere
		&& drop.Contract != Bid_HalfWhist
		&& drop.Contract != Bid_Whist
		&& drop.Contract != Bid_OpenWhist
		&& drop.Contract != Bid_CloseWhist;
}

void DealState::clean()
{
	stateType = GST_Unknown;
	maxBid = Bid_Pass;
	dealer = 0;
	droppedCards = EmptyCardsSet;
	firstWidowCard = UnknownCard;
	secondWidowCard = UnknownCard;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		playersBids[i] = Bid_Unknown;
	}
}

vector<int> DealState::Tricks() const
{
	return vector<int>(&layout.Tricks[0], &layout.Tricks[0] + sizeof(layout.Tricks));	
}

vector<BidType> DealState::Bids() const
{
	return vector<BidType>(&playersBids[0], &playersBids[0] + sizeof(playersBids));
}

vector<CardsSet> DealState::Hands() const
{
	return vector<CardsSet>(&layout.Cards[0], &layout.Cards[0] + sizeof(layout.Cards));
}

vector<Card> DealState::CardsOnDesk() const
{
	return vector<Card>(&layout.Desk[0], &layout.Desk[0] + sizeof(layout.Desk));
}


