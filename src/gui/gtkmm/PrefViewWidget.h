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

#ifndef _PREF_VIEW_WIDGET_H__
#define _PREF_VIEW_WIDGET_H__

class PrefViewWidget {
public:
	virtual ~PrefViewWidget() = 0;

	// coordinates in absolute coordinates of view
	virtual void Draw(Cairo::RefPtr<Cairo::Context> cr, int x, int y) = 0;
	virtual void OnMouseEnter() = 0;
	virtual void OnMouseLeave() = 0;
	virtual void OnClick(int x, int y) = 0;
	// coordinates relative to upper left corner of widget
	virtual bool IsInRect(int x, int y) = 0;
	virtual void SetSize(int width, int height) = 0;
};

inline PrefViewWidget::~PrefViewWidget() {}

#endif // _PREF_VIEW_WIDGET_H__
