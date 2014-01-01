/****************************************************************************
 Preferans: implementation of card-tricking game Preferans (or Preference).
 ****************************************************************************
 Copyright (c) 2010-2011 Eugene Shavlyugin <eshavlyugin@gmail.com>
 
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

#include <Preference.h>
#include <PrefEngineImpl.h>
#include <LocalPrefServer.h>
#include <RemotePrefServer.h>
#include <AIPrefPlayer.h>
#include <RemotePrefPlayer.h>
#include <PrefGameModelImpl.h>

namespace Preference {

PrefGameModel* CreatePrefModel()
{
	GetLog() << "Pref model was created" << endl;
	return new PrefGameModelImpl();
}

PrefEngine* CreatePrefEngine(const char* dataFilePath, const EngineSettings& settings, PrefEngineCallback* callback)
{
	string path(dataFilePath);
	PrefEngineImpl* result = new PrefEngineImpl(dataFilePath);
	GetLog() << "Pref engine was created" << endl;
	result->SetEngineSettings(settings);
	result->SetCallback(callback);
	return result;
}

PrefPlayer* CreateAIPrefPlayer(PrefEngine* engine)
{
	AIPrefPlayer* result = new AIPrefPlayer(engine);
	GetLog() << "AI pref player was created" << endl;
	return result;
}

PrefPlayer* CreateRemovePrefPlayer(Connection* connection)
{
	RemotePrefPlayer* result = new RemotePrefPlayer(connection);
	return result;
}

PrefServer* CreateRemotePrefServer(Connection* connection)
{
	RemotePrefServer* result = new RemotePrefServer(connection);
	return result;
}

PrefServer* CreateLocalPrefServer()
{
	LocalPrefServer* result = new LocalPrefServer();
	GetLog() << "Local Pref Server was created" << std::endl;
	return result;
}

}
