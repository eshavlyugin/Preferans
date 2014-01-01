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

#ifndef _GAME_STATE_TYPE_H__
#define _GAME_STATE_TYPE_H__

namespace Preference {

enum DealStateType {
	GST_Unknown,
	GST_Bidding,
	GST_Drop,
	GST_Whisting,
	GST_OpenOrCloseWhist,
	GST_Passout,
	GST_ContractGame,
	GST_Misere,
	GST_MisereCatcherVote,
	GST_Finished
};

}

#endif // _GAME_STATE_TYPE_H__
