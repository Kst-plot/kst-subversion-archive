/*****************************************************************************
                          directory  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
                           (C) 2004-2005 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the Free Software             *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301 *
 *   USA.                                                                    *
 *                                                                           *
 *   For license exceptions see LICENSE.EXC file, attached with the source   *
 *   code.                                                                   *
 *                                                                           *
 *****************************************************************************/

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

class DirectoryView;
class DirFileIconItem;
class FileIconItem;
class ImageFileIconItem;
class ImageListView;
class ImageViewer;

class QFile;

class KAction;

class Directory:public ListItem
{
public:
	/**
	 If \a path is null, "/" is the default.
	*/
	Directory (const KURL& a_url = KURL());
	Directory (Directory* a_p_parent, const QString& a_filename);
	virtual ~Directory();

	QString displayedName () const;

	void setOpen (bool a_is_open);
	void recursivelyOpen();

	void recursivelyDelete();

	void setup ();

	void load (bool a_loadThumbnails=true);
	void unLoad ();
	bool refresh(bool a_preview=true);

	virtual ListItem *find (const KURL& a_url);
	ListItem *find (const QString& a_end_dir);

	virtual void removeIconItem (FileIconItem * a_p_imf);

	void createDir(const QString& a_dirName);
	void createAlbum(const QString& a_albumName);
	bool rename(const QString& a_newDirName, QString& a_msg);
	void rename();
	void updateChildren();

	virtual bool insertListItem(const KURL& a_url);
	virtual bool insertIconItem(const KURL& a_url);

protected:
	void init();
	DirectoryView* getDirectoryView();

	QString getNewDirName() const {return m_newDirName;};
	void setNewDirName(const QString& a_newDirName)  {m_newDirName = a_newDirName;};

	bool hasSubDirectory (const QString& a_dir);

	/**
		Checks access to the directory we're pointing to.
		Displays error message box on errors.
	*/
	bool checkAccess();

private:
	bool m_loaded;
	bool m_readable;

	QString 
		m_newDirName,
		m_displayedName;

#ifdef Q_WS_WIN
	QString m_driveNodeVolumeName;
#endif
};


#endif
