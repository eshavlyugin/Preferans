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

#include <AIPrefPlayer.h>
#include <PrefEngine.h>
#include <Preference.h>
#include <Errors.h>

AIPrefPlayer::AIPrefPlayer(PrefEngine* _engine) : engine(_engine), server(0), id(0)
{
	model.reset( CreatePrefModel() );
	engine->SetModel(model.get());
	model->SetCallback(engine);
}

AIPrefPlayer::~AIPrefPlayer()
{
}

void AIPrefPlayer::OnMessage(const ServerMessage& _message)
{
	assert( id != 0 );
	GetLog() << "AIPrefPlayer" << id << "::OnMessage(" << _message.Type() << "), " << model->CurrentPlayer() << std::endl;
	switch( _message.Type() ) {
		case SMT_Bid:
		{
			const BidMessage& message = static_cast<const BidMessage&>( _message );
			updateOnBid( message.Bid );
			break;
		}
		case SMT_Move:
		{
			const MoveMessage& message = static_cast<const MoveMessage&>( _message );
			updateOnMove( message.Value );
			break;
		}
		case SMT_Drop:
		{
			const DropMessage& message = static_cast<const DropMessage&>( _message );
			updateOnDrop(message.Contract, message.Cards);
			break;
		}
		case SMT_PassoutWidowCardOpened:
		{
			const PassoutWidowCardOpenedMessage& message = static_cast<const PassoutWidowCardOpenedMessage&>( _message );
			onPassoutWidowCardOpened(message.Value);
			break;
		}
		case SMT_WidowOpened:
		{
			const WidowOpenedMessage& message = static_cast<const WidowOpenedMessage&>( _message );
			onWidowOpened(message.Widow);
			break;
		}
		case SMT_NewLayout:
		{
			const NewLayoutMessage& message = static_cast<const NewLayoutMessage&>( _message );
			onNewLayout(message.Cards, message.YourTurn);
			break;
		}
		case SMT_PlayMisereRequest:
		{
			server->QueueCommand(PlayMisereResponseCommand(RandomNextInt(2) == 0), id);
			break;
		}
		case SMT_BulletStarted:
		{
			const BulletStartedMessage& message = static_cast<const BulletStartedMessage&>( _message );
			onNewBulletStarted( message.Settings );
			break;
		}
		case SMT_HandOpened:
		{
			const HandOpenedMessage& message = static_cast<const HandOpenedMessage&>( _message );
			updateOnHandOpened( message.Cards, message.PlayerTurn );
			break;
		}
		case SMT_MoveRequest:
		{
			processMoveRequest();
			break;
		}
		case SMT_Error:
		{
			const ErrorMessage& message = static_cast<const ErrorMessage&>( _message );
			GetLog() << "AIPrefPlayer error msg: " << message.ErrorText << endl;
			break;
		}
		case SMT_MisereCatcherChoosed:
		{
			const MisereCatcherChoosedMessage& message = static_cast<const MisereCatcherChoosedMessage&>( _message );
			model->ProcessMisereCatcherChoosed((model->FirstHand() + message.Catcher) % NumOfPlayers);
			break;
		}
		default:
			break;
			// Message is ignored. 
			// TODO: Maybe add any warnings?
	}
}

// TODO: add possibility of execution of engine code in separate thread
void AIPrefPlayer::processMoveRequest()
{
	makeMove();
}

void AIPrefPlayer::SetId(int playerId, PrefServer* _server)
{
	id = playerId;
	server = _server;
}

void AIPrefPlayer::updateOnHandOpened(CardsSet hand, int playerTurn)
{
	model->ProcessHandOpening(hand, (model->FirstHand() + playerTurn) % NumOfPlayers);
}

void AIPrefPlayer::updateOnBid(BidType bid)
{
	model->ProcessBid(bid);
}

void AIPrefPlayer::updateOnMove(Card move)
{
	model->ProcessMove(move);
}

void AIPrefPlayer::updateOnDrop(BidType bid, CardsSet cards)
{
	model->ProcessDrop(Drop(cards, bid));
}

void AIPrefPlayer::onNewLayout(CardsSet set, int yourTurn)
{

	Layout layout;
	layout.Cards[0] = set;
	model->ProcessNewLayout(layout.Cards[0], layout.Cards[1], layout.Cards[2], 
		(NumOfPlayers - yourTurn) % NumOfPlayers);
}

void AIPrefPlayer::onNewBulletStarted(const GameSettings& settings)
{
	model->SetSettings(settings);
}

void AIPrefPlayer::onPassoutWidowCardOpened(Card card)
{
	model->ProcessPassoutWidowCardOpening(card);
}
	
void AIPrefPlayer::onWidowOpened(CardsSet widow)
{
	GetLog() << "AIPrefPlayer::onWidowOpened " << widow.Value << endl;
	model->ProcessWidowOpening(widow);
}

void AIPrefPlayer::makeMove()
{
	GetLog() << "AIPrefPlayer::makeMove()" << endl;
	GetLog() << "Hand: " << model->GetPlayerCards(model->CurrentPlayer()).Value << endl;
	switch( model->GetDealStateType() ) {
		case GST_Passout:
		case GST_Misere:
		case GST_ContractGame:
		{
			Card move = engine->DoMove();
			PrefAssert( model->IsValidMove(move) );
			server->QueueCommand(MoveCommand(move), id);
			break;
		}
		case GST_Bidding:
		case GST_Whisting:
		case GST_OpenOrCloseWhist:
		{
			BidType bid = engine->DoBid();
			PrefAssert( model->IsValidBid(bid) );
			GetLog() << "AIPrefPlayer" << bid << std::endl;
			server->QueueCommand(BidCommand(bid), id);
			break;
		}
		case GST_Drop:
		{
			Drop drop = engine->DoDrop();
			PrefAssert( model->IsValidDrop(drop) );
			server->QueueCommand(DropCommand(drop.Contract, drop.Cards), id);
			break;
		}
		default:
			PrefAssert(false);
	}
}

