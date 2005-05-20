/***************************************************************************
                          batchrenamer.h  -  description
                             -------------------
    begin                : Sat Aug 18 2001
    copyright            : (C) 2001 by Dominik Seichter,
                                       Richard Groult
    email                : domseichter@web.de
                           rgroult@jalix.org
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

#ifndef BATCHRENAMER_H
#define BATCHRENAMER_H

// OS includes
#include <stdio.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>

// QT includes
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kmimetype.h>

// Own includes
#include "progressdialog.h"

class KFilePlugin;
class KLocale;

enum mode
{
        COPY, MOVE, RENAME, PREVIEW
};

struct data
{
    QString source;
    QString extension;
    QString source_path;

    QString final;
    QString final_path;
    int count;
};

struct datevals
{
    QDate date;

    bool bDate;
    bool changeModification;
    bool changeAccess;

    int hour;
    int minute;
    int second;
};

// Holds all necessary values
struct values
{
    QString text;
    QString dirname;

    int index;
    bool extension;
    bool overwrite;
    struct datevals dvals;
};

/**
  *@author Dominik Seichter
  */

class BatchRenamer {
public:
	BatchRenamer(ProgressDialog *p=NULL);
	~BatchRenamer();

	void setDateFormat(const QString& format);
	QString getDateFormat();
	void setTimeFormat(const QString& format);
	QString getTimeFormat();

	////////////////////////////////////////////////////////////////////
	void processFiles( struct data* files, enum mode m, struct values* val, bool preview = false );

	QString findOldName( const QString& oldname, const QString& text );
	QString findOldNameLower( const QString& oldname, const QString& text );
	QString findOldNameUpper( const QString& oldname, const QString& text );
	QString findStar ( const QString& oldname, const QString& text );
	QString findNumbers( const QString& text, int index, int count, int i);

	QString findBrackets( const QString& oldname, const QString& text, const QString& filePath );
	QString findToken( const QString& token, const QString& filePath );
	QString processToken( const QString& token, const QString& filePath );
	QString processFileToken(const QString& token, const QString& filePath );

	QStringList getKeys();

	static QString doEscape( const QString& text );
	static QString unEscape( const QString& text );
	static QString escape( const QString& text, const QString& token, const QString& sequence );

private:
	bool fcopy(const QString& src, const QString& dest );
	int getCharacters( int n ) ;
	void work( struct data * files, enum mode m, struct values * val, bool preview );
	bool changeDate( const QString& file, struct datevals dvals );

	void setPattern( KMimeType::Ptr mime );
	const QString getPattern() const;
	void setupKeys();

protected:
	QFile *f;
	ProgressDialog* p;

	QStringList keys;
	KFilePlugin *fileplugin;
	QString m_mimetype, m_name, m_pattern;

	KLocale *m_klocale;
	QString default_DateFormat, default_TimeFormat;
	QString m_DateFormat, m_TimeFormat;
};

#endif
