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

#ifndef _I_PREF_PLAYER_H_
#define _I_PREF_PLAYER_H_

#include <GameSettings.h>
#include <PrefTypes.h>
#include <PrefServer.h>
#include <string>

namespace Preference {

enum ServerMessageType {
	SMT_Unknown = 0,
	SMT_Id,
	SMT_Bid,
	SMT_Move,
	SMT_Drop,
	SMT_BulletStarted,
	SMT_PassoutWidowCardOpened,
	SMT_WidowOpened,
	SMT_HandOpened,
	SMT_GameSettings,
	SMT_NewLayout,
	SMT_MoveRequest,
	SMT_BulletFinished,
	SMT_PlayMisereRequest,
	SMT_MisereCatcherChoosed,
	SMT_Error
};

struct ServerMessage {
	virtual ~ServerMessage() = 0;

	virtual ServerMessageType Type() const = 0;
	virtual ServerMessage* Clone() const = 0;
};

inline ServerMessage::~ServerMessage() {}

struct IdMessage : public ServerMessage {
	int Id;

	IdMessage(int id) : Id(id) {}
	virtual ~IdMessage() {}

	virtual ServerMessageType Type() const { return SMT_Id; }
	virtual ServerMessage* Clone() const { return new IdMessage(Id); }
};

struct BidMessage : public ServerMessage {
	BidType Bid;

	BidMessage(BidType bid) : Bid(bid) {}
	virtual ~BidMessage() {}

	virtual ServerMessageType Type() const { return SMT_Bid; }
	virtual ServerMessage* Clone() const { return new BidMessage(Bid); }
};

struct MoveMessage : public ServerMessage {
	Card Value;

	MoveMessage(Card value) : Value(value) {}
	virtual ~MoveMessage() {}

	virtual ServerMessageType Type() const { return SMT_Move; }
	virtual ServerMessage* Clone() const { return new MoveMessage(Value); }
};

struct DropMessage : public ServerMessage {
	BidType Contract;
	CardsSet Cards;

	DropMessage(BidType contract, CardsSet cards) : Contract(contract), Cards(cards) {}
	virtual ~DropMessage() {}

	virtual ServerMessageType Type() const { return SMT_Drop; }
	virtual ServerMessage* Clone() const { return new DropMessage(Contract, Cards); }
};

struct BulletStartedMessage : public ServerMessage {
	GameSettings Settings;

	BulletStartedMessage(const GameSettings& settings) : Settings(settings) {}
	virtual ~BulletStartedMessage() {}

	virtual ServerMessageType Type() const { return SMT_BulletStarted; }
	virtual ServerMessage* Clone() const { return new BulletStartedMessage(Settings); }
};

struct PassoutWidowCardOpenedMessage : public ServerMessage {
	Card Value;

	PassoutWidowCardOpenedMessage(Card value) : Value(value) {}
	virtual ~PassoutWidowCardOpenedMessage() {}

	virtual ServerMessageType Type() const { return SMT_PassoutWidowCardOpened; }
	virtual ServerMessage* Clone() const { return new PassoutWidowCardOpenedMessage(Value); }
};

struct WidowOpenedMessage : public ServerMessage {
	CardsSet Widow;

	WidowOpenedMessage(CardsSet widow) : Widow(widow) {}
	virtual ~WidowOpenedMessage() {}

	virtual ServerMessageType Type() const { return SMT_WidowOpened; }
	virtual ServerMessage* Clone() const { return new WidowOpenedMessage(Widow); }
};

struct HandOpenedMessage : public ServerMessage {
	// set of cards
	CardsSet Cards;
	// player turn relative to first player
	int PlayerTurn;

	HandOpenedMessage(CardsSet cards, int playerTurn) : Cards(cards), PlayerTurn(playerTurn) {}
	virtual ~HandOpenedMessage() {}

	virtual ServerMessageType Type() const { return SMT_HandOpened; }
	virtual ServerMessage* Clone() const { return new HandOpenedMessage(Cards, PlayerTurn); }
};

struct NewLayoutMessage : public ServerMessage {
	CardsSet Cards;
	int YourTurn;

	NewLayoutMessage(CardsSet cards, int yourTurn) : Cards(cards), YourTurn(yourTurn) {}
	virtual ~NewLayoutMessage() {}

	virtual ServerMessageType Type() const { return SMT_NewLayout; }
	virtual ServerMessage* Clone() const { return new NewLayoutMessage(Cards, YourTurn); }
};

struct GameSettingsMessage : public ServerMessage {
	GameSettings Settings;

	GameSettingsMessage(const GameSettings& settings) : Settings(settings) {}
	~GameSettingsMessage() {}

	virtual ServerMessageType Type() const { return SMT_GameSettings; }
	virtual ServerMessage* Clone() const { return new GameSettingsMessage(Settings); }
};

struct MoveRequestMessage : public ServerMessage {
	MoveRequestMessage() {}
	virtual ~MoveRequestMessage() {}

	virtual ServerMessageType Type() const { return SMT_MoveRequest; }
	virtual ServerMessage* Clone() const { return new MoveRequestMessage(); }
};

struct ErrorMessage : public ServerMessage {
	std::string ErrorText;

	ErrorMessage(const std::string& errorText) : ErrorText(errorText) {}
	~ErrorMessage() {}

	virtual ServerMessageType Type() const { return SMT_Error; }
	virtual ServerMessage* Clone() const { return new ErrorMessage(ErrorText); }
};

// Request if player ready to play misere or is he hands over his cards.
// Expected behaviour after this message - send PlayMisereResponseCommand to server
struct PlayMisereRequestMessage : public ServerMessage {
	PlayMisereRequestMessage() {}
	virtual ~PlayMisereRequestMessage() {}

	virtual ServerMessageType Type() const { return SMT_PlayMisereRequest; }
	virtual ServerMessage* Clone() const { return new PlayMisereRequestMessage(); }
};

struct MisereCatcherChoosedMessage : public ServerMessage {
	int Catcher;

	MisereCatcherChoosedMessage(int catcher) : Catcher(catcher) {}
	virtual ~MisereCatcherChoosedMessage() {}

	virtual ServerMessageType Type() const { return SMT_MisereCatcherChoosed; }
	virtual ServerMessage* Clone() const { return new MisereCatcherChoosedMessage(Catcher); }
};

struct BulletFinishedMessage : public ServerMessage {
	BulletFinishedMessage() {}
	virtual ~BulletFinishedMessage() {}

	virtual ServerMessageType Type() const { return SMT_BulletFinished; }
	virtual ServerMessage* Clone() const { return new BulletFinishedMessage(); }
};

class PrefPlayer {
public:
	virtual ~PrefPlayer() = 0;

	virtual void OnMessage(const ServerMessage&) = 0;
	virtual void SetId(int playerId, PrefServer* server) = 0;
};

inline PrefPlayer::~PrefPlayer() {}

}

#endif // _I_PREF_PLAYER_H_
