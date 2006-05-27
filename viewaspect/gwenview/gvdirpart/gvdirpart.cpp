/*
Copyright 2004 Jonathan Riddell <jr@jriddell.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.

*/
#include "gvdirpart.moc"

#include <qcursor.h>
#include <qfile.h>
#include <qsplitter.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kaction.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kparts/browserextension.h>
#include <kparts/genericfactory.h>
#include <kio/job.h>

#include <gvcore/fileoperation.h>
#include <gvcore/archive.h>
#include <gvcore/cache.h>
#include <gvcore/document.h>
#include <gvcore/fileviewbase.h>
#include <gvcore/fileviewcontroller.h>
#include <gvcore/printdialog.h>
#include <gvcore/imageview.h>
#include <gvcore/slideshow.h>
#include <gvcore/thumbnailloadjob.h>

#include "config.h"
#include "gvdirpartconfig.h"

namespace Gwenview {

// For now let's duplicate
const char CONFIG_CACHE_GROUP[]="cache";


class GVDirPartFileViewController : public FileViewController {
public:
	GVDirPartFileViewController(QWidget* parent, KActionCollection* actionCollection, GVDirPartBrowserExtension* browserExtension)
	: FileViewController(parent, actionCollection)
	, mBrowserExtension(browserExtension) {}

protected:
	virtual void openContextMenu(const QPoint& pos, bool onItem) {
		if (onItem) {
			const KFileItemList* items=currentFileView()->selectedItems();
			emit mBrowserExtension->popupMenu(pos, *items);
		} else {
			emit mBrowserExtension->popupMenu(pos, dirURL(), 0);
		}
	}

private:
	GVDirPartBrowserExtension* mBrowserExtension;
};


//Factory Code
typedef KParts::GenericFactory<GVDirPart> GVDirFactory;
K_EXPORT_COMPONENT_FACTORY( libgvdirpart /*library name*/, GVDirFactory )

GVDirPart::GVDirPart(QWidget* parentWidget, const char* /*widgetName*/, QObject* parent, const char* name,
		     const QStringList &) : KParts::ReadOnlyPart( parent, name )  {
	GVDirFactory::instance()->iconLoader()->addAppDir( "gwenview");
	setInstance( GVDirFactory::instance() );
	KGlobal::locale()->insertCatalogue( "gwenview" );

	Cache::instance()->ref();

	mBrowserExtension = new GVDirPartBrowserExtension(this);
	mBrowserExtension->updateActions();

	mSplitter = new QSplitter(Qt::Horizontal, parentWidget, "gwenview-kpart-splitter");
	mSplitter->setFocusPolicy(QWidget::ClickFocus);
	mSplitter->setOpaqueResize(true);

	// Create the widgets
	mDocument = new Document(this);
	mFileViewController = new GVDirPartFileViewController(mSplitter, actionCollection(), mBrowserExtension);
	int width=GVDirPartConfig::fileViewWidth();
	if (width!=-1) {
		mFileViewController->widget()->resize(width, 10);
	}
	mImageView = new ImageView(mSplitter, mDocument, actionCollection());
	connect( mImageView, SIGNAL(requestContextMenu(const QPoint&)),
		mBrowserExtension, SLOT(openContextMenu(const QPoint&)) );
	mSplitter->setResizeMode(mFileViewController->widget(), QSplitter::KeepSize);

	mSlideShow = new SlideShow(mDocument);

	setWidget(mSplitter);

	KStdAction::saveAs( mDocument, SLOT(saveAs()), actionCollection(), "saveAs" );
	new KAction(i18n("Rotate &Right"), "rotate_cw", CTRL + Key_R, this, SLOT(rotateRight()), actionCollection(), "rotate_right");

	connect(mFileViewController, SIGNAL(urlChanged(const KURL&)),
		this, SLOT(urlChanged(const KURL&)) );
	connect(mFileViewController, SIGNAL(directoryChanged(const KURL&)),
		this, SLOT(directoryChanged(const KURL&)) );
	connect(mSlideShow, SIGNAL(nextURL(const KURL&)),
		this, SLOT(urlChanged(const KURL&)) );
	connect(mDocument, SIGNAL(loaded(const KURL&)),
		this, SLOT(loaded(const KURL&)) );

	// For wheel browsing
	connect(mImageView, SIGNAL(selectPrevious()),
		mFileViewController, SLOT(slotSelectPrevious()) );
	connect(mImageView, SIGNAL(selectNext()),
		mFileViewController, SLOT(slotSelectNext()) );

	mToggleSlideShow = new KToggleAction(i18n("Slide Show..."), "slideshow", 0, this, SLOT(toggleSlideShow()), actionCollection(), "slideshow");
	mToggleSlideShow->setCheckedState( i18n("Stop Slide Show" ));

	setXMLFile( "gvdirpart/gvdirpart.rc" );
}

GVDirPart::~GVDirPart() {
	GVDirPartConfig::setFileViewWidth(mFileViewController->widget()->width());
	GVDirPartConfig::writeConfig();
	delete mSlideShow;
	Cache::instance()->deref();
}


void GVDirPart::partActivateEvent(KParts::PartActivateEvent* event) {
	if (event->activated()) {
		KConfig* config=new KConfig("gwenviewrc");
		Cache::instance()->readConfig(config,CONFIG_CACHE_GROUP);
		delete config;
	}
}


KAboutData* GVDirPart::createAboutData() {
	KAboutData* aboutData = new KAboutData( "gvdirpart", I18N_NOOP("GVDirPart"),
						"0.1", I18N_NOOP("Image Browser"),
						KAboutData::License_GPL,
						"(c) 2004, Jonathan Riddell <jr@jriddell.org>");
	return aboutData;
}

bool GVDirPart::openFile() {
	//unused because openURL implemented

	//mDocument->setFilename(mFile);
	return true;
}

bool GVDirPart::openURL(const KURL& url) {
	if (!url.isValid()) {
		return false;
	}

	emit started( 0 );
	m_url = url;
	m_url.adjustPath(1);

	emit setWindowCaption( m_url.prettyURL() );
	mFileViewController->setDirURL(m_url);

	return true;
}

void GVDirPart::loaded(const KURL& url) {
	QString caption = url.filename();
	if( !mDocument->image().isNull())
		caption += QString(" %1 x %2").arg(mDocument->width()).arg(mDocument->height());
	emit setWindowCaption(caption);
	emit completed();
}

KURL GVDirPart::pixmapURL() {
	return mDocument->url();
}

void GVDirPart::toggleSlideShow() {
	if (mToggleSlideShow->isChecked()) {
        KURL::List list;
        KFileItemListIterator it( *mFileViewController->currentFileView()->items() );
        for ( ; it.current(); ++it ) {
            KFileItem* item=it.current();
            if (!item->isDir() && !Archive::fileItemIsArchive(item)) {
                list.append(item->url());
            }
        }
        if (list.count()==0) {
			mToggleSlideShow->setChecked(false);
            return;
        }
		//FIXME turn on full screen here (anyone know how?)
		mSlideShow->start(list);
	} else {
		//FIXME turn off full screen here
		mSlideShow->stop();
	}
}

void GVDirPart::print() {
	KPrinter printer;
	if ( !mDocument->filename().isEmpty() ) {
		printer.setDocName( m_url.filename() );
		KPrinter::addDialogPage( new PrintDialogPage( mDocument, mImageView, "GV page"));

		if (printer.setup(mImageView, QString::null, true)) {
			mDocument->print(&printer);
		}
	}
}

void GVDirPart::rotateRight() {
	mDocument->transform(ImageUtils::ROT_90);
}

void GVDirPart::directoryChanged(const KURL& dirURL) {
	if( dirURL == m_url ) return;
	emit mBrowserExtension->openURLRequest(dirURL);
}

void GVDirPart::urlChanged(const KURL& url) {
	mDocument->setURL( url );
	mFileViewController->setFileNameToSelect( url.filename());
}


/***** GVDirPartBrowserExtension *****/

GVDirPartBrowserExtension::GVDirPartBrowserExtension(GVDirPart* viewPart, const char* name)
	:KParts::BrowserExtension(viewPart, name) {
	mGVDirPart = viewPart;
	emit enableAction("print", true );
}

GVDirPartBrowserExtension::~GVDirPartBrowserExtension() {
}

void GVDirPartBrowserExtension::updateActions() {
	/*
	emit enableAction( "copy", true );
	emit enableAction( "cut", true );
	emit enableAction( "trash", true);
	emit enableAction( "del", true );
	emit enableAction( "editMimeType", true );
	*/
}

void GVDirPartBrowserExtension::del() {
	kdDebug() << k_funcinfo << endl;
}

void GVDirPartBrowserExtension::trash() {
	kdDebug() << k_funcinfo << endl;
}

void GVDirPartBrowserExtension::editMimeType() {
	kdDebug() << k_funcinfo << endl;
}

void GVDirPartBrowserExtension::refresh() {
	kdDebug() << k_funcinfo << endl;
}

void GVDirPartBrowserExtension::copy() {
	kdDebug() << k_funcinfo << endl;
}
void GVDirPartBrowserExtension::cut() {
	kdDebug() << k_funcinfo << endl;
}

void GVDirPartBrowserExtension::openContextMenu(const QPoint& pos) {
	KURL url=mGVDirPart->url();
	QString mimeType=KMimeType::findByURL(url)->name();
	emit popupMenu(pos, url, mimeType);
}


void GVDirPartBrowserExtension::print() {
	mGVDirPart->print();
}

} // namespace
