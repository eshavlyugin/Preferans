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

#include <Bullet.h>
#include <Errors.h>

void Bullet::Initialize(const GameSettings& _settings)
{
	settings = _settings;
}

void Bullet::UpdateScores(const DealResult& gameResult)
{
	history.push_back(gameResult);
	PlayersScores diff = getGameDiff( gameResult );
	normalizeScores(diff);
	scores.AddScores(diff);
}

PlayersScores Bullet::getGameDiff(const DealResult& gameResult) const
{
	BidType contract = Bid_Pass;
	int dealer = 0;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( gameResult.Bids[i] > contract ) {
			contract = gameResult.Bids[i];
			dealer = i;
		}
	}

	switch( contract ) {
		case Bid_Pass:
			return getPassoutDiff(gameResult);
		case Bid_Misere:
			return getMisereDiff(gameResult, dealer);
		default:
			return getContractGameDiff(gameResult, dealer, GetContractGameDeal(contract));
	}
}

vector<float> Bullet::EvaluateDeal(const DealResult& gameRes) const
{
	PlayersScores diff = getGameDiff(gameRes);
	vector<float> result(diff.Mountain.size());
	for( int i = 0; i < diff.Mountain.size(); i++ ) {
		for( int j = i+1; j < diff.Mountain.size(); j++ ) {
			float dx = (diff.Bullet[j] - diff.Bullet[i] + diff.Mountain[i] - diff.Mountain[j]) * 10.0f / 3.0f + 
				(diff.Whists[j][i] - diff.Whists[i][j]);
			result[i] -= dx;
			result[j] += dx;
		}
	}
	return result;
}

PlayersScores Bullet::getPassoutDiff(const DealResult& gameResult) const
{
	PlayersScores diff;
	if( settings.Rules == RT_Rostov ) {
		// For Rostov rules adding whists instead of mountain
		PrefAssert( false );
	} else {
		int _min = InitialCardsCount;
		for( int i = 0; i < NumOfPlayers; i++ ) {
			_min = min(_min, gameResult.Tricks[i]);
		}
		for( int i = 0; i < NumOfPlayers; i++ ) { 
			// case for clean pass-out
			if( gameResult.Tricks[i] == 0 ) {
				if( scores.Bullet[i] != settings.BulletSize ) {
					diff.Bullet[i] = 1;
				} else {
					diff.Mountain[i] = -1;
				}
			}
			// mountain calculation
			diff.Mountain[i] += gameResult.Tricks[i] - _min;
		}
	}
	return diff;
}

PlayersScores Bullet::getContractGameDiff(const DealResult& gameResult, int dealer, int contract) const
{
	PlayersScores diff;
	// Number of whisted players
	int whistsCount = 0;
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( gameResult.Bids[i] == Bid_Whist || gameResult.Bids[i] == Bid_OpenWhist || gameResult.Bids[i] == Bid_CloseWhist ) {
			whistsCount++;
		}
	}
	// Number of minimum tricks for whist deal
	int minWhistTricks = 0;
	switch( contract ) {
		case 6: 
			minWhistTricks = 4;
			break;
		case 7:
			minWhistTricks = 2;
			break;
		case 8:
		case 9:
			minWhistTricks = 1;
			break;
		case 10:
			minWhistTricks = 0;
			break;
		default:
			PrefAssert( false );
	}

	int dealerTricks = gameResult.Tricks[dealer];
	int contractCost = (contract - 5) * 2;
	if( dealerTricks >= contract ) {
		diff.Bullet[dealer] += contractCost;
	} else {
		diff.Mountain[dealer] += (contract - dealerTricks) * contractCost;
		for( int i = 0; i < NumOfPlayers; i++ ) {
			if( i != dealer ) {
				diff.Whists[i][dealer] = (contract - dealerTricks) * contractCost;
			}
		}
	}
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( gameResult.Bids[i] == Bid_Whist || gameResult.Bids[i] == Bid_OpenWhist || gameResult.Bids[i] == Bid_CloseWhist ) {
			// Adding whist score
			switch( whistsCount ) {
				case 1:
					diff.Whists[i][dealer] += contractCost * (10 - dealerTricks);
					break;
				case 2: 
					diff.Whists[i][dealer] += contractCost * gameResult.Tricks[i];
					break;
				default:
					PrefAssert( false );
			}
			// Adding score to mountain
			if( 10 - dealerTricks < minWhistTricks ) {
				switch( whistsCount ) {
					case 1:
					{
						int shortage = minWhistTricks - (10 - dealerTricks);
						diff.Mountain[i] += settings.IsHalfResponsibleWhist ?
							(shortage * contractCost / 2) : (shortage * contractCost);
						break;
					}
					case 2:
					{
						if( gameResult.Tricks[i] < minWhistTricks / 2 ) {
							int shortage = minWhistTricks / 2 - gameResult.Tricks[i];
							diff.Mountain[i] += settings.IsHalfResponsibleWhist ?
								(shortage * contract / 2) : (shortage * contractCost);
						}
						break;
					}
					default:
						PrefAssert( false );
				}
			}
		} else if( gameResult.Bids[i] == Bid_HalfWhist ) {
			// Half-whist score
			diff.Whists[i][dealer] += contractCost * ((10 - contract) / 2);
		}
	}
	return diff;
}

PlayersScores Bullet::getMisereDiff(const DealResult& gameResult, int dealer) const
{
	PrefAssert( dealer >= 0 && dealer < NumOfPlayers );

	PlayersScores diff;
	if( gameResult.Tricks[dealer] == 0 ) {
		diff.Bullet[dealer] = 10;
	} else {
		diff.Mountain[dealer] = 10 * gameResult.Tricks[dealer];
	}
	return diff;
}

// Normalize score taking into account bullet size
void Bullet::normalizeScores(PlayersScores& diff) const
{
	int overflowPlayer = -1;
	for( int i = 0; i < scores.Bullet.size(); i++ ) {
		if( scores.Bullet[i] + diff.Bullet[i] > settings.BulletSize ) {
			overflowPlayer = i;
		}
	}
	if( overflowPlayer == -1 ) {
		return;
	}
	int remains = scores.Bullet[overflowPlayer] + diff.Bullet[overflowPlayer] - settings.BulletSize;
	// Adding bullet
	GetLog() << "Rem: " << remains << endl;
	int dx = settings.BulletSize - scores.Bullet[overflowPlayer];
	GetLog() << "Dx: " << dx << endl;
	diff.Bullet[overflowPlayer] = dx;
	// Closed yourself - close your partner
	{
		int max, index;
		do {
			max = -1;
			index = 0;
			for( int i = 0; i < diff.Bullet.size(); i++ ) {
				int player = (i + overflowPlayer) % diff.Bullet.size();
				if( scores.Bullet[player] > max && scores.Bullet[player] + diff.Bullet[player] < settings.BulletSize ) {
					max = scores.Bullet[player];
					index = player;
				}
			}
			if( max >= 0 ) {
				dx = min(settings.BulletSize - scores.Bullet[index], remains);
				remains -= dx;
				diff.Bullet[index] += dx;
				diff.Whists[overflowPlayer][index] += dx * 10;
			}
		} while(max >= 0 && remains > 0);
	}
	// Decreasing mountain
	if( remains > 0 ) {
		diff.Mountain[overflowPlayer] -= remains;
	}
	// If mountain is negative
	if( diff.Mountain[overflowPlayer] < 0 ) {
		int dx = -diff.Mountain[overflowPlayer];
		for( int i = 0; i < diff.Mountain.size(); i++ ) {
			diff.Mountain[i] += dx;
		}
	}
}

const vector<DealResult>& Bullet::GetHistory() const 
{
	return history;
}

bool Bullet::IsFinished() const
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		if( scores.Bullet[i] != settings.BulletSize ) {
			return false;
		}
	}
	return true;
}

