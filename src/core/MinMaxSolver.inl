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

#ifndef __MIN_MAX_SOLVER_INL__
#define __MIN_MAX_SOLVER_INL__

template<class POLICY, bool ISMIN>
vector<MoveQuality> MinMaxSolver<POLICY, ISMIN>::Solve(const Layout& layout, int alpha, int beta)
{
	vector<Card> moves;
	vector<MoveQuality> result;
	policy.GenerateMoves(layout, moves);
	for( int i = 0; i < moves.size(); i++ ) {
		Layout tmp = layout;
		tmp.Hash = CalcLayoutHash(tmp.Cards);
		tmp.UpdateOnMove(moves[i]);
		int score = solve(tmp, alpha, beta);
		result.push_back(MoveQuality(moves[i], score));
	}
	return result;
}

template<class POLICY, bool ISMIN>
int MinMaxSolver<POLICY, ISMIN>::GetBestResult(const Layout& layout, int alpha, int beta)
{
	return solve(layout, alpha, beta);
}

template<class POLICY, bool ISMIN>
int MinMaxSolver<POLICY, ISMIN>::solve(const Layout& layout, int alpha, int beta)
{
	if( layout.IsFinished() ) {
		return layout.Tricks[layout.Dealer];
	}

	bool isSearchMax = !ISMIN == (layout.CurrentPlayer == layout.Dealer);
	StoredValue& score = hashTable[layout];

	// First visited state. Generating moves in order defined by policy
	if( score.IsNew ) {
		vector<Card> moves;
		moves.reserve(10);
		policy.GenerateMoves(layout, moves);
		memcpy(score.Candidates, &moves[0], sizeof(moves[0]) * moves.size());
		score.Candidates[moves.size()] = UnknownCard;
		score.IsNew = false;
	}

	if( isSearchMax && beta <= score.Alpha + layout.Tricks[layout.Dealer] ) { 
		return score.Alpha + layout.Tricks[layout.Dealer];
	}
	if( !isSearchMax && alpha >= score.Beta + layout.Tricks[layout.Dealer] ) { 
		return score.Beta + layout.Tricks[layout.Dealer];
	}

	if( isSearchMax && alpha < layout.Tricks[layout.Dealer] + score.Alpha ) {		
		alpha = layout.Tricks[layout.Dealer] + score.Alpha;
	} 
	if( !isSearchMax && beta > layout.Tricks[layout.Dealer] + score.Beta ) {
		beta = layout.Tricks[layout.Dealer] + score.Beta;
	}

	while( score.Candidates[score.LastChecked] != UnknownCard ) {
		Layout tmp = layout;
		tmp.UpdateOnMove(score.Candidates[score.LastChecked]);
		if( tmp.NumCardsOnDesk == 0 ) {
			tmp.Compress();
		}
		
		int res = solve( tmp, alpha, beta ); 

		if( isSearchMax ) {
			if( res >= beta ) {
				return res;
			}
			if( res - layout.Tricks[layout.Dealer] > score.Alpha ) {
				score.Alpha = res - layout.Tricks[layout.Dealer];
			}
			if( alpha < res ) {
				alpha = res;
			}
		} else {
			if( res <= alpha ) {
				return res;
			}
			if( res - layout.Tricks[layout.Dealer] < score.Beta ) {
				score.Beta = res - layout.Tricks[layout.Dealer];
			}
			if( beta > res ) {
				beta = res;
			}
		}
		score.LastChecked++;
	}

	return layout.Tricks[layout.Dealer] + (isSearchMax ? score.Alpha : score.Beta);
}

#endif // __MIN_MAX_SOLVER_INL__

