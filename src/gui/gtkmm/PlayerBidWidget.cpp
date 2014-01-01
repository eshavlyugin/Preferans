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

#include <PlayerBidWidget.h>
#include <sstream>
#include <string.h>

static std::string intToString(int n)
{
	std::ostringstream ost;
	ost << n;
	return ost.str();
}

static std::string BidToString(Preference::BidType bid)
{
	if( bid == Preference::Bid_Unknown ) { 
		// spaces for non-empty balloon
		return "??????";
	} else if( bid == Preference::Bid_Pass ) {
		return "Pass";
	} else if( bid == Preference::Bid_Misere ) {
		return "Misere";
	} else if( bid == Preference::Bid_Whist || bid == Preference::Bid_OpenWhist || bid == Preference::Bid_CloseWhist) {
		return "Whist";
	} else if( bid == Preference::Bid_HalfWhist ) {
		return "Half-Whist";
	} else {
		std::string deal = intToString(GetContractGameDeal(bid));
		std::string trump;
		Preference::Suit suit = GetContractTrump(bid);
		if( suit == Preference::SuitSpades ) {
			trump = "spades";
		} else if( suit == Preference::SuitClubs ) {
			trump = "clubs";
		} else if( suit == Preference::SuitDiamonds ) {
			trump = "diamonds";
		} else if( suit == Preference::SuitHearts ) {
			trump = "hearts";
		} else if( suit == Preference::SuitNoTrump ) {
			trump = "no trump";
		}
		return deal + " " + trump;
	}
}

void PlayerBidWidget::Draw(Cairo::RefPtr<Cairo::Context> cr, int x, int y)
{
	cr->save();
	cr->translate(x, y);
	std::string str = BidToString(bid);
	if( str.empty() ) {
		return;
	}
	cairo_text_extents_t extents;
	cr->set_font_size(0.65 * height);
	cr->get_text_extents(str, extents);
	double textWidth = 1.2 * extents.width;
	double rectWidth = width;
	double rectHeight = 0.75 * height;
	double tailX = isLeftAligned ? rectHeight : width - 1.0 * rectHeight;
	cr->move_to(tailX, rectHeight);
	double tailH = height * 0.25;
//	cr->line_to(tailX + tailH / 2.0, rectHeight + tailH);
	cr->line_to(tailX + tailH, rectHeight);
	cr->line_to(width, rectHeight);
	cr->line_to(width, 0);
	cr->line_to(0, 0);
	cr->line_to(0, rectHeight);
	cr->line_to(tailX, rectHeight);

	if( !isActivePlayer ) {
		cr->set_source_rgb(1.0, 0.9, 0.5);
	} else {
		cr->set_source_rgb(0.9, 0.5, 1.0);
	}
	cr->fill_preserve();
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->stroke();
	cr->move_to(rectWidth * 0.1, 0.8 * rectHeight);
	cr->set_source_rgb(0, 0, 0);
	if( bid != Preference::Bid_Unknown ) {
		cr->show_text(str.c_str());
	}
	cr->restore();
}

bool PlayerBidWidget::IsInRect(int x, int y)
{
	return true;
}

