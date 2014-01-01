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

#ifndef _PREF_GAME_MODEL_H__
#define _PREF_GAME_MODEL_H__

#include <PrefGameModel.h>
#include <PrefModelCallback.h>
#include <Bullet.h>
#include <GameSettings.h>
#include <DealState.h>

class PrefGameModelImpl : public PrefGameModel {
public:
	PrefGameModelImpl();
	virtual ~PrefGameModelImpl(); 

	// From PrefGameModel
	virtual void SetCallback(PrefModelCallback* _callback); 

	virtual void SetSettings(const GameSettings& _settings);
	virtual const GameSettings& GetSettings() const;
	virtual void StartNewBullet();
	virtual bool IsBulletFinished() const;

	virtual bool ProcessBid(BidType bid);
	virtual bool ProcessDrop(const Drop& drop);
	virtual bool ProcessNewLayout(CardsSet hand1, CardsSet hand2, CardsSet hand3, int firstHand);
	virtual bool ProcessMove(Card move);
	virtual bool ProcessWidowOpening(CardsSet widow);
	virtual bool ProcessPassoutWidowCardOpening(Card card);
	virtual bool ProcessHandOpening(CardsSet hand, int player);
	virtual bool ProcessMisereCatcherChoosed(int catcher);

	virtual Suit PrevMoveSuit() const;
	virtual int FirstHand() const;
	virtual bool IsValidBid(BidType bid) const;
	virtual bool IsValidMove(Card move) const;
	virtual bool IsValidDrop(const Drop& drop) const;
	virtual void HandOverCards(int playerFrom, int playerTo);
	virtual int PlayerCardsOwner( int player ) const;
	virtual int CurrentPlayer() const;
	virtual int PrevPlayer() const;
	virtual int Dealer() const;
	virtual BidType Contract() const;
	virtual CardsSet GetPlayerCards(int player) const;
	virtual CardsSet Widow() const; 
	virtual Card FirstWidowCard() const; 
	virtual bool HasCardsOfSuit(int player, Suit suit) const;
	virtual int GetPlayerTricks(int player) const;
	virtual Card SecondWidowCard() const;
	virtual Card GetCardOnDesk(int player) const;	
	virtual int GetPlayerCardsCount(int player) const;
	virtual BidType GetPlayerBid(int player) const;
	virtual BidType MaxBid() const;
	virtual CardsSet UtilizedCards() const;
	virtual CardsSet DroppedCards() const;
	virtual DealStateType GetDealStateType() const;
	virtual Suit CurrentMoveSuit() const;
	virtual Card MaxCardOnDesk() const;
	virtual int DealMovesRemaining() const;
	virtual int NumCardsOnDesk() const;

	virtual int GetWhists(int playerFrom, int playerTo) const;
	virtual int GetMountain(int player) const;
	virtual int GetBullet(int player) const;
	virtual vector<float> EvaluateDealResult(const DealResult& deal) const;

private:
	vector< vector<bool> > emptyHands;
	// Player responsible for given hand. Usually, a[player] == player.
	// In case of misere or open whist in contract game one of players hands over
	// his card to another. In that case a[player] == num of player which moves for hand player
	array<int, NumOfPlayers> handPlayers;
	// Score table
	Bullet bullet;
	// Object representing current deal state
	DealState dealState;
	// Object representing previous deal state (right before last move)
	DealState prevState;
	// First hand of current deal
	int firstHand;

	GameSettings settings;
	PrefModelCallback* callback;

	bool isInProgress;
	bool hasFullInformation;

	void processDealStateChanged( DealStateType oldState, DealStateType newState );
	void handOverPasserHand();
};

#endif // _PREF_GAME_MODEL_H__

