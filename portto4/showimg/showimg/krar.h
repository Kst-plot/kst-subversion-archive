/***************************************************************************
                         krar.h  -  description
                             -------------------
    begin                : wed Apr 20 2005
    copyright            : (C) 2004-2005 by Richard Groult
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

#ifndef KRAR_H
#define KRAR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <karchive.h>

#include <qobject.h>

class KProcess;

class KRarArchiveFile : public KArchiveFile
{
public:
	KRarArchiveFile(KArchive* t, const QString& name, int access, int date,
					const QString& user, const QString& group,
					const QString& symlink);
	virtual ~KRarArchiveFile(){};
	virtual QByteArray data() const;
};

class KRar : public QObject, public KArchive
{
	Q_OBJECT
public:
	KRar (const QString& filename);
	virtual ~KRar(){};

	virtual bool writeDir( const QString& name, const QString& user, const QString& group );
	virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );
	virtual bool doneWriting( uint size );

	static void setUnrarPath(const QString& path);
	static QString getUnrarPath();

protected:
	/**
	 * Opens the archive for reading.
	 * Parses the directory listing of the archive
	 * and creates the KArchiveDirectory/KArchiveFile entries.
	 * @param mode the mode of the file
	 */
	virtual bool openArchive( int mode );
	virtual bool closeArchive();

protected slots:
	void slotMsgRcv (KProcess *, char *, int);

private:
	QString m_filename;
	QString unrar_path;
	QStringList m_list;

};

#endif
