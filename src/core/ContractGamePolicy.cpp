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

#include <ContractGamePolicy.h>
#include <Errors.h>

void ContractGamePolicy::GenerateMoves(const Layout& layout, vector<Card>& moves)
{
	if( layout.NumCardsOnDesk == 0 ) {
		generateMovesForFirstPlayer(layout, moves);
	} else if( layout.NumCardsOnDesk == 1 ) {
		generateMovesForSecondPlayer(layout, moves);
	} else {
		generateMovesForThirdPlayer(layout, moves);
	}
	if( moves.size() == 0 ) {
		GetLog() << "Hand: " << layout.Cards[layout.CurrentPlayer].Value << endl;
		PrefAssert( false );
	}
	PrefAssert( moves.size() != 0 );
}

void ContractGamePolicy::generateMovesForFirstPlayer(const Layout& layout, vector<Card>& moves) 
{
	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];
	bool suitMarks[NumOfSuits];
	memset(suitMarks, 0, sizeof(suitMarks));
	for( RankBackwardIterator itRank; itRank.HasNext(); itRank.Next() ) {
		for( SuitBackwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			if( !suitMarks[itSuit.GetObject().Value] && IsSetContainsCard( currentSet, CreateCard(itSuit.GetObject(), itRank.GetObject()) ) ) {
				moves.push_back( CreateCard(itSuit.GetObject(), itRank.GetObject()) );
				suitMarks[itSuit.GetObject().Value] = true;
			}
		}
	}
	if( isCheckingMode && layout.CurrentPlayer == layout.Dealer ) {
		return;
	}
	memset(suitMarks, 0, sizeof(suitMarks));
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) {
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) {
			if( !suitMarks[itSuit.GetObject().Value] && IsSetContainsCard( currentSet, CreateCard(itSuit.GetObject(), itRank.GetObject()) ) ) {
				moves.push_back( CreateCard(itSuit.GetObject(), itRank.GetObject()) );
				suitMarks[itSuit.GetObject().Value] = true;
			}
		}
	}
}

void ContractGamePolicy::generateMovesForSecondPlayer(const Layout& layout, vector<Card>& moves) 
{
	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];
	CardsSet utilizedCards = AddCardToSet(layout.UtilizedCards, layout.MaxCard);
	if( !HasCardsOfSuit(currentSet, layout.MoveSuit) ) {
		if( HasCardsOfSuit(currentSet, layout.Trump) ) {
			// If we have trump
			for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
				Card card = CreateCard( layout.Trump, itRank.GetObject() );
				if( IsSetContainsCard( currentSet, card ) ) {
					moves.push_back( card );
				}
			}
		} else {
			// If we don't have trump we're dropping card
			array<bool, NumOfSuits> suitMarks;
			fill(suitMarks.begin(), suitMarks.end(), false);
			for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
				for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) {
					if( suitMarks[itSuit.GetObject().Value] ) {
						continue;
					}
					Card card = CreateCard( itSuit.GetObject(), itRank.GetObject() );
					if( IsSetContainsCard( currentSet, card ) ) {
						moves.push_back(card);
						suitMarks[itSuit.GetObject().Value] = true;
					}
				}
			}
		}
		return;
	}

	for( RankBackwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		Card card = CreateCard( layout.MoveSuit, itRank.GetObject() );
		if( IsSetContainsCard(currentSet, card) ) {
			moves.push_back(card);
		}
	}
}

void ContractGamePolicy::generateMovesForThirdPlayer(const Layout& layout, vector<Card>& moves) 
{
	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];
	// Same as the second
	if( !HasCardsOfSuit( currentSet, layout.MoveSuit ) ) {
		generateMovesForSecondPlayer(layout, moves);
	}

	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) {
		Card card = CreateCard(layout.MoveSuit, itRank.GetObject());
		if( IsSetContainsCard(currentSet, card) && IsGreaterCard(card, layout.MaxCard, layout.MoveSuit, layout.Trump) ) {
			moves.push_back(card);
			break;
		}
	}

	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		Card card = CreateCard(layout.MoveSuit, itRank.GetObject());
		if( IsSetContainsCard(currentSet, card) && !IsGreaterCard(card, layout.MaxCard, layout.MoveSuit, layout.Trump) ) {
			moves.push_back(card);
			break;
		}
	}

	return;
}

