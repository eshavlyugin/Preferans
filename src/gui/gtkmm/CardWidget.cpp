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

#include <CardWidget.h>
#include <PrefSlots.h>
#include <iostream>

CardWidget::CardWidget(Preference::Card _card, bool highlight) : 
	card(_card), 
	highlightOnHover(highlight), 
	isMouseInArea(false), 
	width(0), 
	height(0) 
{
	sigClickCard.connect(&PrefSlots::processCardClicked);
}

void CardWidget::Draw(Cairo::RefPtr<Cairo::Context> cr, int x, int y)
{
	bool highlight = highlightOnHover && isMouseInArea;
	Cairo::RefPtr<Cairo::ImageSurface> image = PrefSlots::getImagesStorage().GetCardImage(card);
	cr->save();
	cr->translate(x, y);
	cr->scale(1.0 * width / image->get_width(), 1.0 * height / image->get_height());
	cr->rectangle(0, 0, image->get_width(), image->get_height());
	cr->clip();
	if( highlight ) {
		cr->save();
		cr->set_source_rgb(0.1, 0.8, 0.1);
		cr->paint();
		cr->restore();
	}
	cr->set_source(image, 0, 0);
	cr->paint_with_alpha( highlight ? 0.7 : 1.0 );
	cr->restore();
}

void CardWidget::OnClick(int x, int y)
{
	sigClickCard.emit(card);
}

void CardWidget::OnMouseEnter()
{
	isMouseInArea = true;
}

void CardWidget::OnMouseLeave()
{
	isMouseInArea = false;
}

bool CardWidget::IsInRect(int x, int y)
{
	return true;
}


