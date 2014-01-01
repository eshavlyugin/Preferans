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

#ifndef _PREF_SLOTS_H__
#define _PREF_SLOTS_H__

#include <ImagesStorage.h>
#include <Settings.h>

namespace PrefSlots {
	// timer processor
	bool onUpdateViewTimer();
	// Events processors
	// quit application
	void quit();
	// show about dialog
	void about();
	// start new game with 2 computers
	void startGameWithBots();
	// start network game. In development
	void startNetworkGame();
	// view events processors
	void processCardClicked( Preference::Card );
	void processBidClicked( Preference::BidType );
	// click anywhere on view that doesn't belong to any category of clicks above
	void processClickOnView();
	//  move processors
	void doGameMove( Preference::Card );
	void doGameBid( Preference::BidType );
	void doGameDrop( Preference::CardsSet drop, Preference::BidType contract );
	// access to game information. Returns 0 if no game is present
	const Preference::PrefGameModel* getGameModel();
	// access to settings
	AppSettings& getSettings();

	ImagesStorage& getImagesStorage();
}

#endif // PREF_SLOTS_H__
