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

#include <RandomLayoutsGenerator.h>
#include <EngineData.h>
#include <Errors.h>

// Smallest weight for valid ranks layout. Should be
static const float SmallestWeight = 1e-3;
// Smallest non-zero float. Should be less than SmallestWeight^12 to avoid underflow
static const float Eps = 1e-40;

RandomLayoutsGenerator::RandomLayoutsGenerator() :
	layoutsWeights(boost::extents[NumOfSuits][NumOfPlayers][1<<NumOfRanks]),
	knownWidow(EmptyCardsSet)
{
}

vector<CardsSet> RandomLayoutsGenerator::generateSubsets(CardsSet set, int minCardsCount)
{
	vector<CardsSet> result(1, set);
	for( CardForwardIterator cardIt; cardIt.HasNext(); cardIt.Next() ) {
		vector<CardsSet> next;
		for( int i = 0; i < result.size(); i++ ) {
			next.push_back(result[i]);
			if( IsSetContainsCard(result[i], cardIt.GetObject()) && CardsSetSize(result[i]) > minCardsCount ) {
				result.push_back(RemoveCardFromSet(result[i], cardIt.GetObject()));
			}
		}
		result = next;
	}
	return result;
}

vector<Layout> RandomLayoutsGenerator::GenerateLayouts(int count)
{
	vector<Layout> result;
	generateRandomLayouts(count, result);
	return result;
}

vector<Layout> RandomLayoutsGenerator::SelectLayoutsFromDatabase(int player, int count)
{
	vector<Layout> result;
	vector<Layout> candidates = GenerateLayouts(std::max(300, count));
	vector<int> weights(candidates.size(), 6);
	for( int i = 0; i < candidates.size(); i++ ) {
		vector<CardsSet> sets = generateSubsets(candidates[i].Cards[player], 7);
		for( int j = 0; j < sets.size(); j++ ) {
			if( layoutsCache.LayoutsMap.find(sets[j]) != layoutsCache.LayoutsMap.end() ) {
				weights[i] = std::max(weights[i], layoutsCache.LayoutsMap[sets[i]]);
			}
		}
	}

	for( int weight = 10; weight >= 6; weight-- ) {
		for( int i = 0; i < candidates.size(); i++ ) {
			if( weights[i] == weight && result.size() < count ) {
				result.push_back(candidates[i]);
			}
		}
	}

	return result;
}

vector<Layout> RandomLayoutsGenerator::SelectRandomLayouts(const vector<CardsSet>& sets, int player, int count)
{
	float total = 0.0f;
	for( int i = 0; i < sets.size(); i++ ) {
		float sum = 1.0f;
		for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) {
			RanksSet ranksSet = GetCardsOfSuit(sets[i], itSuit.GetObject());
			sum *= layoutsWeights[itSuit.GetObject().Value][player][ranksSet.Value];
		}
		total += sum;
	}
	if( total < Eps ) {
		// No suitable layouts found
		return vector<Layout>();
	}
	vector<Layout> result;
	for( int sample = 0; sample < count; sample++ ) {
		float probe = RandomNextFloat() * total;
		Layout layout;
		for( int i = 0; i < sets.size(); i++ ) {
			float sum = 1.0f;
			for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
				RanksSet ranksSet = GetCardsOfSuit(sets[i], itSuit.GetObject());
				sum *= layoutsWeights[itSuit.GetObject().Value][player][ranksSet.Value];
			}
			probe -= sum;
			if( sum > Eps ) {
				layout.Cards[player] = sets[i];
			}
			if( probe < 0.0f ) {
				break;
			}
		}
		if( tryToRestoreFullLayout(layout) ) {
			result.push_back(layout);
		}
	}
	return result;
}

void RandomLayoutsGenerator::SetPlayerHasNoMoreCardsOfSuit(int player, Suit suit, CardsSet utilized)
{
	CardsSet inverseSet = RemoveCardsFromSet( FullCardsSet, utilized );
	CardsSet cardsToForbid = CardsSetFromRanksSet( GetCardsOfSuit(inverseSet, suit), suit );
	forbidCardsSet( cardsToForbid, player );
}

void RandomLayoutsGenerator::ResetProbabilitiesForContract(BidType contract, int dealer, const EngineData& data)
{
/*	vector<CardsSet> contractSets = data.GetLayouts(contract);
	for( int i = 0; i < NumOfSuits; i++ ) {
		for( int j = 0; j < (1<<NumOfRanks); j++ ) {
			if( layoutsWeights[i][dealer][j] > 1e-10f ) {
				layoutsWeights[i][dealer][j] = 0.001f;	
			}
		}
	}
	for( int i = 0; i < contractSets.size(); i++ ) {
		for( SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
			RanksSet ranksSet = GetCardsOfSuit(contractSets[i], suitIt.GetObject());
			if( layoutsWeights[suitIt.GetObject().Value][dealer][ranksSet.Value] > 1e-10f ) {
				layoutsWeights[suitIt.GetObject().Value][dealer][ranksSet.Value] += 1.0f;
			}
		}
	}
	normalize();
	for( int i = 0; i < (1<<NumOfRanks); i++ ) {
		for( SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
			RanksSet ranksSet = GetCardsOfSuit(contractSets[i], suitIt.GetObject());
		}
	}*/
	setCurrentContract(contract, data.GetLayouts(contract));
}

void RandomLayoutsGenerator::setCurrentContract(BidType contract, const vector<CardsSet>& layoutsInDb)
{
	if( layoutsCache.Contract == contract ) {
		return;
	}
	layoutsCache.Contract = contract;
	for( int i = 0; i < layoutsInDb.size(); i++ ) {
		vector<CardsSet> sets = generateSubsets(layoutsInDb[i], 7);
		for( int j = 0; j < sets.size(); j++ ) {
			layoutsCache.LayoutsMap[sets[j]] = std::max(layoutsCache.LayoutsMap[sets[j]], CardsSetSize(sets[j]));
		}
	}
}

void RandomLayoutsGenerator::normalize()
{
	for( int i = 0; i < NumOfSuits; i++ ) {
		for( int j = 0; j < NumOfPlayers; j++ ) {
			float maxValue = 0.0f;
			for( int k = 0; k < (1<<NumOfRanks); k++ ) {
				if( layoutsWeights[i][j][k] > maxValue ) {
					maxValue = layoutsWeights[i][j][k];
				}
			}
			PrefAssert( maxValue > 1e-10f );
			for( int k = 0; k < (1<<NumOfRanks); k++ ) {
				layoutsWeights[i][j][k] /= maxValue;
				if( layoutsWeights[i][j][k] > 1e-10f && layoutsWeights[i][j][k] < 0.001f ) {
					layoutsWeights[i][j][k] = 0.001f;
				}
			}
		}
	}
}

bool RandomLayoutsGenerator::tryToRestoreFullLayout(Layout& result) const
{
	CardsSet remainingCards = FullCardsSet;
	int numOfUnknowns = 0;
	int unknownIndex = 0;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		CardsSet newHand = AddCardsToSet( result.Cards[i], knownHands[i] );
		if( CardsSetSize( newHand ) == InitialCardsCount ) {
			result.Cards[i] = newHand;
			remainingCards = RemoveCardsFromSet( remainingCards, newHand );
		} else {
			numOfUnknowns++;
			unknownIndex = i;
		}
	}

	if( numOfUnknowns > 1 ) {
		return false;
	}
	if( numOfUnknowns == 0 ) {
		result.Widow = remainingCards;
		return true;
	}
	result.Widow = EmptyCardsSet;

	// Cards with the least ranks are dropped with higher probability
	for( RankForwardIterator itRank; itRank.HasNext() && CardsSetSize(remainingCards) != InitialCardsCount; itRank.Next() ) { 
		for( SuitForwardIterator itSuit; itSuit.HasNext() && CardsSetSize(remainingCards) != InitialCardsCount; itSuit.Next() ) {
			Card card = CreateCard(itSuit.GetObject(), itRank.GetObject());
			if( IsSetContainsCard( remainingCards, card ) ) {
				if( IsSetContainsCard( knownHands[unknownIndex], card ) ) {
					continue;
				}
				remainingCards = RemoveCardFromSet( remainingCards, card );
				result.Widow = AddCardToSet( result.Widow, card );
			}
		}
	}
	PrefAssert( CardsSetSize( remainingCards ) == InitialCardsCount );	
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( CardsSetSize( result.Cards[i] ) != InitialCardsCount ) {
			result.Cards[i] = remainingCards;
		}
	}
	return true;
}

void RandomLayoutsGenerator::Reset()
{
	for( int i = 0; i < NumOfSuits; i++ ) {
		for( int j = 0; j < NumOfPlayers; j++ ) {
			for( int k = 0; k < (1<<NumOfRanks); k++ ) {
				layoutsWeights[i][j][k] = 1.0f;
			}
		}
	}
	for( int i = 0; i < NumOfPlayers; i++ ) {
		knownHands[i] = EmptyCardsSet;
	}
	knownWidow = EmptyCardsSet;
}

void RandomLayoutsGenerator::ForbidCardsSet(CardsSet cardsSet, int player)
{
	forbidCardsSet(cardsSet, player);
}

void RandomLayoutsGenerator::SetPlayerCards(CardsSet cardsSet, int owner)
{
	for( int player = 0; player < NumOfPlayers; player++ ) {
		if( player == owner ) {
			setCardsOwner(cardsSet, player);
		} else {
			forbidCardsSet(cardsSet, player);
		}
	}
	knownHands[owner] = AddCardsToSet( knownHands[owner], cardsSet );
}

void RandomLayoutsGenerator::forbidCardsSet(CardsSet cardsSet, int player)
{
	// Setting all probabilities that player "player" has at least one card from set "cardsSet" to zero.
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		RanksSet cardsRanks = GetCardsOfSuit(cardsSet, itSuit.GetObject());
		for( RanksSubsetIterator itRanks(FullRanksSet); itRanks.HasNext(); itRanks.Next() ) { 
			if( RanksSetsIntersection( itRanks.GetObject(), cardsRanks ) != EmptyRanksSet ) {
				layoutsWeights[itSuit.GetObject().Value][player][itRanks.GetObject().Value] = 0.0f;
			}
		}
	}
}

void RandomLayoutsGenerator::setCardsOwner(CardsSet cardsSet, int player)
{
	// Setting all probabilities that player "player" doesn't have at least one card of set from "cardsSet" to zero.
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		RanksSet cardsRanks = GetCardsOfSuit(cardsSet, itSuit.GetObject());
		for( RanksSubsetIterator itRanks(FullRanksSet); itRanks.HasNext(); itRanks.Next() ) {
			if( RanksSetsIntersection( itRanks.GetObject(), cardsRanks ) != cardsRanks ) {
				layoutsWeights[itSuit.GetObject().Value][player][itRanks.GetObject().Value] = 0.0f;
			}
		}
	}
}

void RandomLayoutsGenerator::SetWidowCards(CardsSet set)
{
	for( int player = 0; player < NumOfPlayers; player++ ) {
		forbidCardsSet( set, player );
	}
	knownWidow = AddCardsToSet(knownWidow, set);
	PrefAssert( CardsSetSize(knownWidow) <= 2 );
}

void RandomLayoutsGenerator::UpdatePassoutProbabilities(Card move, CardsSet utilizedCards,
	BidType gameType, const EngineData& data)
{
}

void RandomLayoutsGenerator::generateRandomLayouts(int count, vector<Layout>& result) const
{
	// Algorithm for generating random layouts basis on vector of suit distributions for each player is following:
	// 1. Calculating sum of weights of layouts for each suit s, that satisfy condition -
	// 	first player has p1 cards of suit s, second p2 and third p3. 
	// 	Store those values in array weightsForSuit[s][p1][p2][p3]
	// 2. Use dynamic programming to calculate array table[p1][p2][p3] - total sum of weights starting from
	// position [p1][p2][p3] where p1, p2 and p3 are number of distributed cards for players 1, 2, 3.
	// Use weightsForSuit array from step 1 for transitions weights. 
	// 3. Using table[p1][p2][p3] to generate random layout for number of cards for each player
	// 4. For each suit determine card ranks and restore full layout for each player 
	// 	using number of cards distribution from previous step.
	multi_array<float, 4> weightsForSuit(boost::extents[NumOfSuits][NumOfRanks+1][NumOfRanks+1][NumOfRanks+1]);
	multi_array<float, 3> table(boost::extents[InitialCardsCount+1][InitialCardsCount+1][InitialCardsCount+1]);

	calculateWeights(weightsForSuit);
	fillArrangedCardsTable(weightsForSuit, table);
	result.clear();
	for( int i = 0; i < count; i++ ) {
		result.push_back(generateLayout(weightsForSuit, table));
		// Adding rest of cards to widow
		result.back().Widow = RemoveCardsFromSet( FullCardsSet,
			AddCardsToSet( result.back().Cards[0],
				AddCardsToSet( result.back().Cards[1], result.back().Cards[2] ) ) );
	}
}

void RandomLayoutsGenerator::calculateWeights(multi_array<float, 4>& weightsForSuit) const
{
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		// Iterating over all ranks subset
		for( RanksSubsetIterator itSet1(FullRanksSet); itSet1.HasNext(); itSet1.Next() ) { 
			RanksSet inv1 = RemoveRanksFromSet(FullRanksSet, itSet1.GetObject());
			// Iterating over all ranks subsets without ranks in p1
			for( RanksSubsetIterator itSet2(inv1); itSet2.HasNext(); itSet2.Next() ) {
				RanksSet inv2 = RemoveRanksFromSet(inv1, itSet2.GetObject());
				// Iterating over all ranks subsets without ranks in p1 and p2
				for( RanksSubsetIterator itSet3(inv2); itSet3.HasNext(); itSet3.Next() ) { 
					int total = RanksSetSize(itSet1.GetObject()) + RanksSetSize(itSet2.GetObject()) + RanksSetSize(itSet3.GetObject());
					if( total < 6 ) {
						continue;
					}
					weightsForSuit[itSuit.GetObject().Value][RanksSetSize(itSet1.GetObject())]
					[RanksSetSize(itSet2.GetObject())][RanksSetSize(itSet3.GetObject())] += 
						layoutsWeights[itSuit.GetObject().Value][0][itSet1.GetObject().Value]
						* layoutsWeights[itSuit.GetObject().Value][1][itSet2.GetObject().Value]
						* layoutsWeights[itSuit.GetObject().Value][2][itSet3.GetObject().Value];
				}
			}
		}
	}
}

void RandomLayoutsGenerator::fillArrangedCardsTable(const multi_array<float, 4>& weightsForSuit, 
	multi_array<float, 3>& table) const
{
	table[InitialCardsCount][InitialCardsCount][InitialCardsCount] = 1.0f;
	// Iterate over table in backward direction
	for( int c1 = InitialCardsCount; c1 >= 0; c1-- ) {
		for( int c2 = InitialCardsCount; c2 >= 0; c2-- ) {
			for( int c3 = InitialCardsCount; c3 >= 0; c3-- ) {
				int tmp = (c1 + c2 + c3 + 2); // 2 - for widow cards
				int widowCardsCount = tmp % NumOfRanks;
				if( widowCardsCount > 2 ) {
					continue;
				}
				int suit = tmp / NumOfRanks - 1;
				if( suit < 0 ) {
					continue;
				}
				// Trying all possible combinations for each player cards count
				for( int dc1 = 0; dc1 <= NumOfRanks; dc1++ ) {
					int newC1 = c1 - dc1;
					if( newC1 < 0 ) {
						continue;
					}
					for( int dc2 = 0; dc2 <= NumOfRanks; dc2++ ) {
						if( dc1 + dc2 > NumOfRanks ) {
							continue;
						}
						int newC2 = c2 - dc2;
						if( newC2 < 0 ) {
							continue;
						}
						for( int dc3 = 0; dc3 <= NumOfRanks; dc3++ ) {
							int dWidow = NumOfRanks - dc1 - dc2 - dc3;
							if( dWidow < 0 || dWidow + widowCardsCount > 2 ) {
								continue;
							}
							int newC3 = c3 - dc3;
							if( newC3 < 0 ) {
								continue;
							}
							if( newC1 == 0 && newC2 == 0 && newC3 == 0 ) {
								PrefAssert( c1 == dc1 && c2 == dc2 && c3 == dc3 );
							}
							table[newC1][newC2][newC3] += table[c1][c2][c3]	
								* weightsForSuit[suit][dc1][dc2][dc3];
						}
					}
				}
			}
		}
	}
	// Something is going wrong. There has to be at least one valid layout with non-zero weight
	PrefAssert( table[0][0][0] > Eps );
}

Layout RandomLayoutsGenerator::generateLayout(const multi_array<float, 4>& weightsForSuit, 
	const multi_array<float, 3>& table) const
{
	// first generating suit counts for all players
	multi_array<int, 2> counts = generateSuitCounts(table, weightsForSuit);
	// then restoring full layout according to given suit counts
	return restoreFullLayout(weightsForSuit, counts);
}

// return two-dim array [p][s] - the total number of cards of suit "s" player "p" has.
multi_array<int, 2> RandomLayoutsGenerator::generateSuitCounts(const multi_array<float, 3>& table, 
	const multi_array<float, 4>& weightsForSuit) const
{
	multi_array<int, 2> result(boost::extents[NumOfPlayers][NumOfSuits]);
	// number of cards of 1st, 2nd and 3rd player
	int c1 = 0, c2 = 0, c3 = 0;

	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		// first selecting a random number from 0 to total weights of all possible transitions from
		// state [c1][c2][c3]
		float sample = table[c1][c2][c3] * RandomNextFloat();
		// then iterating over all possible transitions and subtracting transition weight
		// while this number is >0.
		for( int dc1 = 0; dc1 <= NumOfRanks; dc1++ ) {
			// Total number of cards can't be more 10
			if( dc1 + c1 > InitialCardsCount ) {
				break;
			}
			for( int dc2 = 0; dc2 <= NumOfRanks; dc2++ ) {
				// Total number of cards can't be more than 10 and total number of suit cards can't be more than 8.
				if( dc1 + dc2 > NumOfRanks || dc2 + c2 > InitialCardsCount) {
					break;
				}
				for( int dc3 = 0; dc3 <= NumOfRanks; dc3++ ) {
					if( dc1 + dc2 + dc3 > NumOfRanks || c3 + dc3 > InitialCardsCount) {
						break;
					}
					// Total number of widow cards can't be more than 2
					if( dc1 + dc2 + dc3 < NumOfRanks - 2 ) {
						continue;
					}
					sample -= table[c1 + dc1][c2 + dc2][c3 + dc3] * 
						weightsForSuit[itSuit.GetObject().Value][dc1][dc2][dc3];
					result[0][itSuit.GetObject().Value] = dc1;
					result[1][itSuit.GetObject().Value] = dc2;
					result[2][itSuit.GetObject().Value] = dc3;
					if( sample < 0.0f ) {
						goto EndOfOuterLoop;
					}
				}
			}
		}
		EndOfOuterLoop:
		// Updating suit counts
		c1 += result[0][itSuit.GetObject().Value];
		c2 += result[1][itSuit.GetObject().Value];
		c3 += result[2][itSuit.GetObject().Value];
	}
	return result;
}

Layout RandomLayoutsGenerator::restoreFullLayout(const multi_array<float, 4>& weightsForSuit, 
	const multi_array<int, 2>& ranksCounts) const
{
	Layout result;
	for( SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		array<CardsSet, NumOfPlayers> currentSet;
		// Generating random number from 0 to total weights of all possible layouts
		float sample = RandomNextFloat() * 
			weightsForSuit[itSuit.GetObject().Value][ranksCounts[0] [itSuit.GetObject().Value]]
			[ranksCounts[1][itSuit.GetObject().Value]][ranksCounts[2][itSuit.GetObject().Value]];
		// iterating over all subsets for a given suit for first player
		// and subtracting layout weigh from generated number while it is >0
		for( RanksSubsetIterator itSet1(FullRanksSet); itSet1.HasNext(); itSet1.Next() ) { 
			if( RanksSetSize(itSet1.GetObject()) != ranksCounts[0][itSuit.GetObject().Value] ) {
				continue;
			}
			// iteting over all subsets for a given suit for second player
			RanksSet inv1 = RemoveRanksFromSet(FullRanksSet, itSet1.GetObject()); // Set of 2nd player cannot contains 1st player cards
			for( RanksSubsetIterator itSet2(inv1); itSet2.HasNext(); itSet2.Next() ) { 
				if( RanksSetSize(itSet2.GetObject()) != ranksCounts[1][itSuit.GetObject().Value] ) {
					continue;
				}
				RanksSet inv2 = RemoveRanksFromSet(inv1, itSet2.GetObject()); // Set of 3rd player cannot contains 1st and 2nd player cards
				// iteting over all subsets for a given suit for second player
				for( RanksSubsetIterator itSet3(inv2); itSet3.HasNext(); itSet3.Next() ) { 
					if( RanksSetSize(itSet3.GetObject()) != ranksCounts[2][itSuit.GetObject().Value] ) {
						continue;
					}
					sample -= layoutsWeights[itSuit.GetObject().Value][0][itSet1.GetObject().Value] 
						* layoutsWeights[itSuit.GetObject().Value][1][itSet2.GetObject().Value] 
						* layoutsWeights[itSuit.GetObject().Value][2][itSet3.GetObject().Value];
					currentSet[0] = CardsSetFromRanksSet(itSet1.GetObject(), itSuit.GetObject());
					currentSet[1] = CardsSetFromRanksSet(itSet2.GetObject(), itSuit.GetObject());		
					currentSet[2] = CardsSetFromRanksSet(itSet3.GetObject(), itSuit.GetObject());
					if( sample < 0 ) {
						goto EndOfOuterLoop;
					}
				}
			}
		}
		EndOfOuterLoop:
		for( int i = 0; i < NumOfPlayers; i++ ) {
			result.Cards[i] = AddCardsToSet( result.Cards[i], currentSet[i] );
		}
	}
	return result;
}

