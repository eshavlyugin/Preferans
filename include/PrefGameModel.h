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

#ifndef _I_PREF_GAME_MODEL_H__
#define _I_PREF_GAME_MODEL_H__

#include <PrefModelCallback.h>
#include <DealStateType.h>
#include <GameSettings.h>
#include <DealResult.h>
#include <vector>

namespace Preference {

class PrefGameModel {
public:
	virtual ~PrefGameModel();
	
	// Sets model callback
	virtual void SetCallback(PrefModelCallback*) = 0;

	// Set/Get game settings
	virtual void SetSettings(const GameSettings& settings) = 0;
	virtual const GameSettings& GetSettings() const = 0;
	// Start new bullet. Valid only if no game running
	virtual void StartNewBullet() = 0;
	// Check if the game in progress
	virtual bool IsBulletFinished() const = 0;
	// Events processing
	// Game events processors. Each processor returns true is game action is valid
	virtual bool ProcessBid(BidType bid) = 0;
	virtual bool ProcessDrop(const Drop& drop) = 0;
	virtual bool ProcessNewLayout(CardsSet hand1, CardsSet hand2, CardsSet hand3, int firstHand) = 0;
	virtual bool ProcessMove(Card move) = 0;
	virtual bool ProcessWidowOpening(CardsSet widow) = 0;
	virtual bool ProcessPassoutWidowCardOpening(Card card) = 0;
	virtual bool ProcessHandOpening(CardsSet hand, int player) = 0;
	virtual bool ProcessMisereCatcherChoosed(int catcher) = 0;
	// Deal state access
	// First hand of current deal
	virtual int FirstHand() const = 0;
	// Move validness checkers
	virtual bool IsValidBid(BidType bid) const = 0;
	virtual bool IsValidMove(Card move) const = 0;
	virtual bool IsValidDrop(const Drop& drop) const = 0;
	// Number of current player
	virtual int CurrentPlayer() const = 0;
	// Number of game dealer. If there is not dealer yet or
	// game type is passout result may be any integer number
	virtual int Dealer() const = 0;
	// Number of Previous player. -1 if there were not moves in current deal
	virtual int PrevPlayer() const = 0;
	// Maximal bid. See BidType declaration for details. If there were no bids Bid_Unknown returns.
	virtual BidType MaxBid() const = 0;
	// Hand over cards of player "playerFrom" to player "playerTo".
	virtual void HandOverCards(int playerFrom, int playerTo) = 0;
	// Get number of player who currently plays for player "player".
	// May be changed in misere by method HandOverCards
	virtual int PlayerCardsOwner( int player ) const = 0;
	// Get cards set of given player. 
	virtual CardsSet GetPlayerCards(int player) const = 0;
	virtual int GetPlayerCardsCount(int player) const = 0;
	// Game contract. Bid_Pass for passout
	virtual BidType Contract() const = 0;

	virtual Suit PrevMoveSuit() const = 0;
	// Returns true if player "p" has cards of suit "s"
	virtual bool HasCardsOfSuit(int p, Suit s) const = 0;
	// Deal widow or EmptyCardsSet if widow is unknown
	virtual CardsSet Widow() const = 0; 
	// First and second widow cards or Card_Unknown if first widow card is unknown.
	// Note: order of cards is important only in passout game
	virtual Card FirstWidowCard() const = 0; 
	virtual Card SecondWidowCard() const = 0;
	// Get card on desk for player "player" or Card_Unknown if this player hasn't moved yet
	virtual Card GetCardOnDesk(int player) const = 0;	
	// Player last word or Bid_Unknown if player hasn't moved in current deal
	virtual BidType GetPlayerBid(int player) const = 0;
	// Set of utilized cards. 
	virtual CardsSet UtilizedCards() const = 0;
	virtual CardsSet DroppedCards() const = 0;
	// Current number of tricks for current player
	virtual int GetPlayerTricks(int player) const = 0;
	// Deal state type
	virtual DealStateType GetDealStateType() const = 0;
	// Suit of current move
	virtual Suit CurrentMoveSuit() const = 0;
	// Maximal card on desk or Card_Unknown if not players moved in this turn has chance to take a trick
	virtual Card MaxCardOnDesk() const = 0;
	// Number of tricks remains before deal finished
	virtual int DealMovesRemaining() const = 0;
	// Number of cards on desk
	virtual int NumCardsOnDesk() const = 0;
	// Bullet access
	virtual int GetWhists(int playerFrom, int playerTo) const = 0;
	virtual int GetMountain(int player) const = 0;
	virtual int GetBullet(int player) const = 0;
	virtual std::vector<float> EvaluateDealResult(const DealResult& deal) const = 0;
};

inline PrefGameModel::~PrefGameModel() {}

} // namespace preference

#endif // _I_PREF_GAME_MODEL_H__

