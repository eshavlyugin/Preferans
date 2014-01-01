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

#ifndef _PREFERENCE_H__
#define _PREFERENCE_H__

#include <EngineSettings.h>
#include <PrefEngine.h>
#include <PrefPlayer.h>
#include <PrefServer.h>
#include <Connection.h>
#include <PrefGameModel.h>
#include <PrefEngineCallback.h>
#include <PrefTypes.h>
#include <Errors.h>

namespace Preference {

PrefGameModel* CreatePrefModel();

PrefEngine* CreatePrefEngine(const char* dataFilePath, const EngineSettings& settings,
	PrefEngineCallback* callback);

PrefPlayer* CreateAIPrefPlayer(PrefEngine* engine);
PrefPlayer* CreateRemovePrefPlayer(Connection* connection);

PrefServer* CreateLocalPrefServer();
PrefServer* CreateRemovePrefServer(Connection* connection);

} // namespace Preference

#endif // _PREFERENCE_H__
