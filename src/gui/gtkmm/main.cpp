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

int main(int argc, char* argv[])
{
	rsvg_init();
	int rseed = time(0);
	srand( rseed );
	Gtk::Main app(argc, argv);
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
	builder->add_from_file("PreferansData/Preferans.glade");
	GameController& gameController = GameController::Instance();
	gameController.Initialize(builder);
	Gtk::Main::run(*gameController.GetMainWindow().operator->());
	rsvg_term();
	return 0;
}

