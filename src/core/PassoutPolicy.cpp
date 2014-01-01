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

#include <PassoutPolicy.h>
#include <EngineData.h>
#include <Errors.h>

Card PassoutPolicy::NextMove(const Layout& layout) const
{
	switch( layout.NumCardsOnDesk ) {
		case 0:
			return makeMove0(layout);
			break;
		case 1:
			return makeMove1(layout);
			break;
		case 2:
			return makeMove2(layout);
			break;
		default:
			PrefAssert( false );
	}
	return 0;
}

// Lowest probability for valid move
const float LowestProbability = 0.000001;
const float eps = 1e-6f;

Card PassoutPolicy::makeMove0(const Layout& layout) const
{
	PrefAssert( data != 0 );
	int player = layout.CurrentPlayer;
	if( !HasCardsOfSuit( layout.Cards[player], layout.MoveSuit ) ) {
		return makeDrop( layout );
	}
	array<float, NumOfSuits> suitWeights;
	data->GetPassoutFirstMoveWeights(layout.Cards[player], layout.UtilizedCards,
		suitWeights);
	// Calculating sum of weights
	float sum = 0.0f;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		if( (layout.MoveSuit == itSuit.GetObject() || layout.MoveSuit == SuitNoTrump)
			&& GetCardsOfSuit( layout.Cards[player], itSuit.GetObject() ) != EmptyRanksSet )
		{
			if( suitWeights[itSuit.GetObject().Value] < eps ) {
				suitWeights[itSuit.GetObject().Value] = eps;
			}
			sum += suitWeights[itSuit.GetObject().Value];
		} else {
			suitWeights[itSuit.GetObject().Value] = 0.0f;
		}
	}
	PrefAssert( sum > eps );
	// selecting random suit according to suit weights
	float sample = RandomNextFloat() * sum;
	Suit moveSuit = SuitFirst;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		moveSuit = itSuit.GetObject();
		if( layout.MoveSuit == itSuit.GetObject() || layout.MoveSuit == SuitNoTrump ) {
			sample -= suitWeights[moveSuit.Value];
			if( sample < eps ) {
				break;
			}
		}
	}
	PrefAssert( moveSuit != SuitInvalid );
	// selecting random card in suit
	array<int, NumOfRanks> freqs;
	data->PassoutGetFrequencies1Move(layout.UtilizedCards, layout.Cards[player],
		moveSuit, freqs);
	sum = accumulate(freqs.begin(), freqs.end(), 0.0f);
	sample = RandomNextFloat() * sum;
	Rank moveRank = RankFirst;
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		moveRank = itRank.GetObject();
		sample -= freqs[moveRank.Value];
		if( sample < eps ) {
			break;
		}
	}
	return CreateCard( moveSuit, moveRank );
}

Card PassoutPolicy::makeMove1(const Layout& layout) const
{
	if( !HasCardsOfSuit( layout.Cards[layout.CurrentPlayer], layout.MoveSuit ) ) {
		return makeDrop(layout);
	}

	Card maxCard = layout.MaxCard;
	Rank maxCardRank = GetCardRank(maxCard);
	Suit moveSuit = layout.MoveSuit;

	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];
	Frac<int> underFreq = data->PassoutUnderFreq2Move(layout.UtilizedCards, currentSet, maxCard);
	int sample = RandomNextInt(underFreq.Denom);
	if( sample <= underFreq.Num ) {
		Rank rank = GetMaxRank( GetCardsOfSuit(currentSet, moveSuit), maxCardRank );
		return CreateCard( moveSuit, rank );
	} else {
		array<int, NumOfRanks> freqs;
		data->PassoutGetFrequencies2Move(layout.UtilizedCards, layout.Cards[layout.CurrentPlayer], maxCard, freqs);
		int sum = 0;
		for( RankForwardIterator itRank(maxCardRank); itRank.HasNext(); itRank.Next() ) { 
			sum += freqs[itRank.GetObject().Value];
		}
		// Error during database building
		PrefAssert( sum != 0 );
		//PrefAssert( sum == underFreq.Denom - underFreq.Num );
		sample = RandomNextInt(sum);
		for( RankForwardIterator itRank(maxCardRank); itRank.HasNext(); itRank.Next() ) {
			if( sample < freqs[itRank.GetObject().Value] ) {
				return CreateCard( moveSuit, itRank.GetObject().Value );
			}
			sample -= freqs[itRank.GetObject().Value];
		}
		PrefAssert( false );
	}
	return UnknownCard;
}

Card PassoutPolicy::makeMove2(const Layout& layout) const
{
	if( !HasCardsOfSuit( layout.Cards[layout.CurrentPlayer], layout.MoveSuit ) ) {
		return makeDrop(layout);
	}

	Card maxCard = IsGreaterCard(layout.Desk[1], layout.Desk[0], layout.MoveSuit, SuitNoTrump) ? layout.Desk[1] : layout.Desk[0];
	Rank maxCardRank = GetCardRank(maxCard);
	Suit moveSuit = layout.MoveSuit;

	CardsSet currentSet = layout.Cards[layout.CurrentPlayer];
	// Generating outcome using frequencies in database. If we cover card we use card with maximum rank. Otherwise -
	// card with highest rank which doesn't cover cards on table
	Frac<int> underFreq = data->PassoutUnderFreq3Move(layout.UtilizedCards, layout.Cards[layout.CurrentPlayer], maxCard);
	int outcome = RandomNextInt(underFreq.Denom);

	if( outcome <= underFreq.Num ) {
		return CreateCard( moveSuit, GetMaxRank( GetCardsOfSuit( currentSet, moveSuit ), maxCardRank ) );
	} else {
		return CreateCard( moveSuit, GetMaxRank( GetCardsOfSuit( currentSet, moveSuit ), RankLast ) );
	}
}

Card PassoutPolicy::makeDrop(const Layout& layout) const
{
	CardsSet currentCards = layout.Cards[layout.CurrentPlayer];
	array<float, NumOfSuits> dropInfo;
	data->GetPassoutDropWeights(layout.UtilizedCards, currentCards, dropInfo);
	float sum = accumulate(dropInfo.begin(), dropInfo.end(), 0.0f);
	float sample = RandomNextFloat() * sum;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		sample -= dropInfo[itSuit.GetObject().Value];
		if( sample < eps ) {
			return CreateCard( itSuit.GetObject(), GetMaxRank( GetCardsOfSuit(currentCards, itSuit.GetObject()), RankLast ) );
		}
	}
	PrefAssert(false);
	return UnknownCard;
}

