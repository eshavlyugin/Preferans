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

#include <ImagesStorage.h>

static const std::string FrenchDeckName = "Svg French Cards 2.0 (By David Bellot)";

ImagesStorage::ImagesStorage()
{
	GError* error = 0;
	RsvgHandle* rsvgCards = rsvg_handle_new_from_file("PreferansData/svg-cards-2.0.svg", &error);
	loadFrenchDeck(rsvgCards);
	rsvg_handle_free(rsvgCards);
}

void ImagesStorage::loadFrenchDeck(RsvgHandle* rsvgCards)
{
	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(rsvgCards, &dim);
	Cairo::RefPtr<Cairo::ImageSurface> allImages = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, dim.width, dim.height);
	Cairo::RefPtr<Cairo::Context> cardsImagesDrawer = Cairo::Context::create( allImages );
	cardsImagesDrawer->set_source_rgb(1, 1, 1);
	rsvg_handle_render_cairo(rsvgCards, cardsImagesDrawer->cobj());

	for( Preference::SuitForwardIterator itSuit; itSuit.HasNext(); itSuit.Next() ) { 
		for( Preference::RankForwardIterator itRank; itRank.HasNext(); itRank.Next() ) { 
			cardsImages[FrenchDeckName][Preference::CreateCard(itSuit.GetObject(), itRank.GetObject())] 
				= loadFrenchCard( allImages, Preference::CreateCard(itSuit.GetObject(), itRank.GetObject()) );
		}
	}
	cardsImages[FrenchDeckName][Preference::UnknownCard] = loadFrenchCard(allImages, Preference::UnknownCard);
}

Cairo::RefPtr<Cairo::ImageSurface> ImagesStorage::loadFrenchCard(Cairo::RefPtr<Cairo::ImageSurface> sourceImages, Preference::Card card)
{
	Cairo::RefPtr<Cairo::ImageSurface> cardImage = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 
		sourceImages->get_width() / 13.0, sourceImages->get_height() / 5.0);
	Cairo::RefPtr<Cairo::Context> cardsImagesDrawer = Cairo::Context::create( cardImage );
	cardsImagesDrawer->set_source_rgb(1, 1, 1);
	double x = getFrenchCardCoord(card, CT_X, sourceImages->get_width());
	double y = getFrenchCardCoord(card, CT_Y, sourceImages->get_height());
	cardsImagesDrawer->set_source(sourceImages, -x, -y);
	cardsImagesDrawer->rectangle(0, 0, sourceImages->get_width() / 13.0, sourceImages->get_height() / 5.0);
	cardsImagesDrawer->clip();
	cardsImagesDrawer->paint();
	return cardImage;
}

std::vector<std::string> ImagesStorage::GetAvailableDecks() const
{
	std::vector<std::string> result;
	result.push_back(FrenchDeckName);
	return result;
}

Cairo::RefPtr<Cairo::ImageSurface> ImagesStorage::GetCardImage(Preference::Card card, const std::string& _deck) 
{
	std::string deck = !_deck.empty() ? _deck : activeDeck;
	return cardsImages[deck][card];
}


static const struct { Preference::Suit Suit; int Pos; } FrenchSuits[] = {
	{Preference::SuitSpades, 3},
	{Preference::SuitClubs, 0},
	{Preference::SuitDiamonds, 1},
	{Preference::SuitHearts, 2}
};

static const struct { Preference::Rank Rank; int Pos; } FrenchRanks[] = {
	{Preference::RankA, 0},
	{Preference::RankK, 12},
	{Preference::RankQ, 11},
	{Preference::RankJ, 10},
	{Preference::Rank10, 9},
	{Preference::Rank9, 8},
	{Preference::Rank8, 7},
	{Preference::Rank7, 6}
};

double ImagesStorage::getFrenchCardCoord(Preference::Card card, CoordType type, double size )
{
	if( card == Preference::UnknownCard ) {
		switch( type ) {
			case CT_X:
				return size / 13.0 * 2.0;
			case CT_Y:
				return size / 5.0 * 4.0;
		}
	}
	for( int i = 0; i < sizeof(FrenchRanks) / sizeof(FrenchRanks[0]); i++ ) {
		for( int j = 0; j < sizeof(FrenchSuits) / sizeof(FrenchSuits[0]); j++ ) {
			if( Preference::CreateCard(FrenchSuits[j].Suit, FrenchRanks[i].Rank ) == card ) {
				switch( type ) {
					case CT_X:
						return size / 13.0 * FrenchRanks[i].Pos;
					case CT_Y:
						return size / 5.0 * FrenchSuits[j].Pos;
				}
			}
		}
	}
	PrefAssert( false );
	return 0.0;
}

