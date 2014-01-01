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

#ifndef __LEARNING_H__
#define __LEARNING_H__

/*
 * Learning for selecting suit
 * Assume we move first and need to choose suit.
 * Result of learning is set of ranks (positive float numbers) 
 * for all existing suit masks.
 *	Probability of suit "x" to be chosen is calculating by formula
 *	pr = x / (s + c + d + h), where s, c, d, h are ranks for spades, clubs, 
 *	diamonds, hearts masks correspondingly. 
 *	Quality of learning is product of all such probabilities. 
 *	Iterative method was used: on each iteration for each
 *	mask best value is selected. New vector of best values for current iteration
 *	is initial vector for next iteration.
 */

class LearningData {
public:
	// Add training set. masksSet[0] is mask selected by player. 
	// masksSet[1..n] - masks of suit cards in player hand (including selected one)
	LearningData(int variablesCount);

	bool AddSet(const vector<int>& masksSet);
	// For debug purposes
	void Dump();
	// Member access methods
	const vector< vector<int> >& GetTrainingSets() const { return trainingSets; }
	const vector< vector<int> >& GetIndexes() const { return indexes; }

private:
	// Each set represents set of masks of layouts
	// TrainingSets[i][0] - mask have been chosen
	// TrainingSets[i][1..n] - masks of all suits (including selected mask)
	vector< vector<int> > trainingSets;
	// Indexes[mask] contains indexes of all training sets which contains given mask
	vector< vector<int> > indexes;
};

vector<float> Learn(const LearningData& data);

#endif // __LEARNING_H__
