/***************************************************************************
                          showimgpart.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2005 by Jonathan Riddell, Richard Groult
    email                : jr@jriddell.org, rgroult@jalix.org
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

// Qt
#include <qcursor.h>
#include <qpoint.h>

// KDE
#include <kaction.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/browserextension.h>
#include <kparts/genericfactory.h>
#include <kprinter.h>
#include <kconfig.h>
#include <kshortcut.h>
#include <kstdaccel.h>
#include <kapplication.h>

// Local
#include "showimgpart.h"

#include <showimg/imageviewer.h>
#include <showimg/imagelistviewsimple.h>


const char* CONFIG_VIEW_GROUP="ShowImgPart View";

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class ShowImgPartView : public ImageViewer {
public:
	ShowImgPartView(QWidget* parent, KActionCollection* actionCollection, ShowImgPartBrowserExtension* browserExtension)
	: ImageViewer(parent/*, actionCollection*/), mBrowserExtension(browserExtension)
	{
		initActions(actionCollection);
		initMenu(actionCollection);
	}

protected:
	void openContextMenu(const QPoint&)
	{
		mBrowserExtension->contextMenu();
	}

private:
	ShowImgPartBrowserExtension* mBrowserExtension;
};


//Factory Code
typedef KParts::GenericFactory<ShowImgPart> ShowImgFactory;
K_EXPORT_COMPONENT_FACTORY( libshowimgpart /*library name*/, ShowImgFactory )

ShowImgPart::ShowImgPart(QWidget* parentWidget, const char* /*widgetName*/, QObject* parent,
			 const char* name, const QStringList&) : KParts::ReadOnlyPart( parent, name )
{
	setInstance( ShowImgFactory::instance() );

	mBrowserExtension = new ShowImgPartBrowserExtension(this);


	(void)new KActionMenu( i18n("&Go"), actionCollection(), "action go");
	// Create the widgets
	mImageViewer = new ShowImgPartView(parentWidget, actionCollection(), mBrowserExtension);
	setWidget(mImageViewer);

	m_imageListSimple = new ImageListViewSimple(this, QString::null, mImageViewer);
	m_imageListSimple->initActions(actionCollection());

	//////////////////////////////////////
	connect (mImageViewer, SIGNAL(sigSetMessage(const QString&)), this, SLOT(setMessage(const QString&)));
	(void)new KAction(i18n("Next"), KShortcut(Key_Space), m_imageListSimple, SLOT(next()), actionCollection(), "simple interface next" );
	connect(mImageViewer, SIGNAL(loaded(const KURL&)), SLOT(loaded(const KURL&)));

	setXMLFile( "showimgpart/showimgpart.rc" );
}

ShowImgPart::~ShowImgPart()
{
}


void ShowImgPart::partActivateEvent(KParts::PartActivateEvent* event)
{
	KConfig *config = new KConfig("showimgrc");
	if (event->activated())
	{
		mImageViewer->readConfig(config, "imageviewer widget");
		m_imageListSimple->readConfig(config, true);
	}
	else
	{
		mImageViewer->writeConfig(config, "imageviewer widget");
// 		m_imageListSimple->writeConfig(config);
		config->sync();
	}
        delete config;
	KParts::ReadOnlyPart::partActivateEvent( event );
}


KAboutData* ShowImgPart::createAboutData() {
	KAboutData* aboutData = new KAboutData( "showimg", I18N_NOOP("ShowImgPart"),
						"0.1", I18N_NOOP("Image Viewer"),
						KAboutData::License_GPL,
						"(c) 2004, Richard Groult <rgroult@jalix.org>");
	return aboutData;
}

bool ShowImgPart::openFile() {
	emit started( 0 );
	MYDEBUG<<m_file<<endl;
	m_imageListSimple->setImageFilePath(m_file);
	m_imageListSimple->load();
	emit setWindowCaption(m_url.prettyURL());
	emit completed( 0 );
	return true;
}

QString
ShowImgPart::filePath() {
	MYDEBUG<<m_file<<endl;
	return m_file;
}

void
ShowImgPart::setMessage(const QString& msg)
{
	emit setStatusBarText(msg);
	kapp->processEvents();
}

void
ShowImgPart::loaded(const KURL& url)
{
	mBrowserExtension->setLocationBarURL( url.path() );

	QString caption = QString("%1 - %2x%3")
			.arg(url.filename())
			.arg(mImageViewer->getImageWidth()).arg(mImageViewer->getImageHeight());
	emit setWindowCaption(caption);
	emit completed();
	emit setStatusBarText(i18n("Done."));
}

void
ShowImgPart::setKonquerorWindowCaption(const KURL& /*url*/, const QString& filename)
{
	QString caption = QString("%1").arg(filename);
	emit setWindowCaption(caption);
}

void ShowImgPart::print()
{
	MYDEBUG<<m_file<<endl;
	mImageViewer->slotPrint();
}


/***** ShowImgPartBrowserExtension *****/

ShowImgPartBrowserExtension::ShowImgPartBrowserExtension(ShowImgPart* viewPart, const char* name)
	:KParts::BrowserExtension(viewPart, name)
{
	mShowImgPart = viewPart;
	emit enableAction("print", true );
}

ShowImgPartBrowserExtension::~ShowImgPartBrowserExtension()
{
}

void
ShowImgPartBrowserExtension::contextMenu()
{
	emit popupMenu(QCursor::pos(), mShowImgPart->url(), 0);
}

void ShowImgPartBrowserExtension::print() {
	mShowImgPart->print();
}

#include "showimgpart.moc"
