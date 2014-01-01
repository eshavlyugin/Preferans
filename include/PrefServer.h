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

#ifndef _I_PREF_SERVER_H_
#define _I_PREF_SERVER_H_

#include <GameSettings.h>
#include <PrefTypes.h>

namespace Preference {

class PrefPlayer;

enum ServerCommandType {
	SCT_Unknown = 0,
	SCT_Bid,
	SCT_Move,
	SCT_Drop,
	SCT_PlayMisereResponse,
	SCT_StartServer,
	SCT_SetSettings
};

struct ServerCommand {
	virtual ~ServerCommand() = 0;

	virtual ServerCommandType Type() const = 0;
	virtual ServerCommand* Clone() const = 0;
};

inline ServerCommand::~ServerCommand() {}

//-------------------------------------------------------------------------------------
//
struct BidCommand : public ServerCommand {
	BidType Bid;

	BidCommand(BidType bid) : Bid(bid) {}
	virtual ~BidCommand() {}

	virtual ServerCommandType Type() const { return SCT_Bid; }
	virtual ServerCommand* Clone() const { return new BidCommand(Bid); }
};

struct StartServerCommand : public ServerCommand {
	StartServerCommand() {}
	virtual ~StartServerCommand() {}

	virtual ServerCommandType Type() const { return SCT_StartServer; }
	virtual ServerCommand* Clone() const { return new StartServerCommand(); }
};

//--------------------------------------------------------------------------------------
//
struct MoveCommand : public ServerCommand {
	Card Move;

	MoveCommand(Card move) : Move(move) {}
	virtual ~MoveCommand() {}

	virtual ServerCommandType Type() const { return SCT_Move; }
	virtual ServerCommand* Clone() const { return new MoveCommand(Move); }
};

//-------------------------------------------------------------------------------------
//
struct DropCommand : public ServerCommand {
	BidType Contract;
	CardsSet Cards;

	virtual ~DropCommand() {}
	DropCommand(BidType contract, CardsSet cards) : Contract(contract), Cards(cards) {}

	virtual ServerCommandType Type() const { return SCT_Drop; }
	virtual ServerCommand* Clone() const { return new DropCommand(Contract, Cards); }
};

struct PlayMisereResponseCommand : public ServerCommand {
	bool PlayByMyself;

	PlayMisereResponseCommand(bool playByMyself) : PlayByMyself(playByMyself) {}
	virtual ~PlayMisereResponseCommand() {}

	virtual ServerCommandType Type() const { return SCT_PlayMisereResponse; }
	virtual ServerCommand* Clone() const { return new PlayMisereResponseCommand(PlayByMyself); }
};

struct SetSettingsCommand : public ServerCommand {
	GameSettings Settings;

	virtual ~SetSettingsCommand() {}
	SetSettingsCommand(const GameSettings& settings) : Settings(settings) {}

	virtual ServerCommandType Type() const { return SCT_SetSettings; }
	virtual ServerCommand* Clone() const { return new SetSettingsCommand(Settings); }
};

class PrefServer {
public:
	virtual ~PrefServer() = 0;

	// Try connect to server. False means server is full 
	virtual bool Connect(PrefPlayer* player) = 0;
	// Send command to server
	virtual bool QueueCommand(const ServerCommand& command, int id) = 0;
	// Process one command
	virtual bool ProcessNextCommand() = 0;
	// Run until bullet finished
	virtual void Run() = 0;
	// Return if bullet finished. Calling before initialization causes assert
	virtual bool IsInProgress() const = 0;
};

inline PrefServer::~PrefServer()
{
}

}

#endif // _I_PREF_SERVER_H_

