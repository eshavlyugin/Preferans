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

#include <RemotePrefServer.h>
#include <Connection.h>

RemotePrefServer::RemotePrefServer(Connection* conn)
{
}

RemotePrefServer::~RemotePrefServer()
{
}

bool RemotePrefServer::Connect(PrefPlayer* player)
{
	return false;
}

bool RemotePrefServer::QueueCommand(const ServerCommand& command, int id)
{
	return false;
}

void RemotePrefServer::Run()
{
}

bool RemotePrefServer::ProcessNextCommand()
{
}

bool RemotePrefServer::IsInProgress() const
{
	return true;
}

