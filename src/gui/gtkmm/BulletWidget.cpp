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

#include <BulletWidget.h>
#include <iostream>
#include <sstream>
#include <string>

static const double sizeX = 1.0;
static const double sizeY = 1.0;
static const double centerX = 0.5;
static const double centerY = 0.5;
static const double mountainX = 0.73;
static const double mountainY = mountainX * sizeX * centerX / centerY / sizeY;
static const double bulletX = 0.81;
static const double bulletY = bulletX * sizeX * centerX / centerY / sizeY;
static const double radius = 0.06 * std::min(sizeX, sizeY);
static const double bulletFontSize = 0.05;
static const double scoreFontSize = 0.04;

BulletWidget::BulletWidget() :
	bulletSize(0),
	width(0),
	height(0)
{
	Clear();
}

void BulletWidget::Clear()
{
	bullets = std::vector< std::vector<int> >(3, std::vector<int>(1, 0));
	mountains = std::vector< std::vector<int> >(3, std::vector<int>(1, 0));
	whists = std::vector< std::vector< std::vector<int> > >(3, std::vector< std::vector<int> >(3, std::vector<int>(1, 0)));
	bulletSize = 0;
}

static std::string intToString(int i)
{
	std::ostringstream ost;
	ost << i;
	return ost.str();
}

void BulletWidget::AddScores(const std::vector<int>& _bullets, const std::vector<int>& _mountains, 
	const std::vector< std::vector<int> >& _whists)
{
	for( int i = 0; i < _bullets.size(); i++ ) {
		bullets[i].push_back(_bullets[i]);
		mountains[i].push_back(_mountains[i]);
		for( int j = 0; j < _whists[i].size(); j++ ) {
			whists[i][j].push_back(_whists[i][j]);
		}
	}
}

void BulletWidget::Draw(Cairo::RefPtr<Cairo::Context> cr, int left, int top) 
{
	cr->save();
	cr->translate(left, top);
	drawAll(cr);
	cr->restore();
}

void BulletWidget::OnMouseEnter() 
{
}

void BulletWidget::OnClick(int x, int y)
{
}

void BulletWidget::OnMouseLeave()
{
}

bool BulletWidget::IsInRect(int relX, int relY) 
{
	return true;
}

void BulletWidget::drawAll( Cairo::RefPtr<Cairo::Context>& cr )
{
	cr->save();
	cr->scale(width / sizeX, height / sizeY);
	cr->set_line_width(0.005);
	// filling backgroung
	cr->save();
	cr->save();
	cr->rectangle(0.0, 0.0, 1.0, 1.0);
	cr->set_source_rgb(1.0, 0.9, 0.5);
	cr->fill_preserve();
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->stroke();
	cr->restore();
	// drawing bullet shape
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->move_to(centerX, centerY);
	cr->line_to(0.0, sizeY);
	cr->move_to(centerX, centerY);
	cr->line_to(sizeX, sizeY);
	cr->move_to(centerX, centerY);
	cr->line_to(centerX, 0.0);
	cr->move_to(mountainX, 0.0);
	cr->line_to(mountainX, mountainY);
	cr->line_to(sizeX - mountainX, mountainY);
	cr->line_to(sizeX - mountainX, 0.0);
	cr->move_to(bulletX, 0.0);
	cr->line_to(bulletX, bulletY);
	cr->line_to(sizeX - bulletX, bulletY);
	cr->line_to(sizeX - bulletX, 0.0);
	cr->move_to(centerX, bulletY);
	cr->line_to(centerX, sizeY);
	cr->move_to(sizeX - bulletX, centerY);
	cr->line_to(0.0, centerY);
	cr->move_to(bulletX, centerY);
	cr->line_to(sizeX, centerY);
	cr->stroke();
	cr->restore();
	
	// drawing bullet circle
	// filling circle
	cr->save();
	cr->arc(centerX, centerY, radius, 0.0, 2 * M_PI);
	cr->save();
	cr->clip();
	cr->set_source_rgb(1.0, 0.8, 0.3);
	cr->paint();
	cr->restore();
	cr->arc(centerX, centerY, radius, 0.0, 2 * M_PI);
	cr->stroke();
	cr->restore();
	// drawing max bullet size
	cr->save();
	cr->move_to(centerX, centerY);
	cr->set_font_size(bulletFontSize);
	std::string str = intToString(bulletSize); 
	cairo_text_extents_t extents;
	cr->get_text_extents(str, extents);
	double x = centerX;
	double y = centerY;
	cr->move_to(x - extents.width / 2.0, y + extents.height / 2.0);
	cr->show_text(str);
	cr->restore();

	// drawing scores
	cr->set_font_size(scoreFontSize);
	drawBullets(cr);
	drawMountains(cr);
	drawWhists(cr);

	cr->restore();
}


void BulletWidget::drawBullets(Cairo::RefPtr<Cairo::Context>& cr) 
{
	// assume context is already scaled to (1.0, 1.0)
	cr->save();

	// first player (bottom)
	cr->save();
	cr->move_to(1.0 - (bulletX - scoreFontSize), bulletY - scoreFontSize / 5.0);
	drawText(cr, bullets[0], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// second player (left)
	cr->save();
	cr->move_to(1.0 - bulletX + scoreFontSize / 5.0, scoreFontSize / 5.0);
	cr->rotate(M_PI / 2.0);
	drawText(cr, bullets[1], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// third player (right)
	cr->save();
	cr->move_to(bulletX - scoreFontSize / 5.0, bulletY - scoreFontSize);
	cr->rotate(-M_PI / 2.0);
	drawText(cr, bullets[2], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();

	cr->restore();
}

void BulletWidget::drawText(Cairo::RefPtr<Cairo::Context>& cr, const std::vector<int>& scores, double maxWidth)
{
	if( scores.back() == 0 ) {
		return;
	}
	cr->save();
	int scoresSize = scores.size();
	// assume context is already scaled and we're staying on the left upper corner of text we're going to draw
	std::string currentScore = scores[scoresSize - 1] == scores[scoresSize - 2]
		? "" : intToString(scores.back()) + ".";
	std::string history;
	int j = scoresSize - 2;
	// calculating number of scores to draw (including history of deals)
	int remains = 4;
	while( scores[j] != 0 && remains > 0 ) {
		cairo_text_extents_t extents;
		std::string newHistory = history;
		if( scores[j] != scores[j-1] ) {
			newHistory = intToString(scores[j]) + "." + history;
			remains--;
		}
		cr->get_text_extents(newHistory, extents);
		history = newHistory;
		j--;
	}
	// drawing text
	cairo_text_extents_t extents;
	cr->show_text(history);
	cr->set_source_rgb(1.0, 0.0, 0.0);
	cr->show_text(currentScore);
	cr->restore();
}

void BulletWidget::drawMountains(Cairo::RefPtr<Cairo::Context>& cr)
{
	cr->save();

	cr->save();
	// first player (bottom)
	cr->move_to(1.0 - (mountainX - scoreFontSize), mountainY - scoreFontSize / 5.0);
	drawText(cr, mountains[0], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// second player (left)
	cr->save();
	cr->move_to(1.0 - mountainX + scoreFontSize / 5.0, scoreFontSize / 5.0);
	cr->rotate(M_PI / 2.0);
	drawText(cr, mountains[1], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// third player (right)
	cr->save();
	cr->move_to(mountainX - scoreFontSize / 5.0, mountainY - scoreFontSize);
	cr->rotate(-M_PI / 2.0);
	drawText(cr, mountains[2], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();

	cr->restore();
}

void BulletWidget::drawWhists(Cairo::RefPtr<Cairo::Context>& cr)
{
	cr->save();
	// 1 -> 2
	cr->save();
	cr->move_to(1.0 - bulletX - 0.4 * scoreFontSize, bulletY + scoreFontSize * 1.2);
	drawText(cr, whists[0][1], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// 1 -> 3
	cr->move_to(centerX + 0.4 * scoreFontSize, bulletY + scoreFontSize * 1.2);
	drawText(cr, whists[0][2], 2 * (1.0 - mountainY + scoreFontSize));
	// 2 -> 3
	cr->save();
	cr->move_to(1.0 - bulletX - scoreFontSize * 1.2, scoreFontSize * 0.2);
	cr->rotate(M_PI / 2.0);
	drawText(cr, whists[1][2], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// 2 -> 1
	cr->save();
	cr->move_to(1.0 - bulletX - scoreFontSize * 1.2, centerY + scoreFontSize * 0.2);
	cr->rotate(M_PI / 2.0);
	drawText(cr, whists[1][0], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// 3 -> 1
	cr->save();
	cr->move_to(bulletX + scoreFontSize * 1.2, bulletY + scoreFontSize * 0.2);
	cr->rotate(-M_PI / 2.0);
	drawText(cr, whists[2][0], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();
	// 3 -> 2
	cr->save();
	cr->move_to(bulletX + scoreFontSize * 1.2, centerY - scoreFontSize * 0.2);
	cr->rotate(-M_PI / 2.0);
	drawText(cr, whists[2][1], 2 * (1.0 - mountainY + scoreFontSize));
	cr->restore();

	cr->restore();
}

