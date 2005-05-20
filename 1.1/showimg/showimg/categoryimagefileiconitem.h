/***************************************************************************
                         categoryimagefileiconitem.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef CATEGORYIMAGEFILEICONITEM_H
#define CATEGORYIMAGEFILEICONITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

//Local
#include "imagefileiconitem.h"

class CategoryListItem;
class MainWindow;
class CategoryDBManager;

class CategoryImageFileIconItem : public ImageFileIconItem
{
public:
	CategoryImageFileIconItem(
			CategoryDBManager *parent,
			const QString& fullname,
			MainWindow *mw);

protected:
	CategoryDBManager *parent;
};

#endif /* WANT_LIBKEXIDB */

#endif /* __CATEGORYIMAGEFILEICONITEM_H___ */
