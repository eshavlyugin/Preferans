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
#include <PrefSlots.h>
#include <iostream>

namespace PrefSlots {
	bool onUpdateViewTimer()
	{
		try {
			return GameController::Instance().GetMainWindow()->OnUpdateViewTimer();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
		return true;
	}

	void quit()
	{
		try {
			GameController::Instance().QuitApp();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void about()
	{
		try {
			GameController::Instance().ShowAbout();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void startGameWithBots()
	{
		try {
			GameController::Instance().StartGameWithBots();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void startNetworkGame()
	{
		try {
			GameController::Instance().StartGameWithBots();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void processClickOnView()
	{
		try {
			GameController::Instance().GetMainWindow()->OnClickOnView();
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void processCardClicked( Preference::Card card )
	{
		try {
			GameController::Instance().GetMainWindow()->OnCardClicked(card);
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void processBidClicked( Preference::BidType bid )
	{
		try {
			GameController::Instance().GetMainWindow()->OnBidClicked(bid);
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void doGameMove( Preference::Card card )
	{
		try {
			GameController::Instance().ProcessMove(card);
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void doGameBid( Preference::BidType bid )
	{
		try {
			GameController::Instance().ProcessBid(bid);
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	void doGameDrop( Preference::CardsSet set, Preference::BidType contract )
	{
		try {
			GameController::Instance().ProcessDrop( set, contract );
		} catch( Preference::Exception& e ) {
			GameController::Instance().ProcessCriticalError(e.GetErrorText());
		}
	}

	const Preference::PrefGameModel* getGameModel()
	{
		return GameController::Instance().GetGameModel();
	}

	AppSettings& getSettings()
	{
		return GameController::Instance().GetSettings();
	}

	ImagesStorage& getImagesStorage()
	{
		return GameController::Instance().GetImagesStorage();
	}
}

