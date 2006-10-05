/***************************************************************************
                          album.h  -  description
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

#ifndef ALBUM_H
#define ALBUM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "listitem.h"

// Qt
#include <qfile.h>
#include <qptrlist.h>

class Directory;
class DirectoryView;
class ImageViewer;
class ImageListView;
class MainWindow;
class AlbumImageFileIconItem;

class QFile;

/**
  *@author Richard Groult
  */

class Album : public ListItem
{
public:
	Album(
		ListItem * parent,
		const QString& filename,
		MainWindow * mw);
	~Album();

	void addURL(const QStringList& lst);
	QString pathTo(const QString& fileName);

	virtual  void load (bool refresh=true);
	virtual  void unLoad ();

	virtual  ListItem* find (const char *);
	virtual  void goTo (const char *dest);

	virtual  void removeImage ( ListItem* imf);
	virtual  void removeImage (AlbumImageFileIconItem * imf);

	void create(const QString& dirName);
	bool add(const QStringList& uris);
	bool acceptDrop(){return true;};

	virtual  bool rename(const QString& newDirName, QString& msg);
	virtual  void rename();
	void updateChildren();

	virtual  void properties();

protected:
	QPtrList < AlbumImageFileIconItem > list;
	void init();
};

#endif
