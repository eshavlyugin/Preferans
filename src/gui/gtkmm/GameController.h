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

#ifndef _GAME_CONTROLLER_H__
#define _GAME_CONTROLLER_H__

#include <ConnectionManagerCallback.h>
#include <PrefWindow.h>
#include <Settings.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <ConnectionManager.h>
#include <ImagesStorage.h>
#include <queue>

class ConnectionManager;

class GameController : 
	public Preference::PrefPlayer,
	public Preference::PrefEngineCallback,
	public ConnectionManagerCallback
{
public:
	~GameController();
	// singleton access	
	static GameController& Instance();

	void Initialize(Glib::RefPtr<Gtk::Builder>&);

	AppSettings& GetSettings() { return settings; }
	ImagesStorage& GetImagesStorage() { return imagesStorage; }

	// Main window access
	Glib::RefPtr<PrefWindow> GetMainWindow();
	
	// Game signals processors
	void ProcessMove( Preference::Card card );
	void ProcessBid( Preference::BidType bid );
	void ProcessDrop( Preference::CardsSet set, Preference::BidType contract );
	// Error handling
	void ProcessCriticalError(const std::string& errorText);
	// Menu
	// game
	void StartGameWithBots();
	void StartNetworkGame();
	void ConnectToServer();
	// signals
	void QuitApp();
	void ShowAbout();

	const Preference::PrefGameModel* GetGameModel() const { return context.model.get(); }

	// From PrefPlayer
	virtual void OnMessage(const Preference::ServerMessage&);
	virtual void SetId(int playerId, Preference::PrefServer* server);
	// From ConnectionManagerCallback
	virtual void OnNewPlayerConnected(Preference::Connection*);
	virtual void OnConnectionInterrupted(Preference::Connection*);

protected:
	// From PrefEngineCallback
	virtual void onEngineCallback();
	
private:
	AppSettings settings;

	struct GameContext {
		boost::scoped_ptr<Preference::PrefEngine> engine1;
		boost::scoped_ptr<Preference::PrefEngine> engine2;
		boost::scoped_ptr<Preference::PrefPlayer> opponent1;
		boost::scoped_ptr<Preference::PrefPlayer> opponent2;
		boost::scoped_ptr<Preference::PrefServer> server;
		boost::scoped_ptr<Preference::PrefGameModel> model;
		boost::scoped_ptr<ConnectionManager> connectionManager;
		std::queue< boost::shared_ptr<Preference::ServerMessage> > messages;
		int playerId;
		bool isGameFinished;

		GameContext() : playerId(0), isGameFinished(false) {}

		void Clear();
		bool IsGameInProgress() const { return server.get() != 0; }
	} context;

	Glib::RefPtr<PrefWindow> mainWindow;
	ImagesStorage imagesStorage;
	GameController() { settings.Load(); } // Singleton

	void showCriticalErrorMessage( const std::string& msg );
	bool processNextMessage();
	void pumpEvents();
	void doGameLoop();
};

inline void GameController::GameContext::Clear()
{
	server.reset();
	opponent1.reset();
	opponent2.reset();
	model.reset();
	connectionManager.reset();
	while( !messages.empty() ) {
		messages.pop();
	}
	playerId = 0;
	isGameFinished = false;
}

#endif // _GAME_CONTROLLER_H__

