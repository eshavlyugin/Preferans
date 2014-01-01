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

#ifndef _LOCAL_PREF_SERVER_H_
#define _LOCAL_PREF_SERVER_H_

#include <PrefServer.h>
#include <PrefPlayer.h>
#include <PrefModelCallback.h>
#include <PrefGameModel.h>
#include <GameSettings.h>
#include <RandomLayoutsGenerator.h>

class LocalPrefServer : public PrefServer, public PrefModelCallback {
public:
	LocalPrefServer();
	virtual ~LocalPrefServer();

	// From PrefServer
	virtual bool Connect(PrefPlayer*);
	virtual bool QueueCommand(const ServerCommand&, int id);
	virtual void Run();
	virtual bool ProcessNextCommand();
	virtual bool IsInProgress() const;

protected:
	// From GameStateCallback
	virtual void processNewLayoutStart();
	virtual void processDealStateChanged(DealStateType prevState, DealStateType newState);
	virtual void processPlayerBid(BidType);
	virtual void processPlayerDrop(CardsSet drop, BidType deal);
	virtual void processPlayerMove(Card);
	virtual void processHandOpening(CardsSet set, int player);
	virtual void processPassoutWidowCardOpening(Card card);
	virtual void processWidowOpening(CardsSet widow);
	virtual void processModelChanged();

private:
	struct PlayerInfo {
		PrefPlayer* Player;
		int Id;

		PlayerInfo(PrefPlayer* player, int id) : Player(player), Id(id) {}
	};

	struct Command {
		shared_ptr<const ServerCommand> Command;
		int PlayerNum;
	};
	
	enum PlayMisereResponse {
		PMR_Unknown = 0,
		PMR_PlayByMyself,
		PMR_HandOverCards
	};

	queue<Command> commands;
	array<PlayMisereResponse, NumOfPlayers> playMisereResponses;
	scoped_ptr<PrefGameModel> model;
	vector<PlayerInfo> players;
	scoped_ptr<RandomLayoutsGenerator> generator;
	Layout initialLayout;

	int generatePlayerId();
	int idToPlayerNum(int id);
	void processBulletFinish();
	void processPlayMisereResponse(PlayMisereResponse response, int playerNum);
	void sendMisereCatcherVoteRequest();
	int getPlayerIndex(int statePlayerNum);
	void startNewLayout();
	void startNewGame();
	void addCommandToQueue(const ServerCommand& command, int playerNum);
	// Commands processors
	bool processNextCommand();
	void processCommand(const ServerCommand* command, int playerNum);
	void processBid(BidType, int playerNum);
	void processMove(Card, int playerNum);
	void processDrop(CardsSet, BidType, int playerNum);
	void processSetSettings(const GameSettings& settings);
	int mapModelToRealPlayer(int player) const;
	int mapRealToModelPlayer(int player) const;
	void notifyAll(const ServerMessage& message);
};

#endif // _LOCAL_PREF_SERVER_H_

