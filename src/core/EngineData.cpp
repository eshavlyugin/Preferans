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

#include <EngineData.h>
#include <PrefTools.h>
#include <Errors.h>

// Current version of data file
static const int CurrentVersion = 1;
// Maximum number of stored layouts for single deal
extern const int MaxDataLayouts = 4000;

EngineData::EngineData()
{
	memset(passoutFrequencies1Move, 0, sizeof(passoutFrequencies1Move));
	memset(passoutFrequencies2Move, 0, sizeof(passoutFrequencies2Move));
	memset(underFrequencies2Move, 0, sizeof(underFrequencies2Move));
	memset(underFrequencies3Move, 0, sizeof(underFrequencies3Move));
	memset(passoutDropWeights, 0, sizeof(passoutDropWeights));
	memset(passoutFirstMoveWeights, 0, sizeof(passoutFirstMoveWeights));
	memset(misereDropWeights, 0, sizeof(misereDropWeights));
	memset(contractDropWeights, 0, sizeof(contractDropWeights));
	layouts.resize(static_cast<int>(Bid_LastBid) + 1);
}


EngineData::~EngineData()
{
}

void EngineData::Load(istream& in)
{
	int version = 0;
	in.read(reinterpret_cast<char*>(&version), sizeof(int));
	CheckError(version == CurrentVersion, "Incompatible engine data version");
	in.read(reinterpret_cast<char*>(passoutFrequencies1Move), sizeof(passoutFrequencies1Move));
	in.read(reinterpret_cast<char*>(passoutFrequencies2Move), sizeof(passoutFrequencies2Move));
	in.read(reinterpret_cast<char*>(underFrequencies2Move), sizeof(underFrequencies2Move));
	in.read(reinterpret_cast<char*>(underFrequencies3Move), sizeof(underFrequencies3Move));
	in.read(reinterpret_cast<char*>(passoutDropWeights), sizeof(passoutDropWeights));
	in.read(reinterpret_cast<char*>(passoutFirstMoveWeights), sizeof(passoutFirstMoveWeights));
	in.read(reinterpret_cast<char*>(misereDropWeights), sizeof(misereDropWeights));
	in.read(reinterpret_cast<char*>(contractDropWeights), sizeof(contractDropWeights));
	layouts.clear();
	layouts.resize(static_cast<int>(Bid_LastBid) + 1);
	for( int i = 0; i < layouts.size(); i++ ) {
		int size = 0;
		in.read(reinterpret_cast<char*>(&size), sizeof(int));
		if( size > 0 ) {
			layouts[i].resize(size);
			in.read(reinterpret_cast<char*>(&layouts[i][0]), size * sizeof(layouts[i][0]));
		}
	}
}

void EngineData::Save(ostream& out)
{
	out.write(reinterpret_cast<const char*>(&CurrentVersion), sizeof(int));
	out.write(reinterpret_cast<const char*>(passoutFrequencies1Move), sizeof(passoutFrequencies1Move));
	out.write(reinterpret_cast<const char*>(passoutFrequencies2Move), sizeof(passoutFrequencies2Move));
	out.write(reinterpret_cast<const char*>(underFrequencies2Move), sizeof(underFrequencies2Move));
	out.write(reinterpret_cast<const char*>(underFrequencies3Move), sizeof(underFrequencies3Move));
	out.write(reinterpret_cast<const char*>(passoutDropWeights), sizeof(passoutDropWeights));
	out.write(reinterpret_cast<const char*>(passoutFirstMoveWeights), sizeof(passoutFirstMoveWeights));
	out.write(reinterpret_cast<const char*>(misereDropWeights), sizeof(misereDropWeights));
	out.write(reinterpret_cast<const char*>(contractDropWeights), sizeof(contractDropWeights));
	for( int i = 0; i < layouts.size(); i++ ) {
		int size = layouts[i].size();
		out.write(reinterpret_cast<const char*>(&size), sizeof(int));
		if( size > 0 ) {
			out.write(reinterpret_cast<const char*>(&layouts[i][0]), size * sizeof(layouts[i][0]));
		}
	}
}

void EngineData::AddPassoutMove1(CardsSet utilized, CardsSet hand, Card move)
{
	/*
	 * Saving information about distribution of chances to make given move
	 */
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	// Determining database indexes
	getPassoutIndexes(utilized, hand, move, counter, mask, moveIndex);
	PrefAssert( (mask & (1<<moveIndex)) != 0 );
	// Saving information
	passoutFrequencies1Move[counter][mask][moveIndex]++;
}

void EngineData::AddPassoutMove2(CardsSet utilized, CardsSet hand, Card move1, Card move2)
{
	// if hand doesn't have cards of move suit adding informaiton about drop
	Suit moveSuit = GetCardSuit( move1 );
	if( !HasCardsOfSuit(hand, moveSuit) ) {
		return;
	}
	/*
	 * Saving information about:
	 * 1. What chance for given layout player x moves under 1 card
	 * 2. If player x cover 1 card what distribution of chances to make move "X"
	 */

	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	// Determining database indexes
	CardsSet srcUtilized = RemoveCardFromSet(utilized, move1);
	CardsSet srcSet = AddCardToSet(hand, move1);
	getPassoutIndexes(srcUtilized, srcSet, move1, counter, mask, moveIndex);
	PrefAssert( (mask & (1<<moveIndex)) != 0 );
	// Adding information about chance for player 2 move under 1st card
	if( HasCardsOfSuit( hand, moveSuit ) ) {
		int takeMoveIndex = 0;
		getPassoutIndexes(srcUtilized, srcSet, move2, counter, mask, takeMoveIndex);
		passoutFrequencies2Move[counter - moveIndex][mask >> moveIndex][takeMoveIndex - moveIndex]++;
	}
	// Adding information about distribution of chances
	if( HasCardsOfSuit( hand, moveSuit ) ) {
		underFrequencies2Move[counter][mask][moveIndex].Denom++;
		if( !IsGreaterCard( move2, move1, moveSuit, SuitNoTrump ) ) {
			underFrequencies2Move[counter][mask][moveIndex].Num++;
		}
	}
}

void EngineData::AddLayout(BidType contract, CardsSet set)
{
	int index = static_cast<int>(contract);
	if( layouts[index].size() < MaxDataLayouts ) {
		layouts[index].push_back(set);
	}
}

const vector<CardsSet>& EngineData::GetLayouts(BidType contract) const
{
	int index = static_cast<int>(contract);
	return layouts[index];
}
	
void EngineData::AddPassoutMove3(CardsSet utilized, CardsSet hand, Card move1, Card move2, Card move3)
{
	// if hand doesn't have cards of move suit adding informaiton about drop
	Suit moveSuit = GetCardSuit( move1 );
	if( !HasCardsOfSuit(hand, moveSuit) ) {
		return;
	}
	/*
	 * Saving information about chance for player 3 move under 1st and 2nd cards
	 */
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	// Determining database indexes
	Card maxMove = IsGreaterCard( move2, move1, moveSuit, SuitNoTrump ) ? move2 : move1;
	CardsSet srcUtilized = RemoveCardFromSet( utilized, maxMove );
	CardsSet srcSet = AddCardToSet( hand, maxMove );
	getPassoutIndexes(srcUtilized, srcSet, maxMove, counter, mask, moveIndex);
	PrefAssert( (mask & (1<<moveIndex)) != 0 );
	// Saving information
	if( HasCardsOfSuit( hand, moveSuit ) ) {
		underFrequencies3Move[counter][mask][moveIndex].Denom++;
		if( !IsGreaterCard( move3, maxMove, moveSuit, SuitNoTrump ) ) {
			underFrequencies3Move[counter][mask][moveIndex].Num++;
		}
	}
}

void EngineData::PassoutGetFrequencies1Move(CardsSet utilized, CardsSet hand, 
	Suit suit, array<int, NumOfRanks>& result) const
{
	// assume result has size >= NumOfRanks
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	getPassoutIndexes(utilized, hand, CreateCard(suit, RankFirst), counter, mask, moveIndex);
	// current index of passoutFrequencies1Move array
	int curIndex = 1;
	RanksSet utilizedRanks = GetCardsOfSuit(utilized, suit);
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		if( IsSetContainsRank( utilizedRanks, itRank.GetObject() ) ) {
			result[itRank.GetObject().Value] = 0;
			continue;
		}
		// valid moves should have non-zero probability to be chosen
		result[itRank.GetObject().Value] = passoutFrequencies1Move[moveIndex][mask][curIndex] * 10 + 1;	
		curIndex++;
	}
}

void EngineData::PassoutGetFrequencies2Move(CardsSet utilized, CardsSet hand, 
	Card firstCard, array<int, NumOfRanks>& result) const
{
	CardsSet srcUtilized = RemoveCardFromSet(utilized, firstCard);
	CardsSet srcHand = AddCardToSet(hand, firstCard);
	// assume result has size >= NumOfRanks
	Suit suit = GetCardSuit(firstCard);
	Rank firstRank = GetCardRank(firstCard);
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	getPassoutIndexes(srcUtilized, srcHand, firstCard, counter, mask, moveIndex);
	fill(result.begin(), result.end(), 0);
	int curIndex = 0;
	RanksSet utilizedRanks = GetCardsOfSuit( utilized, suit );
	for( RankForwardIterator itRank(firstRank); itRank.HasNext(); itRank.Next() ) {
		if( IsSetContainsRank( utilizedRanks, itRank.GetObject() ) ) {
			continue;
		}
		// valid moves should have non-zero probability to be chosen
		result[itRank.GetObject().Value] = passoutFrequencies2Move[counter - moveIndex][mask >> moveIndex][curIndex] * 10 + 1;
		curIndex++;
	}
}

Frac<int> EngineData::PassoutUnderFreq2Move(CardsSet utilized, CardsSet hand, Card firstMove) const
{
	CardsSet srcUtilized = RemoveCardFromSet(utilized, firstMove);
	CardsSet srcHand = AddCardToSet(hand, firstMove);
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	getPassoutIndexes(srcUtilized, srcHand, firstMove, counter, mask, moveIndex);
	Frac<int> res = underFrequencies2Move[counter][mask][moveIndex];
	if( res.Denom == 0 ) {
		res.Num = 1;
		res.Denom = 2;
	}
	return res;
}

Frac<int> EngineData::PassoutUnderFreq3Move(CardsSet utilized, CardsSet hand, Card maxCard) const
{
	CardsSet srcUtilized = RemoveCardFromSet(utilized, maxCard);
	CardsSet srcHand = AddCardToSet(hand, maxCard);
	int moveIndex = 0;
	int mask = 0;
	int counter = 0;
	getPassoutIndexes(srcUtilized, srcHand, maxCard, counter, mask, moveIndex);
	Frac<int> res = underFrequencies3Move[counter][mask][moveIndex];
	if( res.Denom == 0 ) {
		res.Num = 1;
		res.Denom = 2;
	}
	return res;
}

void EngineData::GetPassoutFirstMoveWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const
{
	static const float MinPassoutMoveWeight = 1e-5f;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		int index = GetRanksSetIndex(GetCardsOfSuit(hand, itSuit.GetObject()), GetCardsOfSuit(utilized, itSuit.GetObject()));
		result[itSuit.GetObject().Value] = passoutFirstMoveWeights[index];
		if( result[itSuit.GetObject().Value] < MinPassoutMoveWeight && HasCardsOfSuit( hand, itSuit.GetObject() ) ) {
			result[itSuit.GetObject().Value] = MinPassoutMoveWeight;
		}
	}
}

void EngineData::GetPassoutDropWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const
{
	static const float MinPassoutDropWeight = 1e-5f;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		int index = GetRanksSetIndex(GetCardsOfSuit(hand, itSuit.GetObject()), GetCardsOfSuit(utilized, itSuit.GetObject()));
		result[itSuit.GetObject().Value] = passoutDropWeights[index];
		if( result[itSuit.GetObject().Value] < MinPassoutDropWeight && HasCardsOfSuit( hand, itSuit.GetObject() ) ) {
			result[itSuit.GetObject().Value] = MinPassoutDropWeight;
		}
	}
}

const char Ranks[] = "789TJQKA";

static string maskToString(int mask) {
	string res = "";
	for( int i = 0; i < NumOfRanks; i++ ) {
		if( (mask & (1<<i)) != 0 ) {
			res = res + Ranks[i];
		}
	}
	return res;
}

void EngineData::DumpPassoutDB() const
{
	// TODO: not implemented
}

void EngineData::SetPassoutDropWeights(const vector<float>& weights)
{
	PrefAssert(weights.size() == sizeof(passoutDropWeights) / sizeof(passoutDropWeights[0]));
	memcpy(passoutDropWeights, &weights[0], sizeof(passoutDropWeights));
}

void EngineData::SetPassoutFirstMoveWeights(const vector<float>& weights)
{
	PrefAssert(weights.size() == sizeof(passoutFirstMoveWeights) / sizeof(passoutFirstMoveWeights[0]));
	memcpy(passoutFirstMoveWeights, &weights[0], sizeof(passoutFirstMoveWeights));
}

void EngineData::SetMisereDropWeights(const vector<float>& weights)
{
	PrefAssert( weights.size() == sizeof( misereDropWeights ) / sizeof( misereDropWeights[0] ) );
	memcpy( misereDropWeights, &weights[0], sizeof( misereDropWeights ) );
}

void EngineData::GetMisereDropWeights(CardsSet hand, CardsSet utilized, array<float, 4>& result) const
{
	static const float MinMisereDropWeight = 1e-5f;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		int index = GetRanksSetIndex(GetCardsOfSuit(hand, itSuit.GetObject()), GetCardsOfSuit(utilized, itSuit.GetObject()));
		result[itSuit.GetObject().Value] = misereDropWeights[index];
		if( result[itSuit.GetObject().Value] < MinMisereDropWeight && HasCardsOfSuit( hand, itSuit.GetObject() ) ) {
			result[itSuit.GetObject().Value] = MinMisereDropWeight;
		}
	}
}

void EngineData::SetContractDropWeights(const vector<float>& weights)
{
	PrefAssert( weights.size() == sizeof( contractDropWeights ) / sizeof( contractDropWeights[0] ) );
	memcpy( contractDropWeights, &weights[0], sizeof(contractDropWeights) );
}

void EngineData::GetContractDropWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const
{
	static const float MinMisereDropWeight = 1e-5f;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		int index = GetRanksSetIndex(GetCardsOfSuit(hand, itSuit.GetObject()), GetCardsOfSuit(utilized, itSuit.GetObject()));
		result[itSuit.GetObject().Value] = contractDropWeights[index];
		if( result[itSuit.GetObject().Value] < MinMisereDropWeight && HasCardsOfSuit(hand, itSuit.GetObject()) ) {
			result[itSuit.GetObject().Value] = MinMisereDropWeight;
		}
	}
}

void EngineData::getPassoutIndexes(CardsSet utilized, CardsSet hand, Card card, int& remainsCount, int& mask, int& moveIndex)
{
	Suit suit = GetCardSuit(card);
	RanksSet utilizedRanks = GetCardsOfSuit(utilized, suit);
	RanksSet handRanks = GetCardsOfSuit(hand, suit);
	Rank cardRank = GetCardRank(card);
	moveIndex = 0;
	mask = 0;
	remainsCount = 0;
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		if( IsSetContainsCard(hand, CreateCard(suit, itRank.GetObject())) ) {
			mask |= (1 << remainsCount);
		}
		if( itRank.GetObject() == cardRank ) {
			moveIndex = remainsCount;
		}
		if( !IsSetContainsCard(utilized, CreateCard(suit, itRank.GetObject())) ) {
			remainsCount++;
		}
	}
}	

