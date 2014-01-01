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
#ifndef _ENGINE_DATA_H__
#define _ENGINE_DATA_H__

//---------database main class------------------------------------------
//

class LayoutsWeights;

class EngineData {
public:
	EngineData();
	~EngineData();
	
	// Loading saving
	void Load(istream& st);
	void Save(ostream& st);
	// Get statistics for one suit. For each card rank i retVal[i] means number of times card i was played
	void PassoutGetFrequencies1Move(CardsSet utilized, CardsSet hand, 
		Suit suit, array<int, NumOfRanks>& result) const;
	void PassoutGetFrequencies2Move(CardsSet utilized, CardsSet hand, 
		Card firstCard, array<int, NumOfRanks>& result) const;
	Frac<int> PassoutUnderFreq2Move(CardsSet utilized, CardsSet hand, Card firstCard) const;
	Frac<int> PassoutUnderFreq3Move(CardsSet utilized, CardsSet hand, Card maxCard) const;
	// add passout move to layouts database
	void AddPassoutMove1(CardsSet utilized, CardsSet hand, Card card1);
	void AddPassoutMove2(CardsSet utilized, CardsSet hand, Card card1, Card card2);
	void AddPassoutMove3(CardsSet utilized, CardsSet hand, Card card1, Card card2, Card card3);

	void SetPassoutDropWeights(const vector<float>& weights);
	void GetPassoutDropWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const;
	void SetPassoutFirstMoveWeights(const vector<float>& weights);
	void GetPassoutFirstMoveWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& weights) const;
	void SetMisereDropWeights(const vector<float>& weights);
	void GetMisereDropWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const;
	void SetContractDropWeights(const vector<float>& weights);
	void GetContractDropWeights(CardsSet hand, CardsSet utilized, array<float, NumOfSuits>& result) const;
	// add contract layout to database
	void AddLayout(BidType contract, CardsSet layout);

	const vector<CardsSet>& GetLayouts(BidType contract) const;
	// These methods used only for debugging purposes
	// Output entire database to cout
	void DumpPassoutDB() const;

private:
	// position - [i][j] where i - number of cards remains in suit, j - cards mask of current player
	// [i][j][k] - numbers of moves [k] played from given position [i][j] in DataBase
	int passoutFrequencies1Move[NumOfRanks+1][1<<NumOfRanks][NumOfRanks];
	int passoutFrequencies2Move[NumOfRanks+1][1<<NumOfRanks][NumOfRanks];

	Frac<int> underFrequencies2Move[NumOfRanks+1][1<<NumOfRanks][NumOfRanks];
	Frac<int> underFrequencies3Move[NumOfRanks+1][1<<NumOfRanks][NumOfRanks];

	// Suit weights for games. To obtain index of cards use getSuitIndex method, where 
	// hand - set of ranks of given suit and utilized - set of retired cards of given suit
	float contractDropWeights[NumOfRanks*(1<<NumOfRanks)];
	float misereDropWeights[NumOfRanks*(1<<NumOfRanks)];
	float passoutDropWeights[NumOfRanks*(1<<NumOfRanks)];
	float passoutFirstMoveWeights[NumOfRanks*(1<<NumOfRanks)];
	
	// layouts for different types of games. First index - game bid, second - layout
	vector< vector<CardsSet> > layouts;


	vector<Layout> getLayouts(int layoutsCount, const vector<Layout>& layouts, const LayoutsWeights& layoutsRanks) const; 

	static void generateRandomLayouts(const LayoutsWeights& layoutsRanks, int count, vector<Layout>& result);
	
	static float getLayoutRank(const Layout& layout, const LayoutsWeights& ranks);
	static void getPassoutIndexes(CardsSet utilized, CardsSet hand, Card card, int& i1, int& i2, int& i3);

};

#endif // _ENGINE_DATA_H__

