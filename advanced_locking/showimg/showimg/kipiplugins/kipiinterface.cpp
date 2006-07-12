/***************************************************************************
                          kipiinterface.h  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2005 by Aurélien Gâteau, Richard Groult
    email                : aurelien.gateau@free.fr, rgroult@jalix.org
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

#include "kipiinterface.h"

#ifdef HAVE_KIPI

// Local
#include <mainwindow.h>
#include <imagelistview.h>
#include <directoryview.h>
#include <listitem.h>
#include <imagefileinfo.h>

#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <sys/stat.h>

// Qt
#include <qdir.h>

// KDE
#include <kdebug.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kurl.h>

#include <libkipi/imagecollectionshared.h>
#include <libkipi/imageinfoshared.h>

#include "kipiinterface.moc"

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class ShowImgImageCollection : public KIPI::ImageCollectionShared {
public:
	ShowImgImageCollection(const QString& name, const KURL::List& images)
	: KIPI::ImageCollectionShared(), mName(name), mImages(images)
	{
	}

	virtual ~ShowImgImageCollection()
	{
	}

	virtual QString name()
	{
// 		MYDEBUG<<mName<<endl;
		return mName;
	}

	virtual QString comment()
	{
		return QString::null;
	}

	virtual KURL::List images()
	{
// 		MYDEBUG<<mImages<<endl;
		return mImages;
	}

	virtual KURL path()
	{
		KURL url;
		url.setProtocol("file");
		url.setPath(path_);
// 		MYDEBUG<< url<<" "<< url.path()<<endl;
		return url;
	}
	virtual KURL uploadPath()
	{
		KURL url;
		url.setProtocol("file");
		url.setPath(path_);
// 		MYDEBUG<< url.path()<<endl;
		return url;
	}

	virtual KURL uploadRoot()
	{
		KURL url;
		url.setProtocol("file");
 		//url.setPath("/");
 		url.setPath(QFileInfo(path_).dirPath());
// 		MYDEBUG<<  url.path()<<endl;
		return url;
	}
	virtual QString uploadRootName()
	{
// 		return i18n("root");
		return uploadRoot().path();
	}

	virtual bool isDirectory()
	{
// 		MYDEBUG<<  true<<endl;
		return true;
	}

	void setPath(QString path)
	{
// 		MYDEBUG<< path<<endl;
		path_ = path;
	}



private:
	QString mName;
	KURL::List mImages;
	QString path_;
};



class ShowImgImageInfo : public KIPI::ImageInfoShared {
public:
	ShowImgImageInfo(KIPI::Interface* interface, const KURL& url) : KIPI::ImageInfoShared(interface, url) {}

	QString title()
	{
		return _url.fileName();
	}

	QString description()
	{
		ImageFileInfo info(path().path(), IMAGE);
		return info.getTitle();
	}

	void setDescription(const QString& descr)
	{
		ImageFileInfo info(path().path(), IMAGE);
		info.write(descr, "", "", "", "", "");
	}

        void setTime( const QDateTime& time, KIPI::TimeSpec /* spec = KIPI::FromInfo */ )
	{
		FILE * f;
		struct utimbuf * t = new utimbuf();
		struct tm tmp;
		struct stat st;

		time_t ti;

		f = fopen(path().path().ascii (), "r");
		if( f == NULL )
		{
			return ;
		}

		fclose( f );

		tmp.tm_mday = time.date().day();
		tmp.tm_mon = time.date().month() - 1;
		tmp.tm_year = time.date().year() - 1900;

		tmp.tm_hour = time.time().hour();
		tmp.tm_min = time.time().minute();
		tmp.tm_sec = time.time().second();
		tmp.tm_isdst = -1;

		ti = mktime( &tmp );
		if( ti == -1 )
		{
			return ;
		}

		if( stat(path().path().ascii (), &st ) == -1 )
		{
			return ;
		}
		t->modtime = ti;
		if(utime(path().path().ascii(), t ) != 0)
		{
			return ;
		}



	}

	QMap<QString,QVariant> attributes()
	{
		return QMap<QString,QVariant>();
	}

	void clearAttributes()
	{
	}


	void addAttributes(const QMap<QString, QVariant>&)
	{
	}
};



struct ShowImgKIPIInterfacePrivate
{
	MainWindow *mainwindow;
	ImageListView *imageList;
	DirectoryView *dirView;
};


ShowImgKIPIInterface::ShowImgKIPIInterface( MainWindow* parent )
:KIPI::Interface(parent, "ShowImg kipi interface")
{
	d=new ShowImgKIPIInterfacePrivate;

	d->mainwindow=parent;
	d->dirView = parent->getDirectoryView();
	d->imageList = parent->getImageListView();
}


ShowImgKIPIInterface::~ShowImgKIPIInterface() {
	delete d;
}


KIPI::ImageCollection
ShowImgKIPIInterface::currentAlbum()
{
	KURL::List list;
	list=d->imageList->allItemsURL();

	ShowImgImageCollection* col=new ShowImgImageCollection(i18n("Folder content"), list);
	col->setPath(path_);
	return KIPI::ImageCollection(col);

}


KIPI::ImageCollection
ShowImgKIPIInterface::currentSelection()
{
	KURL::List list=d->imageList->selectedImageURLs();

	ShowImgImageCollection* col=new ShowImgImageCollection(i18n("Selected images"), list);
	col->setPath(path_);
	return KIPI::ImageCollection(col);

}


QValueList<KIPI::ImageCollection>
ShowImgKIPIInterface::allAlbums()
{
	QValueList<KIPI::ImageCollection> list;
	list << currentAlbum() << currentSelection();
	return list;
}


KIPI::ImageInfo
ShowImgKIPIInterface::info(const KURL& url)
{
	return KIPI::ImageInfo( new ShowImgImageInfo(this, url) );
}

int
ShowImgKIPIInterface::features() const
{
	return
		KIPI::AcceptNewImages |
		KIPI::AlbumsHaveComments | KIPI::ImagesHasComments |
		KIPI::ImagesHasTime | KIPI::AlbumsUseFirstImagePreview;
}


void
ShowImgKIPIInterface::refreshImages( const KURL::List& /*urls*/ )
{
	d->mainwindow->slotRefresh();
}


/**
 * We don't need to do anything here, the KDirLister will pick up the image if
 * necessary
 */
bool
ShowImgKIPIInterface::addImage(const KURL&, QString&)
{
	return true;
}

void
ShowImgKIPIInterface::selectionChanged( bool  b  )
{
	if(b && d->imageList->hasImageSelected())
	    emit Interface::selectionChanged(true);
	else
	    emit Interface::selectionChanged(d->imageList->hasImages());
}

void
ShowImgKIPIInterface::currentAlbumChanged( const QString &path )
{
	if(path.endsWith("/"))
		this->path_ = path.left(path.length()-1);
	else
		this->path_ = path;
// 	MYDEBUG<<path_<<endl;
	emit Interface::currentAlbumChanged(true);

}

#else
#ifdef __GNUC__
#warning NO KIPI
#endif
#endif /* HAVE_KIPI */
