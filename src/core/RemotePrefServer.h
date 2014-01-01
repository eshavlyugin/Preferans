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

#ifndef _REMOTE_PREF_SERVER_H_
#define _REMOVE_PREF_SERVER_H_

#include <PrefServer.h>
#include <PrefPlayer.h>
#include <Connection.h>

class RemotePrefServer : public PrefServer {
public:
	RemotePrefServer(Connection*);
	virtual ~RemotePrefServer();
	
	// From PrefServer
	virtual bool Connect(PrefPlayer* player);
	virtual bool QueueCommand(const ServerCommand& command, int id);
	virtual bool ProcessNextCommand();
	virtual void Run();
	virtual bool IsInProgress() const;
};

#endif // _REMOTE_PREF_SERVER_H_
