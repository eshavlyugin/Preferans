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

#ifndef _BULLET_H__ 
#define _BULLET_H__

#include <GameSettings.h>
#include <DealResult.h>

//-------------------------PlayerScore------------------------------------------------
//
struct PlayersScores {
	vector<int> Bullet;
	vector<int> Mountain;
	vector< vector<int> > Whists;

	PlayersScores();

	void AddScores(const PlayersScores& scores);
	vector<float> Evaluations() const;
};

inline PlayersScores::PlayersScores() : 
	Bullet(3),
	Mountain(3),
	Whists(3,vector<int>(3))
{
}

inline void PlayersScores::AddScores(const PlayersScores& scores)
{
	for( int i = 0; i < Bullet.size(); i++ ) {
		Bullet[i] += scores.Bullet[i];
	}
	for( int i = 0; i < Mountain.size(); i++ ) {
		Mountain[i] += scores.Mountain[i];
	}
	for( int i = 0; i < Whists.size(); i++ ) {
		for( int j = 0; j < Whists[0].size(); j++ ) {
			Whists[i][j] += scores.Whists[i][j];
		}
	}
}

//----------------------Bullet--------------------------------------------------
//
class Bullet {
public:
	// Initialize new bullet
	void Initialize(const GameSettings& settings);
	// Get current scores
	const PlayersScores& GetScores() const { return scores; }
	// Update scores with given game outcome
	void UpdateScores(const DealResult& gameResult);
	// Get bullet history
	const vector<DealResult>& GetHistory() const;
	// Is bullet finished
	bool IsFinished() const;
	// Get float evaluations for game
	vector<float> EvaluateDeal(const DealResult& gameResult) const;

private:
	GameSettings settings;
	// Current scores of all players
	PlayersScores scores;
	
	vector<DealResult> history;

	PlayersScores getGameDiff(const DealResult& gameResult) const;
	PlayersScores getPassoutDiff(const DealResult& gameResult) const;
	PlayersScores getContractGameDiff(const DealResult& gameResult, int dealer, int contract) const;
	PlayersScores getMisereDiff(const DealResult& gameResult, int dealer) const;
	void normalizeScores(PlayersScores& diff) const;
};

#endif // _BULLET_H__

