/***************************************************************************
                          dirfileiconitem.h  -  description
                             -------------------
    begin                : dim jun 23 2002
    copyright            : (C) 2002-2005 by Richard Groult
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

#ifndef DIRFILEICONITEM_H
#define DIRFILEICONITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "fileiconitem.h"

class Directory;
class ImageListView;
class ImageViewer;
class MainWindow;

class QListView;

class DirFileIconItem:public FileIconItem
{
public:
	DirFileIconItem (
			Directory *parentDir,
			const QString& filename,
			const QString& path,
			MainWindow *mw,
			QString description="");

	virtual ~DirFileIconItem();

	virtual void setSelected (bool s);

	virtual void setText(const QString& text );
	virtual QString text (int column=0) const;

	virtual int compare ( QIconViewItem * i ) const;

	virtual void setKey ( const QString& k );

	virtual bool suppression();
	virtual bool suppression(bool suprFile);
	virtual bool moveToTrash();
	virtual bool shred();

protected:
	int nbrImg;
};


#endif
