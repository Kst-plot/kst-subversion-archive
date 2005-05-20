/***************************************************************************
                          imageloader.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#include "imageloader.h"

// Local
#include "imagelistview.h"
#include "imagefileinfo.h"
#include "fileiconitem.h"
#include "exif.h"

#ifdef HAVE_LIBEXIF
#include <misc/jpeg-data.h>

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* HAVE_LIBEXIF */

#endif

#ifdef HAVE_LIBKEXIF
#include <libkexif/kexifdata.h>
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */

// KDE
#include <kio/previewjob.h>
#include <kurl.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kpixmapio.h>
#include <kimageio.h>
#include <kimageeffect.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#include <kpixmap.h>
#include <kmdcodec.h>
#include <kurifilter.h>
#include <kprogress.h>

// QT
#include <qstring.h>
#include <qimage.h>
#include <qlabel.h>
#include <qdir.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qfile.h>

#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>



#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class ImageListView;

#ifndef Q_WS_WIN
void *
__thread_start (void *arg)
{
	pthread_cleanup_push (__thread_cleanup, arg);
	((ImageLoader *) arg)->thread_start ();
	pthread_cleanup_pop (0);
	pthread_detach (pthread_self ());
	return 0;
}

void
__thread_cleanup (void *arg)
{
	((ImageLoader *) arg)->thread_cleanup ();
}
#endif


ImageLoader::ImageLoader (ImageListView * parent, const char *name):
QObject (parent, name)
{
	imageList = parent;

	EventList.setAutoDelete (true);
	installEventFilter (this);
	Loading = Running = false;

	size=QSize(80,60);
	setThumbnailSize(size);
	showFrame=false;

	setStoreThumbnails(true);

	ptop=QImage(locate("appdata", "pics/border.png"));
	pbgxpm=QPixmap(locate("appdata", "pics/bgxpm.png"));

	kPio=new KPixmapIO();

	thumbRoot_ShowImg=QDir::homeDirPath () + "/.showimg/cache/";
	thumbRoot_KDE=QDir::homeDirPath() + "/.thumbnails/normal/";
	if(! KStandardDirs::exists(thumbnailRootPath()))
			KStandardDirs::makeDir(thumbnailRootPath());
}

ImageLoader::~ImageLoader ()
{
	stopLoading (true);
}

void
ImageLoader::setShowFrame(bool show)
{
	showFrame=show;
}
bool
ImageLoader::getShowFrame()
{
	return showFrame;
}

void
ImageLoader::setUseEXIF(bool useexif)
{
	useEXIF=useexif;
}
bool
ImageLoader::getUseEXIF()
{
	return useEXIF;
}

QPixmap
ImageLoader::addBorder(QPixmap *pix, bool hasAlpha)
{
// 	if(!showFrame || (pix->width()<getThumbnailSize().width() && pix->height()<getThumbnailSize().height()) ) return *pix;
	if(!showFrame || (pix->width()<16 && pix->height()<16) ) return *pix;
	QPixmap res(pix->size());
	if(hasAlpha)
// 		p.drawTiledPixmap (0, 0, pix->width(), pix->height(), pbgxpm);
		res.fill(imageList->paletteBackgroundColor());
	QPainter p(&res);

	QWMatrix m_scale;
	m_scale.scale(0.79738562092, 0.76691729323);
	p.drawImage(0, 0, ptop.scale(pix->width(), pix->height()));
/*	p.drawPixmap((int)floor((float)pix->width()/ptop.width()*14),
			(int)floor((float)pix->height()/ptop.height()*13),
			pix->xForm(m_scale));*/

	p.drawImage((int)floor((float)pix->width()/ptop.width()*14),
			(int)floor((float)pix->height()/ptop.height()*13),
			pix->convertToImage().smoothScale(
				(int)ceil(pix->width()*0.79738562092)+1,
				(int)ceil(pix->height()*0.76691729323)+1));

	p.end();
	return res;
}

QPixmap
ImageLoader::addForeground(QPixmap *pix, bool hasAlpha)
{
	if(!hasAlpha || (pix->width()<getThumbnailSize().width() && pix->height()<getThumbnailSize().height())) return *pix;

	QPixmap res(pix->size());
	res.fill(imageList->paletteBackgroundColor());
// 	res.setMask(QBitmap(pix->size(), true));
	QPainter p(&res);
// 	p.drawTiledPixmap (0, 0, pix->width(), pix->height(), pbgxpm);
	p.drawPixmap(0,0,*pix);
	p.end();
	return res;
}

void
ImageLoader::setThumbnailSize(const QSize newSize)
{
/*	if(size==newSize)
		return;
	else*/
		size=newSize;
}


QSize
ImageLoader::getThumbnailSize()
{
	return size;
}

void
ImageLoader::reduce(QImage *im, int w, int h, bool force)
{
	if(im->isNull()) return;

	double
		wexpand = (double) w / (double) im->width (),
		hexpand = (double) h / (double) im->height ();
	if (!force && (wexpand >= 1.0 || hexpand >= 1.0)) // don't expand small images  !
	{
		return;
	}


	int neww, newh;
// 	QWMatrix m_scale;
	if (wexpand < hexpand)
	{
		neww = (int) ceil(im->width () * wexpand );
		newh = (int) ceil(im->height () * wexpand );
// 		m_scale.scale(wexpand,wexpand);
	}
	else
	{
		neww = (int) ceil(im->width () * hexpand );
		newh = (int) ceil(im->height () * hexpand );
// 		m_scale.scale(hexpand,hexpand);
	}
// 	*im = im->xForm(m_scale);
	*im=im->smoothScale(neww,newh);
}
void
ImageLoader::loadMiniImage (QFileInfo * fi, bool threaded, bool force, bool forceEXIF)
{
	bool cont=true;
	QFileInfo thumb;
	if(fi->size()<=0x2800) //10kb
		thumb = QFileInfo(*fi);
	else
		thumb = QFileInfo(thumbRoot_ShowImg + fi->absFilePath ());

	if(!thumb.exists())
	{
		thumb = QFileInfo(thumbnailPath(fi->absFilePath()));
		if(!(force||forceEXIF) && !thumb.exists() && useEXIF && fi->extension().lower()=="jpg")
		{
#ifndef HAVE_LIBKEXIF
			QString tmpthumb = locateLocal("tmp", "thumb.jpg");
			if("OK" == ProcessFile(
				QFile::encodeName(fi->absFilePath()),
				false,
				QFile::encodeName(tmpthumb)))
			{
				InternalImage.load (tmpthumb);
				ImageLoadEvent *e = new ImageLoadEvent(new QFileInfo(*fi), true, force, forceEXIF);
				finishLoading(e);
				return;
			}
#else
// 			MYDEBUG<<endl;
			KExifData exifData;
			exifData.readFromFile(fi->absFilePath ());
// 			if(!InternalImage.isNull())
			{
				QWMatrix mxForm;
// 				MYDEBUG<<fi->absFilePath ()<<" "<<exifData.getImageOrientation()<<endl;
				switch(exifData.getImageOrientation())
				{
					case NOT_AVAILABLE : /*mxForm.rotate(0);*/ break;
					case NORMAL : /*mxForm.rotate(0);*/ break;
					case HFLIP : mxForm.scale( 1, -1 ); break;
					case ROT_180 : mxForm.rotate(180); break;
					case VFLIP : mxForm.scale( -1, 1 ); break;
					case ROT_90_HFLIP : mxForm.rotate(90); mxForm.scale( 1, -1 ); break;
					case ROT_90 : mxForm.rotate(90); break;
					case ROT_90_VFLIP : mxForm.rotate(0);  mxForm.scale( -1, 1 ); break;
					case ROT_270 : mxForm.rotate(270); break;
				}
				if(!exifData.getThumbnail().isNull())
				{
					InternalImage = exifData.getThumbnail().xForm(mxForm);

					ImageLoadEvent *e = new ImageLoadEvent(new QFileInfo(*fi), true, force, forceEXIF);
					finishLoading(e);
					return;
				}
			}
#endif /* HAVE_LIBKEXIF */
		}
	}

	if( fi->lastModified() <= thumb.lastModified())
	{
		QImage im (thumb.absFilePath ());
// 		im.setAlphaBuffer(true);

		if((thumb.absFilePath () != fi->absFilePath ()) ||
		   (im.width() > im.height() && im.width() > getThumbnailSize().width()  ||  im.height() > getThumbnailSize().height()) ||
		   (im.width() < im.height() && im.width() > getThumbnailSize().height() ||  im.height() > getThumbnailSize().width()))
			reduce(&im, getThumbnailSize().width(), getThumbnailSize().height(), true);

		if(!im.isNull())
		{
			QPixmap mini_image;
			//#if  KDE_VERSION >= 0x30200
			if(!im.hasAlphaBuffer())
				im = KImageEffect::sharpen(im, 1, 1);
			//#endif
			if(im.hasAlphaBuffer())
			{
				int options=ThresholdAlphaDither||ColorOnly||ThresholdDither||ThresholdAlphaDither;//||AvoidDither;
				mini_image.convertFromImage(im, options);
				mini_image = addForeground(&mini_image, true);
				im = kPio->convertToImage(mini_image);
				im.setAlphaBuffer (true);

			}
			mini_image = kPio->convertToPixmap(im);
			if(!im.isNull())
			{
				cont=false;
				imageList->slotSetPixmap(addBorder(&mini_image, im.hasAlphaBuffer()), fi, true, force, forceEXIF);
				im.reset();
			}
		}
	}

	if(cont)
	{
		ImageLoadEvent *e = new ImageLoadEvent (fi, threaded, force, forceEXIF);
		EventList.append (e);
		if (EventList.count () > 0 && !Running)
		{
			Running = true;
			startTimer (1);
			nextImage ();
		}
	}
}

void
ImageLoader::startLoading ()
{
	Running = true;
	ImageLoadEvent *e = ((int) (EventList.count ()) > 0 ? EventList.take (0) : 0);
	if (!e)
	{
		Running = Loading = false;
		killTimers ();

		return;
	}
	if (!initLoading (e))
	{
		cantLoad (e);
		return;
	}
	Loading = true;
	loadImageInternal (e);
}


void
ImageLoader::stopLoading (bool clean)
{
	if (Loading)
	{
#ifndef Q_WS_WIN
		pthread_cancel (ThreadID);
		pthread_join (ThreadID, NULL);
#endif
		Loading = Running = false;
		killTimers ();
		ImageLoadedList.clear ();
	}
	if (clean)
	{
		EventList.clear ();
	}
	InternalImage.reset();
	QFile::remove(locateLocal("tmp", "thumb.jpg"));
}

void
ImageLoader::cantLoad (ImageLoadEvent * e)
{
	(void)kapp->postEvent (imageList, e);
	Loading = false;
	nextImage ();
}

bool
ImageLoader::eventFilter (QObject *, QEvent * e)
{
	switch (e->type ())
	{
		case Event_NextImage:
			startLoading ();
			return true;
		case Event_ImageLoad:
		{
		 	 Loading = false;
		 	 ImageLoadEvent *ev = new ImageLoadEvent (*((ImageLoadEvent *) e));
		 	 finishLoading (ev);
		 	 (void)kapp->postEvent (imageList, ev);
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
	if (!Loading)
	{
		NextImageEvent *e = new NextImageEvent;
		(void)kapp->postEvent (this, e);
	}
}


void
ImageLoader::thread_start ()
{
	InternalImage.load (InternalPath);
	ImageLoadedList.append (InternalEvent);
}


void
ImageLoader::thread_cleanup ()
{
}


void
ImageLoader::timerEvent (QTimerEvent * )
{
	while (ImageLoadedList.count () > 0)
	{
		ImageLoadEvent *e = ImageLoadedList.take (0);
		(void)kapp->postEvent (this, e);
	}
}


bool
ImageLoader::initLoading (ImageLoadEvent * e)
{
	QFileInfo *fi = e->fileInfo ();
	image_path = QString (fi->absFilePath ());
	image_url.setPath(image_path);

	if (!mini_image_file_exists || mini_image_outdated)
		return true;
	return false;
}


void
ImageLoader::finishLoading (ImageLoadEvent * e)
{
	QFileInfo *fi = e->fileInfo ();
	bool force = e->force();
	bool forceEXIF = e->forceEXIF();
	QImage image = InternalImage;
	bool success = true;

	if (image.isNull ())
	{
		image = BarIcon("file_broken",48).convertToImage();
		success = false;
	}

	FileIconItem *item;
	if(forceEXIF && success && (item=imageList->findItem(fi->filePath(), true))!=NULL)
	{
		int neww, newh;
		if(image.width ()>image.height ())
			{neww=160;newh=120;}
		else
			{neww=120;newh=160;}

		reduce(&image, neww, newh);

		if(item->isSelected())
		{
			if(!setEXIFThumbnail(fi->absFilePath(), image))
				kdWarning()<<"Unable to save EXIF thumbnail "<<fi->absFilePath()<<endl;
		}
	}
	if(getStoreThumbnails() && success)
	{
		int neww, newh;
		if(image.width ()>image.height ())
			{neww=128;newh=96;}
		else
			{neww=96;newh=128;}

		reduce(&image, neww, newh);

		// set the thumbnail attributes using the
	        // freedesktop.org standards
	        image.setText(QString("Thumb::URI").latin1(),
	                      0, "file://" + QDir::cleanDirPath(fi->absFilePath () ));
	        image.setText(QString("Thumb::MTime").latin1(),
	                      0, QString::number(fi->lastModified().toTime_t()));
	        image.setText(QString("Software").latin1(),
	                      0, QString("ShowImg Thumbnail Generator"));
		if(image.save(createCahePath(fi->absFilePath()), "PNG", 0))
		{
			chmod(QFile::encodeName(createCahePath(fi->absFilePath())), S_IREAD|S_IWRITE);
		}
		else
		{
			kdWarning()<<"Unable to save thumbnail "<<fi->absFilePath()<<endl;
		}
	}

	reduce(&image, getThumbnailSize().width(), getThumbnailSize().height(), true);

	if(image.hasAlphaBuffer())
	{
		int options=ThresholdAlphaDither;//ColorOnly||ThresholdDither||ThresholdAlphaDither||AvoidDither;
		mini_image.convertFromImage(image, options);
		mini_image = addForeground(&mini_image, true);
		image = kPio->convertToImage(mini_image);
		image.setAlphaBuffer (true);
	}
	mini_image = kPio->convertToPixmap(image);
	if(!mini_image.isNull ())
		imageList->slotSetPixmap (addBorder(&mini_image, false), fi, success, force, forceEXIF);
	else
		imageList->slotSetPixmap (BarIcon("file_broken",48), fi, success, force, forceEXIF);

	InternalImage.reset ();
	image.reset();
}


void
ImageLoader::loadImageInternal (ImageLoadEvent * e)
{
	InternalPath = image_url.path ();
	InternalEvent = e;
	InternalImage.reset ();
	// Problem here with xpm and xbm image, interfere with X system and can cause crash.
	// As it is probably a small image, it's probably not a problem to load it synchronously.

	if (!e->threaded ())
		thread_start ();
#ifndef Q_WS_WIN
	else
		if (pthread_create (&ThreadID, 0, __thread_start, this) == 0);
#endif
	else
		qWarning ("%s %d  ImageLoader::loadImageInternal (ImageLoadEvent * e) : unable to start loading thread",__FILE__,__LINE__);

}


void
ImageLoader::setStoreThumbnails(bool store)
{
	storeth=store;
}

bool
ImageLoader::getStoreThumbnails()
{
	return storeth;
}

QString
ImageLoader::createCahePath(const QString& path)
{
	return thumbnailPath(path);
}

void
ImageLoader::rotateThumbnailLeft(QFileInfo* /* fi*/ )
{
	kdWarning()<<"ImageLoader::rotateThumbnailLeft(QFileInfo *fi)"<<endl;
}

void
ImageLoader::rotateThumbnailRight(QFileInfo* /* fi */)
{
	kdWarning()<<"ImageLoader::rotateThumbnailRight(QFileInfo *fi)"<<endl;
}

QString
ImageLoader::thumbnailRootPath()
{
	return thumbRoot_KDE;
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
	KProgressDialog	*m_progressDlg = new KProgressDialog(imageList, "Remove cached thumbnails",
				i18n("Remove cached thumbnails in progress"),
				QString::null,
				true);
		m_progressDlg->adjustSize();
		m_progressDlg->show();

	QDir d(thumbnailRootPath());
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *thlist;
	if(!(thlist = d.entryInfoList()))
		return KURL::List();

	m_progressDlg->progressBar()->setTotalSteps(thlist->count());

	int m_current=0;
	int totalSize=0;
	KURL::List thlist_rm;
	QFileInfoListIterator it( *thlist );
	QFileInfo *thfi;
	QImage im;
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
		m_progressDlg->progressBar()->setProgress(m_current);
		if(!fi.fileName().isEmpty())
		  m_progressDlg->setLabel(
			"<qt>"
			+i18n("Directory: ")
				+QString("<b>%1</b><br>")
					.arg(dirpath)
			+i18n("Files to remove:")
				+QString("<center>%1 (%2 Kb)</center>")
					.arg(thlist_rm.count())
					.arg(KGlobal::locale()->formatNumber((float)totalSize/1024, 2))
			+i18n("Studying:")
				+QString("<center>%1</center>")
					.arg(fi.fileName())
			+"</qt>");
		kapp->processEvents();
		if(m_progressDlg->wasCancelled())
		{
			delete(m_progressDlg);
			return KURL::List();
		}
	}
	return thlist_rm ;
}




KURL::List
ImageLoader::updateThumbnailDir(const QString& dirpath)
{
	KProgressDialog	*m_progressDlg = new KProgressDialog(imageList, "Update Thumbnails",
				i18n("Update cached thumbnails in progress"),
				QString::null,
				true);
		m_progressDlg->adjustSize();
		m_progressDlg->show();

	QDir d(thumbnailRootPath());
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *thlist;
	if(!(thlist = d.entryInfoList()))
		return KURL::List();

	m_progressDlg->progressBar()->setTotalSteps(thlist->count());

	int m_current=0;
	int totalSize=0;
	KURL::List thlist_rm;
	QFileInfoListIterator it( *thlist );
	QFileInfo *thfi;
	QImage im;
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

		m_progressDlg->progressBar()->setProgress(m_current);
		if(!fi.fileName().isEmpty())
		  m_progressDlg->setLabel(
			"<qt>"
			+i18n("Directory: ")
				+QString("<b>%1</b><br>")
					.arg(dirpath)
			+i18n("Files to remove:")
				+QString("<center>%1 (%2 Kb)</center>")
					.arg(thlist_rm.count())
					.arg(KGlobal::locale()->formatNumber((float)totalSize/1024, 2))
			+i18n("Studying and seeking:")
				+QString("<center>%1</center>")
					.arg(fi.fileName())
			+"</qt>");
		kapp->processEvents();
		if(m_progressDlg->wasCancelled())
		{
			delete(m_progressDlg);
			return KURL::List();
		}
	}
	return thlist_rm ;

}


////////////////
bool
ImageLoader::setEXIFOrientation(const QString& path, const Orientation orient)
{
#ifdef HAVE_LIBEXIF
	KMimeType::Ptr mime = KMimeType::findByPath( path, 0, true );
#if KDE_VERSION < 0x30200
	if(! mime->name() == "image/jpeg")
#else /* KDE_VERSION < 0x30200 */
	if(! mime->is("image/jpeg"))
#endif /* KDE_VERSION < 0x30200 */
		return false;


	QFile file(path);
	if (!file.open(IO_ReadOnly))
	{
		kdWarning() << "Unable to open " << path << " for reading"<<endl;
		return false;
	}
	QByteArray m_RawData;
	ExifData* m_ExifData=NULL;

	m_RawData = file.readAll();
	if (m_RawData.size()==0)
	{
		kdWarning() << "No data available: empty file"<<endl;
		file.close();
		return false;
	}

	m_ExifData = exif_data_new_from_data((unsigned char*)m_RawData.data(), m_RawData.size());
	if (!m_ExifData)
	{
		kdWarning() << "Unable to load exif data"<<endl;
		file.close();
		return false;
	}
	file.close();

	ExifByteOrder m_ByteOrder = exif_data_get_byte_order(m_ExifData);

	ExifEntry *m_OrientationEntry =
		exif_content_get_entry(m_ExifData->ifd[EXIF_IFD_0],
			EXIF_TAG_ORIENTATION);
	if(!m_OrientationEntry)
	{
		kdWarning() << "Unable to load exif orientation"<<endl;
		return false;
	}

	exif_set_short(m_OrientationEntry->data, m_ByteOrder,
		Orientation(orient));


	/////////////save EXIF
	JPEGData* jpegData=jpeg_data_new_from_data((unsigned char*)m_RawData.data(), m_RawData.size());
	if (!jpegData)
	{
		kdWarning() << "Unable to create JPEGData object"<<endl;
		file.close();
		return false;
	}

	file.close();
	if (!file.open(IO_WriteOnly))
	{
		kdWarning() << "Unable to open " << path << " for writing"<<endl;
		return false;
	}
	jpeg_data_set_exif_data(jpegData, m_ExifData);
	unsigned char* dest=0L;
	unsigned int destSize=0;
	jpeg_data_save_data(jpegData, &dest, &destSize);
	jpeg_data_unref(jpegData);

	QDataStream stream(&file);
	stream.writeRawBytes((char*)dest, destSize);
	free(dest);
	file.close();
	return true;

#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	kdWarning() << "libexif not found, unable to rotate EXIF thumbnail"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}


bool
ImageLoader::rotateEXIFThumbnail(const QString& path, const Orientation orient)
{
#ifdef HAVE_LIBEXIF
	KMimeType::Ptr mime = KMimeType::findByPath( path, 0, true );
#if KDE_VERSION < 0x30200
	if(! mime->name() == "image/jpeg")
#else /* KDE_VERSION < 0x30200 */
	if(! mime->is("image/jpeg"))
#endif /* KDE_VERSION < 0x30200 */
		return false;


	QFile file(path);
	if (!file.open(IO_ReadOnly))
	{
		kdWarning() << "Unable to open " << path << " for reading"<<endl;
		return false;
	}
	QByteArray m_RawData;
	ExifData* m_ExifData=NULL;

	m_RawData = file.readAll();
	if (m_RawData.size()==0)
	{
		kdWarning() << "No data available: empty file"<<endl;
		file.close();
		return false;
	}

	m_ExifData = exif_data_new_from_data((unsigned char*)m_RawData.data(), m_RawData.size());
	if (!m_ExifData)
	{
		kdWarning() << "Unable to load exif data"<<endl;
		file.close();
		return false;
	}
	file.close();

	QImage thumbnail;
	if (m_ExifData->data)
        	thumbnail.loadFromData(m_ExifData->data, m_ExifData->size);
	else
	{
		kdWarning() << "No data available: no ExifData found"<<endl;
		file.close();
		return false;
	}
	QWMatrix mxForm;
	switch (orient)
	{
		case NOT_AVAILABLE : mxForm.rotate(0); break;
		case NORMAL : mxForm.rotate(0); break;
		case HFLIP : mxForm.scale( 1, -1 ); break;
		case ROT_180 : mxForm.rotate(180); break;
		case VFLIP : mxForm.scale( -1, 1 ); break;
		case ROT_90_HFLIP : mxForm.rotate(90); mxForm.scale( 1, -1 ); break;
		case ROT_90 : mxForm.rotate(90); break;
		case ROT_90_VFLIP : mxForm.rotate(0);  mxForm.scale( -1, 1 ); break;
		case ROT_270 : mxForm.rotate(270); break;
	}

	if(!thumbnail.isNull())
		return ImageLoader::setEXIFThumbnail(path, thumbnail.xForm(mxForm));
	else
		return false;

#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	kdWarning() << "libexif not found, unable to rotate EXIF thumbnail"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}


bool
ImageLoader::setEXIFThumbnail(const QString& path, const QImage& thumbnail)
{
#ifdef HAVE_LIBEXIF
	KMimeType::Ptr mime = KMimeType::findByPath( path, 0, true );
#if KDE_VERSION < 0x30200
	if(! mime->name() == "image/jpeg")
#else /* KDE_VERSION < 0x30200 */
	if(! mime->is("image/jpeg"))
#endif /* KDE_VERSION < 0x30200 */
		return false;

	QFile file(path);
	if (!file.open(IO_ReadOnly))
	{
		kdWarning() << "Unable to open " << path << " for reading"<<endl;
		return false;
	}
	QByteArray m_RawData;
	ExifData* m_ExifData=NULL;

	m_RawData = file.readAll();
	if (m_RawData.size()==0)
	{
		kdWarning() << "No data available; empty file"<<endl;
		file.close();
		return false;
	}

	m_ExifData = exif_data_new_from_data((unsigned char*)m_RawData.data(), m_RawData.size());
	if (!m_ExifData)
	{
		kdWarning() << "Unable to load exif data"<<endl;
		file.close();
		return false;
	}

	/////////////set thumbnail
	if(m_ExifData->data)
	{
		free(m_ExifData->data);
		m_ExifData->data=NULL;
	}
	else
	{
		kdWarning() << "No EXIF data, thumbnail will be ADDED"<<endl;
		//kdWarning() << "No EXIF data, thumbnail will not be written"<<endl;
		//file.close();
		//return false;
	}
	m_ExifData->size=0;

	QByteArray m_array;
	QBuffer m_buffer(m_array);
	m_buffer.open(IO_WriteOnly);
	QImageIO m_ImgIO(&m_buffer, "JPEG");
	m_ImgIO.setQuality ( 90 ) ;
	//#if  KDE_VERSION >= 0x30200
	//m_ImgIO.setImage( KImageEffect::sharpen((QImage&)thumbnail, 0, 1));
	//#else
	m_ImgIO.setImage(thumbnail);
	//#endif
	if (!m_ImgIO.write())
	{
		kdWarning() << "Unable to write thumbnail"<<endl;
		file.close();
		return false ;
	}
	m_ExifData->size=m_array.size();
	m_ExifData->data=(unsigned char*)malloc(m_ExifData->size);
	if (!m_ExifData->data)
	{
		kdWarning() << "Unable to allocate memory for thumbnail"<<endl;
		file.close();
		return false ;
	}
	memcpy(m_ExifData->data, m_array.data(), m_array.size());


	/////////////save EXIF
	JPEGData* jpegData=jpeg_data_new_from_data((unsigned char*)m_RawData.data(), m_RawData.size());
	if (!jpegData)
	{
		kdWarning() << "Unable to create JPEGData object"<<endl;
		file.close();
		return false;
	}

	file.close();
	if (!file.open(IO_WriteOnly))
	{
		kdWarning() << "Unable to open " << path << " for writing"<<endl;
		return false;
	}
	jpeg_data_set_exif_data(jpegData, m_ExifData);
	unsigned char* dest=0L;
	unsigned int destSize=0;
	jpeg_data_save_data(jpegData, &dest, &destSize);
	jpeg_data_unref(jpegData);

	QDataStream stream(&file);
	stream.writeRawBytes((char*)dest, destSize);
	free(dest);
	file.close();
	return true;


#else /* HAVE_LIBEXIF */
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
	kdWarning() << "libexif not found, unable to update EXIF thumbnail"<<endl;
	return false;
#endif /* HAVE_LIBEXIF */
}
