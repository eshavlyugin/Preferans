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

#ifndef _LAYOUT_H__
#define _LAYOUT_H__

struct Layout {
	// Bit mask for player cards. Use CardToInt and IntToCard functions.
	CardsSet Cards[NumOfPlayers];
	// Widow
	CardsSet Widow;
	// Trump for game. Set only for contract games. 
	Suit Trump;
	// Number of Current Player
	int CurrentPlayer;
	// Cards on desk
	int NumCardsOnDesk;
	// Suit for move. Usually first card sets suit. Exceptions are rostov passout rules
	Suit MoveSuit;
	// Maximal card
	Card MaxCard;
	// Cards on desk
	Card Desk[NumOfPlayers];
	// Number of players tricks
	int Tricks[NumOfPlayers];
	// Cards out of game
	CardsSet UtilizedCards;
	// Dealer. Valid only for misere and contract games
	int Dealer;
	// Num of remaining moves
	int MovesRemaining;
	// Hash value
	int Hash;

	Layout();

	// clears all fields of structure
	void Clean();
	// updates state;
	void UpdateOnMove(Card move);
	// returns true if no cards remains
	bool IsFinished() const;
	// Check whether layout is valid (num of cards for each player <= 10 and widow cards count <= 2)
	bool IsValid() const;
	// compress state. uses only to reduce number of states
	void Compress();
};

inline Layout::Layout()
{
	Clean();
}

inline void Layout::Clean()
{
	memset(this, 0, sizeof(Layout));
	Trump = SuitNoTrump;
	MoveSuit = SuitNoTrump;
	MaxCard = UnknownCard;
	MovesRemaining = 10;
}

inline int CalcLayoutHash(CardsSet* cards)
{
	return cards[0].Value * 7171 + cards[1].Value * 123 + cards[2].Value * 1234567;
}		

inline int NextPlayer(int currentPlayer)
{
	static int nextTable[3] = {1,2,0};
	return nextTable[currentPlayer];
}

inline void Layout::UpdateOnMove(Card move)
{
	// Updating structures
	if( Cards[CurrentPlayer] != EmptyCardsSet ) {
		Cards[CurrentPlayer] = RemoveCardFromSet(Cards[CurrentPlayer], move);
	}
	Desk[CurrentPlayer] = move;
	UtilizedCards = AddCardToSet(UtilizedCards, move);
	// First move declares suit
	if( MoveSuit == SuitNoTrump ) {
		MoveSuit = GetCardSuit(move);
	}
	// Updating max card
	if( (MaxCard == UnknownCard && MoveSuit == GetCardSuit(move)) 
		|| (MaxCard != UnknownCard && IsGreaterCard(move, MaxCard, MoveSuit, Trump)) ) 
	{
		MaxCard = move;
	}
	NumCardsOnDesk++;
	CurrentPlayer = NextPlayer(CurrentPlayer);
	if( NumCardsOnDesk == NumOfPlayers ) {
		// Updating internal structures
		for( int i = 0; i < NumOfPlayers; i++ ) {
			if( MaxCard == Desk[i] ) { 
				Tricks[i]++;
				CurrentPlayer = i;
				NumCardsOnDesk = 0;
				MoveSuit = SuitNoTrump;
				MovesRemaining--;
				MaxCard = UnknownCard;
				return;
			}
		}
		PrefAssert( false );
	}
}

inline void Layout::Compress()
{
	CardsSet newSet[NumOfPlayers];
	memset(newSet, 0, sizeof(newSet));

	// Updating cards sets
	for( SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
		int curRank = 0;
		for( RankForwardIterator rankIt; rankIt.HasNext(); rankIt.Next() ) {
			Card card = CreateCard(suitIt.GetObject(), rankIt.GetObject());
			for( int player = 0; player < NumOfPlayers; player++ ) {
				if( IsSetContainsCard(Cards[player], card) ) {
					newSet[player] = AddCardToSet(newSet[player], CreateCard(suitIt.GetObject(), curRank));
					curRank++;
					break;
				}
			}
		}
	}
	memcpy(Cards, newSet, sizeof(newSet));
	// recalculating hash
	Hash = CalcLayoutHash(Cards);
}	

inline bool Layout::IsFinished() const
{
	return MovesRemaining == 0;
}

inline bool Layout::IsValid() const
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( CardsSetSize(Cards[i]) > 10 ) {
			return false;
		}
	}
	return CardsSetSize(Widow) <= 2;
}

inline bool operator == (const Layout& l1, const Layout& l2)
{
	return l1.Cards[0] == l2.Cards[0] 
		&& l1.Cards[1] == l2.Cards[1] 
		&& l1.Cards[2] == l2.Cards[2] 
		&& l1.CurrentPlayer == l2.CurrentPlayer 
		&& l1.MaxCard == l2.MaxCard;
}

inline bool operator != (const Layout& l1, const Layout& l2) {
	return !(l1 == l2);
}

// boost instantinations for Layout-----------------------------

namespace boost {
	template<>
	struct hash<Layout> {
		std::size_t operator() (const Layout& l) const 
		{
			return (std::size_t)(l.Hash);
		}
	};
}

#endif // _LAYOUT_H__
