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

#ifndef _DATABASE_PARSER_H__
#define _DATABASE_PARSER_H__

struct ParsedGame {
	vector<CardsSet> Hands;
	vector<BidType> Bidding;
	BidType Contract;
	vector<Card> Moves;
	vector<Card> Widow;
	int FirstPlayer;

	ParsedGame() : Contract(Bid_Pass), FirstPlayer(0) {}
};

class DatabaseParser {
public:
	DatabaseParser(istream& _inFile) : inFile(_inFile), gameNum(0) {}

	bool ParseNextGame(ParsedGame& game);

private:
	int gameNum;

	istream& inFile;
};

#endif // _DATABASE_PARSER_H__
