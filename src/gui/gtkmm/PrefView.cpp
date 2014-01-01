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

#include <PrefView.h>
#include <CardWidget.h>
#include <DealScoreWidget.h>
#include <PlayerBidWidget.h>
#include <BulletWidget.h>
#include <PrefSlots.h>

static const double DefaultCardHeight = 0.22;

PrefView::PrefView(GtkDrawingArea* base, const Glib::RefPtr<Gtk::Builder>& builder) : 
	Gtk::DrawingArea(base),
	m_lastMouseX(0),
	m_lastMouseY(0)
{
	bulletWidget.reset( new BulletWidget() );
	sigClickView.connect(&PrefSlots::processClickOnView);
}

bool PrefView::on_expose_event(GdkEventExpose* event)
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	Gtk::Allocation allocation = get_allocation();
	if( window == 0 ) {
		return true;
	}
	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
	drawAll(cr);
	return true;
}

bool PrefView::on_button_press_event(GdkEventButton* event)
{
	switch( event->type ) {
		case GDK_BUTTON_PRESS:
		{
			WidgetInfo* widget = findWidget(event->x, event->y);
			if( widget != 0 ) {
				widget->impl->OnClick(event->x, event->y);
			}
			sigClickView.emit();
		}
		default:
			break;
	}
	return true;
}

bool PrefView::on_motion_notify_event(GdkEventMotion* event)
{
	WidgetInfo* prevWidget = findWidget(m_lastMouseX, m_lastMouseY);
	WidgetInfo* newWidget = findWidget(event->x, event->y);
	if( prevWidget != newWidget ) {
		if( prevWidget != 0 ) {
			prevWidget->impl->OnMouseLeave();
		}
		if( newWidget != 0 ) {
			newWidget->impl->OnMouseEnter();
		}
		queue_draw();
	}
	m_lastMouseX = event->x;
	m_lastMouseY = event->y;
	return true;
}

PrefView::WidgetInfo* PrefView::findWidget(int ptX, int ptY) 
{
	for( std::vector<WidgetInfo>::reverse_iterator it = m_widgets.rbegin(); it != m_widgets.rend(); *it++ ) {
		double x = it->relLeft;
		double y = it->relTop;
		double width = it->relWidth;
		double height = it->relHeight;
		transformRelToReal(it->originX, it->originY, x, y, width, height);
		if( x < ptX 
			&& y < ptY 
			&& x + width > ptX 
			&& y + height > ptY ) 
		{
			return &(*it);
		}
	}
	return 0;
}

void PrefView::Clear()
{
	m_widgets.clear();
	queue_draw();
}

void PrefView::ProcessStartOfNewBullet(int bulletSize)
{
	Clear();
	bulletWidget->Clear();
	bulletWidget->SetBulletSize(bulletSize);
}

bool PrefView::IsBulletShown() const
{
	for( std::vector<WidgetInfo>::const_iterator it = m_widgets.begin(); it != m_widgets.end(); *it++ ) {
		if( dynamic_cast<const BulletWidget*>(it->impl.get()) != 0 ) {
			return true;
		}
	}
	return false;
}

void PrefView::UpdateBulletInfo(const std::vector<int>& bullets, const std::vector<int>& mountains,
	const std::vector< std::vector<int> >& whists )
{
	bulletWidget->AddScores(bullets, mountains, whists);
}

void PrefView::ShowBullet()
{
	m_widgets.push_back(WidgetInfo(0.5, 0.5, 0.2, 0.2, 0.6, 0.6, bulletWidget));
	queue_draw();
}

void PrefView::HideBullet()
{
	std::vector<WidgetInfo> newWidgets;
	for( std::vector<WidgetInfo>::iterator it = m_widgets.begin(); it != m_widgets.end(); *it++ ) {
		if( dynamic_cast<BulletWidget*>(it->impl.get()) == 0 ) {
			newWidgets.push_back(*it);
		}
	}
	m_widgets = newWidgets;
	queue_draw();
}

void PrefView::UpdateView(const PrefViewContext& context)
{
	static const double deskX[] = {0.37, 0.23, 0.51};
	static const double deskY[] = {0.45, 0.31, 0.34};
	double cardHeight = DefaultCardHeight;
	double cardWidth = cardHeight * 0.6;

	m_widgets.clear();
	// Score
	m_widgets.push_back(WidgetInfo(0.5, 0.0, 0.35, 0.0, 0.3, 0.15, boost::shared_ptr<PrefViewWidget>(new DealScoreWidget(context.Tricks))));
	// Player bids
	m_widgets.push_back(WidgetInfo(0.0, 0.0, 0.05, 0.05, 0.4, 0.1, 
		boost::shared_ptr<PrefViewWidget>(new PlayerBidWidget(context.Bids[1], true, context.ActivePlayer == 1))));
	m_widgets.push_back(WidgetInfo(1.0, 0.0, 0.55, 0.05, 0.4, 0.1, 
		boost::shared_ptr<PrefViewWidget>(new PlayerBidWidget(context.Bids[2], false, context.ActivePlayer == 2))));
	m_widgets.push_back(WidgetInfo(0.5, 1.0, 0.55, 0.63, 0.4, 0.1, 
		boost::shared_ptr<PrefViewWidget>(new PlayerBidWidget(context.Bids[0], false, context.ActivePlayer == 0))));
	// Widow
	double baseX = 0.4;
	double baseY = 0.20;
	for( std::vector<Preference::Card>::const_iterator it = context.Widow.begin(); it != context.Widow.end(); *it++ ) {
		m_widgets.push_back(WidgetInfo(0.5, 0.5, baseX, baseY, cardWidth, cardHeight, boost::shared_ptr<PrefViewWidget>(new CardWidget(*it, false))));
		baseX += cardWidth * 0.4;
	}
	// Players hands
	addCardsRow(context.Hands[0], context.ValidMoves, false, 0.5, 1.0, 0.0, 0.95 - cardHeight, AT_Center,
		context.IsDroppingMode && (Preference::CardsSetSize(context.DropCandidate) < 2), 
		context.DropCandidate);
	if( context.Hands[1] == Preference::EmptyCardsSet ) {
		addHiddenCardsRow(context.CardsCount[1], 0.0, 0.0, 0.05, 0.15, AT_Left);
	} else {
		addCardsRow(context.Hands[1], context.ValidMoves, true, 0.0, 0.0, 0.05, 0.15, AT_Left,
		false, Preference::EmptyCardsSet);
	}
	if( context.Hands[2] == Preference::EmptyCardsSet ) { 
		addHiddenCardsRow(context.CardsCount[2], 1.0, 0.0, 0.95, 0.15, AT_Right);
	} else {
		addCardsRow(context.Hands[2], context.ValidMoves, true, 1.0, 0.0, 0.95, 0.15, AT_Right,
			false, Preference::EmptyCardsSet);
	}
	// Current move cards on desk
	for( int i = 0; i < Preference::NumOfPlayers; i++ ) {
		if( context.CardsOnDesk[i] != Preference::UnknownCard ) {
			m_widgets.push_back(WidgetInfo(0.5, 0.5, deskX[i], deskY[i], cardWidth, 
				cardHeight, boost::shared_ptr<PrefViewWidget>(new CardWidget(context.CardsOnDesk[i], false))));
		}
	}

	WidgetInfo* widget = findWidget(m_lastMouseX, m_lastMouseY);
	if( widget != 0 ) { 
		widget->impl->OnMouseEnter();
	}
	queue_draw();
}

void PrefView::addHiddenCardsRow(int cardsCount, double originX, double originY, double _baseX, double _baseY, AlignType alignType)
{
	if( cardsCount == 0 ) {
		return;
	}
	double cardHeight = DefaultCardHeight;
	double cardWidth = cardHeight * 0.6;
	double baseX = _baseX;
	double baseY = _baseY;
	std::vector<WidgetInfo> newWidgets;
	for( int i = 0; i < cardsCount; i++ ) {
		newWidgets.push_back(WidgetInfo(originX, originY, baseX, baseY, cardWidth, cardHeight, 
			boost::shared_ptr<PrefViewWidget>(new CardWidget(Preference::UnknownCard, false))));
		baseX += cardWidth * 0.4;
	}
	if( alignType == AT_Right ) {
		double dx = -_baseX + newWidgets.rbegin()->relLeft + newWidgets.rbegin()->relWidth;
		for( std::vector<WidgetInfo>::iterator it = newWidgets.begin(); it != newWidgets.end(); *it++ ) {
			it->relLeft -= dx;
		}
	}
	m_widgets.insert(m_widgets.end(), newWidgets.begin(), newWidgets.end());
}

void PrefView::addCardsRow(Preference::CardsSet cards, Preference::CardsSet validCards, bool isVerOriented, 
	double originX, double originY, double _baseX, double _baseY, 
	AlignType alignType, bool highlightAll, Preference::CardsSet drop)
{
	double cardHeight = DefaultCardHeight;
	double cardWidth = cardHeight * 0.6;
	double baseX = _baseX;
	double baseY = _baseY;
	std::vector<WidgetInfo> widgetsToAdd;
	for( Preference::SuitForwardIterator suitIt; suitIt.HasNext(); suitIt.Next() ) {
		std::vector<WidgetInfo> newWidgets;
		for( Preference::RankBackwardIterator rankIt; rankIt.HasNext(); rankIt.Next() ) {
			Preference::Card card = Preference::CreateCard(suitIt.GetObject(), rankIt.GetObject());
			if( Preference::IsSetContainsCard(cards, card) ) {
				bool isDropCandidate = Preference::IsSetContainsCard(drop, card);
				bool highlight = Preference::IsSetContainsCard(validCards, card) || highlightAll || isDropCandidate;
				newWidgets.push_back(WidgetInfo(originX, originY, baseX, baseY - (isDropCandidate ? 0.03 : 0.0), 
					cardWidth, cardHeight, boost::shared_ptr<PrefViewWidget>(new CardWidget(card, highlight))));
				baseX += cardWidth * 0.35;
			}
		}
		if( newWidgets.empty() ) {
			continue;
		}
		if( alignType == AT_Right && newWidgets.size() > 0 ) {
			double dx = -_baseX + newWidgets.rbegin()->relLeft + newWidgets.rbegin()->relWidth;
			for( std::vector<WidgetInfo>::iterator it = newWidgets.begin(); it != newWidgets.end(); *it++ ) {
				it->relLeft -= dx;
			}
		}
		if( isVerOriented ) {
			baseY += cardHeight * 0.5;
			baseX = _baseX;
		} else { 
			baseX += cardWidth * 0.8;
		}
		widgetsToAdd.insert(widgetsToAdd.end(), newWidgets.begin(), newWidgets.end());
	}
	if( alignType == AT_Center && widgetsToAdd.size() > 0 ) {
		double rowLength = -_baseX + widgetsToAdd.rbegin()->relLeft + widgetsToAdd.rbegin()->relWidth;
		double dx = (0.5 - rowLength / 2.0) - _baseX;
		for( std::vector<WidgetInfo>::iterator it = widgetsToAdd.begin(); it != widgetsToAdd.end(); *it++ ) {
			it->relLeft += dx;
		}
	}
	m_widgets.insert(m_widgets.end(), widgetsToAdd.begin(), widgetsToAdd.end());
}

static const double StandardRelWidth = 1.5;
static const double StandardRelHeight = 1.0;

void PrefView::drawAll(Cairo::RefPtr<Cairo::Context> cr)
{
	Gtk::Allocation allocation = get_allocation();
	int areaWidth = allocation.get_width();
	int areaHeight = allocation.get_height();
	// drawing background
	cr->save();
	cr->rectangle(0, 0, areaWidth, areaHeight);
	cr->clip();
	cr->set_source_rgb(0.6, 0.8, 0.4);
	cr->paint();
	cr->restore();
	// drawing widgets
	for( std::vector<WidgetInfo>::iterator it = m_widgets.begin(); it != m_widgets.end(); *it++ ) {
		double x = it->relLeft;
		double y = it->relTop;
		double width = it->relWidth;
		double height = it->relHeight;
		transformRelToReal(it->originX, it->originY, x, y, width, height);
		it->impl->SetSize(width, height);
		it->impl->Draw(cr, static_cast<int>(x), static_cast<int>(y));
	}
}

// transform relative coordinates to real ones
void PrefView::transformRelToReal(double originX, double originY, double& x, double& y, double& width, double& height)
{
	Gtk::Allocation allocation = get_allocation();
	int areaWidth = allocation.get_width();
	int areaHeight = allocation.get_height();
	double scale = std::min( areaWidth / StandardRelWidth, areaHeight / StandardRelHeight );
	x = originX * areaWidth + (x - originX) * scale;
	y = originY * areaHeight + (y - originY) * scale;
	width = width * scale;
	height = height * scale;
}

