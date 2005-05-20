/***************************************************************************
                          directory  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
                           (C) 2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
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

#ifndef DIRECTORY_H
#define DIRECTORY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "listitem.h"

// Qt
#include <qfile.h>
#include <qptrlist.h>

class ImageListView;
class ImageFileIconItem;
class DirFileIconItem;
class FileIconItem;
class ImageViewer;
class MainWindow;
class DirectoryView;

class QFile;

class KAction;

class Directory:public ListItem
{
public:
	/**
	 If \a path is null, "/" is the default.
	*/
	Directory (MainWindow * mw, const QString& path = QString::null);
	Directory (Directory* parent, const QString& filename, MainWindow * mw);
	virtual ~Directory();

	virtual QString text (int column) const;

	QString path ();

	void setOpen (bool);
	void recursivelyOpen();

	void recursivelyDelete();

	void setup ();

	void load (bool loadThumbnails=true);
	void unLoad ();
	bool refresh(bool preview=true);

	virtual ListItem *find (const QString&);
	void goTo (const QString& dest);
	void loadFirst ();

	void removeImage (ImageFileIconItem * imf);

	void createDir(const QString& dirName);
	void createAlbum(const QString& albumName);
	bool rename(const QString& newDirName, QString& msg);
	void rename();
	void updateChildren();

	void properties();

protected:
	void init();
	DirectoryView* getDirectoryView();
	/**
		Checks access to the directory we're pointing to.
		Displays error message box on errors.
	*/
	bool checkAccess();

	bool loaded;
	bool readable;
	int size;

	QString newDirName;

#ifdef Q_WS_WIN
	QString *driveNodeVolumeName;
#endif
};


#endif
