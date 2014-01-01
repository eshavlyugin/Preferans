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

#include <DatabaseParser.h>
#include <PrefTools.h>

bool DatabaseParser::ParseNextGame(ParsedGame& game)
{
	bool isParsed = false;
	bool isBadLayout = false;
	int currentPlayer = 0;
	game.Bidding.clear();
	game.Hands.clear();
	game.Moves.clear();
	game.Widow.clear();

	while(!isParsed && !inFile.eof()) {
		 try {
			 string command;
			inFile >> command;
			if( command.length() == 0 ) {
				break;
			}
			if( command == "begin" ) {
				gameNum++;
				currentPlayer = 2;
				game.Bidding.clear();
				game.Widow.resize(2, 0);
				game.Moves.clear();
				game.FirstPlayer = -1;
				game.Hands.resize(3, EmptyCardsSet);
			} else if( command == "end" ) {
				if( !isBadLayout ) {
					isParsed = true;
				}
				GetLog() << "Game processes " << gameNum << endl;
				isBadLayout = false;
			} else if( isBadLayout ) {
				continue;
			} else if( command == "CONTRACT" ) {
				string contractName;
				inFile >> contractName;
				game.Contract = StringToBid(contractName);
			} else if( command == "FIRSTPLAYER" ) {
				inFile >> game.FirstPlayer;
				game.FirstPlayer %= NumOfPlayers;
			} else if( command == "HAND" ) {
				string card;
				for( int i = 0; i < InitialCardsCount; i++ ) {
						inFile >> card;
						game.Hands[currentPlayer] = AddCardToSet(game.Hands[currentPlayer], CardFromString(card));
				}
				currentPlayer = (currentPlayer + 1) % NumOfPlayers;
			} else if( command == "WIDOW" ) {
				string card1, card2;
				inFile >> card1 >> card2;
				game.Widow[0] = CardFromString(card1);
				game.Widow[1] = CardFromString(card2);
			} else if( command == "TRADE" ) {
				string trade;
				inFile >> trade;
				game.Bidding.push_back( StringToBid(trade) );
			} else if( command == "MOVE" ) {
				string card;
				inFile >> card;
				game.Moves.push_back(CardFromString(card));
			}
		} catch(exception& e) {
			isParsed = false;
			isBadLayout = true;
			GetLog() << e.what() << endl;
		}
	}
	return isParsed;
}

