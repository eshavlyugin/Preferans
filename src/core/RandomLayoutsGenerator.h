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

#ifndef _LAYOUTS_RANDOM_GENERATOR_H__
#define _LAYOUTS_RANDOM_GENERATOR_H__

#include <Layout.h>

// Note: all layouts have non-zero probability to be generated except you explicitly forbid them
// by calling methods SetCardsOwner or SetWidowCards. Generator don't check validness of those calls;
// invalid sequence reveals only after GenerateLayouts call by assertion.

class EngineData;

namespace boost {
	template<>
	struct hash<CardsSet> {
		std::size_t operator()(const CardsSet& s) const {
			return s.Value;
		}
	};
} // namespace boost

class RandomLayoutsGenerator {
public:
	RandomLayoutsGenerator();

	vector<Layout> GenerateLayouts(int count);
	
	// select layouts from database
	vector<Layout> SelectLayoutsFromDatabase(int player, int count);
	// Selects count random layouts from given set based on weights
	// Argument "sets" is possible cards sets for player "player"
	vector<Layout> SelectRandomLayouts(const vector<CardsSet>& sets, int player, int count);
	// Set all generated layouts equiprobabale
	void Reset();

	void SetPlayerHasNoMoreCardsOfSuit(int player, Suit suit, CardsSet utilized);
	void ForbidCardsSet(CardsSet set, int player);
	// Set owner for given set of cards. It means that all generated
	// layouts will have set of cards in owner
	void SetPlayerCards(CardsSet set, int player);
	// Set widow cards
	void SetWidowCards(CardsSet set);
	// Updates layout probabilities after move
	void UpdatePassoutProbabilities(Card move, CardsSet utilizedCards, BidType gameType, 
		const EngineData& data);
	/**
	 Reset probabilities for given contract. In the beginning of deal all layouts are equiprobable. However if one of players is playing game we could make hypothesyses about his cards. For example it's hardly possible to play 9 spades with two trumps. Probabilities of layouts are stored in database. Based on this data engine fills probabilities table.
	 */
	void ResetProbabilitiesForContract(BidType contract, int dealer, const EngineData&);

private:
	multi_array<float, 3> layoutsWeights;

	array<CardsSet, 3> knownHands;
	CardsSet knownWidow;

	struct {
		unordered_map<CardsSet, int> LayoutsMap;
		BidType Contract;
	} layoutsCache;

	void generateRandomLayouts(int count, vector<Layout>& result) const;
	bool tryToRestoreFullLayout(Layout& result) const;
	void calculateWeights(multi_array<float, 4>& weightsForSuit) const;
	void fillArrangedCardsTable(const multi_array<float, 4>& weightsForSuit, multi_array<float, 3>& table) const;
	Layout generateLayout(const multi_array<float, 4>& weightsForSuit, const multi_array<float, 3>& table) const;
	Layout restoreFullLayout(const multi_array<float, 4>& weightsForSuit, const multi_array<int, 2>& counts) const;
	multi_array<int, 2> generateSuitCounts(const multi_array<float, 3>& table,
		const multi_array<float, 4>& weightsForSuit ) const;

	void setCurrentContract(BidType contract, const vector<CardsSet>& layoutsInDb);
	vector<CardsSet> generateSubsets(CardsSet set, int minCardsCount);
	void forbidCardsSet(CardsSet cardsSet, int player);
	void setCardsOwner(CardsSet cardsSet, int player);
	void normalize();
};

#endif // _LAYOUTS_RANDOM_GENERATOR_H__
