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

#ifndef _IMAGES_STORAGE__
#define _IMAGES_STORAGE__

#include <vector>
#include <map>
#include <string>

class ImagesStorage {
public:
	ImagesStorage();

	std::vector<std::string> GetAvailableDecks() const;
	void SetActiveDeck(const std::string& deckName) { activeDeck = deckName; }
	std::string GetActiveDeck() const { return activeDeck; }
	Cairo::RefPtr<Cairo::ImageSurface> GetCardImage(Preference::Card card, const std::string& deck = "");

private:
	enum CoordType {
		CT_X,
		CT_Y
	};

	std::string activeDeck;
	std::map<std::string, std::map<Preference::Card, Cairo::RefPtr<Cairo::ImageSurface> > > cardsImages;
	
	void loadFrenchDeck(RsvgHandle*);
	Cairo::RefPtr<Cairo::ImageSurface> loadFrenchCard(Cairo::RefPtr<Cairo::ImageSurface> source, Preference::Card card);
	double getFrenchCardCoord(Preference::Card card, CoordType type, double size);
};

#endif // _IMAGES_STORAGE__
