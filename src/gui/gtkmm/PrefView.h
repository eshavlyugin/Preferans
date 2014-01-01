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

#ifndef _PREF_VIEW_H__
#define _PREF_VIEW_H__

#include <PrefModelCallback.h>
#include <PrefViewWidget.h>
#include <vector>
#include <boost/shared_ptr.hpp>

struct PrefViewContext {
	std::vector< Preference::CardsSet > Hands;
	std::vector< Preference::Card > Widow;
	std::vector< Preference::Card > CardsOnDesk;
	std::vector<int> Tricks;
	std::vector<Preference::BidType> Bids;
	std::vector<int> CardsCount;
	Preference::CardsSet ValidMoves;
	Preference::CardsSet DropCandidate;
	int ActivePlayer;
	bool IsDroppingMode;

	void Reset();
};

class BulletWidget;

class PrefView : public Gtk::DrawingArea {
public:
	PrefView(GtkDrawingArea* base, const Glib::RefPtr<Gtk::Builder>& builder);
	
	void UpdateView(const PrefViewContext&);
	void ProcessStartOfNewBullet(int bulletSize);
	void Clear(); // delete all widgets from view
	bool IsBulletShown() const;
	void ShowBullet();
	void HideBullet();
	void UpdateBulletInfo(const std::vector<int>& bullets, const std::vector<int>& mountains,
		const std::vector< std::vector<int> >& whists );

protected:
	virtual bool on_expose_event(GdkEventExpose* event);
	virtual bool on_motion_notify_event(GdkEventMotion* event);
	virtual bool on_button_press_event(GdkEventButton* event);

private:
	void drawAll(Cairo::RefPtr<Cairo::Context> cr);

	struct AnimationContext {
		bool isInProgress;
	};

	struct WidgetInfo {
		double originX;
		double originY;
		double relLeft;
		double relTop;
		double relWidth;
		double relHeight;
		boost::shared_ptr<PrefViewWidget> impl;

		WidgetInfo(double originX, double originY, double left, double top, double width, double height, boost::shared_ptr<PrefViewWidget> impl);
	};

	sigc::signal<void> sigClickView;
	// Cursor coordinates of last processed mouse event
	int m_lastMouseX;
	int m_lastMouseY;
	// set of widgets of view descendingly sorted by z-index 
	std::vector<WidgetInfo> m_widgets;
	boost::shared_ptr<BulletWidget> bulletWidget; 

	WidgetInfo* findWidget(int x, int y);
	void transformRelToReal(double originX, double originY, double& x, double& y, double& width, double& height);

	enum AlignType {
		AT_Left,
		AT_Right,
		AT_Center
	};
	void addCardsRow(Preference::CardsSet cards, Preference::CardsSet validCards, bool isVerOriented, 
		double originX, double originY, double _baseX, double _baseY, 
		AlignType alignType, bool highlightAll, Preference::CardsSet drop);
	void addHiddenCardsRow(int cardsCount, double originX, double originY, double _baseX, double _baseY, AlignType alignType);
};

inline PrefView::WidgetInfo::WidgetInfo(double _originX, double _originY, double _relLeft, double _relTop, 
	double _relWidth, double _relHeight, boost::shared_ptr<PrefViewWidget> _impl) :
	originX(_originX),
	originY(_originY),
	relLeft(_relLeft),
	relTop(_relTop),
	relWidth(_relWidth),
	relHeight(_relHeight),
	impl(_impl)
{
}

inline void PrefViewContext::Reset()
{
	Hands.clear();
	Widow.clear();
	CardsOnDesk.clear();
	Tricks.clear();
	Bids.clear();
	CardsCount.clear();
	ValidMoves = Preference::EmptyCardsSet;
	DropCandidate = Preference::EmptyCardsSet;
	ActivePlayer = 0;
	IsDroppingMode = false;
}

#endif // _PREF_VIEW_H__
