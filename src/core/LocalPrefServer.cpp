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

#include <LocalPrefServer.h>
#include <RandomLayoutsGenerator.h>
#include <Errors.h>
#include <Preference.h>

static const int InvalidPlayerNum = -1;

LocalPrefServer::LocalPrefServer() 
{
	generator.reset( new RandomLayoutsGenerator() );
	generator->Reset();
	model.reset( CreatePrefModel() );
	model->SetCallback(this);
}

LocalPrefServer::~LocalPrefServer()
{
}

bool LocalPrefServer::Connect(PrefPlayer* player)
{
	if( players.size() >= 3 ) {
		return false;
	}

	int id = generatePlayerId();
	player->SetId(id, this);
	player->OnMessage(GameSettingsMessage(model->GetSettings()));
	players.push_back(PlayerInfo(player, id));
	return true;
}

int LocalPrefServer::mapModelToRealPlayer(int player) const
{
	return (player + model->FirstHand()) % NumOfPlayers;
}

int LocalPrefServer::mapRealToModelPlayer(int player) const
{
	return (NumOfPlayers + player - model->FirstHand()) % NumOfPlayers;
}

void LocalPrefServer::processNewLayoutStart()
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		players[mapModelToRealPlayer((model->FirstHand() + i) % NumOfPlayers)].Player->OnMessage(
			NewLayoutMessage(initialLayout.Cards[(model->FirstHand() + i) % NumOfPlayers], i));
	}
}

void LocalPrefServer::processDealStateChanged(DealStateType prevState, DealStateType newState)
{
}

void LocalPrefServer::processPlayerBid(BidType bid)
{
	notifyAll(BidMessage(bid));
}

void LocalPrefServer::processPlayerDrop(CardsSet drop, BidType deal)
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		players[mapModelToRealPlayer(i)].Player->OnMessage(
			DropMessage(deal, i == model->Dealer() ? drop : EmptyCardsSet));
	}
	notifyAll(DropMessage(deal, drop) );
}

void LocalPrefServer::processPlayerMove(Card move)
{
	notifyAll( MoveMessage(move) );
}

void LocalPrefServer::processHandOpening(CardsSet set, int player)
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		players[i].Player->OnMessage(
			HandOpenedMessage(set, (NumOfPlayers + player - model->FirstHand()) % NumOfPlayers));
	}
}

void LocalPrefServer::processModelChanged()
{
	DealStateType type = model->GetDealStateType();
	switch( type ) {
		case GST_MisereCatcherVote:
		{
			sendMisereCatcherVoteRequest();
			break;
		}
		case GST_Finished:
			if( model->IsBulletFinished() ) {
				processBulletFinish();
			} else {
				startNewLayout();
			}
			break;
		default:
			players[mapModelToRealPlayer(model->PlayerCardsOwner(model->CurrentPlayer()))].Player->
				OnMessage( MoveRequestMessage() );
			break;
	}
}

void LocalPrefServer::processPassoutWidowCardOpening(Card card)
{
	notifyAll( PassoutWidowCardOpenedMessage(card) );
}

void LocalPrefServer::processWidowOpening(CardsSet widow)
{
	notifyAll( WidowOpenedMessage( widow ) );
}

void LocalPrefServer::processSetSettings(const GameSettings& settings)
{
	model->SetSettings(settings);
	for( int i = 0; i < players.size(); i++ ) {
		players[i].Player->OnMessage(GameSettingsMessage(settings));
	}
}

bool LocalPrefServer::QueueCommand(const ServerCommand& command, int id)
{
	int playerNum = idToPlayerNum(id);
	if( playerNum == InvalidPlayerNum ) {
		return false;
	}

	addCommandToQueue(command, playerNum);
	return true;
}

void LocalPrefServer::addCommandToQueue(const ServerCommand& command, int playerNum) 
{
	Command cmd;
	cmd.PlayerNum = playerNum;
	cmd.Command.reset(command.Clone());
	commands.push(cmd);
}

void LocalPrefServer::processCommand(const ServerCommand* _command, int playerNum)
{
	GetLog() << "LocalPrefServer: Processing command of type " << _command->Type() << " from player " << playerNum << endl;
	switch( _command->Type() ) {
		case SCT_SetSettings:
		{
			const SetSettingsCommand* command = static_cast<const SetSettingsCommand*>( _command );
			processSetSettings(command->Settings);
			break;
		}
		case SCT_Bid:
		{
			const BidCommand* command = static_cast<const BidCommand*>( _command );
			processBid(command->Bid, playerNum);
			break;
		}
		case SCT_Drop:
		{
			const DropCommand* command = static_cast<const DropCommand*>(_command);
			processDrop(command->Cards, command->Contract, playerNum);
			break;
		}
		case SCT_Move:
		{
			const MoveCommand* command = static_cast<const MoveCommand*>(_command);
			processMove(command->Move, playerNum);
			break;
		}
		case SCT_StartServer:
		{
			startNewGame();
			break;
		}
		case SCT_PlayMisereResponse:
		{
			const PlayMisereResponseCommand* command = static_cast<const PlayMisereResponseCommand*>(_command);
			processPlayMisereResponse(command->PlayByMyself ? PMR_PlayByMyself : PMR_HandOverCards, playerNum);
			break;
		}
		default:
			GetLog() << "Warning: unsupported command of type " << _command->Type() << " will be ignoerd." << endl;
			break;
	}
}

void LocalPrefServer::processPlayMisereResponse( PlayMisereResponse response, int playerNum )
{
	if( model->GetDealStateType() != GST_MisereCatcherVote
		|| model->GetPlayerBid(mapRealToModelPlayer(playerNum)) == Bid_Misere
		|| playMisereResponses[mapRealToModelPlayer(playerNum)] != PMR_Unknown )
	{
		players[playerNum].Player->OnMessage( ErrorMessage("Invalid command") );
		return;
	}

	playMisereResponses[mapRealToModelPlayer(playerNum)] = response;

	int numOfResponses = 3;
	int numOfPlayByMyselfs = 0;
	int player = -1;
	int passer = -1;
	for( int i = 0; i < 3; i++ ) {
		switch( playMisereResponses[i] ) {
			case PMR_Unknown:
				numOfResponses--;
				break;
			case PMR_PlayByMyself:
				numOfPlayByMyselfs++;
				player = i;
				break;
			case PMR_HandOverCards:
				passer = i;
				break;
			default:
				PrefAssert(false);
		}
	}

	if( numOfResponses == 2 ) {
		if( numOfPlayByMyselfs == 1 ) {
			PrefAssert( passer != -1 && player != -1 );
			for( int i = 0; i < NumOfPlayers; i++ ) {
				players[i].Player->OnMessage(MisereCatcherChoosedMessage((NumOfPlayers + player - model->FirstHand()) % NumOfPlayers));
			}
			model->HandOverCards( passer, player );
		} else {
			sendMisereCatcherVoteRequest();
		}
	}
}

void LocalPrefServer::sendMisereCatcherVoteRequest()
{
	for( int i = 0; i < NumOfPlayers; i++ ) {
		playMisereResponses[i] = PMR_Unknown;
		if( model->GetPlayerBid(i) != Bid_Misere ) {
			players[mapModelToRealPlayer(i)].Player->OnMessage(
				PlayMisereRequestMessage());
		}
	}
}

void LocalPrefServer::Run()
{
	while( !model->IsBulletFinished() ) {
		processNextCommand();
	}
}

bool LocalPrefServer::ProcessNextCommand()
{
	processNextCommand();
}

bool LocalPrefServer::processNextCommand()
{
	Command cmd;
	{
		if( commands.empty() ) {
			return false;
		}
		cmd = commands.front();
		commands.pop();
	}
	PrefAssert( cmd.Command.get() != 0 );
	processCommand(cmd.Command.get(), cmd.PlayerNum);
	return true;
}

bool LocalPrefServer::IsInProgress() const
{
	return !model->IsBulletFinished();
}

int LocalPrefServer::generatePlayerId() 
{
	return RandomNextInt();
}

int LocalPrefServer::idToPlayerNum(int id)
{
	for( int i = 0; i < players.size(); i++ ) {
		if( players[i].Id == id ) {
			return i;
		}
	}
	return InvalidPlayerNum;
}

void LocalPrefServer::processBid(BidType bid, int playerNum)
{
	// Checking validness on command
	GetLog() << "LocalPrefServer: bidding, currentPlayer = " << model->CurrentPlayer() << endl;
	if(mapModelToRealPlayer(model->PlayerCardsOwner(model->CurrentPlayer())) != playerNum ) {
		players[playerNum].Player->OnMessage(ErrorMessage("Not your turn"));
		return;
	}
	if( !model->IsValidBid(bid) ) {
		players[playerNum].Player->OnMessage(ErrorMessage("Invalid move"));
		players[playerNum].Player->OnMessage(MoveRequestMessage());
		return;
	}
	// Updating game state
	PrefAssert( model->ProcessBid(bid) );
}

// Send message for all players
void LocalPrefServer::notifyAll(const ServerMessage& message)
{
	for( int i = 0; i < players.size(); i++ ) {
		players[i].Player->OnMessage(message);
	}
}

void LocalPrefServer::processDrop(CardsSet drop, BidType bid, int playerNum)
{
	// Checking validness of move
	if(mapModelToRealPlayer(model->PlayerCardsOwner(model->CurrentPlayer())) != playerNum) {
		players[playerNum].Player->OnMessage(ErrorMessage("Not your turn"));
		return;
	}

	if( !model->IsValidDrop(Drop(drop, bid)) ) {
		players[playerNum].Player->OnMessage(ErrorMessage("Invalid move"));
		players[playerNum].Player->OnMessage(MoveRequestMessage());
		return;
	}

	// Updating state
	PrefAssert( model->ProcessDrop(Drop(drop, bid)) );
}

void LocalPrefServer::processMove(Card card, int playerNum)
{
	// Checking validness of move
	if(mapModelToRealPlayer(model->PlayerCardsOwner(model->CurrentPlayer())) != playerNum) {
		players[playerNum].Player->OnMessage(ErrorMessage("Not your turn"));
		return;
	}

	if( !model->IsValidMove(card) ) {
		// First, sending error notification
		players[playerNum].Player->OnMessage(ErrorMessage("Invalid move"));
		// Second, sending request for move again
		players[playerNum].Player->OnMessage(MoveRequestMessage());
		return;
	}
	// Updating state
	PrefAssert( model->ProcessMove(card) );
}	

void LocalPrefServer::startNewGame()
{
	PrefAssert( model != 0 );
	model->StartNewBullet();
	notifyAll(BulletStartedMessage(model->GetSettings()));
	startNewLayout();
}

void LocalPrefServer::startNewLayout()
{
	// Generating new layout
	Layout layout = generator->GenerateLayouts(1)[0];
	initialLayout = layout;
	// Starting new game
	// Player 0 always moves first
	model->ProcessNewLayout(layout.Cards[0], layout.Cards[1], layout.Cards[2],
		(model->FirstHand() + 1) % NumOfPlayers);
}	

void LocalPrefServer::processBulletFinish()
{
	PrefAssert( model->IsBulletFinished() );
	notifyAll(BulletFinishedMessage());
}

