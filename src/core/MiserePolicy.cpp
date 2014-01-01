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

#include <MiserePolicy.h>

void MiserePolicy::GenerateMoves(const Layout& layout, vector<Card>& moves)
{
	switch( layout.NumCardsOnDesk ) {
		case 0:
			generateMovesForFirstPlayer(layout, moves);
			break;
		case 1:
			generateMovesForSecondPlayer(layout, moves);
			break;
		case 2:
			generateMovesForThirdPlayer(layout, moves);
			break;
	}
}

void MiserePolicy::generateMovesForFirstPlayer(const Layout& layout, vector<Card>& result)
{
	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];

	array<bool, NumOfSuits> suitMarks;
	fill(suitMarks.begin(), suitMarks.end(), false);
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			Card card = CreateCard(itSuit.GetObject(), itRank.GetObject());
			if( !suitMarks[itSuit.GetObject().Value] && IsSetContainsCard( currentSet, card ) ) {
				result.push_back( card );
				suitMarks[itSuit.GetObject().Value] = true;
			}
		}
	}
	fill(suitMarks.begin(), suitMarks.end(), false);
	for( RankBackwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
			Card card = CreateCard( itSuit.GetObject(), itRank.GetObject() );
			if( !suitMarks[itSuit.GetObject().Value] && IsSetContainsCard( currentSet, card ) ) {
				result.push_back(card);
				suitMarks[itSuit.GetObject().Value] = true;
			}
		}
	}
}

void MiserePolicy::generateMovesForSecondPlayer(const Layout& layout, vector<Card>& result)
{
	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];

	if( !HasCardsOfSuit(currentSet, layout.MoveSuit) ) {
		bool suitMarks[NumOfSuits];
		memset(suitMarks, 0, sizeof(suitMarks));
		for( RankBackwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
			for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
				if( suitMarks[itSuit.GetObject().Value] ) {
					continue;
				}
				Card card = CreateCard( itSuit.GetObject(), itRank.GetObject() );
				if( IsSetContainsCard( currentSet, card ) ) {
					result.push_back( card );
					suitMarks[itSuit.GetObject().Value] = true;
				}
			}
		}
		return;
	}
	
	for( RankBackwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		Card card = CreateCard(layout.MoveSuit, itRank.GetObject());
		if( IsSetContainsCard(currentSet, card) && !IsGreaterCard(card, layout.MaxCard, layout.MoveSuit, layout.Trump)) {
			result.push_back(card);
			// In case of checking mode return after assume bidder always moves under
			if( isCheckingMode ) {
				break;
			}
		}
	}

	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
 		Card card = CreateCard(layout.MoveSuit, itRank.GetObject());
		if( IsSetContainsCard(currentSet, card) && IsGreaterCard(card, layout.MaxCard, layout.MoveSuit, layout.Trump) ) {
			result.push_back(card);
			// In case of checking mode break
			if( isCheckingMode ) {
				break;
			}
		}
	}
}

void MiserePolicy::generateMovesForThirdPlayer(const Layout& layout, vector<Card>& result)
{
	return generateMovesForSecondPlayer(layout, result);
}

