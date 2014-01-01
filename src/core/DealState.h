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

#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#include <GameSettings.h>
#include <DealStateType.h>

class DealState {
public:
	DealState();

	void SetGameSettings(const GameSettings& _settings) { settings = _settings; }
	void Clean();
	DealStateType CurrentStateType() const;
	
	void SetFirstWidowCard(Card card) { firstWidowCard = card; }
	void SetSecondWidowCard(Card card) { secondWidowCard = card; }

	void UpdateOnBid(BidType bid);
	void UpdateOnDrop(const Drop& drop);
	void UpdateOnNewLayout(const Layout& layout, int player);
	void UpdateOnMove(Card move);
	void UpdateOnWidowOpen(CardsSet widow);
	void UpdateOnPassoutWidowCardOpen(Card card);
	void UpdateOnHandOpen(CardsSet hand, int player);

	bool IsValidBid(BidType bid) const;
	bool IsValidMove(Card move) const;
	bool IsValidDrop(const Drop& drop) const;

	// Member access
	int CurrentPlayer() const { return layout.CurrentPlayer; }
	int Dealer() const { return dealer; }
	BidType Contract() const { return maxBid; } 
	Suit MoveSuit() const;
	BidType CurrentBid() const { return maxBid; }
	CardsSet Widow() const { return layout.Widow; }
	Card FirstWidowCard() const { return firstWidowCard; }
	Card SecondWidowCard() const { return secondWidowCard; }
	CardsSet UtilizedCards() const { return layout.UtilizedCards; }
	CardsSet DroppedCards() const { return droppedCards; }
	vector<int> Tricks() const;
	vector<CardsSet> Hands() const;
	vector<BidType> Bids() const;
	vector<Card> CardsOnDesk() const;
	int NumCardsOnDesk() const { return layout.NumCardsOnDesk; }
	int MovesRemaining() const { return layout.MovesRemaining; }
	Card MaxCardOnDesk() const { return layout.MaxCard; }
	bool IsDarkContractGame() const;
	void ProcessMisereCatcherVoteFinished();

private:
	GameSettings settings;
	DealStateType stateType;
	Layout layout;
	array<BidType, NumOfPlayers> playersBids;
	int firstPlayer;
	BidType maxBid;
	int dealer;
	CardsSet droppedCards;
	Card firstWidowCard;
	Card secondWidowCard;
	
	void processGameBidding(BidType bid);
	void processWhistBidding(BidType bid);
	void processOpenOrCloseWhistBidding(BidType bid);
	bool isValidGameBid(BidType bid) const;
	bool isValidWhistBid(BidType bid) const;
	bool isValidMove(Card move) const;
	bool isValidDrop(const Drop& drop) const;
	void clean();
};

#endif // _GAME_STATE_H_

