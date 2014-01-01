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

#ifndef _BULLET_WIDGET_H__
#define _BULLET_WIDGET_H__

#include <PrefViewWidget.h>

class BulletWidget : public PrefViewWidget {
public:
	BulletWidget();
	virtual ~BulletWidget() {}

	void SetBulletSize(int size) { bulletSize = size; }
	void AddScores(const std::vector<int>& bullets, const std::vector<int>& mountains, 
		const std::vector< std::vector<int> >& whists);
	void Clear();

	// PrefViewWidget
	virtual void Draw(Cairo::RefPtr<Cairo::Context> cr, int x, int y);
	virtual void OnClick(int x, int y);
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual bool IsInRect(int x, int y);
	virtual void SetSize(int _width, int _height) { width = _width; height = _height; }

private:
	int bulletSize;
	int width;
	int height;

	void drawAll( Cairo::RefPtr<Cairo::Context>& );

	std::vector< std::vector<int> > bullets;
	std::vector< std::vector<int> > mountains;
	std::vector< std::vector< std::vector<int> > > whists;

	void drawBullets( Cairo::RefPtr<Cairo::Context>& );
	void drawMountains( Cairo::RefPtr<Cairo::Context>& );
	void drawWhists( Cairo::RefPtr<Cairo::Context>& );
	void drawText(Cairo::RefPtr<Cairo::Context>& cr, const std::vector<int>& scores, double maxWidth);
};

#endif // _BULLET_WIDGET_H__

