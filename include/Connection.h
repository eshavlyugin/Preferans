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

#ifndef _ICONNECTION_H_
#define _ICONNECTION_H_

// Connection state. TODO: add more values
//
namespace Preference {

enum ConnectionState {
	CS_Connected = 0,
	CS_Disconnected,
	CS_EnumSize
};

class Connection {
	virtual ~Connection() = 0;

	virtual int Send(const char* data, int size) = 0;
	virtual int Recv(char* data, int size) = 0;
	virtual ConnectionState GetConnectionState() = 0;
};

inline Connection::~Connection() {}

} // namespace Preference

#endif // _ICONNECTION_H_
