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

#include <PrefWindow.h>
#include <PrefSlots.h>
#include <iostream>

PrefWindow::PrefWindow(GtkWindow* base, const Glib::RefPtr<Gtk::Builder>& builder) : 
	Gtk::Window(base)
{
	windowState.Reset();
	// initializing menu items
	Gtk::MenuItem* item = 0;
	builder->get_widget("menuGameQuit", item);
	PrefAssert( item != 0 );
	item->signal_activate().connect(sigc::ptr_fun(&PrefSlots::quit));
	item = 0;
	builder->get_widget("menuHelpAbout", item);
	PrefAssert( item != 0 );
	item->signal_activate().connect(sigc::ptr_fun(&PrefSlots::about));
	item = 0;
	builder->get_widget("menuGameNew", item);
	PrefAssert( item != 0 );
	item->signal_activate().connect(sigc::ptr_fun(&PrefSlots::startGameWithBots));
	// initializing status bar
	Gtk::Statusbar* _statusbar = 0;
	builder->get_widget("mainWindowStatusBar", _statusbar);
	PrefAssert( _statusbar != 0 );
	statusbar = Glib::RefPtr<Gtk::Statusbar>(_statusbar);
	// initializing bidding dialog
	BiddingDialog* _biddingDialog = 0;
	builder->get_widget_derived("biddingWindow", _biddingDialog);
	PrefAssert( _biddingDialog != 0 );
	biddingDialog = Glib::RefPtr<BiddingDialog>( _biddingDialog );
	PrefView* _gameView = 0;
	builder->get_widget_derived("gameView", _gameView);
	PrefAssert( _gameView != 0 );
	gameView = Glib::RefPtr<PrefView>( _gameView );
}

void PrefWindow::SetMoveMode(bool enable)
{
	if( windowState.CanMoveNow == enable ) {
		return;
	}
	windowState.CanMoveNow = enable;
	if( enable ) {
		windowState.ViewContext.ValidMoves = Preference::EmptyCardsSet;
	} else {
		updateGameView();
	}
}

void PrefWindow::OnNewGameStarted()
{
	GetLog() << "OnNewGameStarted()" << std::endl;
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	windowState.Reset();
	gameView->ProcessStartOfNewBullet(model->GetSettings().BulletSize);
}

bool PrefWindow::CanAcceptChanges() const
{
	return windowState.WaitingState == WWS_NoWait;
}

void PrefWindow::updateGameView()
{
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	PrefViewContext& context = windowState.ViewContext;
	context.Reset();
	context.ActivePlayer = model->CurrentPlayer();
	context.Tricks.resize(3);
	context.Bids.resize(3);
	context.Hands.resize(3);
	context.CardsCount.resize(3);
	context.CardsOnDesk.resize(3, Preference::UnknownCard);
	context.Widow.push_back(model->FirstWidowCard());
	context.Widow.push_back(model->SecondWidowCard());
	switch( model->GetDealStateType() ) {
		case Preference::GST_Drop:
		case Preference::GST_Whisting:
		case Preference::GST_OpenOrCloseWhist:
		case Preference::GST_ContractGame:
		case Preference::GST_Misere:
			context.Widow.clear();
			break;
		case Preference::GST_Passout:
			context.Widow.erase(context.Widow.begin(), context.Widow.begin() + 
				std::min(2, 10 - model->DealMovesRemaining()));
			break;
		default:
			break;
	}
	for( int i = 0; i < 3; i++ ) {
		context.Hands[i] = model->GetPlayerCards(i);
		context.Bids[i] = model->GetPlayerBid(i);
		context.Tricks[i] = model->GetPlayerTricks(i);
		context.CardsOnDesk[i] = model->GetCardOnDesk(i);
		context.CardsCount[i] = model->GetPlayerCardsCount(i);
	}
	context.IsDroppingMode = model->GetDealStateType() == Preference::GST_Drop;
	context.DropCandidate = windowState.CardsToDrop; 
	if( model->CurrentPlayer() == 0 && (model->GetDealStateType() == Preference::GST_Bidding 
		|| model->GetDealStateType() == Preference::GST_Whisting
		|| model->GetDealStateType() == Preference::GST_OpenOrCloseWhist) ) 
	{
		enableBids();
		biddingDialog->show();
	} else {
		biddingDialog->hide();
	}
	if( model->PlayerCardsOwner(model->CurrentPlayer()) == 0 
		&& windowState.CanMoveNow
		&& (model->GetDealStateType() == Preference::GST_Passout
		|| model->GetDealStateType() == Preference::GST_Misere
		|| model->GetDealStateType() == Preference::GST_ContractGame) )
	{
		context.ValidMoves = getValidMovesSet();
	}
	gameView->UpdateView(context);
}

void PrefWindow::OnClickOnView()
{
	onClickOnView();
}

void PrefWindow::onClickOnView()
{
	const Preference::PrefGameModel* gameModel = PrefSlots::getGameModel();
	if( windowState.TimerConnection.connected() ) {
		windowState.TimerConnection.disconnect();
	}
	if( gameModel == 0 ) {
		gameView->Clear();
		return;
	}
	if( gameView->IsBulletShown() ) {
		gameView->HideBullet();
	}
	if( windowState.WaitingState == WWS_WaitClick
		|| windowState.WaitingState == WWS_WaitTimerOrClick )
	{
		windowState.WaitingState = WWS_NoWait;
		if( windowState.UpdateOnModelScheduled ) {
			windowState.UpdateOnModelScheduled = false;
			updateGameView();
		}
	}
}

void PrefWindow::OnDealFinished()
{
	windowState.Reset();
	updateGameView();
	windowState.WaitingState = WWS_WaitClick;
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	std::vector<int> bullets(3);
	std::vector<int> mountains(3);
	std::vector< std::vector<int> > whists(3, std::vector<int>(3));
	for( int i = 0; i < 3; i++ ) {
		bullets[i] = model->GetBullet(i);
		mountains[i] = model->GetMountain(i);
		for( int j = 0; j < 3; j++ ) {
			whists[i][j] = model->GetWhists(i, j);
		}
	}
	gameView->UpdateBulletInfo(bullets, mountains, whists);
	gameView->ShowBullet();
}

void PrefWindow::setTimer(int delay, bool isTimerOrClick )
{
	PrefAssert( windowState.WaitingState == WWS_NoWait );
	windowState.TimerConnection = Glib::signal_timeout().connect(sigc::ptr_fun(&PrefSlots::onUpdateViewTimer), delay);
	windowState.WaitingState = isTimerOrClick ? WWS_WaitTimerOrClick : WWS_WaitTimer;
}

bool PrefWindow::OnUpdateViewTimer()
{
	if( windowState.WaitingState == WWS_WaitTimer
		|| windowState.WaitingState == WWS_WaitTimerOrClick )
	{
		windowState.WaitingState = WWS_NoWait;
		if( windowState.UpdateOnModelScheduled ) {
			updateGameView();
			windowState.UpdateOnModelScheduled = false;
		}
	}
	if( windowState.TimerConnection.connected() ) {
		windowState.TimerConnection.disconnect();
	}
	return false;
}

void PrefWindow::updateBulletInfo()
{
}

void PrefWindow::OnGameFinished()
{
	gameView->Clear();
}

void PrefWindow::enableDrops()
{
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	for( Preference::BidType bid = Preference::Bid_FirstBid; bid <= Preference::Bid_LastBid; 
		bid = static_cast<Preference::BidType>(bid + 1) ) 
	{
		biddingDialog->EnableBid( bid, model->IsValidDrop(Preference::Drop(windowState.CardsToDrop, bid)) );
	}
}

void PrefWindow::enableBids()
{
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	for( Preference::BidType bid = Preference::Bid_FirstBid; bid <= Preference::Bid_LastBid; 
		bid = static_cast<Preference::BidType>(bid + 1) ) 
	{
		biddingDialog->EnableBid( bid, model->IsValidBid(bid) );
	}
}

Preference::CardsSet PrefWindow::getValidMovesSet()
{
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	Preference::CardsSet result;
	for( Preference::SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
		for( Preference::RankForwardIterator rankIt; rankIt.HasNext(); rankIt.Next() ) {
			Preference::Card card = Preference::CreateCard(suitIt.GetObject(), rankIt.GetObject());
			if( model->IsValidMove(card) ) {
				result = Preference::AddCardToSet(result, card);
			}
		}
	}
	return result;
}

void PrefWindow::OnCardClicked( const Preference::Card& cardClicked )
{
	if( !windowState.CanMoveNow ) {
		return;
	}
	const Preference::PrefGameModel* model = PrefSlots::getGameModel();
	PrefAssert( model != 0 );
	switch( model->GetDealStateType() ) {
	case Preference::GST_Drop:
		{
			if( Preference::CardsSetSize(windowState.CardsToDrop) == 2
				&& !Preference::IsSetContainsCard( windowState.CardsToDrop, cardClicked ) )
			{
				return;
			}
			windowState.CardsToDrop = Preference::IsSetContainsCard( windowState.CardsToDrop, cardClicked )
				? Preference::RemoveCardFromSet( windowState.CardsToDrop, cardClicked )
				: Preference::AddCardToSet( windowState.CardsToDrop, cardClicked );
			updateGameView();
			if( Preference::CardsSetSize( windowState.CardsToDrop ) == 2 ) {
				enableDrops();
				biddingDialog->show();
			} else {
				biddingDialog->hide();
			}
			return;
		}
		case Preference::GST_Passout:
		case Preference::GST_ContractGame:
		case Preference::GST_Misere:
		{
			if( model->IsValidMove( cardClicked ) ) {
				int playerNum = model->CurrentPlayer();
				windowState.ViewContext.CardsOnDesk[playerNum] = cardClicked;
				windowState.ViewContext.Hands[playerNum] == 
					RemoveCardFromSet(windowState.ViewContext.Hands[playerNum], cardClicked);
				gameView->UpdateView(windowState.ViewContext);
				PrefSlots::doGameMove( cardClicked );
			}
			return;
		}
		default:
			return;
	}
}

void PrefWindow::OnBidClicked( const Preference::BidType& bid )
{
	if( !windowState.CanMoveNow ) {
		return;
	}
	const Preference::PrefGameModel* model = PrefSlots::getGameModel(); 
	PrefAssert( model != 0 );
	biddingDialog->hide();
	gameView->queue_draw();
	switch( model->GetDealStateType() ) {
		case Preference::GST_Drop:
		{
			PrefAssert( Preference::CardsSetSize( windowState.CardsToDrop ) == 2 );
			PrefSlots::doGameDrop( windowState.CardsToDrop, bid );
			windowState.CardsToDrop = Preference::EmptyCardsSet;
			break;
		}
		case Preference::GST_Bidding:
		case Preference::GST_OpenOrCloseWhist:
		case Preference::GST_Whisting:
		{
			PrefSlots::doGameBid( bid );
			break;
		}
		default:
			break;
	}
}

void PrefWindow::OnModelChanged(int updateDelayMs)
{
	updateGameView();
	if( updateDelayMs > 0 ) {
		setTimer(updateDelayMs, true);
	}
}

void PrefWindow::OnWidowOpened(Preference::Card card1, Preference::Card card2)
{
	windowState.ViewContext.Widow.resize(2);
	windowState.ViewContext.Widow[0] = card1;
	windowState.ViewContext.Widow[1] = card2;
	gameView->UpdateView(windowState.ViewContext);
	windowState.UpdateOnModelScheduled = true;
	setTimer(2000, true);
}

void PrefWindow::OnMove(Preference::Card card, int player, bool scheduleUpdateOnModel)
{
	if( scheduleUpdateOnModel ) {
		windowState.ViewContext.CardsOnDesk[player] = card;
		if( player == 0 ) {
			windowState.ViewContext.Hands[player] = 
				Preference::RemoveCardFromSet(windowState.ViewContext.Hands[player], card);
		}
		gameView->UpdateView(windowState.ViewContext);
		windowState.UpdateOnModelScheduled = true;
		setTimer(1400, true);
	} else {
		updateGameView();
		setTimer(100, true);
		windowState.UpdateOnModelScheduled = false;
	}
}

