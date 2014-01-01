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

#ifndef _DB_CONVERTER_H_
#define _DB_CONVERTER_H_

class EngineData;
class LearningData;

class DBConverter {
public:
	DBConverter();
	~DBConverter();

	void Reset();
	// Converts raw database to engine format
	// Database could be obtained by python script in source file
	void Convert(const string& rawDbFile, const string& resultFile);

private:
	// Required information about single passout turn (3 moves and additional data).
	struct DBSet {
		CardsSet Utilized;
		CardsSet Hand;
		Card Move1;
		Card Move2;
		Card Move3;
		int MoveNumber;

		DBSet(CardsSet utilized, CardsSet hand, Card move1, Card move2, Card move3, int moveNumber) : Utilized(utilized), 
			Hand(hand), Move1(move1), Move2(move2), Move3(move3), MoveNumber(moveNumber) {}
	};

	EngineData* data;
	LearningData* passoutDropData;
	LearningData* misereDropData;
	LearningData* contractDropData;
	LearningData* passoutFirstMoveData;

	void commitDBSet(const DBSet& set);
	bool isGoodLayout(const Layout& layout, int firstPlayer, int gameBid);
	Suit determineMoveSuit(const vector<CardsSet>& hands, const vector<Card>& moves, Suit widowSuit) const;
	void calcPassoutProbabilities(const vector<CardsSet>& hands, 
		const vector<Card>& widow, const vector<Card>& moves, int firstPlayer);
	void reset();
	bool isPossibleMoveSuit(const vector<CardsSet>& hands, const vector<Card>& moves, Suit suit) const;
	void addPassoutTurn(Suit moveSuit, vector<CardsSet>& hands, int& firstPlayer, const vector<Card>& moves, 
		int turnNumber, vector<DBSet>& dbCandidates, CardsSet& utilizedCards); 
	void addContractGame(const vector<CardsSet>& hands, const vector<Card>& widow, 
		const vector<BidType>& bidding, BidType contract, int firstPlayer, int dealer);
	void addMisereGame(const vector<CardsSet>& hands, const vector<Card>& widow, 
		const vector<BidType>& bidding, int firstPlayer, int dealer);
	int determineDealer(int firstPlayer, const vector<BidType>& bidding);
	void addDropsDataToLearningSet(CardsSet dealerWithWidow, BidType contract, const vector< vector<Card> >& bestDrops, LearningData*);
};

#endif // _DB_CONVERTER_H_
