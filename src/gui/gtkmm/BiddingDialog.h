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

#ifndef _BIDDING_DIALOG_H__
#define _BIDDING_DIALOG_H__

#include <map>

class BidButton : public Gtk::Button {
public:
	BidButton(GtkButton* base, const Glib::RefPtr<Gtk::Builder>& builder);
	
	void SetBid(Preference::BidType bid);

protected:
	virtual void on_clicked();

private:
	sigc::signal1<void, Preference::BidType> sigBid;

	Preference::BidType bid;
};

class BiddingDialog : public Gtk::Window {
public:
	BiddingDialog(GtkWindow* base, const Glib::RefPtr<Gtk::Builder>& builder);
	
	void EnableBid(Preference::BidType bid, bool enable);
	
private:
	std::map<Preference::BidType, Glib::RefPtr<BidButton> > buttons;
};

#endif // _BIDDING_DIALOG_H__

