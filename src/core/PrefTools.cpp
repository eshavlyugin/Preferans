/****************************************************************************
 Preferans: implementation of card-tricking game Preferans (or Preference).
 ****************************************************************************
 Copyright (c) 2010-2011 Eugene Shavlyugin <eshavlyugin@gmail.com>
 
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

#include <PrefTools.h>
#include <Errors.h>

vector< vector<Card> > GetPossibleDrops(CardsSet dealerWithWidow, bool dropMax)
{
	vector< vector<Card> > result;
	for( SuitForwardIterator itSuit1; itSuit1.HasNext(); itSuit1.Next() ) { 
		Rank rank1 = dropMax ? GetMaxRank(GetCardsOfSuit(dealerWithWidow, itSuit1.GetObject()), RankLast)
			: GetMinRank(GetCardsOfSuit(dealerWithWidow, itSuit1.GetObject()), RankFirst);
		if( rank1 == RankInvalid ) {
			continue;
		}
		
		Card card1 = CreateCard(itSuit1.GetObject(), rank1);
		CardsSet _dealerWithWidow = RemoveCardFromSet(dealerWithWidow, card1);
		for( SuitForwardIterator itSuit2; itSuit2.HasNext(); itSuit2.Next() ) {
			Rank rank2 = dropMax ? GetMaxRank(GetCardsOfSuit(_dealerWithWidow, itSuit2.GetObject()), RankLast)
				: GetMinRank(GetCardsOfSuit(_dealerWithWidow, itSuit2.GetObject()), RankFirst);
			if( rank2 == RankInvalid ) {
				continue;
			}
			Card card2 = CreateCard(itSuit2.GetObject(), rank2);
			vector<Card> tmp(2);
			tmp[0] = card1;
			tmp[1] = card2;
			result.push_back(tmp);
		}
	}
	return result;
}

Rank RankFromChar(char c)
{
	switch(c) {
		case '7':
			return 0;
		case '8':
			return 1;
		case '9':
			return 2;
		case '0':
			return 3;
		case 'J':
			return 4;
		case 'Q':
			return 5;
		case 'K':
			return 6;
		case 'A':
			return 7;
		default:
			return RankInvalid;
	}
}

Suit SuitFromChar(char c)
{
	switch(c) {
		case 's':
			return SuitSpades;
		case 'c':
			return SuitClubs;
		case 'd':
			return SuitDiamonds;
		case 'h':
			return SuitHearts;
		default:
			return SuitInvalid;
	}
}

void PrintFloatArray(const vector<float>& a)
{
	for( int i = 0; i < a.size(); i++ ) {
		GetLog() << i << " " << a[i] << endl;
	}
}

Card CardFromString(const string& st)
{
	Rank rank = RankFromChar(st[st.length() - 2]);
	Suit suit = SuitFromChar(st[st.length() - 1]);
	return CreateCard(suit, rank);
}

string SuitToString(Suit suit)
{
	switch( suit.Value ) {
		case 0: 
			return "s";
		case 1:
			return "c";
		case 2:
			return "d";
		case 3:
			return "h";
		case 4:
			return "nt";
		default:
			PrefAssert( false );
			return "";
	}
}

BidType StringToBid(const string& st)
{
	if( st.empty() ) {
		return Bid_Invalid;
	}
	if( st == "misere" || st == "miser") {
		return Bid_Misere;
	} else if( st == "pass" || st == "pas" ) {
		return Bid_Pass;
	} else {
		int contract = 0;
		if( st.find("10") != string::npos ) {
			contract = 10;
		} else if( st.find("9") != string::npos ) {
			contract = 9;
		} else if( st.find("8") != string::npos ) {
			contract = 8;
		} else if( st.find("7") != string::npos ) {
			contract = 7;
		} else if( st.find("6") != string::npos ) {
			contract = 6;
		}
		Suit suit = SuitFromChar(st[st.length()-1]);
		return ConstructContractGameBid(contract, suit);
	}
	return Bid_Pass;
}

string BidToString(BidType bid)
{
	switch( bid ) {
		case Bid_Pass:
			return "pass";
		case Bid_Misere:
			return "misere";
		case Bid_Whist:
			return "whist";
		case Bid_HalfWhist:
			return "half-whist";
		case Bid_OpenWhist:
			return "open-whist";
		case Bid_CloseWhist:
			return "close-whist";
		default:
		{
			if( bid >= Bid_6s && bid <= Bid_10nt ) {
				int deal = GetContractGameDeal(bid);
				Suit trump = GetContractTrump(bid);
				ostringstream ost;
				ost << deal << SuitToString(trump);
				return ost.str();
			}
			PrefAssert( false );
			return "";
		}
		
	}
}

int GetRanksSetIndex(RanksSet hand, RanksSet utilized)
{
	int res = 0;
	int count = 0;
	for( RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
		if( IsSetContainsRank(utilized, itRank.GetObject()) ) {
			continue;
		}
		count++;
		if( IsSetContainsRank(hand, itRank.GetObject()) ) {
			res |= (1<<(count-1));
		}
	}
	return res + (count - 1) * (1 << NumOfRanks);
}


