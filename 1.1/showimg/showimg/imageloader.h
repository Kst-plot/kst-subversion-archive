/***************************************************************************
                         imageloader.h  -  description
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

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kurl.h>

// QT
#include <qevent.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qpixmap.h>
#include <qimage.h>

#ifndef Q_WS_WIN
#include <pthread.h>
#endif

#define	Event_ImageLoad	QEvent::Enter

class ImageListView;

class KFileItem;
class KPixmapIO;

class QFileInfo;

class ImageLoadEvent : public QEvent
{
public:
	ImageLoadEvent (QFileInfo * fi, bool t = true, bool f = false, bool fEXIF=false)
	: QEvent (Event_ImageLoad),
	  Info (fi),
	  Threaded (t),
	  Force(f),
	  ForceEXIF(fEXIF)
	 { }

	QFileInfo *fileInfo () const
	{
		return Info;
	}
	bool threaded () const
	{
		return Threaded;
	}
	bool force () const
	{
		return Force;
	}
	bool forceEXIF () const
	{
		return ForceEXIF;
	}
private:
	QFileInfo * Info;
	bool Threaded;
	bool Force;
	bool ForceEXIF;
};



#define	Event_NextImage		QEvent::Close
class NextImageEvent:public QEvent
{
public:
	NextImageEvent ():QEvent (Event_NextImage)
	{
	}
};

class ImageLoader:public QObject
{
	friend void *__thread_start (void *);
	friend void __thread_cleanup (void *);

public:
enum Orientation {
	NOT_AVAILABLE=0,
	NORMAL=1,
	HFLIP=2,
	ROT_180=3,
	VFLIP=4,
	ROT_90_HFLIP=5,
	ROT_90=6,
	ROT_90_VFLIP=7,
	ROT_270=8
};

	ImageLoader (ImageListView * parent = 0, const char *name = 0);
	~ImageLoader ();

	void loadMiniImage (QFileInfo * fi, bool threaded = true, bool force = false, bool forceEXIF=false);
	void stopLoading (bool clean = true);

	void setThumbnailSize(const QSize newSize);
	QSize getThumbnailSize();

	void setShowFrame(bool show);
	bool getShowFrame();

	void setUseEXIF(bool useexif);
	bool getUseEXIF();

	void setStoreThumbnails(bool store);
	bool getStoreThumbnails();

	void rotateThumbnailLeft(QFileInfo *fi);
	void rotateThumbnailRight(QFileInfo *fi);

	QString thumbnailRootPath();
	QString thumbnailPath(const QString& path);

	KURL::List clearThumbnailDir(const QString& dirpath);
	KURL::List updateThumbnailDir(const QString& dirpath);

	static bool setEXIFThumbnail(const QString& path, const QImage& thumbnail);
	static bool rotateEXIFThumbnail(const QString& path, const Orientation orient);
	static bool setEXIFOrientation(const QString& path, const Orientation orient);



protected:
	QSize size;
	bool showFrame, useEXIF;

	void startLoading ();
	void loadImageInternal (ImageLoadEvent * e);
	void cantLoad (ImageLoadEvent * e);
	bool initLoading (ImageLoadEvent * e);
	void finishLoading (ImageLoadEvent * e);
	void loadWithNoThread (QFileInfo * fi, QWidget * w);
	void nextImage ();
	QPixmap addBorder(QPixmap *pix, bool hasAlpha=false);
	QPixmap addForeground(QPixmap *pix, bool hasAlpha);

	void reduce(QImage *im, int w, int h, bool force=false);

	virtual bool eventFilter (QObject * o, QEvent * e);
	virtual void timerEvent (QTimerEvent * e);

	void thread_start ();
	void thread_cleanup ();

	QString createCahePath(const QString& path);

private:
	QPtrList < ImageLoadEvent > EventList;
	QPtrList < ImageLoadEvent > ImageLoadedList;
#ifndef Q_WS_WIN
	pthread_t ThreadID;
	pthread_mutex_t Mutex;
#endif
	bool Running;

	bool mini_image_outdated, mini_image_file_exists, xvpics_dir_exists;
	QString mini_image_path, xvpics_path, image_path;
	QPixmap mini_image;
	KURL image_url;
	bool Loading;
	QImage InternalImage;
	QString InternalPath;
	ImageLoadEvent *InternalEvent;

	QImage ptop, pbottom, pright, pleft;
	QPixmap pbgxpm;

	KPixmapIO *kPio;

	ImageListView *imageList;

	bool storeth;

	QString thumbRoot_KDE, thumbRoot_ShowImg;
};

#endif
