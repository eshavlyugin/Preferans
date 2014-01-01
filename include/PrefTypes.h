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

#ifndef _PREF_TYPES_H__
#define _PREF_TYPES_H__

#include <ostream>
#include "Errors.h"

namespace Preference {

// Set of card ranks
struct RanksSet {
	unsigned int Value;

	RanksSet(unsigned int value) : Value(value) {}
};

static const RanksSet EmptyRanksSet(0);
static const RanksSet FullRanksSet(0xff);

struct RanksSubsetIterator {
	RanksSet InitialSet;
	RanksSet Subset;
	bool __hasNext;

	RanksSubsetIterator(const RanksSet& initialSet) : InitialSet(initialSet), Subset(initialSet), __hasNext(true) {}

	bool HasNext() { return __hasNext; }
	void Next() { __hasNext = Subset.Value != 0; Subset.Value = (InitialSet.Value & (Subset.Value - 1)); }
	const RanksSet& GetObject() const { return Subset; }
};

inline bool operator == (const RanksSet& set1, const RanksSet& set2)
{
	return set1.Value == set2.Value;
}

inline bool operator != (const RanksSet& set1, const RanksSet& set2)
{
	return set1.Value != set2.Value;
}

// Set of cards
struct CardsSet {
	unsigned int Value;

	CardsSet(unsigned int value) : Value(value) {}
	CardsSet() : Value(0) {}
};

inline bool operator == (const CardsSet& set1, const CardsSet& set2)
{
	return set1.Value == set2.Value;
}

inline bool operator != (const CardsSet& set1, const CardsSet& set2)
{
	return set1.Value != set2.Value;
}


static const CardsSet EmptyCardsSet(0);
static const CardsSet FullCardsSet(0xffffffff);

// Single card type 
struct Card {
	unsigned int Value;

	Card(unsigned int value) : Value(value) {}
	Card() : Value(0xffffffff) {}
};

inline bool operator == (const Card& card1, const Card& card2)
{
	return card1.Value == card2.Value;
}

inline bool operator != (const Card& card1, const Card& card2)
{
	return card1.Value != card2.Value;
}

static const Card UnknownCard(0xffffffff);

// Card suit. Possible values are: 
// 0 - spades, 1 - clubs, 2 - diamonds, 3 - heart, 4 - no trump

struct Suit {
	unsigned int Value;

	Suit(int value) : Value(value) {}
	Suit() : Value(0) {}
};

inline bool operator == (const Suit& suit1, const Suit& suit2)
{
	return suit1.Value == suit2.Value;
}

inline bool operator != (const Suit& suit1, const Suit& suit2)
{
	return suit1.Value != suit2.Value;
}

static const Suit SuitSpades(0);
static const Suit SuitClubs(1);
static const Suit SuitDiamonds(2);
static const Suit SuitHearts(3);
static const Suit SuitNoTrump(4);
static const Suit SuitInvalid(0xffffffff);
static const Suit SuitFirst(0);
static const Suit SuitLast(3);

struct SuitForwardIterator {
	Suit Obj;

	SuitForwardIterator() : Obj(SuitFirst) {}
	bool HasNext() const { return Obj.Value != SuitInvalid.Value; }
	const Suit& GetObject() const { return Obj; }
	void Next() { Obj.Value = Obj.Value == SuitLast.Value ? SuitInvalid.Value : Obj.Value + 1; }
};

struct SuitBackwardIterator {
	Suit Obj;

	SuitBackwardIterator() : Obj(SuitLast) {}
	bool HasNext() const { return Obj.Value != SuitInvalid.Value; }
	const Suit& GetObject() const { return Obj; }
	void Next() { Obj.Value = Obj.Value == SuitFirst.Value ? SuitInvalid.Value : Obj.Value - 1; }
};

// Card rank. Possible values are:
// 0 - 7, 
// 1 - 8, 
// 2 - 9,
// 3 - 10
// 4 - J
// 5 - Q
// 6 - K
// 7 - A
struct Rank {
	int Value;

	Rank(int value) : Value(value) {}
};

inline bool operator == (const Rank& rank1, const Rank& rank2)
{
	return rank1.Value == rank2.Value;
}

inline bool operator != (const Rank& rank1, const Rank& rank2)
{
	return rank1.Value != rank2.Value;
}

static const Rank RankFirst(0);
static const Rank RankLast(7);
static const Rank RankInvalid(0xffffffff);

static const Rank Rank7(0);
static const Rank Rank8(1);
static const Rank Rank9(2);
static const Rank Rank10(3);
static const Rank RankJ(4);
static const Rank RankQ(5);
static const Rank RankK(6);
static const Rank RankA(7);

struct RankForwardIterator {
	Rank Obj;

	RankForwardIterator() : Obj(RankFirst) {}
	RankForwardIterator(const Rank& startFrom) : Obj(startFrom) {}
	bool HasNext() const { return Obj.Value != RankInvalid.Value; }
	const Rank& GetObject() const { return Obj; }
	void Next() { Obj.Value = Obj.Value == RankLast.Value ? RankInvalid.Value : Obj.Value + 1; }
};

struct RankBackwardIterator {
	Rank Obj;

	RankBackwardIterator() : Obj(RankLast) {}
	RankBackwardIterator(const Rank& startFrom) : Obj(startFrom) {}
	bool HasNext() const { return Obj.Value != RankInvalid.Value; }
	const Rank& GetObject() const { return Obj; }
	void Next() { Obj.Value = Obj.Value == RankFirst.Value ? RankInvalid.Value : Obj.Value - 1; }
};

struct CardForwardIterator {
	Card Obj;

	CardForwardIterator() : Obj(1) {}
	CardForwardIterator(const Card& startFrom) : Obj(startFrom) {}
	bool HasNext() const { return Obj.Value != UnknownCard.Value; }
	const Card& GetObject() const { return Obj; }
	void Next() { Obj.Value = (Obj.Value == (1 << 31)) ? Obj.Value << 1 : UnknownCard.Value; }
};

//----------------------Methods to handle ranks and suits----------------------
//-----------------------------------------------------------------------------
inline Rank GetCardRank(Card card)
{
	int mask = ((card.Value & 0xff) | 
	((card.Value & 0xff00) >> 8) | 
	((card.Value & 0xff0000) >> 16) | 
	((card.Value & 0xff000000) >> 24));
	switch( mask ) {
		case (1<<0):
			return Rank(0);
		case (1<<1):
			return Rank(1);
		case (1<<2):
			return Rank(2);
		case (1<<3):
			return Rank(3);
		case (1<<4):
			return Rank(4);
		case (1<<5):
			return Rank(5);
		case (1<<6):
			return Rank(6);
		case (1<<7):
			return Rank(7);
		default:
			return RankInvalid;
	}
}

inline Card CreateCard(Suit suit, Rank rank)
{
	return Card(1 << ((suit.Value << 3) + rank.Value));
}

inline Suit GetCardSuit(Card card)
{
	int high = (card.Value & 0x0000ffff) == 0 ? 2 : 0;
	int low = (card.Value & 0x00ff00ff) == 0 ? 1 : 0;
	return high | low;
}

inline bool HasCardsOfSuit(CardsSet cards, Suit suit)
{
	return (cards.Value & (0xff << (suit.Value << 3))) != 0;
}

inline RanksSet GetCardsOfSuit(CardsSet cards, Suit suit)
{
	return (cards.Value >> (suit.Value << 3)) & 0xff;
}

inline CardsSet CardsSetsIntersection( CardsSet set1, CardsSet set2 ) 
{
	return (set1.Value & set2.Value);
}

inline bool IsSetContainsRank(RanksSet set, Rank rank)
{
	return (set.Value & (1<<rank.Value)) != 0;
}

inline bool IsSubsetOfCards(CardsSet set, CardsSet subset)
{
	return (set.Value & subset.Value) == subset.Value;
}

inline bool IsSetContainsCard(CardsSet set, Card card)
{
	return (set.Value & card.Value) == card.Value;
}

inline CardsSet AddCardToSet(CardsSet set, Card card)
{
	return (set.Value | card.Value);
}

inline CardsSet AddCardsToSet(CardsSet set, CardsSet toAdd)
{
	return (set.Value | toAdd.Value);
}

inline int CardsSetSize(CardsSet set)
{
	// TODO: optimize
	int result = 0;
	for( int i = 0; i < 32; i++ ) {
		if( (set.Value & (1<<i)) != 0 ) {
			result++;
		}
	}
	return result;
}

inline int SuitSetSize(CardsSet set, Suit suit)
{
	int result = 0;
	for( RankForwardIterator it; it.HasNext(); it.Next() ) {
		if( IsSetContainsCard(set, CreateCard(suit, it.GetObject()) ) ) {
			result++;
		}
	}
	return result;
}

inline int RanksSetSize(RanksSet set)
{
	int res = 0;
	for( RankForwardIterator it; it.HasNext(); it.Next() ) {
		if( IsSetContainsRank(set, it.GetObject()) ) {
			res++;
		}
	}
	return res;
}

inline CardsSet CardsSetFromRanksSet(RanksSet set, Suit suit)
{
	return static_cast<CardsSet>(set.Value << (suit.Value << 3));
}

inline RanksSet GetRanksOfSuit(CardsSet set, Suit suit)
{
	return RanksSet((set.Value >> (suit.Value << 3)) & 0xff);
}

inline RanksSet RemoveRanksFromSet(RanksSet set1, RanksSet set2)
{
	return (set1.Value & ~set2.Value);
}

inline RanksSet RemoveRankFromSet(RanksSet set1, Rank rank)
{
	return (set1.Value & ~(1<<rank.Value));
}

inline RanksSet RanksSetsIntersection(RanksSet set1, RanksSet set2)
{
	return (set1.Value & set2.Value);
}

inline bool IsSetsHaveCommonRank(RanksSet set1, RanksSet set2)
{
	return (set1.Value & set2.Value) != 0;
}

inline CardsSet RemoveCardFromSet(CardsSet set, Card card)
{
	return (set.Value & ~card.Value);
}

inline CardsSet RemoveCardsFromSet(CardsSet set, CardsSet setToRemove)
{
	return (set.Value & ~setToRemove.Value);
}

inline bool IsGreaterCard(Card test, Card currentMax, Suit moveSuit, Suit trump)
{
	Suit testSuit = GetCardSuit( test );
	Suit currentSuit = GetCardSuit( currentMax );

	if( testSuit.Value == currentSuit.Value ) {
		return GetCardRank(test).Value >= GetCardRank(currentMax).Value;
	}
	return testSuit.Value == trump.Value;
}

inline Rank GetMaxRank(RanksSet set, Rank startFrom)
{
	for( RankBackwardIterator it(startFrom); it.HasNext(); it.Next() ) {
		if( IsSetContainsRank( set, it.GetObject() ) ) {
			return it.GetObject();
		}
	}
	return RankInvalid;
}

inline Rank GetMinRank(RanksSet set, Rank startFrom)
{
	for( RankForwardIterator it(startFrom); it.HasNext(); it.Next() ) {
		if( IsSetContainsRank( set, it.GetObject() ) ) {
			return it.GetObject();
		}
	}
	return RankInvalid;
}

//----------------------Constants-----------------------------------------------

static const int NumOfPlayers = 3;
static const int NumOfSuits = 4;
static const int NumOfRanks = 8;
static const int InitialCardsCount = 10;

//------------------------Contracts---------------------------------------------

enum BidType {
	Bid_Unknown = 0,
	Bid_FirstBid = 1,
	Bid_Pass = Bid_FirstBid,
	Bid_HalfWhist,
	Bid_Whist,
	Bid_OpenWhist,
	Bid_CloseWhist,
	Bid_FirstContractGame = 20,
	Bid_6s = Bid_FirstContractGame,
	Bid_6c,
	Bid_6d,
	Bid_6h,
	Bid_6nt,
	Bid_7s,
	Bid_7c,
	Bid_7d,
	Bid_7h,
	Bid_7nt,
	Bid_8s,
	Bid_8c,
	Bid_8d,
	Bid_8h,
	Bid_8nt,
	Bid_Misere,
	Bid_9s,
	Bid_9c,
	Bid_9d,
	Bid_9h,
	Bid_9nt,
	Bid_10s,
	Bid_10c,
	Bid_10d,
	Bid_10h,
	Bid_10nt,
	Bid_LastContractGame = Bid_10nt,
	Bid_LastBid = Bid_LastContractGame,
	Bid_Invalid // Special value for errors
};

inline BidType ConstructContractGameBid(int deal, Suit suit)
{
	switch( deal ) {
		case 6:
			return static_cast<BidType>(Bid_6s + suit.Value);
		case 7:
			return static_cast<BidType>(Bid_7s + suit.Value);
		case 8:
			return static_cast<BidType>(Bid_8s + suit.Value);
		case 9:
			return static_cast<BidType>(Bid_9s + suit.Value);
		case 10:
			return static_cast<BidType>(Bid_10s + suit.Value);
		default:
			return Bid_Unknown;
	}
}

inline int GetContractGameDeal(BidType bid)
{
	if( bid >= Bid_6s && bid <= Bid_6nt ) return 6;
	if( bid >= Bid_7s && bid <= Bid_7nt ) return 7;
	if( bid >= Bid_8s && bid <= Bid_8nt ) return 8;
	if( bid >= Bid_9s && bid <= Bid_9nt ) return 9;
	if( bid >= Bid_10s && bid <= Bid_10nt ) return 10;
	return -1;
}

inline Suit GetContractTrump(BidType bid)
{
	switch( bid ) {
		case Bid_6s:
		case Bid_7s:
		case Bid_8s:
		case Bid_9s:
		case Bid_10s:
			return SuitSpades;
		case Bid_6c:
		case Bid_7c:
		case Bid_8c:
		case Bid_9c:
		case Bid_10c:
			return SuitClubs;
		case Bid_6d:
		case Bid_7d:
		case Bid_8d:
		case Bid_9d:
		case Bid_10d:
			return SuitDiamonds;
		case Bid_6h:
		case Bid_7h: 
		case Bid_8h: 
		case Bid_9h:
		case Bid_10h:
			return SuitHearts;
		default:
			return SuitNoTrump;
	}
}

//----------------Drop------------------------------------
// Set of dropped cards and bidded game

struct Drop { 
	CardsSet Cards;
	BidType Contract;

	Drop(CardsSet cards, BidType contract) : Cards(cards), Contract(contract) {}
	Drop() : Cards(EmptyCardsSet), Contract(Bid_Pass) {}
};

} // namespace Preferans

namespace std {

template<>
struct less<Preference::Card> {
	bool operator()(const Preference::Card& c1, const Preference::Card& c2) {
		return c1.Value < c2.Value;
	}
};

inline ostream& operator << (ostream& ost, const Preference::Suit& suit)
{
	static char suits[] = "scdh";
	PrefAssert( suit.Value >= 0 && suit.Value <= Preference::NumOfSuits - 1 );
	ost << suits[suit.Value];
	return ost;
}

inline ostream& operator << (ostream& ost, const Preference::Rank& rank)
{
	static char ranks[] = "789TJQKA";
	PrefAssert( rank.Value >= 0 && rank.Value <= Preference::NumOfRanks - 1 );
	ost << ranks[rank.Value];
	return ost;
}

inline ostream& operator << (ostream& ost, const Preference::Card& card)
{
	ost << Preference::GetCardRank(card) << Preference::GetCardSuit(card);
	return ost;
}

inline ostream& operator << (ostream& ost, const Preference::CardsSet& cardsSet)
{
	ost << "[ ";
	for( Preference::SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) {
		for( Preference::RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) {
			if( Preference::IsSetContainsCard(cardsSet, Preference::CreateCard( itSuit.GetObject(), itRank.GetObject() ) ) ) {
				ost << Preference::CreateCard( itSuit.GetObject(), itRank.GetObject() ) << " ";
			}
		}
	}
	ost << "]";
	return ost;
}

inline ostream& operator << (ostream& ost, const Preference::RanksSet& ranksSet ) {
	for( Preference::RankForwardIterator rankIt; rankIt.HasNext(); rankIt.Next() ) {
		if( Preference::IsSetContainsRank(ranksSet, rankIt.GetObject()) ) {
			ost << rankIt.GetObject();
		}
	}
	return ost;
}

} // namespace std


#endif /* PREFTYPES_H_ */

