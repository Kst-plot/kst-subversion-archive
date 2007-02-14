/***************************************************************************
                         categoryimagefileiconitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
    email                : rgroult@jalix.org
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

#include "categoryimagefileiconitem.h"

#ifdef WANT_LIBKEXIDB

#include "categorylistitem.h"
#include "mainwindow.h"

CategoryImageFileIconItem::CategoryImageFileIconItem(
		CategoryDBManager *a_p_categoryDBManager,
	 	const QString& fullname
		)
	:ImageFileIconItem(
			QFileInfo(fullname).fileName(),
			QFileInfo(fullname).dirPath(true)+'/',
			"",
			true)
{
	m_p_categoryDBManager = a_p_categoryDBManager;

// 	setType("CategoryImageFileIconItem");
	setIsMovable(false);
	setKey(getMainWindow()->getImageListView()->getCurrentKey());
}

#endif /* WANT_LIBKEXIDB */
