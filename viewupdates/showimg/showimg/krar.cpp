/***************************************************************************
                         krar.cpp  -  description
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

 // Local
#include "krar.h"


// KDE
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/job.h>

//Qt
#include <qfile.h>
#include <qfileinfo.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

static QString m_unrarpath;

KRarArchiveFile::KRarArchiveFile(KArchive* t, const QString& name, int access, int date,
	const QString& user, const QString& group,
	const QString& symlink)
	: KArchiveFile(t, name, access, date, user, group, symlink, 0, 0)
{
}


QByteArray
KRarArchiveFile::data() const
{
	QFile f(archive()->directory()->name()+"/"+name());
	f.open(IO_ReadOnly);
	QByteArray m_data(f.readAll());
	f.close();
	return m_data;
}

KRar::KRar (const QString& filename)
	: KArchive( 0L )
{
	m_filename = filename;

	setUnrarPath("/usr/bin/unrar");
}

void
KRar::setUnrarPath(const QString& path)
{
	m_unrarpath = path;
}
QString
KRar::getUnrarPath()
{
	return m_unrarpath;
}

bool
KRar::writeDir( const QString& /*name*/, const QString& /*user*/, const QString& /*group*/ )
{
	return false;
}

bool
KRar::prepareWriting( const QString& /*name*/, const QString& /*user*/, const QString& /*group*/, uint /*size*/ )
{
	return false;
}


bool
KRar::doneWriting( uint /*size*/ )
{
	return false;
}

bool
KRar::openArchive( int mode )
{
	if(mode != IO_ReadOnly)
		return false;
	if ( !QFile::exists( m_filename ) )
	{
		MYDEBUG<<"File does not exists "<<m_filename<<endl;
		return false;
	}


	//--------------------------------------------------
		//--------------------------------------------------
	QFileInfo info(m_filename);
	QString  tmpdir = locateLocal("tmp", "showimg-cpr/arc/"+info.fileName()+"/");

	KShellProcess * proc = new KShellProcess ();
	QString com;
	com = QString("%1 e -y %2 %3")
			.arg(getUnrarPath())
			.arg(KProcess::quote(m_filename))
			.arg(KProcess::quote(tmpdir));
	*proc << com;
	proc->start (KShellProcess::Block, KShellProcess::Stdout);

	//--------------------------------------------------
	com = QString("%1 vb %2")
		.arg(getUnrarPath())
		.arg(KProcess::quote(m_filename));
	proc->clearArguments();
	connect (proc, SIGNAL (receivedStdout (KProcess *, char *, int)), this,
			 SLOT (slotMsgRcv (KProcess *, char *, int)));
	*proc << com;
	proc->start (KShellProcess::Block, KShellProcess::Stdout);
	delete(proc);

	//--------------------------------------------------
	KArchiveDirectory *m_rootDir;
	m_rootDir = new KArchiveDirectory( this, tmpdir, IO_ReadOnly,
									 info.lastModified().toTime_t(),
									 info.owner(), info.group(), info.readLink() );
	setRootDir(m_rootDir);


	//--------------------------------------------------
	KRarArchiveFile* e;
// 	int pos=0;
	for (QStringList::Iterator it = m_list.begin(); it!=m_list.end(); ++it)
	{
		QFileInfo f_entry(tmpdir+*it);
		if(f_entry.exists())
		{
			e = new KRarArchiveFile( this, *it, IO_ReadOnly,
									f_entry.lastModified().toTime_t(), f_entry.owner(), f_entry.group(), f_entry.readLink());
			rootDir()->addEntry( e );
		}
	}

	return true;
}


bool
KRar::closeArchive()
{
	QFileInfo info(m_filename);
	QString  tmpdir = locateLocal("tmp", "showimg-cpr/arc/"+info.fileName());
	KURL::List list;
	KURL url;
	url.setPath(tmpdir);
	KIO::del( url );
	return true;
}


void
KRar::slotMsgRcv (KProcess * , char *buffer, int buflen)
{
// 	m_stdout = new QCString (buffer, buflen);
	QString name = QString(QCString (buffer, buflen));
	int pos = name.findRev( '/' );
	if ( pos != -1 )
		name = name.mid( pos + 1 );
	m_list.append( name );
}

#include "krar.moc"



