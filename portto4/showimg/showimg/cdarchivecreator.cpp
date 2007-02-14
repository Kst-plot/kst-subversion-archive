/*****************************************************************************
                          cdarchivecreator.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "cdarchivecreator.h"

//-----------------------------------------------------------------------------
#define CDARCHIVECREATOR_DEBUG 0
//-----------------------------------------------------------------------------

// Local
#include "cdarchive.h"
#include "listitemview.h"
#include "misc/exif.h"
#include "showimg_common.h"
#include "tools.h"

#ifdef HAVE_LIBEXIV2
#include <exiv2/exif.hpp>
#include "metadata/dmetadata.h"
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIV2
#endif
#endif /* HAVE_LIBEXIV2 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kapplication.h>
#include <karchive.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kimageeffect.h>
#include <kio/job.h>
#include <kimageio.h>
#include <klocale.h>
#include <kpixmapio.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kurl.h>
#include <ktempfile.h>
#include <ktempdir.h>

// Qt
#include <qdir.h>
#include <qimage.h>
#include <qstring.h>
#include <qwmatrix.h>

CDArchiveCreator::CDArchiveCreator(QWidget *a_p_parent, const QString& a_rootPath, const QString& a_archiveName)
	:QObject(a_p_parent),
	QThread()
{
#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<"a_rootPath="<<a_rootPath<< " a_archiveName="<<a_archiveName<<endl;
#endif
	m_p_parent        = a_p_parent;
	m_rootPath        = a_rootPath+'/';
	m_archiveName     = a_archiveName;
	m_p_kPio          = new KPixmapIO();
	m_p_rotateProcess = NULL;
	m_was_cancelled   = false;

	KStandardDirs::makeDir(QDir::homeDirPath()+"/.showimg/cdarchive/");
	
	KTempDir l_temp_dir(locateLocal("tmp", "showimg-arc/"));
	m_temp_rootPath = l_temp_dir.name()+m_rootPath;

#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<"m_temp_rootPath="<<m_temp_rootPath<<endl;
#endif
}

CDArchiveCreator::~CDArchiveCreator()
{
	delete(m_p_kPio);
}


void
CDArchiveCreator::run()
{
	createThumbnails();
}

void
CDArchiveCreator::parseDirectory()
{
	KIO::ListJob *job=KIO::listRecursive(KURL("file:"+m_rootPath), false);
	connect(job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
		this, SLOT(listRecursiveFinished(KIO::Job*, const KIO::UDSEntryList&)));
	connect(job, SIGNAL(result(KIO::Job* )),
		this, SLOT(listRecursiveDone(KIO::Job* )));

	EventData *l_p_data_event = new EventData;
		l_p_data_event->action   = Parse;
		l_p_data_event->fileName = m_rootPath;
		l_p_data_event->starting = true;
	KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_data_event));
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
				if(QFileInfo(m_rootPath+(*itUSDEntry).m_str).isFile())
				{
					m_fileList.append((*itUSDEntry).m_str);
				}
			}
		}
	}
}

void
CDArchiveCreator::listRecursiveDone(KIO::Job * )
{
	emit parseDirectoryDone();
}

void
CDArchiveCreator::tryTerminate()
{
#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<""<endl;
#endif
	m_was_cancelled = true;
	removeCahePath();
	//QThread::tryTerminate();
}

void
CDArchiveCreator::createThumbnails()
{
	EventData *l_p_data_event = new EventData;
	l_p_data_event->action   = Parse;
	l_p_data_event->fileName = m_rootPath;
	l_p_data_event->success = true;
	l_p_data_event->total = m_fileList.count();
	KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_data_event));

#ifndef HAVE_LIBEXIV2
	bool HAVE_convert  = !KStandardDirs::findExe(Tools::getConvertPath()).isNull();
	bool HAVE_jpegtran = !KStandardDirs::findExe(Tools::getJpegtranPath()).isNull();
#endif /* HAVE_LIBEXIV2 */

	////////////
	QString dest = m_temp_rootPath;
	for ( 
		QStringList::Iterator it = m_fileList.begin(); 
		it != m_fileList.end() && !m_was_cancelled; 
		++it )
	{
		QString l_original_image_path(m_rootPath+*it);
		QString l_thumb_path(dest+*it);
#if CDARCHIVECREATOR_DEBUG > 0
		MYDEBUG
			<<"l_original_image_path="<<l_original_image_path << " "
			<<"l_thumb_path="<<l_thumb_path << " "
			 <<endl;
#endif
		l_p_data_event = new EventData;
			l_p_data_event->action   = Progress;
			l_p_data_event->fileName = *it;
			l_p_data_event->starting = true;
			KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_data_event));

		if(QFileInfo(l_original_image_path).extension().lower() == QString::fromLatin1("sia") )
		{
			QFile orgF(l_original_image_path);
			QFile destF(l_thumb_path);
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
		if(Tools::isJPEG(l_original_image_path))
		{
#if CDARCHIVECREATOR_DEBUG > 0
		MYDEBUG<< "Tools::isJPEG(l_original_image_path))"<<endl;		
#endif

#ifndef HAVE_LIBEXIV2
			ProcessFile(
				QFile::encodeName(l_original_image_path),
				false,
				QFile::encodeName(l_thumb_path));


			KFileMetaInfo metadatas(l_original_image_path);
			if ( metadatas.isValid() )
			{
				KFileMetaInfoItem metaitem = metadatas.item("Orientation");
				if ( metaitem.isValid()
				#if QT_VERSION >= 0x030100
				&& !metaitem.value().isNull()
				#endif
        			)
				{
					rotateThumb(l_thumb_path, metaitem.value().toInt(), HAVE_jpegtran, HAVE_convert);
				}
			}
#else /* #ifndef HAVE_LIBEXIV2 */
			DMetadata l_exif_data(l_original_image_path, KImageIO::typeForMime(Tools::getJPEGMimeType()));
			QWMatrix l_transform_matrix;
			switch(l_exif_data.getImageOrientation())
			{
				case DMetadata::ORIENTATION_UNSPECIFIED   :  break;
				case DMetadata::ORIENTATION_NORMAL        :   break;
				case DMetadata::ORIENTATION_HFLIP         : l_transform_matrix.scale( 1, -1 ); break;
				case DMetadata::ORIENTATION_ROT_180       : l_transform_matrix.rotate(180);    break;
				case DMetadata::ORIENTATION_VFLIP         : l_transform_matrix.scale( -1, 1 ); break;
				case DMetadata::ORIENTATION_ROT_90_HFLIP  : l_transform_matrix.rotate(90);     l_transform_matrix.scale( 1, -1 ); break;
				case DMetadata::ORIENTATION_ROT_90        : l_transform_matrix.rotate(90);     break;
				case DMetadata::ORIENTATION_ROT_90_VFLIP  : l_transform_matrix.rotate(0);      l_transform_matrix.scale( -1, 1 ); break;
				case DMetadata::ORIENTATION_ROT_270       : l_transform_matrix.rotate(270);    break;
			}
			QImage l_exif_thumb(l_exif_data.getExifThumbnail(true).xForm(l_transform_matrix));
			if(!l_exif_thumb.isNull())
			{
#if CDARCHIVECREATOR_DEBUG > 0
				MYDEBUG<<"Save thumb" <<endl;
#endif
				if(!l_exif_thumb.save(l_thumb_path, KImageIO::typeForMime(Tools::getJPEGMimeType()).latin1(), 90))
				{
#if CDARCHIVECREATOR_DEBUG > 0
					MYDEBUG<<"Error saving thumb" <<endl;
#endif
				}
			}
#endif  /* #ifndef HAVE_LIBEXIV2 */			
		}
#if CDARCHIVECREATOR_DEBUG > 0
		else
			MYDEBUG<< "NOT ListItemView::isJPEG(l_original_image_path))"<<endl;
			
#endif

		if(
			! QFileInfo(l_thumb_path).exists() && 
			Tools::isImage(l_original_image_path) &&
			QFileInfo(l_original_image_path).extension().lower()!="xcf" 
		)
		{
#if CDARCHIVECREATOR_DEBUG > 0
			MYDEBUG<<"! QFileInfo(l_thumb_path).exists()=" << l_thumb_path <<endl;
#endif
			createThumb(*it);
		}



		EventData *l_p_end_data_event = new EventData;
			l_p_end_data_event->action   = Progress;
			l_p_end_data_event->fileName = *it;
			l_p_end_data_event->success = true;
			KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_end_data_event));
	}
	//---------------------------------------------------------------------
	if(m_was_cancelled)
	{
#if CDARCHIVECREATOR_DEBUG > 0
		MYDEBUG<<"m_was_cancelled !!!" <<endl;
#endif
	}
	else
	{
		QString arcDest = QDir::homeDirPath()+"/.showimg/cdarchive/"
			+m_archiveName
			+QString(".")+CDArchive_EXTENSION;
		l_p_data_event = new EventData;
			l_p_data_event->action   = Archive;
			l_p_data_event->fileName = arcDest;
			l_p_data_event->starting = true;
			l_p_data_event->success =  false;
			KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_data_event));

		EventData *l_p_end_data_event = new EventData;
			l_p_end_data_event->action   = Archive;
			l_p_end_data_event->fileName = arcDest;
			l_p_end_data_event->starting = false;

		if(m_p_rotateProcess != NULL) 
			m_p_rotateProcess->start (KShellProcess::Block);
#if KDE_VERSION >= 0x30200
		KArchive *arc = new KTar( arcDest, "application/x-tar" );
		if (!arc->open( IO_WriteOnly ))
		{
			l_p_end_data_event->success = false;
		}
		else
		{
			arc->addLocalDirectory(dest, "/");
			arc->close();
			l_p_end_data_event->success = true;
		}
		delete(arc);
#else
		QString com = QString("cd ") + KProcess::quote(dest) + " ; ";
		com += "tar cf " +KProcess::quote(arcDest) + " * ; ";
		KShellProcess* proc = new KShellProcess ();
		proc->clearArguments();
		*proc << com;
		proc->start (KShellProcess::Block);
	
		l_p_end_data_event->success = true;
#endif /* KDE_VERSION >= 0x30200 */

		KApplication::postEvent(m_p_parent, new QCustomEvent(QEvent::User, l_p_end_data_event));
		removeCahePath();
	}
}

QString
CDArchiveCreator::createCahePath(const QString& path)
{
	QString dest = m_temp_rootPath+QFileInfo(path).dirPath()+'/';
	KStandardDirs::makeDir(dest);
#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<"path="<<path<<"=>"<< dest <<endl;
#endif
	return dest;
}

void
CDArchiveCreator::removeCahePath()
{
#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<m_temp_rootPath<<endl;
#endif
	KIO::del(KURL("file:"+m_temp_rootPath), false, false);
}


bool
CDArchiveCreator::createThumb(const QString& filename)
{
			QFileInfo thumb(m_rootPath+filename);
			QImage im (thumb.absFilePath ());
			im.setAlphaBuffer(true);

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
					im.save(createCahePath(filename)+thumb.fileName(), KImageIO::typeForMime(Tools::getJPEGMimeType()).latin1(), 90);
					im.reset();
					return true;
				//}
			}
			return false;
}

void
CDArchiveCreator::rotateThumb(const QString& filename, int orientation, bool a_HAVE_jpegtran, bool a_HAVE_convert)
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
    	if(a_HAVE_jpegtran)
	{
		KTempFile l_temp_file(locateLocal("tmp", "showimg-tmp"));
		QString dest = l_temp_file.name();
			com = QString("%1 ").arg(KStandardDirs::findExe(Tools::getJpegtranPath()));
			com+= options;
			com += " -copy all -outfile ";
			com += ' ' 		+ KProcess::quote(dest);
			com += ' ' 		+ KProcess::quote(filename);
			com += "&& mv -f " 	+ KProcess::quote(dest);
			com += ' ' 		+ KProcess::quote(filename);
	}
	else
	if(a_HAVE_convert)
	{
			com = QString("%1 ").arg(Tools::getConvertPath());
			com += options;
			com += ' ' + KProcess::quote(filename);
			com += ' ' + KProcess::quote(filename);
	}
	else
	{
		MYWARNING<<"Unable to rotate file: jpegtran and convert are missing"<<endl;
	}
	if(!m_p_rotateProcess)
	{
		m_p_rotateProcess = new KShellProcess ();
		m_p_rotateProcess->clearArguments();
	}
#if CDARCHIVECREATOR_DEBUG > 0
	MYDEBUG<<com<<endl;
#endif
	*m_p_rotateProcess << com << " ; " ;
    }
}

#include "cdarchivecreator.moc"


