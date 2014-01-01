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

#include <GameController.h>
#include <PrefWindow.h>
#include <ConnectionManager.h>
#include <iostream>

GameController::~GameController()
{
	settings.Save();
}

GameController& GameController::Instance()
{
	static GameController controller;
	return controller;
}

void GameController::Initialize(Glib::RefPtr<Gtk::Builder>& builder)
{
	PrefWindow* window = 0;	
	builder->get_widget_derived("mainWindow", window);
	PrefAssert( window != 0 );
	mainWindow = Glib::RefPtr<PrefWindow>(window);
	std::vector<std::string> deckNames = imagesStorage.GetAvailableDecks();
	imagesStorage.SetActiveDeck(deckNames[0]);
}

void GameController::ProcessMove( Preference::Card card )
{
	PrefAssert(context.server != 0);
	mainWindow->SetMoveMode(false);
	context.server->QueueCommand(Preference::MoveCommand(card), context.playerId);
}

void GameController::doGameLoop()
{
	GetLog() << "doGameLoop" << std::endl;
	while( context.server.get() != 0 && !context.isGameFinished && mainWindow->is_visible() ) {
		// pumping gtk events
		int eventsToProcess = 100;
		while( Gtk::Main::events_pending() && eventsToProcess > 0) {
			Gtk::Main::iteration(true);
			eventsToProcess--;
		}
		bool needBlock = true;
		// if server has unprocessed commands - don't block current iteration
		needBlock = needBlock && context.server.get() != 0 && !context.server->ProcessNextCommand();
		// if messages queue is empty or we cannot process message now (waiting for timer or 
		// waiting for click) then block current iteration.
		needBlock = needBlock && context.server.get() != 0 && !processNextMessage();
		needBlock = needBlock && !context.isGameFinished;
		if( needBlock && mainWindow->is_visible() ) {
			// lock until next gtk iteration (server queue and message queue are empty)
			// context.server.get() condition required in case engine throws exception
			// and game cannot go on
			Gtk::Main::iteration(true);
		}
	}
}

void GameController::ProcessBid( Preference::BidType bid )
{
	PrefAssert( context.server != 0 );
	mainWindow->SetMoveMode(false);
	context.server->QueueCommand(Preference::BidCommand(bid), context.playerId);
}

Glib::RefPtr<PrefWindow> GameController::GetMainWindow()
{
	return mainWindow;
}

void GameController::ProcessDrop( Preference::CardsSet cards, Preference::BidType contract )
{
	PrefAssert(context.server != 0);
	mainWindow->SetMoveMode(false);
	context.server->QueueCommand(Preference::DropCommand(contract, cards), context.playerId);
}

void GameController::ProcessCriticalError(const std::string& errorText)
{
	Gtk::MessageDialog dialog("Fatal error. The current game will be terminated. Description: " + errorText,
		false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
	dialog.run();
	context.Clear();
	mainWindow->OnGameFinished();
}

void GameController::StartGameWithBots()
{
	Preference::EngineSettings engineSettings;
	engineSettings.MaxMisereFailures = 2;
	engineSettings.MinMisereWins = 10;
	engineSettings.ContractGameChecks = 12;
	engineSettings.NumOfSamplesPerMove = 15;
	engineSettings.NumOfPassoutLayouts = 8;
	engineSettings.NumOfPassoutSimulations = 2000;

	Preference::GameSettings gameSettings;
	gameSettings.FirstMoveInDark = false;
	gameSettings.BulletSize = 25;
	settings.gameSettings = gameSettings;

	context.engine1.reset( Preference::CreatePrefEngine("PreferansData/engine.dat", engineSettings, this) );
	context.engine1->SetEngineCallback(this);
	context.engine2.reset( Preference::CreatePrefEngine("PreferansData/engine.dat", engineSettings, this) );
	context.engine2->SetEngineCallback(this);
	context.opponent1.reset( Preference::CreateAIPrefPlayer( context.engine1.get() ) );
	context.opponent2.reset( Preference::CreateAIPrefPlayer( context.engine2.get() ) );
	context.model.reset( Preference::CreatePrefModel() );
	context.server.reset( Preference::CreateLocalPrefServer () );
	context.server->Connect(this);
	context.server->Connect(context.opponent1.get());
	context.server->Connect(context.opponent2.get());
	context.server->QueueCommand(Preference::SetSettingsCommand(settings.gameSettings), context.playerId);
	context.server->QueueCommand(Preference::StartServerCommand(), context.playerId);
	// We must execute server intialization commands before initialization of gui 
	PrefAssert(context.server->ProcessNextCommand());
	PrefAssert(context.server->ProcessNextCommand());

	mainWindow->OnNewGameStarted();
	doGameLoop();

	context.Clear();
}

// From PrefPlayer
void GameController::OnMessage(const Preference::ServerMessage& message)
{
	PrefAssert( context.model != 0 );
	context.messages.push(boost::shared_ptr<Preference::ServerMessage>(message.Clone()));
}

bool GameController::processNextMessage()
{
	if( context.messages.empty() || !mainWindow->CanAcceptChanges() ) {
		return false;
	}
	boost::shared_ptr<Preference::ServerMessage> message = context.messages.front();
	context.messages.pop();
	GetLog() << "GameController: Message received " << message->Type() << std::endl;
	switch( message->Type() ) {
		case Preference::SMT_GameSettings:
		{
			const Preference::GameSettingsMessage* msg = static_cast
				<const Preference::GameSettingsMessage*>(message.get());
			context.model->SetSettings(msg->Settings);
			GetLog() << "Bullet size: " << msg->Settings.BulletSize << std::endl;
			break;
		}
		case Preference::SMT_BulletStarted:
		{
			const Preference::BulletStartedMessage* msg = static_cast
				<const Preference::BulletStartedMessage*>(message.get());
			context.model->SetSettings(msg->Settings);
			context.model->StartNewBullet();
			break;
		}
		case Preference::SMT_Bid:
		{
			const Preference::BidMessage* msg = static_cast<const Preference::BidMessage*>(message.get());
			context.model->ProcessBid(msg->Bid);
			mainWindow->OnModelChanged(200);
			GetLog() << "Bid: " << msg->Bid << std::endl;
			break;
		}
		case Preference::SMT_Move:
		{
			Preference::Card card = static_cast<Preference::MoveMessage*>(message.get())->Value;
			int currentPlayer = context.model->CurrentPlayer();
			int numOfMove = context.model->NumCardsOnDesk();
			context.model->ProcessMove(card);
			mainWindow->OnMove(card, currentPlayer, numOfMove == 2);
			break;
		}
		case Preference::SMT_Drop:
		{
			const Preference::DropMessage* msg = static_cast
				<const Preference::DropMessage*>(message.get());
			context.model->ProcessDrop(Preference::Drop(msg->Cards, msg->Contract));
			mainWindow->OnModelChanged(200);
			break;
		}
		case Preference::SMT_WidowOpened:
		{
			const Preference::WidowOpenedMessage* msg = static_cast
				<const Preference::WidowOpenedMessage*>(message.get());
			context.model->ProcessWidowOpening(msg->Widow);
			mainWindow->OnWidowOpened(context.model->FirstWidowCard(), context.model->SecondWidowCard());
			break;
		}
		case Preference::SMT_PassoutWidowCardOpened:
			context.model->ProcessPassoutWidowCardOpening(static_cast
				<Preference::PassoutWidowCardOpenedMessage*>(message.get())->Value);
			mainWindow->OnModelChanged();
			break;
		case Preference::SMT_HandOpened:
		{
			const Preference::HandOpenedMessage* msg = static_cast
				<const Preference::HandOpenedMessage*>(message.get());
			context.model->ProcessHandOpening(msg->Cards, (msg->PlayerTurn + context.model->FirstHand()) % Preference::NumOfPlayers);
			break;
		}
		case Preference::SMT_MoveRequest:
		{
			mainWindow->SetMoveMode(true);
			mainWindow->OnModelChanged();
			break;
		}
		case Preference::SMT_NewLayout:
		{
			const Preference::NewLayoutMessage* msg = static_cast
				<const Preference::NewLayoutMessage*>(message.get());
			context.model->ProcessNewLayout(msg->Cards, Preference::EmptyCardsSet, Preference::EmptyCardsSet,
				(Preference::NumOfPlayers - msg->YourTurn) % Preference::NumOfPlayers);
			mainWindow->OnModelChanged();
			break;
		}
		case Preference::SMT_PlayMisereRequest:
		{
			context.server->QueueCommand(Preference::PlayMisereResponseCommand(true), context.playerId);	
			break;
		}
		case Preference::SMT_MisereCatcherChoosed:
		{
			const Preference::MisereCatcherChoosedMessage* msg = static_cast
				<const Preference::MisereCatcherChoosedMessage*>(message.get());
			int catcher = (Preference::NumOfPlayers - msg->Catcher) % Preference::NumOfPlayers;
			int spectator = -1;
			for( int i = 0; i < Preference::NumOfPlayers; i++ ) {
				if( i != context.model->Dealer() && i != catcher ) {
					spectator = i;
					break;
				}
			}
			PrefAssert( spectator >= 0 );
			context.model->HandOverCards(spectator, catcher);
			mainWindow->OnModelChanged();
			break;
		}
		case Preference::SMT_Error:
		{
			const Preference::ErrorMessage* msg = static_cast<const Preference::ErrorMessage*>(message.get());
			GetLog() << "Error: " << msg->ErrorText << std::endl;
			PrefAssert(false);
			break;
		}
		case Preference::SMT_BulletFinished:
			mainWindow->OnGameFinished();
			context.isGameFinished = true;
			break;
		default:
			GetLog() << "GameController Warning: unknown message with ID " << message->Type() << std::endl;
			break;
	}
	if( context.model->GetDealStateType() == Preference::GST_Finished ) {
		mainWindow->OnDealFinished();
	}
	return true;
}

void GameController::SetId(int _playerId, Preference::PrefServer* server)
{
	context.playerId = _playerId;
}

// From ConnectionManagerCallback
void GameController::OnNewPlayerConnected(Preference::Connection*)
{
}

void GameController::OnConnectionInterrupted(Preference::Connection*)
{
}

void GameController::StartNetworkGame()
{
}

void GameController::ConnectToServer()
{
}

void GameController::onEngineCallback()
{
	GetLog() << "onEngineCallback" << std::endl;
	processNextMessage();
	// pumping gui events
	while( Gtk::Main::events_pending() ) {
		Gtk::Main::iteration(false);
	}
}

void GameController::QuitApp()
{
	mainWindow->hide();
}

void GameController::ShowAbout()
{
	Gtk::AboutDialog dialog;
	dialog.set_version("0.01");
	dialog.set_authors(std::vector<Glib::ustring>(1, "Eugene Shavlyugin (eshavlyugin@gmail.com)"));
	dialog.run();
}


