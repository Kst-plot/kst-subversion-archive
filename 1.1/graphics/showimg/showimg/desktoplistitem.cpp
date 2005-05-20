/***************************************************************************
                          desktop list item
                          -------------------
    begin                : 2004-11-30
    copyright            : (C) 2004 by Jaroslaw Staniek
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

// Local
#include "desktoplistitem.h"
#include "directoryview.h"
#include "imageviewer.h"
#include "imagefileiconitem.h"
#include "dirfileiconitem.h"
#include "compressedimagefileiconitem.h"
#include "compressedfileitem.h"
#include "fileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "directoryview.h"
#include "album.h"
#include "extract.h"

SpecialListItem::SpecialListItem( DirectoryView* parent,
			 ImageViewer *iv,
			 ImageListView *imageList,
			 MainWindow *mw, const QString& path, const QString& iconName, const QString& itemName )
	: Directory(parent, iv, imageList, mw, path)
{
	hasSpecialIcon = true;
	setText(DirectoryView::COLUMN_NAME, itemName);
	setPixmap(DirectoryView::COLUMN_NAME, BarIcon(iconName, dirView->getIconSize() ));
}

SpecialListItem::~SpecialListItem()
{
}

QString
SpecialListItem::text( int column ) const
{
	if ( column == DirectoryView::COLUMN_NAME )
		return KListViewItem::text(column);

	return Directory::text(column);
}

