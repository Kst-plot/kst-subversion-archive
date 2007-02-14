/*****************************************************************************
                          cdarchivecreator.h  -  description
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

#ifndef CDARCHIVECREATOR_H
#define CDARCHIVECREATOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kio/job.h>

// Qt
#include <qobject.h>
#include <qthread.h>
#include <qstring.h>

class KPixmapIO;
class KShellProcess;

class QFileInfo;

enum Action
{
	Parse = 0,
	Progress,
	Archive,
	Canceled
};

class EventData
{
public:
    EventData()
       {
       starting = false;
       success  = false;
       }

    QString fileName;
    QString errString;
    int     total;
    bool    starting;
    bool    success;
    Action  action;
};


class CDArchiveCreator : public QObject, public QThread
{
    Q_OBJECT


public:
	CDArchiveCreator(QWidget *a_p_parent, const QString& a_rootPath, const QString& a_archiveName);
	virtual ~CDArchiveCreator();

	void parseDirectory();
	virtual void run();
	virtual void tryTerminate();
signals:
	void parseDirectoryDone();

protected:
	void createThumbnails();

	QString createCahePath(const QString& path);
	void removeCahePath();
	bool createThumb(const QString& filename);
	void rotateThumb(const QString& filename, int orientation, bool HAVE_jpegtran, bool HAVE_convert);

protected slots:
	void listRecursiveFinished(KIO::Job*, const KIO::UDSEntryList&);
	void listRecursiveDone(KIO::Job *);
private:
	QWidget 
		*m_p_parent;
	QString 
		m_rootPath,
		m_archiveName,
		m_temp_rootPath;
	bool
		m_was_cancelled;
	QStringList 
		m_fileList;
	KPixmapIO 
		*m_p_kPio;

	KShellProcess 
		*m_p_rotateProcess;
};

#endif
