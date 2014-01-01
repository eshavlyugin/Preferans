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

#include <BiddingDialog.h>
#include <PrefSlots.h>
#include <iostream>

using namespace Preference;

static const struct { std::string Name; BidType Bid; } buttonMap[] = {
	{"6s", Bid_6s},
	{"6c", Bid_6c},
	{"6d", Bid_6d},
	{"6h", Bid_6h},
	{"6nt", Bid_6nt},
	{"7s", Bid_7s},
	{"7c", Bid_7c},
	{"7d", Bid_7d},
	{"7h", Bid_7h},
	{"7nt", Bid_7nt},
	{"8s", Bid_8s},
	{"8c", Bid_8c},
	{"8d", Bid_8d},
	{"8h", Bid_8h},
	{"8nt", Bid_8nt},
	{"9s", Bid_9s},
	{"9c", Bid_9c},
	{"9d", Bid_9d},
	{"9h", Bid_9h},
	{"9nt", Bid_9nt},
	{"10s", Bid_10s},
	{"10c", Bid_10c},
	{"10d", Bid_10d},
	{"10h", Bid_10h},
	{"10nt", Bid_10nt},
	{"misere", Bid_Misere},
	{"pass", Bid_Pass},
	{"whist", Bid_Whist},
	{"half-whist", Bid_HalfWhist},
	{"open-whist", Bid_OpenWhist},
	{"close-whist", Bid_CloseWhist}
};

BidButton::BidButton(GtkButton* base, const Glib::RefPtr<Gtk::Builder>& builder) : 
	bid(Bid_Unknown),
	Gtk::Button(base)
{
	sigBid.connect(&PrefSlots::processBidClicked);
}

void BidButton::SetBid(BidType _bid)
{
	bid = _bid;
}

void BidButton::on_clicked()
{
	sigBid.emit(bid);
}

BiddingDialog::BiddingDialog(GtkWindow* base, const Glib::RefPtr<Gtk::Builder>& builder) :
	Gtk::Window(base)
{
	for( int i = 0; i < sizeof(buttonMap) / sizeof(buttonMap[0]); i++ ) {
		BidButton* button = 0;
		builder->get_widget_derived(buttonMap[i].Name, button);
		PrefAssert( button != 0 );
		buttons[buttonMap[i].Bid] = Glib::RefPtr<BidButton>(button);
		buttons[buttonMap[i].Bid]->SetBid( buttonMap[i].Bid );
	}
}
	
void BiddingDialog::EnableBid(BidType bid, bool enable)
{
	if( buttons.find(bid) == buttons.end() ) {
		return;
	}
	buttons[bid]->set_sensitive(enable);
}

