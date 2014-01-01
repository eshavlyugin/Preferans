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

#ifndef __MIN_MAX_SOLVER_H__
#define __MIN_MAX_SOLVER_H__

struct MoveQuality {
	Card Move;
	int Score;

	MoveQuality(Card move, int score) : Move(move), Score(score) {}
	MoveQuality() : Move(0), Score(0) {}
};

struct StoredValue {
	int Alpha;
	int Beta;
	// Last checked move
	int LastChecked;
	bool IsNew;

	Card Candidates[InitialCardsCount + 1];

	StoredValue() : Alpha(0), Beta(InitialCardsCount), LastChecked(0), IsNew(true) {}
};

template<class POLICY, bool ISMIN>
class MinMaxSolver {
public:
	explicit MinMaxSolver(POLICY& _policy) : policy(_policy) {}

	vector<MoveQuality> Solve(const Layout& layout, int alpha, int beta);
	int GetBestResult(const Layout& layout, int alpha, int beta);
	int StatesCount() const { return hashTable.size(); }

private:
	POLICY& policy;
	unordered_map<Layout, StoredValue> hashTable;

	int solve(const Layout& layout, int alpha, int beta);
};

#include <MinMaxSolver.inl>

#endif // __MIN_MAX_SOLVER_H__
