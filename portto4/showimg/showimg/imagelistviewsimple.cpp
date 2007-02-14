/*****************************************************************************
                          imagelistviewsimple.cpp  -  description
                             -------------------
    begin                : Jan 9 2005
    copyright            : (C) 2006 by Richard Groult
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

#include "imagelistviewsimple.h"

//Local
#include "imagemetainfo.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "osd.h"
#include "showimg_common.h"
#include "tools.h"

//KDE
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfileitem.h>

//QT
#include <qdir.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qstringlist.h>
#include <qtimer.h>

ImageListViewSimple::ImageListViewSimple(
				QObject     * a_p_parent,
				const KURL  & a_imageURL, 
				ImageViewer * a_p_imageviewer)
	: QObject(a_p_parent, "ImageListViewSimple"),

	m_p_imageviewer(NULL),
	m_p_imageMetaInfo(NULL),
	m_p_OSDWidget(NULL),
	m_currentPos(NULL),

	m_p_timer(NULL)
{
	MYDEBUG << endl;

	m_p_imageviewer = a_p_imageviewer;

	setImageURL(a_imageURL);
}


ImageListViewSimple::~ImageListViewSimple()
{
	if(m_p_timer) 
		m_p_timer->stop();
	delete(m_p_timer);
}

void
ImageListViewSimple::setImageURL(const KURL  & a_imageURL)
{
	MYDEBUG << endl;

	m_imageURL       = a_imageURL;
	m_currentPathURL = a_imageURL.upURL();
}

void
ImageListViewSimple::readConfig(KConfig *config, bool hideOSD)
{
	config->setGroup("OSD");
	if(!config->readBoolEntry("showOSD", true))
		return;

	m_p_imageMetaInfo = new ImageMetaInfo(NULL);

	m_p_OSDWidget = new ShowimgOSD(m_p_imageviewer);
	m_p_OSDWidget->setDuration(5*1000);
	m_p_OSDWidget->setDrawShadow(false);
	QFont m_font(m_p_OSDWidget->font());
	m_p_OSDWidget->initOSD(config->readBoolEntry("showOSD", true) && !hideOSD, config->readBoolEntry("OSDOnTop", true),  config->readFontEntry("OSDFont", &m_font), config->readBoolEntry("showFilename", true), config->readBoolEntry("showFullpath", false), config->readBoolEntry("showDimensions", true), config->readBoolEntry("showComments", true), config->readBoolEntry("showDatetime", true), config->readBoolEntry("showExif", false) );
}

void
ImageListViewSimple::load()
{
	MYDEBUG << endl;

	if(m_currentPathURL.isLocalFile())
	{
		QString s( m_currentPathURL.path() );
		QDir l_thisDir( s );
		l_thisDir.setFilter(QDir::Files);
		const QFileInfoList *files = l_thisDir.entryInfoList();
		if ( files )
		{
			QFileInfoListIterator it( *files );
			QFileInfo * f;
			QString ext;
			while( (f=it.current()) != 0 )
			{
				++it;
				if(f->isFile() && Tools::isImage(*f))
				{
					m_imagepathList.append(f->absFilePath());
				}
			}
		}
		m_currentPos =  m_imagepathList.find(m_imageURL.path());
	}
	else
	{
		m_imagepathList.append(m_imageURL.url());
		m_currentPos =  m_imagepathList.find(m_imageURL.url());
	}
	MYDEBUG << "m_imagepathList="<<m_imagepathList<<endl;
	MYDEBUG << "*m_currentPos="<<*m_currentPos<<endl;

	m_p_imageviewer->openURL(*m_currentPos);
	updateOSD(*m_currentPos);
}

void
ImageListViewSimple::initActions(KActionCollection * )
{
	if(m_p_imageviewer==NULL) {MYWARNING<<"pb in imagelistview: ImageViewer is NULL!!!"<<endl; return;}

	//
	connect(m_p_imageviewer, SIGNAL(askForPreviousImage()), 
		this, SLOT(previous()));
	connect(m_p_imageviewer, SIGNAL(askForNextImage()), 
		this, SLOT(next()));
	connect(m_p_imageviewer, SIGNAL(askForFirstImage()), 
		this, SLOT(first()));
	connect(m_p_imageviewer, SIGNAL(askForLastImage()), 
		this, SLOT(last()));
}

QString
ImageListViewSimple::currentAbsImagePath()
{
	MYDEBUG << endl;

	return *m_currentPos;
}


void
ImageListViewSimple::startSlideshow(int slideshowTime)
{
	MYDEBUG << endl;

	m_p_timer = new QTimer (this);
	m_p_timer->start (slideshowTime*1000, false);
	connect (m_p_timer, SIGNAL (timeout ()), this, SLOT (next ()));
}

void
ImageListViewSimple::next ()
{
	MYDEBUG << endl;
	m_currentPos++;
	if(m_currentPos==m_imagepathList.end())
	{
		first();
		return;
	}
	m_p_imageviewer->openURL(*m_currentPos);

	//----------
	updateOSD(*m_currentPos);

	//----------
	if(m_currentPos==m_imagepathList.end()) return;
	m_currentPos++;
	m_p_imageviewer->preopenURL(*m_currentPos);
	m_currentPos--;

}


void
ImageListViewSimple::previous ()
{
	MYDEBUG << endl;

	if(m_currentPos==m_imagepathList.begin())
	{
		last();
		return;
	}

	m_currentPos--;
	m_p_imageviewer->openURL(*m_currentPos);

	//----------
	updateOSD(*m_currentPos);

	//----------
	if(m_currentPos==m_imagepathList.begin()) return;
	m_currentPos--;
	m_p_imageviewer->preopenURL(*m_currentPos);
	m_currentPos++;
}


void
ImageListViewSimple::first ()
{
	MYDEBUG << endl;

	m_currentPos=m_imagepathList.begin();
	MYDEBUG<<*m_currentPos<<endl;
	m_p_imageviewer->openURL(*m_currentPos);

	//----------
	updateOSD(*m_currentPos);
}


void
ImageListViewSimple::last ()
{
	MYDEBUG << endl;

	m_currentPos = m_imagepathList.end();
	m_currentPos--;
	MYDEBUG<<*m_currentPos<<endl;
	m_p_imageviewer->openURL(*m_currentPos);

	//----------
	updateOSD(*m_currentPos);
}


void
ImageListViewSimple::updateOSD(const QString& currentfile)
{
	MYDEBUG << endl;

	if(!m_p_imageMetaInfo)
		return;

	KFileItem *item = new KFileItem(KFileItem::Unknown,
									KFileItem::Unknown,
									KURL::fromPathOrURL(currentfile));
	m_p_imageMetaInfo->setURL(item->url(), item->mimetype());

	QRect toberepainted(m_p_OSDWidget->geometry());
	QFileInfo info(m_p_imageMetaInfo->getURL().path());
	m_p_OSDWidget->setTexts(info.fileName(), info.dirPath(),
						  m_p_imageMetaInfo->getDimensions(), m_p_imageMetaInfo->getComments(),
						  m_p_imageMetaInfo->getDatetime().toString(),
						  m_p_imageMetaInfo->toString());
	m_p_OSDWidget->show();
	m_p_imageviewer->repaint(toberepainted);
	kapp->processEvents();
}


#include "imagelistviewsimple.moc"
