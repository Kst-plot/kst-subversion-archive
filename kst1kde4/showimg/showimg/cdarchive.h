/*****************************************************************************
                          cdarchive.h  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2006 by Richard Groult
    email                : rgroult@jalix.org
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

#ifndef CDARCHIVE_H
#define CDARCHIVE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "directory.h"

// KDE
#include <kstandarddirs.h>

// Qt
#include <qfile.h>
#include <qptrlist.h>

#define CDArchive_EXTENSION "sca"
#define CDArchive_ROOTPATH (QDir::homeDirPath()+"/.showimg/cdarchive/")

class KArchiveDirectory;
class KArchive;

class CDArchive : public ListItem
{
public:
	CDArchive ();
	CDArchive (CDArchive *a_p_parent, const QString& a_filename);
	virtual ~CDArchive();

	void load(bool);
	void unLoad();
	bool refresh(bool);

	virtual ListItem* find (const KURL& a_url);
	virtual ListItem* find (const QString& a_end_dir);

	virtual bool rename(const QString& a_newDirName, QString& a_msg);

	const KArchiveDirectory* getKArchiveDirectory() const;
	KArchive* getKArchive() const;
	QString getRelativePath() const;
	static QString CDArchive_TEMP_ROOTPATH();

protected:
	void init();

private:
	QString m_CDArchive_TEMP_ROOTPATH;

	bool      m_loaded;
	KArchive *m_p_arc;
	QString   m_relativePath;
	bool      m_isRoot;
};

#endif /* CDARCHIVE_H */
