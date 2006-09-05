/***************************************************************************
                          desktop list item
                          -------------------
    begin                : 2004-11-30
    copyright            : (C) 2004-2005 by Jaroslaw Staniek
    email                : js@iidea.pl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef DESK_LIST_ITEM_H
#define DESK_LIST_ITEM_H

// Local
#include "directory.h"

class SpecialListItem:public Directory
{
public:
	SpecialListItem (DirectoryView* parent, ImageViewer * iv, ImageListView * imageList, MainWindow * mw,
		const QString& path, const QString& iconName, const QString& itemName);
	virtual ~SpecialListItem();

	virtual QString text (int column) const;
protected:
};


#endif
