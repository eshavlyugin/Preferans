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

#ifndef _PREF_FRAME_H__
#define _PREF_FRAME_H__

#include <BiddingDialog.h>
#include <PrefView.h>

class PrefWindow : public Gtk::Window {
public:
	PrefWindow(GtkWindow* base, const Glib::RefPtr<Gtk::Builder>&);
	
	Glib::RefPtr<BiddingDialog> GetBiddingDialog() { return biddingDialog; }
	Glib::RefPtr<Gtk::Statusbar> GetStatusbar() { return statusbar; }

	// events from view
	void OnCardClicked( const Preference::Card& card );
	void OnBidClicked( const Preference::BidType& bid );
	void OnClickOnView();

	// check whether window is ready to accept game events
	bool CanAcceptChanges() const;

	// events from game controller
	void OnModelChanged(int updateDelayMs = 0);
	void OnWidowOpened(Preference::Card card1, Preference::Card card2); 
	void OnMove(Preference::Card move, int player, bool scheduleUpdateOnModel); 
	void OnDealFinished();
	void OnGameFinished();
	bool OnUpdateViewTimer();
	void OnNewGameStarted();
	void SetMoveMode(bool enable);

private:
	Glib::RefPtr<BiddingDialog> biddingDialog;
	Glib::RefPtr<Gtk::Statusbar> statusbar;
	Glib::RefPtr<PrefView> gameView;

	enum WindowWaitingState {
		WWS_NoWait = 0,
		WWS_WaitClick,
		WWS_WaitTimer,
		WWS_WaitTimerOrClick
	};

	struct PrefWindowState {
		Preference::CardsSet CardsToDrop;
		WindowWaitingState WaitingState;
		PrefViewContext ViewContext;
		bool UpdateOnModelScheduled;
		bool CanMoveNow;
		sigc::connection TimerConnection;

		PrefWindowState() { Reset(); }
		void Reset();
	};
	
	PrefWindowState windowState;

	void updateGameView();
	void updateBulletInfo();
	void onClickOnView();
	void enableBids();
	void enableDrops();
	void setTimer(int dealy, bool waitTimerOrClick);
	Preference::CardsSet getValidMovesSet();
};

inline void PrefWindow::PrefWindowState::Reset()
{ 
	CardsToDrop = Preference::EmptyCardsSet; 
	WaitingState = WWS_NoWait;
	UpdateOnModelScheduled = true;
	CanMoveNow = false;
}

#endif // _PREF_VIEW_H__

