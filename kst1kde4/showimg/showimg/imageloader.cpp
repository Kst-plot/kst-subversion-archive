/*****************************************************************************
                          imageloader.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
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

#include "imageloader.h"

//-----------------------------------------------------------------------------
#define IMALOADER_DEBUG_THREAD       0
//-----------------------------------------------------------------------------

// Local
#include "fileiconitem.h"
#include "imagefileinfo.h"
#include "imagelistview.h"
#include "misc/exif.h"
#include "showimg_common.h"
#include "tools.h"

#ifdef HAVE_LIBEXIF
#include "misc/jpeg-data.h"

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* HAVE_LIBEXIF */

#endif

#ifdef HAVE_LIBEXIV2
#include <exiv2/exif.hpp>
#include <libkexiv2/kexiv2.h>
//#include "metadata/dmetadata.h"
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIV2
#endif
#endif /* HAVE_LIBEXIV2 */

// KDE
#include <kapplication.h>
#include <kglobal.h>
#include <kimageeffect.h>
#include <kimageio.h>
#include <kio/previewjob.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kpixmap.h>
#include <kpixmapio.h>
#include <kprocess.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kurifilter.h>
#include <kurl.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

// QT
#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstring.h>

// System
#include <math.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


class ImageLoadEvent : public QEvent
{
public:
	ImageLoadEvent (const QFileInfo& fi, bool t = true, bool f = false, bool fEXIF=false)
	: QEvent (Event_ImageLoad),

	  m_info (fi),
	  m_is_threaded (t),
	  m_is_force(f),
	  m_is_forceEXIF(fEXIF)
	 {
	 }

	 virtual ~ImageLoadEvent()
	 {
	 }

	QFileInfo fileInfo () const
	{
		return m_info;
	}
	bool threaded () const
	{
		return m_is_threaded;
	}
	bool force () const
	{
		return m_is_force;
	}
	bool forceEXIF () const
	{
		return m_is_forceEXIF;
	}
private:
	QFileInfo m_info;
	bool m_is_threaded;
	bool m_is_force;
	bool m_is_forceEXIF;
};



#define	Event_NextImage		QEvent::Close
class NextImageEvent:public QEvent
{
public:
	NextImageEvent ():QEvent (Event_NextImage)
	{
	}
	 virtual ~NextImageEvent()
	 {
	 }
};



#ifndef Q_WS_WIN
void
__thread_cleanup (void *arg)
{
	((ImageLoader *) arg)->thread_cleanup ();
}

void *
__thread_start (void *arg)
{
	pthread_cleanup_push (__thread_cleanup, arg);
	((ImageLoader *) arg)->thread_start ();
	pthread_cleanup_pop (0);
	pthread_detach (pthread_self ());
	return 0;
}
#endif


ImageLoader::ImageLoader (ImageListView * a_p_parent, const char *a_p_name)
	: QObject (a_p_parent, a_p_name)
{
	m_p_imageList = a_p_parent;

	m_eventList.setAutoDelete (true);
	m_size=QSize(80,60);
	m_showFrame=false;
	m_is_loading = m_is_unning = false;
	m_force = m_forceEXIF = false;


	m_top_image=QImage(locate("appdata", "pics/border.png"));
	m_bg_pixmap=QPixmap(locate("appdata", "pics/bgxpm.png"));

	m_p_kPio = new KPixmapIO();

	m_thumbRoot_ShowImg=QDir::homeDirPath () + "/.showimg/cache/";
	m_thumbRoot_KDE=QDir::homeDirPath() + "/.thumbnails/normal/";

	installEventFilter (this);
	setThumbnailSize(m_size);
	setStoreThumbnails(true);
	if(! KStandardDirs::exists(thumbnailRootPath()))
			KStandardDirs::makeDir(thumbnailRootPath());
}

ImageLoader::~ImageLoader ()
{
	stopLoading (true);
	delete(m_p_kPio);
}

void
ImageLoader::setShowFrame(bool show)
{
	m_showFrame=show;
}
bool
ImageLoader::getShowFrame()
{
	return m_showFrame;
}

void
ImageLoader::setUseEXIF(bool useexif)
{
	m_is_use_EXIF=useexif;
}
bool
ImageLoader::getUseEXIF()
{
	return m_is_use_EXIF;
}

QPixmap
ImageLoader::addBorder(QPixmap *pix, bool hasAlpha)
{
	if(
			!m_showFrame ||
			(pix->width()<16 && pix->height()<16)
	)
		return *pix;

	QPixmap res(pix->size());
	if(hasAlpha)
		res.fill(m_p_imageList->paletteBackgroundColor());
	QWMatrix l_scale;
	l_scale.scale(0.79738562092, 0.76691729323);
	QPainter p(&res);
		p.drawImage(0, 0, m_top_image.scale(pix->width(), pix->height()));
		p.drawImage(
			(int)floor((float)pix->width()/m_top_image.width()*14),
			(int)floor((float)pix->height()/m_top_image.height()*13),
			pix->convertToImage().smoothScale
			(
				(int)ceil(pix->width()*0.79738562092)+1,
				(int)ceil(pix->height()*0.76691729323)+1
			)
		);
	p.end();

	return res;
}

QPixmap
ImageLoader::addForeground(QPixmap *a_p_pix, bool hasAlpha)
{
	if(
		!hasAlpha ||
		(
			a_p_pix->width()  < getThumbnailSize().width() &&
			a_p_pix->height() < getThumbnailSize().height()
		)
	)
		return *a_p_pix;

	QPixmap l_res_pix(a_p_pix->size());
	l_res_pix.fill(m_p_imageList->paletteBackgroundColor());
	QPainter p(&l_res_pix);
		p.drawPixmap(0,0,*a_p_pix);
	p.end();
	return l_res_pix;
}

void
ImageLoader::setThumbnailSize(const QSize newSize)
{
		m_size=newSize;
}


QSize
ImageLoader::getThumbnailSize()
{
	return m_size;
}

void
ImageLoader::reduce(QImage *im, int w, int h, bool force)
{
#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG << "QImage *im="<< im <<" int w="<< w <<", int h="<< h<<", bool force="<< force<<endl;
#endif
	if(im->isNull()) 
		return;

	double
		wexpand = (double) w / (double) im->width (),
		hexpand = (double) h / (double) im->height ();
	if (!force && (wexpand >= 1.0 || hexpand >= 1.0)) // don't expand small images  !
	{
		return;
	}


	int neww, newh;
	if (wexpand < hexpand)
	{
		neww = (int) ceil(im->width () * wexpand );
		newh = (int) ceil(im->height () * wexpand );
	}
	else
	{
		neww = (int) ceil(im->width () * hexpand );
		newh = (int) ceil(im->height () * hexpand );
	}
	*im = im->smoothScale(neww,newh);
}
void
ImageLoader::loadMiniImage (const QFileInfo& a_image_fileinfo, bool threaded, bool force, bool forceEXIF)
{
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<endl;
#endif
		ImageLoadEvent *l_p_event = new ImageLoadEvent (a_image_fileinfo, threaded, force, forceEXIF);
		m_eventList.append (l_p_event);
		if (m_eventList.count () > 0 && !m_is_unning)
		{
			m_is_unning = true;
			startTimer (1);
			nextImage ();
		}
}

void
ImageLoader::startLoading ()
{
	m_is_unning = true;
	ImageLoadEvent *l_p_event = ((int) (m_eventList.count ()) > 0 ? m_eventList.take (0) : 0);
	if (!l_p_event)
	{
		m_is_unning = m_is_loading = false;
		killTimers ();

		return;
	}
	if (!initLoading (l_p_event))
	{
		cantLoad (l_p_event);
		return;
	}
	m_is_loading = true;
	loadImageInternal (l_p_event);
}


void
ImageLoader::stopLoading (bool clean)
{
	if (m_is_loading)
	{
#ifndef Q_WS_WIN
		pthread_cancel (m_threadID);
		pthread_join (m_threadID, NULL);
#endif
		m_is_loading = m_is_unning = false;
		killTimers ();
		m_imageLoadedList.clear ();
	}
	if (clean)
	{
		m_eventList.clear ();
	}
	m_internal_image.reset();
}

void
ImageLoader::cantLoad (ImageLoadEvent * a_p_event)
{
	(void)kapp->postEvent (m_p_imageList, a_p_event);
	m_is_loading = false;
	nextImage ();
}

bool
ImageLoader::eventFilter (QObject *, QEvent * a_p_event)
{
	switch (a_p_event->type ())
	{
		case Event_NextImage:
			startLoading ();
			return true;
		case Event_ImageLoad:
		{
		 	 m_is_loading = false;
		 	 ImageLoadEvent *ev = new ImageLoadEvent (*((ImageLoadEvent *) a_p_event));
		 	 finishLoading (ev);
		 	 (void)kapp->postEvent (m_p_imageList, ev);
		 	 nextImage ();
		 	 return true;
		}
		default : return false;
	}
	return false;
}


void
ImageLoader::nextImage ()
{
	if (!m_is_loading)
	{
		NextImageEvent *l_p_event = new NextImageEvent;
		(void)kapp->postEvent (this, l_p_event);
	}
}


void
ImageLoader::thread_start ()
{
#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	bool l_is_cache_used = false;

#ifndef IMALOADER_USE_CACHE 
	QFileInfo
		l_thumb_fileinfo,
		l_image_fileinfo;

	if(m_image_url.isLocalFile() && QFileInfo(m_image_url.path()).exists())	
	{
		l_image_fileinfo = QFileInfo(m_image_url.path());
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"l_image_fileinfo="<<l_image_fileinfo.absFilePath()<<endl;
#endif
		if(l_image_fileinfo.size()<=0x2800) //10kb
			l_thumb_fileinfo = QFileInfo(l_image_fileinfo);
		else
			l_thumb_fileinfo = QFileInfo(thumbnailPath(l_image_fileinfo.absFilePath()));
			
		if(!l_thumb_fileinfo.exists())
		{
#if IMALOADER_DEBUG_THREAD > 0
			MYDEBUG<<"!l_thumb_fileinfo.exists() = "<< l_thumb_fileinfo.absFilePath() <<endl;
			MYDEBUG
				<<"m_force="<<m_force<< " "
				<<"m_forceEXIF="<<m_forceEXIF<< " "
				<<"m_is_use_EXIF="<<m_is_use_EXIF<< " "
				<<endl;
#endif
			if(
	 				(!m_force || !m_forceEXIF) &&
					m_is_use_EXIF &&
					Tools::isJPEG(l_image_fileinfo)
			)
			{
#ifndef HAVE_LIBEXIV2
#if IMALOADER_DEBUG_THREAD > 0
				MYDEBUG<<endl;
#endif
				QString l_tmp_thumb_path = locateLocal("tmp", "thumb.jpg");
				if("OK" == ProcessFile(
					QFile::encodeName(l_image_fileinfo.absFilePath()),
					false,
					QFile::encodeName(l_tmp_thumb_path)))
				{
					m_internal_image.load (l_tmp_thumb_path);
					l_is_cache_used = true;
				}
#else // HAVE_LIBEXIV2
#if IMALOADER_DEBUG_THREAD > 0
				MYDEBUG<<"KExiv2Iface::KExiv2 l_exif_data(l_image_fileinfo.absFilePath ())="<<l_image_fileinfo.absFilePath ()<<endl;
#endif
				KExiv2Iface::KExiv2 l_exif_data(l_image_fileinfo.absFilePath ());
				QWMatrix l_transform_matrix;
				switch(l_exif_data.getImageOrientation())
				{
					case KExiv2Iface::KExiv2::ORIENTATION_UNSPECIFIED   :    break;
					case KExiv2Iface::KExiv2::ORIENTATION_NORMAL        :    break;
					case KExiv2Iface::KExiv2::ORIENTATION_HFLIP         : l_transform_matrix.scale( 1, -1 ); break;
					case KExiv2Iface::KExiv2::ORIENTATION_ROT_180       : l_transform_matrix.rotate(180);    break;
					case KExiv2Iface::KExiv2::ORIENTATION_VFLIP         : l_transform_matrix.scale( -1, 1 ); break;
					case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP  : l_transform_matrix.rotate(90);     l_transform_matrix.scale( 1, -1 ); break;
					case KExiv2Iface::KExiv2::ORIENTATION_ROT_90        : l_transform_matrix.rotate(90);     break;
					case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_VFLIP  : l_transform_matrix.rotate(0);      l_transform_matrix.scale( -1, 1 ); break;
					case KExiv2Iface::KExiv2::ORIENTATION_ROT_270       : l_transform_matrix.rotate(270);    break;
				}
				QImage l_exif_thumb(l_exif_data.getExifThumbnail(true).xForm(l_transform_matrix));
				if(!l_exif_thumb.isNull())
				{
#if IMALOADER_DEBUG_THREAD > 2
					MYDEBUG<< "!l_exif_data.getExifThumbnail(true).isNull()" <<endl;
#endif
					m_internal_image = l_exif_thumb;
					l_is_cache_used = true;
				}
#if IMALOADER_DEBUG_THREAD > 0
				else
				{
					MYDEBUG<<"l_exif_data.getThumbnail().isNull()"<<endl;
				}
#endif
#endif /* HAVE_LIBEXIF2 */
			} // (!m_force || !m_forceEXIF) &&  m_is_use_EXIF && Tools::isJPEG(l_image_fileinfo)
		} /*  !l_thumb_fileinfo.exists() */

		if(
			!l_is_cache_used &&
			l_image_fileinfo.lastModified() <= l_thumb_fileinfo.lastModified()
		)
		{
#if IMALOADER_DEBUG_THREAD > 0
			MYDEBUG<< "l_image_fileinfo.lastModified() <= l_thumb_fileinfo.lastModified()" <<endl;
#endif
#if IMALOADER_DEBUG_THREAD > 0
			MYDEBUG<<"I'll load from cache l_thumb_fileinfo = "<< l_thumb_fileinfo.absFilePath() <<endl;
#endif
			m_internal_image.load(l_thumb_fileinfo.absFilePath ());
			l_is_cache_used = true;
		} // !l_is_cache_used && l_image_fileinfo.lastModified() <= l_thumb_fileinfo.lastModified()
#else
#warning ! IMALOADER_USE_CACHE
#endif // IMALOADER_USE_CACHE
		if(!l_is_cache_used)
		{
#if IMALOADER_DEBUG_THREAD > 0
			MYDEBUG<<"!l_is_cache_used, I'll load from original file="<< m_image_url.url() <<endl;
#endif
			m_internal_image.load (m_image_url.path());
		}
	} /* m_image_url.isLocalFile() */

	m_imageLoadedList.append (m_p_internalEvent);
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"END"<<endl;
#endif

}


void
ImageLoader::thread_cleanup ()
{
}


void
ImageLoader::timerEvent (QTimerEvent * )
{
	while (m_imageLoadedList.count () > 0)
	{
		ImageLoadEvent *l_p_event = m_imageLoadedList.take (0);
		(void)kapp->postEvent (this, l_p_event);
	}
}


bool
ImageLoader::initLoading (ImageLoadEvent * a_p_event)
{
#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG<<"a_p_event->fileInfo ().absFilePath ()="<<a_p_event->fileInfo ().absFilePath ()<<endl;
#endif
	m_image_path = a_p_event->fileInfo ().absFilePath ();
	m_force=a_p_event->force ();
	m_forceEXIF=a_p_event->forceEXIF ();
	
	m_image_url.setPath(m_image_path);

	if (!m_is_mini_image_file_exists || m_is_mini_image_outdated)
		return true;
	return false;
}


void
ImageLoader::finishLoading (ImageLoadEvent * a_p_event)
{
#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG<<"BEGIN " << a_p_event->fileInfo ().absFilePath() <<endl;
#endif
	QFileInfo l_image_fileinfo(a_p_event->fileInfo ());
	bool l_force = a_p_event->force();
	bool l_force_EXIF = a_p_event->forceEXIF();
	QImage l_image = m_internal_image;
	bool l_success = true;

	if (l_image.isNull ())
	{
		l_image = BarIcon("file_broken",48).convertToImage();
		l_success = false;
	}

	FileIconItem *l_p_item = NULL;
	if(
			l_force_EXIF &&
			l_success
			&& (l_p_item=m_p_imageList->findItem(l_image_fileinfo.filePath(), true)) != NULL
		)
	{
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"All is ok! : l_force_EXIF && l_success && l_p_item" <<endl;
#endif
		int neww, newh;

		//-------------------------------------------------------------
		if(l_image.width ()>l_image.height ())
			{neww=160;newh=120;}
		else
			{neww=120;newh=160;}

#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"I'll reduce" <<endl;
#endif
		reduce(&l_image, neww, newh);

		if(l_p_item->isSelected())
		{
#if IMALOADER_DEBUG_THREAD > 3
			MYDEBUG<<"Item is selected" <<endl;
#endif
			bool l_is_exif_set  = true;
			l_is_exif_set = ImageLoader::setEXIFThumbnail(l_image_fileinfo.absFilePath(), l_image);
#if IMALOADER_DEBUG_THREAD > 3
			if(l_is_exif_set)
				MYDEBUG<<" EXIF saved l_thumbnail_image="<<l_image_fileinfo.absFilePath()<<endl;
			else
				MYDEBUG<<"Unable to save EXIF l_thumbnail_image "<<l_image_fileinfo.absFilePath()<<endl;
#endif
		}
#if IMALOADER_DEBUG_THREAD > 3
		else
		{
			MYDEBUG<<"Item is NOT selected"<<endl;
		}
#endif
	}
	
	//---------------------------------------------------------------------
	QFileInfo l_thumb_fileinfo(thumbnailPath(l_image_fileinfo.absFilePath()));
	if(
		getStoreThumbnails() &&
		l_success&&
		l_image_fileinfo.lastModified() > l_thumb_fileinfo.lastModified()
	)
	{
		int neww, newh;
		if(l_image.width ()>l_image.height ())
			{neww=128;newh=96;}
		else
			{neww=96;newh=128;}

#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"I'll reduce" <<endl;
#endif
		reduce(&l_image, neww, newh);

#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"I set thumb info" <<endl;
#endif
		// set the l_thumbnail_image attributes using the
		// freedesktop.org standards
		l_image.setText(QString("Thumb::URI").latin1(),
					0, "file://" + QDir::cleanDirPath(l_image_fileinfo.absFilePath () ));
		l_image.setText(QString("Thumb::MTime").latin1(),
					0, QString::number(l_image_fileinfo.lastModified().toTime_t()));
		l_image.setText(QString("Software").latin1(),
					0, QString("ShowImg Thumbnail Generator"));
#if IMALOADER_DEBUG_THREAD > 2
		MYDEBUG<<"Thumb will be saved" <<endl;
#endif
 		if(l_image.save(createCahePath(l_image_fileinfo.absFilePath()), "PNG", 0))
		{
#if IMALOADER_DEBUG_THREAD > 2
			MYDEBUG<<"Thumb saved" <<endl;
#endif
			chmod(QFile::encodeName(createCahePath(l_image_fileinfo.absFilePath())), S_IREAD|S_IWRITE);
		}
 		else
 		{
 			MYWARNING<<"Unable to save l_thumbnail_image "<<l_image_fileinfo.absFilePath()<<endl;
 		}
	}

	reduce(&l_image, getThumbnailSize().width(), getThumbnailSize().height(), true);

	QPixmap l_mini_pixmap;
	if(l_image.hasAlphaBuffer())
	{
		int options=ThresholdAlphaDither;//ColorOnly||ThresholdDither||ThresholdAlphaDither||AvoidDither;
		l_mini_pixmap.convertFromImage(l_image, options);
		l_mini_pixmap = addForeground(&l_mini_pixmap, true);
		l_image = m_p_kPio->convertToImage(l_mini_pixmap);
		l_image.setAlphaBuffer (true);
	}
	l_mini_pixmap = m_p_kPio->convertToPixmap(l_image);
	if(!l_mini_pixmap.isNull ())
	{
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"!l_mini_pixmap.isNull ()" <<endl;
#endif
		m_p_imageList->slotSetPixmap (addBorder(&l_mini_pixmap, false), l_image_fileinfo, l_success, l_force, l_force_EXIF);
	}
	else
	{
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"l_mini_pixmap.isNull ()" <<endl;
#endif
		m_p_imageList->slotSetPixmap (BarIcon("file_broken",48), l_image_fileinfo, l_success, l_force, l_force_EXIF);
	}
	m_internal_image.reset ();
	l_image.reset();
#if IMALOADER_DEBUG_THREAD > 0
		MYDEBUG<<"END" <<endl;
#endif
}


void
ImageLoader::loadImageInternal (ImageLoadEvent * a_p_event)
{
	//m_internalPath = m_image_url.path ();
	m_p_internalEvent = a_p_event;
	m_internal_image.reset ();
	// Problem here with xpm and xbm image, interfere with X system and can cause crash.
	// As it is probably a small image, it's probably not a problem to load it synchronously.

	if (!a_p_event->threaded ())
		thread_start ();
#ifndef Q_WS_WIN
	else
		if (pthread_create (&m_threadID, 0, __thread_start, this) == 0);
#endif
	else
		MYWARNING<< " ImageLoader::loadImageInternal (ImageLoadEvent * a_p_event) : unable to start loading thread";

}


void
ImageLoader::setStoreThumbnails(bool store)
{
	m_is_store_th=store;
}

bool
ImageLoader::getStoreThumbnails()
{
	return m_is_store_th;
}

QString
ImageLoader::createCahePath(const QString& path)
{
	return thumbnailPath(path);
}

void
ImageLoader::rotateThumbnailLeft(QFileInfo* /* fi*/ )
{
	MYWARNING<<"ImageLoader::rotateThumbnailLeft(QFileInfo *fi)"<<endl;
}

void
ImageLoader::rotateThumbnailRight(QFileInfo* /* fi */)
{
	MYWARNING<<"ImageLoader::rotateThumbnailRight(QFileInfo *fi)"<<endl;
}

QString
ImageLoader::thumbnailRootPath()
{
	return m_thumbRoot_KDE;
}

QString
ImageLoader::thumbnailPath(const QString& path)
{
	KMD5 md5( QFile::encodeName( "file://" + QDir::cleanDirPath(path)));
	QString encName = QFile::encodeName( md5.hexDigest() ) + ".png";
	return thumbnailRootPath()+encName;
}

KURL::List
ImageLoader::clearThumbnailDir(const QString& dirpath)
{
	KProgressDialog	*l_p_progressDlg = new KProgressDialog(m_p_imageList, "Remove cached thumbnails",
				i18n("Remove cached thumbnails in progress..."),
				QString::null,
				true);
	l_p_progressDlg->show();
	l_p_progressDlg->setLabel(i18n("Checking for thumbnails in cache..."));
	l_p_progressDlg->adjustSize();
 	kapp->processEvents();

	QDir d(thumbnailRootPath());
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *thlist;
	KApplication::setOverrideCursor (waitCursor);
	if(!(thlist = d.entryInfoList()))
		;//return KURL::List();
	KApplication::restoreOverrideCursor();

	l_p_progressDlg->progressBar()->setTotalSteps(thlist->count());

	int m_current=0;
	int totalSize=0;
	KURL::List thlist_rm;
	QFileInfoListIterator it( *thlist );
	QFileInfo *thfi;
	QImage im;
	QDateTime date_last=QDateTime::currentDateTime ();
	while ( (thfi = it.current()) != 0 )
	{
		im.load(thfi->absFilePath());
		KURIFilterData urid = im.text("Thumb::URI");
		KURIFilter::self()->filterURI( urid );
		QFileInfo fi(urid.uri().path());
		if(fi.filePath().startsWith(dirpath))
		{
			thlist_rm.append(thfi->absFilePath());
			totalSize+=thfi->size();
		}
		++it;
		m_current++;

		l_p_progressDlg->progressBar()->setProgress(m_current);
		if(date_last.time().msecsTo(QDateTime::currentDateTime().time()) >= 500)
		{
			date_last=QDateTime::currentDateTime ();

			if(!fi.fileName().isEmpty())
				l_p_progressDlg->setLabel(
						"<qt>"
						+i18n("Directory: ")
						+QString("<b>%1</b><br>")
							.arg(dirpath)
						+i18n("Number of thumbnails to delete:")
						+QString("<center>%1 (%2 Kb)</center>")
							.arg(thlist_rm.count())
							.arg(KGlobal::locale()->formatNumber((float)totalSize/1024, 2))
						+i18n("Studying and seeking:")
						+QString("<center>%1</center>")
							.arg(fi.fileName())
						+"</qt>");
			l_p_progressDlg->adjustSize();
 			kapp->processEvents();
		}
		if(l_p_progressDlg->wasCancelled())
		{
			delete(l_p_progressDlg);
			return KURL::List();
		}
	}
	delete(l_p_progressDlg);
	return thlist_rm ;
}




KURL::List
ImageLoader::updateThumbnailDir(const QString& dirpath)
{
	KProgressDialog	*l_p_progressDlg = new KProgressDialog(m_p_imageList, "Update Thumbnails",
				i18n("Update cached thumbnails in progress..."),
				QString::null,
				true);
	l_p_progressDlg->show();
	l_p_progressDlg->setLabel(i18n("Checking for thumbnails in cache..."));
	l_p_progressDlg->adjustSize();
 	kapp->processEvents();

	KApplication::setOverrideCursor (waitCursor);
	QDir d(thumbnailRootPath());
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *thlist;
	if(!(thlist = d.entryInfoList()))
		;//	return KURL::List();
	KApplication::restoreOverrideCursor();

	//--------------------------
	l_p_progressDlg->progressBar()->setTotalSteps(thlist->count());

	int m_current=0;
	int totalSize=0;
	KURL::List thlist_rm;
	QFileInfoListIterator it( *thlist );
	QFileInfo *thfi;
	QImage im;
	QDateTime date_last=QDateTime::currentDateTime ();
	while ( (thfi = it.current()) != 0 )
	{
		im.load(thfi->absFilePath());
		KURIFilterData urid = im.text("Thumb::URI");
		KURIFilter::self()->filterURI( urid );
		QFileInfo fi(urid.uri().path());
		if(fi.filePath().startsWith(dirpath))
		{
			if(!fi.exists())
			{
				thlist_rm.append(thfi->absFilePath());
				totalSize+=thfi->size();
			}
		}
		++it;
		m_current++;
		//-----------
		l_p_progressDlg->progressBar()->setProgress(m_current);
		if(date_last.time().msecsTo(QDateTime::currentDateTime().time()) >= 500)
		{
			date_last=QDateTime::currentDateTime ();

			if(!fi.fileName().isEmpty())
				l_p_progressDlg->setLabel(
					"<qt>"
					+i18n("Directory: ")
						+QString("<b>%1</b><br>")
							.arg(dirpath)
					+i18n("Number of thumbnails to delete:")
						+QString("<center>%1 (%2 Kb)</center>")
							.arg(thlist_rm.count())
							.arg(KGlobal::locale()->formatNumber((float)totalSize/1024, 2))
					+i18n("Studying and seeking:")
						+QString("<center>%1</center>")
							.arg(fi.fileName())
					+"</qt>");
			l_p_progressDlg->adjustSize();
 			kapp->processEvents();
		}
		if(l_p_progressDlg->wasCancelled())
		{
			delete(l_p_progressDlg);
			return KURL::List();
		}
	}
	delete(l_p_progressDlg);
	return thlist_rm ;
}


////////////////
bool
ImageLoader::setEXIFOrientation(const QString& a_path, const Orientation orient)
{
#ifdef HAVE_LIBEXIF
	if(!Tools::isJPEG(a_path))
		return false;

	QFile file(a_path);
	if (!file.open(IO_ReadOnly))
	{
		MYWARNING << "Unable to open " << a_path << " for reading"<<endl;
		return false;
	}
	QByteArray l_raw_data_byte_array;
	ExifData* l_p_ExifData=NULL;

	l_raw_data_byte_array = file.readAll();
	if (l_raw_data_byte_array.size()==0)
	{
		MYWARNING << "No data available: empty file"<<endl;
		file.close();
		return false;
	}

	l_p_ExifData = exif_data_new_from_data((unsigned char*)l_raw_data_byte_array.data(), l_raw_data_byte_array.size());
	if (!l_p_ExifData)
	{
		MYWARNING << "Unable to load exif data"<<endl;
		file.close();
		return false;
	}
	file.close();

	ExifByteOrder m_ByteOrder = exif_data_get_byte_order(l_p_ExifData);

	ExifEntry *m_OrientationEntry =
		exif_content_get_entry(l_p_ExifData->ifd[EXIF_IFD_0],
			EXIF_TAG_ORIENTATION);
	if(!m_OrientationEntry)
	{
		MYWARNING << "Unable to load exif orientation"<<endl;
		return false;
	}

	exif_set_short(m_OrientationEntry->data, m_ByteOrder,
		Orientation(orient));


	/////////////save EXIF
	JPEGData* jpegData=jpeg_data_new_from_data((unsigned char*)l_raw_data_byte_array.data(), l_raw_data_byte_array.size());
	if (!jpegData)
	{
		MYWARNING << "Unable to create JPEGData object"<<endl;
		file.close();
		return false;
	}

	file.close();
	if (!file.open(IO_WriteOnly))
	{
		MYWARNING << "Unable to open " << a_path << " for writing"<<endl;
		return false;
	}
	jpeg_data_set_exif_data(jpegData, l_p_ExifData);
	unsigned char* dest=0L;
	unsigned int destSize=0;
	jpeg_data_save_data(jpegData, &dest, &destSize);
	jpeg_data_unref(jpegData);

	QDataStream l_stream(&file);
	l_stream.writeRawBytes((char*)dest, destSize);
	free(dest);
	file.close();
	return true;

#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	MYWARNING << "libexif not found, unable to rotate EXIF l_thumbnail_image"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}


bool
ImageLoader::rotateEXIFThumbnail(const QString& a_path, const Orientation orient)
{
#ifdef HAVE_LIBEXIF
	if(!Tools::isJPEG(a_path))
		return false;

	QFile l_file(a_path);
	if (!l_file.open(IO_ReadOnly))
	{
		MYWARNING << "Unable to open " << a_path << " for reading"<<endl;
		return false;
	}
	QByteArray l_raw_data_byte_array;
	ExifData* l_p_ExifData = NULL;

	l_raw_data_byte_array = l_file.readAll();
	if (l_raw_data_byte_array.size()==0)
	{
		MYWARNING << "No data available: empty file"<<endl;
		l_file.close();
		return false;
	}

	l_p_ExifData = exif_data_new_from_data((unsigned char*)l_raw_data_byte_array.data(), l_raw_data_byte_array.size());
	if (!l_p_ExifData)
	{
		MYWARNING << "Unable to load exif data"<<endl;
		l_file.close();
		return false;
	}
	l_file.close();

	QImage l_thumbnail_image;
	if (l_p_ExifData->data)
        	l_thumbnail_image.loadFromData(l_p_ExifData->data, l_p_ExifData->size);
	else
	{
		MYWARNING << "No data available: no ExifData found"<<endl;
		l_file.close();
		return false;
	}
	QWMatrix l_transform_matrix;
	switch (orient)
	{
		case NOT_AVAILABLE : l_transform_matrix.rotate(0); break;
		case NORMAL : l_transform_matrix.rotate(0); break;
		case HFLIP : l_transform_matrix.scale( 1, -1 ); break;
		case ROT_180 : l_transform_matrix.rotate(180); break;
		case VFLIP : l_transform_matrix.scale( -1, 1 ); break;
		case ROT_90_HFLIP : l_transform_matrix.rotate(90); l_transform_matrix.scale( 1, -1 ); break;
		case ROT_90 : l_transform_matrix.rotate(90); break;
		case ROT_90_VFLIP : l_transform_matrix.rotate(0);  l_transform_matrix.scale( -1, 1 ); break;
		case ROT_270 : l_transform_matrix.rotate(270); break;
	}

	if(!l_thumbnail_image.isNull())
		return ImageLoader::setEXIFThumbnail(a_path, l_thumbnail_image.xForm(l_transform_matrix));
	else
		return false;

#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	MYWARNING << "libexif not found, unable to rotate EXIF l_thumbnail_image"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}


/* static */
bool
ImageLoader::setEXIFThumbnail(const QString& a_path, const QImage& a_thumbnail_image)
{
#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG<<"a_path="<<a_path <<endl;
#endif

#ifdef HAVE_LIBEXIF
	if(!Tools::isJPEG(a_path))
		return false;

	QFile file(a_path);
	if (!file.open(IO_ReadOnly))
	{
		MYDEBUG << "Unable to open " << a_path << " for reading"<<endl;
		return false;
	}
	QByteArray l_raw_data_byte_array;
	ExifData * l_p_ExifData = NULL;

	l_raw_data_byte_array = file.readAll();
	if (l_raw_data_byte_array.size()==0)
	{
		MYDEBUG << "No data available; empty file"<<endl;
		file.close();
		return false;
	}

	l_p_ExifData = exif_data_new_from_data((unsigned char*)l_raw_data_byte_array.data(), l_raw_data_byte_array.size());
	if (!l_p_ExifData)
	{
		MYDEBUG << "Unable to load exif data"<<endl;
		file.close();
		return false;
	}

	/////////////set l_thumbnail_image
	if(l_p_ExifData->data)
	{
		free(l_p_ExifData->data);
		l_p_ExifData->data=NULL;
	}
	else
	{
		MYDEBUG << "No EXIF data, l_thumbnail_image will be ADDED"<<endl;
	}
	l_p_ExifData->size=0;

	QByteArray l_byte_array;
	QBuffer l_buffer(l_byte_array);
	l_buffer.open(IO_WriteOnly);
	QImageIO l_imgIO(&l_buffer, KImageIO::typeForMime(Tools::getJPEGMimeType()).latin1());
	l_imgIO.setQuality ( 90 ) ;
	l_imgIO.setImage(a_thumbnail_image);
	if (!l_imgIO.write())
	{
		MYDEBUG << "Unable to write l_thumbnail_image"<<endl;
		file.close();
		return false ;
	}
	l_p_ExifData->size=l_byte_array.size();
	l_p_ExifData->data=(unsigned char*)malloc(l_p_ExifData->size);
	if (!l_p_ExifData->data)
	{
		MYDEBUG << "Unable to allocate memory for l_thumbnail_image"<<endl;
		file.close();
		return false ;
	}
	memcpy(l_p_ExifData->data, l_byte_array.data(), l_byte_array.size());


	/////////////save EXIF
	JPEGData* jpegData=jpeg_data_new_from_data((unsigned char*)l_raw_data_byte_array.data(), l_raw_data_byte_array.size());
	if (!jpegData)
	{
		MYDEBUG << "Unable to create JPEGData object"<<endl;
		file.close();
		return false;
	}

	file.close();
	if (!file.open(IO_WriteOnly))
	{
		MYDEBUG << "Unable to open " << a_path << " for writing"<<endl;
		return false;
	}
	jpeg_data_set_exif_data(jpegData, l_p_ExifData);
	unsigned char* dest=0L;
	unsigned int destSize=0;
	jpeg_data_save_data(jpegData, &dest, &destSize);
	jpeg_data_unref(jpegData);

	QDataStream l_stream(&file);
	l_stream.writeRawBytes((char*)dest, destSize);
	free(dest);
	file.close();

#if IMALOADER_DEBUG_THREAD > 0
	MYDEBUG << "Done !"<<endl;
#endif 
	return true;


#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	MYWARNING << "libexif not found, unable to update EXIF thumbnail"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}
