/***************************************************************************
                          cdarchivecreator.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004 by Richard Groult
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

#include "cdarchivecreator.h"

// Local
#include "cdarchive.h"
#include "exif.h"

// KDE
#include <kurl.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kimageeffect.h>
#include <kpixmapio.h>
#include <ktar.h>
#include <karchive.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kfilemetainfo.h>
#include <klocale.h>

// Qt
#include <qdir.h>
#include <qimage.h>
#include <qstring.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

CDArchiveCreator::CDArchiveCreator(QWidget *parent, const QString& rootPath, const QString& archiveName)
	:QObject(parent),
	QThread()
{
	this->parent=parent;
	this->rootPath=rootPath+"/";
	this->archiveName=archiveName;
	kPio=new KPixmapIO();
	rotateProcess=NULL;

	QDir().mkdir(QDir::homeDirPath()+"/.showimg/cdarchive/");
}

CDArchiveCreator::~CDArchiveCreator()
{
}


void
CDArchiveCreator::run()
{
	createThumbnails();
}

void
CDArchiveCreator::parseDirectory()
{
	KIO::ListJob *job=KIO::listRecursive(KURL("file:"+rootPath), false);
	connect(job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
		this, SLOT(listRecursiveFinished(KIO::Job*, const KIO::UDSEntryList&)));
	connect(job, SIGNAL(result(KIO::Job*)),
		this, SLOT(listRecursiveDone(KIO::Job*)));

	EventData *d = new EventData;
		d->action   = Parse;
		d->fileName = rootPath;
		d->starting = true;
	QApplication::postEvent(parent, new QCustomEvent(QEvent::User, d));
}

void
CDArchiveCreator::listRecursiveFinished(KIO::Job *, const KIO::UDSEntryList& list)
{
	KIO::UDSEntryList::const_iterator it = list.begin();
	for(; it != list.end(); ++it)
	{
		KIO::UDSEntry::const_iterator itUSDEntry = (*it).begin();
		for ( ; itUSDEntry != (*it).end(); ++itUSDEntry )
		{
			if((*itUSDEntry).m_uds == KIO::UDS_NAME)
			{
				if(QFileInfo(rootPath+(*itUSDEntry).m_str).isFile())
				{
					fileList.append((*itUSDEntry).m_str);
				}
			}
		}
	}
}

void
CDArchiveCreator::listRecursiveDone(KIO::Job *)
{
	emit parseDirectoryDone();
}
void
CDArchiveCreator::terminate()
{
	removeCahePath();
	QThread::terminate();
}

void
CDArchiveCreator::createThumbnails()
{
	EventData *d = new EventData;
	d->action   = Parse;
	d->fileName = rootPath;
	d->success = true;
	d->total = fileList.count();
	QApplication::postEvent(parent, new QCustomEvent(QEvent::User, d));

	/////////
	bool HAVE_convert = !KStandardDirs::findExe("convert").isNull();
	bool HAVE_jpegtran = ! KStandardDirs::findExe("jpegtran").isNull();

	////////////
	QString dest = locateLocal("tmp", "showimg-arc/"+rootPath);
	for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		d = new EventData;
			d->action   = Progress;
			d->fileName = *it;
			d->starting = true;
			QApplication::postEvent(parent, new QCustomEvent(QEvent::User, d));

		if(QFileInfo(rootPath+*it).extension().lower()=="sia" )
		{
			QFile orgF(rootPath+*it);
			QFile destF(dest+*it);
			if (orgF.open(IO_ReadOnly) && destF.open(IO_WriteOnly))
			{
				QTextStream tsorgF(&orgF);
				QTextStream tsdestF(&destF);
				QString lut;
				while(!tsorgF.eof())
				{
					tsdestF << tsorgF.readLine () << "\n";
				}
				orgF.close(); destF.close();
			}
		}
		else
		if(QFileInfo(rootPath+*it).extension().lower()=="jpg" )
		{
			ProcessFile(
				QFile::encodeName(rootPath+*it),
				false,
				QFile::encodeName(createCahePath(*it)+QFileInfo(*it).fileName()));

			KFileMetaInfo metadatas(rootPath+*it);
			if ( metadatas.isValid() )
			{
				KFileMetaInfoItem metaitem = metadatas.item("Orientation");
				if ( metaitem.isValid()
				#if QT_VERSION >= 0x030100
				&& !metaitem.value().isNull()
				#endif
        			)
				{
					rotateThumb(dest+*it, metaitem.value().toInt(), HAVE_jpegtran, HAVE_convert);
				}
			}
		}
		if(! QFileInfo(dest+*it).exists() && QFileInfo(rootPath+*it).extension().lower()!="xcf" )
		{
			createThumb(*it);
		}

		EventData *e = new EventData;
			e->action   = Progress;
			e->fileName = *it;
			e->success = true;
			QApplication::postEvent(parent, new QCustomEvent(QEvent::User, e));
	}

	QString arcDest = QDir::homeDirPath()+"/.showimg/cdarchive/"
		+archiveName
		+QString(".")+CDArchive_EXTENSION;
	d = new EventData;
		d->action   = Archive;
		d->fileName = arcDest;
		d->starting = true;
		d->success =  false;
		QApplication::postEvent(parent, new QCustomEvent(QEvent::User, d));

	EventData *e = new EventData;
		e->action   = Archive;
		e->fileName = arcDest;
		e->starting = false;

	if(rotateProcess != NULL) rotateProcess->start (KShellProcess::Block);
#if KDE_VERSION >= 0x30200
	KArchive *arc = new KTar( arcDest, "application/x-tar" );
	if (!arc->open( IO_WriteOnly ))
	{
		e->success = false;
	}
	else
	{
		arc->addLocalDirectory(dest, "/");
		arc->close();
		e->success = true;
	}
#else
	QString com = QString("cd ") + KProcess::quote(dest) + " ; ";
	com += "tar cf " +KProcess::quote(arcDest) + " * ; ";
	KShellProcess* proc = new KShellProcess ();
	proc->clearArguments();
	*proc << com;
	proc->start (KShellProcess::Block);

	e->success = true;
#endif /* KDE_VERSION >= 0x30200 */

	QApplication::postEvent(parent, new QCustomEvent(QEvent::User, e));
	removeCahePath();
}

QString
CDArchiveCreator::createCahePath(const QString& path)
{
	QString dest = locateLocal("tmp", "showimg-arc/")+rootPath+QFileInfo(path).dirPath()+"/";
	KStandardDirs::makeDir(dest);
	return dest;
}

void
CDArchiveCreator::removeCahePath()
{
	QString dest = locateLocal("tmp", "showimg-arc/"+rootPath);
	KIO::del(KURL("file:"+dest), false, false);
}


bool
CDArchiveCreator::createThumb(const QString& filename)
{
			QFileInfo thumb(rootPath+filename);
			QImage im (thumb.absFilePath ());
			im.setAlphaBuffer(true);
			/*
			double
			 wexpand = (double) im.width () / (double)getThumbnailSize().width(),
			 hexpand = (double) im.height () / (double)getThumbnailSize().height();
			*/
			double
			 wexpand = (double) im.width () / (double)160,
			 hexpand = (double) im.height () / (double)120;
			if (wexpand >= 1.0 || hexpand >= 1.0) // don't expand small images  !
			if(!im.isNull())
			{
				int neww, newh;
				if (wexpand > hexpand)
				{
					neww = (int) (im.width () / wexpand );
					newh = (int) (im.height () / wexpand );
				}
				else
				{
					neww = (int) (im.width () / hexpand );
					newh = (int) (im.height () / hexpand );
				 }
				im=im.smoothScale(neww,newh);
			}
			if(!im.isNull())
			{
					im.save(createCahePath(filename)+thumb.fileName(), "JPEG", 90);
					im.reset();
					return true;
				//}
			}
			return false;
}

void
CDArchiveCreator::rotateThumb(const QString& filename, int orientation, bool HAVE_jpegtran, bool HAVE_convert)
{
	QString options;
    switch ( orientation )
    {
        //  Orientation:
        //  1:      normal
        //  2:      flipped horizontally
        //  3:      ROT 180
        //  4:      flipped vertically
        //  5:      ROT 90 -> flip horizontally
        //  6:      ROT 90
        //  7:      ROT 90 -> flip vertically
        //  8:      ROT 270

        case 1:
		return;
		break;
        case 2:
			options="";
		break;
        case 3:
			options="-rotate 180";
		break;
        case 4:
			options="";
		break;
        case 5:
			options="";
		break;
        case 6:
			options="-rotate 90";
		break;
        case 7:
			options="";
		break;
        case 8:
			options="-rotate 270";
                break;
    }
    QString com;
    if(!options.isEmpty())
    {
    	if(HAVE_jpegtran)
	{
		QString dest = locateLocal("tmp", "showimg-tmp/"+filename);
			com = QString("jpegtran ");
			com+= options;
			com += " -copy all -outfile ";
			com += " " 		+ KProcess::quote(dest);
			com += " " 		+ KProcess::quote(filename);
			com += "&& mv -f " 	+ KProcess::quote(dest);
			com += " " 		+ KProcess::quote(filename);
	}
	else
	if(HAVE_convert)
	{
			com = QString("convert ");
			com += options;
			com += " " + KProcess::quote(filename);
			com += " " + KProcess::quote(filename);
	}
	else
	{
		kdWarning()<<"Unable to rotate file: jpegtran and convert are missing"<<endl;
	}
	if(!rotateProcess)
	{
		rotateProcess = new KShellProcess ();
		rotateProcess->clearArguments();
	}
	*rotateProcess << com << " ; " ;
    }
}

#include "cdarchivecreator.moc"


