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

#include <DealScoreWidget.h>
#include <sstream>
#include <iostream>

bool DealScoreWidget::IsInRect(int x, int y)
{
	return false;
}

static std::string intToString(int x)
{
	std::ostringstream ost;
	ost << x;
	return ost.str();
}

void DealScoreWidget::Draw(Cairo::RefPtr<Cairo::Context> cr, int x, int y)
{
	cr->save();
	
	double radius = std::min(width / 2.0, 1.0 * height);
	cr->translate(x + width / 2.0, y);
	cr->arc(0.0, 0.0, radius, 0, M_PI);
	cr->set_source_rgb(1.0, 0.9, 0.5);
	cr->fill_preserve();
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->stroke();
	cr->move_to(0.0, 0.0);
	cr->line_to(0.5 * radius, 0.5 * radius * sqrt(3));
	cr->stroke();
	cr->move_to(0.0, 0.0);
	cr->line_to(-0.5 * radius, 0.5 * radius * sqrt(3));
	cr->stroke();
	cr->set_font_size(radius * 0.45);
	cr->set_source_rgb(0.0, 0.0, 0.0);
	drawText(cr, intToString(scores[0]), 0.0, radius * 0.7);
	drawText(cr, intToString(scores[1]), -radius * 0.55, radius * 0.3);
	drawText(cr, intToString(scores[2]), radius * 0.55, radius * 0.3);

	cr->restore();
}

void DealScoreWidget::drawText(Cairo::RefPtr<Cairo::Context> cr, const std::string& text, double x, double y)
{
	cr->save();
	cairo_text_extents_t extents;
	cr->get_text_extents(text, extents);
	cr->move_to(x - extents.width / 2.0, y + extents.height / 2.0);
	cr->show_text(text);
	cr->restore();
}

