/***************************************************************************
                          imagelistviewsimple.cpp  -  description
                             -------------------
    begin                : Jan 9 2005
    copyright            : (C) 2005 by Richard Groult
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

#include "imagelistviewsimple.h"

//Local
#include "mainwindow.h"
#include "listitemview.h"
#include "imageviewer.h"
#include "imagemetainfo.h"
#include "osd.h"

//KDE
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kfileitem.h>

//QT
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qtimer.h>
#include <qfont.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

ImageListViewSimple::ImageListViewSimple(QObject *parent,
										const QString& imageFilePath, ImageViewer *imageviewer)
	: QObject(parent, "ImageListViewSimple"),

	m_imageviewer(NULL),
	m_imageMetaInfo(NULL),
	m_OSDWidget(NULL),

	timer(NULL)
{
	this->m_imageviewer=imageviewer;
	this->imagepathList=new QStringList();

	setImageFilePath(imageFilePath);
}


ImageListViewSimple::~ImageListViewSimple()
{
	delete(imagepathList);
	if(timer) timer->stop();
}

void
ImageListViewSimple::setImageFilePath(const QString& imageFilePath)
{
	this->imageFilePath=imageFilePath;
	this->currentPath=QFileInfo(imageFilePath).dirPath();
}

void
ImageListViewSimple::readConfig(KConfig *config, bool hideOSD)
{
	config->setGroup("OSD");
	if(!config->readBoolEntry("showOSD", true))
		return;

	m_imageMetaInfo = new ImageMetaInfo(NULL);

	m_OSDWidget = new ShowimgOSD(m_imageviewer);
	m_OSDWidget->setDuration(5*1000);
	m_OSDWidget->setDrawShadow(false);
	QFont m_font(m_OSDWidget->font());
	m_OSDWidget->initOSD(config->readBoolEntry("showOSD", true) && !hideOSD, config->readBoolEntry("OSDOnTop", true),  config->readFontEntry("OSDFont", &m_font), config->readBoolEntry("showFilename", true), config->readBoolEntry("showFullpath", false), config->readBoolEntry("showDimensions", true), config->readBoolEntry("showComments", true), config->readBoolEntry("showDatetime", true), config->readBoolEntry("showExif", false) );
}

void
ImageListViewSimple::load()
{
	QString s( currentPath );
	QDir thisDir( s );
	thisDir.setFilter(QDir::Files);
	const QFileInfoList *files = thisDir.entryInfoList();
	if ( files )
	{
		QFileInfoListIterator it( *files );
		QFileInfo * f;
		QString ext;
		while( (f=it.current()) != 0 )
		{
			++it;
			if(f->isFile() && ListItemView::isImage(f))
			{
				imagepathList->append(f->absFilePath());
			}
		}
	}
	currentPos =  imagepathList->find(imageFilePath);

	m_imageviewer->loadImage(*currentPos);
	updateOSD(*currentPos);
}

void
ImageListViewSimple::initActions(KActionCollection *)
{
	if(m_imageviewer==NULL) {kdWarning()<<"pb in imagelistview: ImageViewer is NULL!!!"<<endl; return;}
	//
	connect(m_imageviewer, SIGNAL(askForPreviousImage()), this, SLOT(previous()));
	connect(m_imageviewer, SIGNAL(askForNextImage()), this, SLOT(next()));
	connect(m_imageviewer, SIGNAL(askForFirstImage()), this, SLOT(first()));
	connect(m_imageviewer, SIGNAL(askForLastImage()), this, SLOT(last()));
}

QString
ImageListViewSimple::currentAbsImagePath()
{
	return *currentPos;
}


void
ImageListViewSimple::startSlideshow(int slideshowTime)
{
	timer = new QTimer (this);
	timer->start (slideshowTime*1000, false);
	connect (timer, SIGNAL (timeout ()), this, SLOT (next ()));
}

void
ImageListViewSimple::next ()
{
	currentPos++;
	if(currentPos==imagepathList->end())
	{
		first();
		return;
	}
	m_imageviewer->loadImage(*currentPos);

	//----------
	updateOSD(*currentPos);

	//----------
	if(currentPos==imagepathList->end()) return;
	currentPos++;
	m_imageviewer->preloadImage(*currentPos);
	currentPos--;

}


void
ImageListViewSimple::previous ()
{
	if(currentPos==imagepathList->begin())
	{
		last();
		return;
	}

	currentPos--;
	m_imageviewer->loadImage(*currentPos);

	//----------
	updateOSD(*currentPos);

	//----------
	if(currentPos==imagepathList->begin()) return;
	currentPos--;
	m_imageviewer->preloadImage(*currentPos);
	currentPos++;
}


void
ImageListViewSimple::first ()
{
	currentPos=imagepathList->begin();
	MYDEBUG<<*currentPos<<endl;
	m_imageviewer->loadImage(*currentPos);

	//----------
	updateOSD(*currentPos);
}


void
ImageListViewSimple::last ()
{
	currentPos=imagepathList->end();
	currentPos--;
	MYDEBUG<<*currentPos<<endl;
	m_imageviewer->loadImage(*currentPos);

	//----------
	updateOSD(*currentPos);
}


void
ImageListViewSimple::updateOSD(const QString& currentfile)
{
	if(!m_imageMetaInfo)
		return;

	KFileItem *item = new KFileItem(KFileItem::Unknown,
									KFileItem::Unknown,
									KURL::fromPathOrURL(currentfile));
	m_imageMetaInfo->setURL(item->url(), item->mimetype());

	QRect toberepainted(m_OSDWidget->geometry());
	QFileInfo info(m_imageMetaInfo->getURL().path());
	m_OSDWidget->setTexts(info.fileName(), info.dirPath(),
						  m_imageMetaInfo->getDimensions(), m_imageMetaInfo->getComments(),
						  m_imageMetaInfo->getDatetime().toString(),
						  m_imageMetaInfo->toString());
	m_OSDWidget->show(); //m_OSDWidget->repaint(); kapp->processEvents();
	m_imageviewer->repaint(toberepainted); kapp->processEvents();
}


#include "imagelistviewsimple.moc"
