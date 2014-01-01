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

#include <MonteCarloSolver.h>
#include <DBConverter.h>
#include <EngineData.h>
#include <PrefServer.h>
#include <PrefPlayer.h>
#include <PrefEngine.h>
#include <SolversTest.h>
#include <Preference.h>

int main(int argc, char* argv[])
{
#ifdef RUN_TESTS
	DoPassoutTest();
	DoSolversTest();
	DoBulletTest();
	DoEngineTest();
	DoEngineDataTest();
	DoGameStateTest();
	DoServerTest();
	return 0;
#endif

#ifdef RUN_GAME 
		EngineSettings engineSettings;
		engineSettings.MaxMisereFailures = 2;
		engineSettings.MinMisereWins = 6;
		engineSettings.ContractGameChecks = 8;
		engineSettings.NumOfSamplesPerMove = 10;
		engineSettings.NumOfPassoutLayouts = 8;
		engineSettings.NumOfPassoutSimulations = 3000;

		GameSettings gameSettings;
		gameSettings.FirstMoveInDark = false;
		EngineSettings bot1 = engineSettings;
		gameSettings.BulletSize = 20;

		// Creating all game instances 
		boost::scoped_ptr<PrefServer> _server( Preference::CreateLocalPrefServer() );
		server = _server.get();
		boost::scoped_ptr<PrefEngine> engine1( Preference::CreatePrefEngine("../../../data/engine.dat", bot1, this) );
		boost::scoped_ptr<PrefEngine> engine2( Preference::CreatePrefEngine("../../../data/engine.dat", bot2, this) );
		boost::scoped_ptr<PrefEngine> engine3( Preference::CreatePrefEngine("../../../data/engine.dat", bot3, this) );
		boost::scoped_ptr<PrefPlayer> player1( Preference::CreateAIPrefPlayer( engine1.get() ) );
		boost::scoped_ptr<PrefPlayer> player2( Preference::CreateAIPrefPlayer( engine2.get() ) );
		boost::scoped_ptr<PrefPlayer> player3( Preference::CreateAIPrefPlayer( engine3.get() ) );
		playerId = 0;
		if( !server->Connect(player3.get()) 
			|| !server->Connect(player1.get()) 
			|| !server->Connect(player2.get()) ) 
		{
			return;
		}
		// Running message loop
		server->QueueCommand(SetSettingsCommand(gameSettings), playerId);
		server->QueueCommand(StartServerCommand(), playerId);
#endif // RUN_GAME

#ifdef BUILD_DATABASE
	DBConverter converter;
	if( argc != 3 ) {
		GetLog() << "Usage: BuildDatabase [path to raw data file] [target path]" << endl;
		return 1;
	}
	converter.Convert(argv[1], argv[2]);
	EngineData db;
#endif // BUILD_DATABASE

#ifdef SOLVERSTEST
#endif // SOLVERSTEST

	return 0;
}

