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

#ifndef _STRING_OPERATIONS_H__
#define _STRING_OPERATIONS_H__

#include <PrefTypes.h>

// Get all possible drops for dealer with widow. Assume we drop only cards with 
// lowest (highest) ranks. If dropMax is true than we drop cards with highest ranks, otherwise - smallest.
vector< vector<Card> > GetPossibleDrops(CardsSet dealerWithWidow, bool dropMax);

Rank RankFromChar(char c);

Suit SuitFromChar(char c);

void PrintFloatArray(const vector<float>& a);

Card CardFromString(const string& st);

BidType StringToBid(const string& st);

string SuitToString(Suit suit);

string BidToString(BidType bid);

int GetRanksSetIndex(RanksSet hand, RanksSet utilized);

#endif // _STRING_OPERATIONS_H__
