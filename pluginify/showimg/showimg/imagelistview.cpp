/***************************************************************************
                          imagelistview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#include "imagelistview.h"

// Local
#include "imageloader.h"
#include "directoryview.h"
#include "mainwindow.h"
#include "osd.h"
#include "viewer.h"
#include "categoryimageproperties.h"
#include "categorydbmanager.h"
#include <showimgdb/imageentry.h>

#include "kexifpropsplugin.h"
#ifndef Q_WS_WIN
#include "khexeditpropsplugin.h"
#endif
#include "imagefileiconitem.h"
#include "dirfileiconitem.h"
#include "imageviewer.h"
#include "imagemetainfo.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */

#ifdef HAVE_LIBKEXIF
	#if LIBKEXIF_VERSION < 020
	#warning LIBKEXIF_VERSION < 020
	#include <libkexif/kexif.h>
	#else
	#include <libkexif/kexifdialog.h>
	#endif
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */

// KDE
#include <kprocess.h>
#include <kiconview.h>
#include <kfileiconview.h>
#include <kaccel.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpropsdlg.h>
#include <kopenwith.h>
#include <klineeditdlg.h>
#include <kurlrequesterdlg.h>
#include <kio/job.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconeffect.h>
#include <kpixmapio.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#include <kapplication.h>
#include <kmimetype.h>
#if  KDE_VERSION >= 0x30200
#include <kinputdialog.h>
#endif

// QT
#include <qmessagebox.h>
#include <qmime.h>
#include <qiconview.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qpopupmenu.h>
#include <qdragobject.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qtextcodec.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class KToolTip : public QToolTip
{
public:
	KToolTip(QWidget *parent, ImageListView *imageList);
	virtual ~KToolTip();

	void setShow(bool show);

	void maybeTip(const QPoint &pos);

private:
	ImageListView *imageList;
	bool m_show;
};


KToolTip::KToolTip(QWidget *parent, ImageListView *imageList)
: QToolTip(parent)
{
	this->imageList=imageList;
}
KToolTip::~KToolTip()
{
}
void
KToolTip::setShow(bool show)
{
	m_show = show;
}
void
KToolTip::maybeTip(const QPoint &pos)
{
	if(!m_show || !imageList->isEnabled()) return;

	FileIconItem *item = imageList->itemAt(imageList->viewportToContents(pos));
	if(!item) return;
	QRect r = item->pixmapRect(false);
	r.moveTopLeft(imageList->contentsToViewport(r.topLeft()));
	if (!r.isValid() )	return;
	if (!item->toolTipStr().isEmpty()) tip(r, "<font size=\"-1\">"+item->toolTipStr()+"</font>");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

QtFileIconDrag::QtFileIconDrag( QWidget * dragSource, const char* name )
    : QIconDrag( dragSource, name )
{}

const char*
QtFileIconDrag::format( int i ) const
{
    if ( i == 0 )
		return "application/x-qiconlist";
    else if ( i == 1 )
		return "text/uri-list";
    else
	return 0;
}

QByteArray
QtFileIconDrag::encodedData( const char* mime ) const
{
	QByteArray a;
	if ( QString( mime ) == QString::fromLatin1("application/x-qiconlist") )
	{
		a = QIconDrag::encodedData( mime );
	}
	else if ( QString( mime ) == QString::fromLatin1("text/uri-list") )
	{
		QString s = urls.join( "\r\n" );
		a.resize( s.length() );
		memcpy( a.data(), s.latin1(), s.length() );
	}
	return a;
}

bool
QtFileIconDrag::canDecode( QMimeSource* e )
{
	return e->provides(
		"application/x-qiconlist" ) ||
		e->provides( "text/uri-list" );
}

void
QtFileIconDrag::append( const QIconDragItem &item, const QRect &pr,
			     const QRect &tr, const QString &url )
{
    QIconDrag::append( item, pr, tr );
    urls << url;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ImageListView::ImageListView (
			QWidget * parent,
			const QString& name,
			MainWindow * mw)
	:KIconView (parent, name.ascii()),

	curIt(NULL),
	sortMode(0),
	currentIconItem(NULL),
	dscr(NULL),
	loop_(true),
	preload_(true),
	random_(false),
	isLoadingThumbnail(false),
	toolTips(NULL),
	mouseIsPressed(false),
	m_currentDragItemAreMovable(false)
{
	this->mw = mw;

	m_popup = new KPopupMenu (); m_popup->insertTitle("", 1);
	m_popup_openWith = new KPopupMenu ();
	m_popupEmpty = new KPopupMenu ();

	il = new ImageLoader (this);

	connect(this, SIGNAL(selectionChanged()),
			SLOT(selectionChanged()));
	connect(this, SIGNAL(onItem(QIconViewItem*)),
			SLOT(highlight(QIconViewItem *)));
	connect(this, SIGNAL(onViewport()),
			SLOT(onViewport()));
	connect(this, SIGNAL(contextMenuRequested(QIconViewItem *, const QPoint &)),
			SLOT(popup(QIconViewItem *, const QPoint &)));

	connect(mw, SIGNAL(lastDestDirChanged(const QString&)),
			this, SLOT(updateDestDirTitle(const QString&)));


	setResizeMode (Adjust);
	setWordWrapIconText(true);
	setSelectionMode (Extended);
	setItemsMovable ( true );
	setItemTextPos( Bottom );
// 	setArrangement(LeftToRight);
	setSpacing(5);

	iconEffect=new KIconEffect();

	m_OSDWidget = new ShowimgOSD(mw->getImageViewer());
	m_OSDWidget->setDuration(5*1000);
	m_OSDWidget->setDrawShadow(false);
	connect(mw, SIGNAL(toggleFullscreen(bool)), this, SLOT(toggleShowHideOSD(bool)));

}

ImageListView::~ImageListView()
{
}

void
ImageListView::readConfig(KConfig *config)
{
	config->setGroup("Options");
	il->setStoreThumbnails(config->readBoolEntry("storeth", true));
	il->setShowFrame(config->readBoolEntry("showFrame", true));

	setWordWrapIconText(config->readBoolEntry("WordWrapIconText", true));
	setShowMimeType(config->readBoolEntry("ShowMimeType", false));
	setShowSize(config->readBoolEntry("ShowSize", true));
	setShowDate(config->readBoolEntry("ShowDate", true));
	setShowDimension(config->readBoolEntry("ShowDimension", false));
	setShowCategoryInfo(config->readBoolEntry("ShowCategoryInfo", true));
	setShowToolTips(config->readBoolEntry("ShowToolTips", false));
	//
	setPreloadIm(config->readBoolEntry("preloadIm", true));
	setShowMeta(config->readBoolEntry("showMeta", true));
	setShowHexa(config->readBoolEntry("showHexa", true));

	config->setGroup("Icons");
	int icSize = config->readNumEntry("size", 1);
	switch(icSize)
	{
		case 0  : aIconSmall->setChecked(true); break;
		case 1  : aIconMed->setChecked(true); break;
		case 2  : aIconBig->setChecked(true); break;
		case 3  : aIconTiny->setChecked(true); break;
		default : aIconMed->setChecked(true); break;
	};
	setThumbnailSize(icSize);

	config->setGroup("Slideshow");
	setLoop(config->readBoolEntry("loop", false));

	config->setGroup("confirm");
	il->setUseEXIF(mw->getImageViewer()->useEXIF());

	config->setGroup("Paths");
	setgimpPath(config->readPathEntry("gimpPath", "gimp-remote -n"));

	config->setGroup("OSD");
	QFont m_font(font());
	m_OSDWidget->initOSD(config->readBoolEntry("showOSD", true), config->readBoolEntry("OSDOnTop", false),  config->readFontEntry("OSDFont", &m_font), config->readBoolEntry("showFilename", true), config->readBoolEntry("showFullpath", false), config->readBoolEntry("showDimensions", true), config->readBoolEntry("showComments", true), config->readBoolEntry("showDatetime", true), config->readBoolEntry("showExif", false) );

}

void
ImageListView::writeConfig(KConfig *config)
{
	config->setGroup("Options");
	config->writeEntry( "storeth", il->getStoreThumbnails() );
	config->writeEntry( "showFrame", il->getShowFrame() );
	config->writeEntry( "preloadIm", preloadIm());
	config->writeEntry( "showMeta", showMeta());
	config->writeEntry( "showHexa", showHexa());
	config->writeEntry("WordWrapIconText", wordWrapIconText());
	config->writeEntry("ShowMimeType", getShowMimeType());
	config->writeEntry("ShowSize", getShowSize());
	config->writeEntry("ShowDate", getShowDate());
	config->writeEntry("ShowDimension", getShowDimension());
	config->writeEntry("ShowCategoryInfo", getShowCategoryInfo());
	config->writeEntry("ShowToolTips", getShowToolTips());

	config->setGroup("Slideshow");
	config->writeEntry( "loop", doLoop() );

	config->setGroup("Paths");
	config->writePathEntry( "gimpPath", getgimpPath() );

	config->setGroup("Icons");
	int icSize=0;
	if(aIconSmall->isChecked())     icSize=0;
	else if (aIconMed->isChecked()) icSize=1;
	else if (aIconBig->isChecked()) icSize=2;
	else if (aIconTiny->isChecked()) icSize=3;
	else icSize=1;
	config->writeEntry("size", icSize);

	config->setGroup("OSD");
	config->writeEntry("showOSD", m_OSDWidget->getShowOSD());
	config->writeEntry("OSDOnTop", m_OSDWidget->getOSDOnTop());
	config->writeEntry("OSDFont", m_OSDWidget->font());
	config->writeEntry("showFilename", m_OSDWidget->getOSDShowFilename());
	config->writeEntry("showFullpath", m_OSDWidget->getOSDShowFullpath());
	config->writeEntry("showDimensions", m_OSDWidget->getOSDShowDimensions());
	config->writeEntry("showComments", m_OSDWidget->getOSDShowComments());
	config->writeEntry("showDatetime", m_OSDWidget->getOSDShowDatetime());
	config->writeEntry("showExif", m_OSDWidget->getOSDShowExif());
}


void
ImageListView::initActions(KActionCollection *actionCollection)
{
	aRename		=new KAction(i18n("Rename File"), "item_rename", KShortcut(Key_F2), this, SLOT(slotRename()),actionCollection , "rename");
	aDelete		=new KAction(i18n("Delete File"), "editdelete", KShortcut(SHIFT+Key_Delete), this, SLOT(slotSupprimmer()), actionCollection, "editdelete");
	aTrash		=new KAction(i18n("Move File to Trash"), "edittrash", KShortcut(Key_Delete), this, SLOT(slotMoveToTrash()), actionCollection, "edittrash");
	aShred		=new KAction(i18n("Shred File"), "editshred", KShortcut(SHIFT+CTRL+Key_Delete), this, SLOT(slotShred()), actionCollection, "editshred");

	aFileProperties	=new KAction(i18n("Properties"), "info", 0, this, SLOT(slotFileProperty()), actionCollection, "Properties");
	aCategoryProperties	=new KAction(i18n("Category Properties"), "kexi_kexi", 0, this, SLOT(slotCategoryProperties()), actionCollection, "ImageCategoryProperties");
	aImageInfo	=new KAction(i18n("Image Info"), 0, this, SLOT(slotImageInfo()), actionCollection, "Image Info");

	aSelect		=new KAction(i18n("Select All"), KStdAccel::shortcut(KStdAccel::SelectAll) , this, SLOT(slotSelectAll()), actionCollection, "SelectAll");
	aUnselectAll	=new KAction(i18n("Unselect All"),0, this , SLOT(slotUnselectAll()), actionCollection, "Unselect All");
	aInvertSelection=new KAction(i18n("Invert Selection"), KShortcut(CTRL+Key_I),this , SLOT(slotInvertSelection()), actionCollection, "Invert Selection");

	//------------------------------------------------------------------------------
	aIconTiny	= new KRadioAction(i18n("Tiny Icons"), "tinythumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Tiny Icons");
	aIconSmall	= new KRadioAction(i18n("Small Icons"), "smallthumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Small Icons");
	aIconMed	= new KRadioAction(i18n("Medium Icons"), "medthumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Medium Icons");
	aIconBig	= new KRadioAction(i18n("Large Icons"), "largethumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Big Icons");
	aIconTiny->setExclusiveGroup("IconSize");
	aIconSmall->setExclusiveGroup("IconSize");
	aIconMed->setExclusiveGroup("IconSize"); //aIconMed->setChecked(true);
	aIconBig->setExclusiveGroup("IconSize");
	KActionMenu *actionIconSize = new KActionMenu( i18n("Icon Size"), "view_icon", actionCollection, "view_icons" );
	actionIconSize->insert(aIconTiny);
	actionIconSize->insert(aIconSmall);
	actionIconSize->insert(aIconMed);
	actionIconSize->insert(aIconBig);


	//
	aSortByName	=new KRadioAction(i18n("By Name"), 0, this, SLOT(slotByName()), actionCollection, "by name");
	aSortByType	=new KRadioAction(i18n("By Type"), 0, this, SLOT(slotByExtension()), actionCollection, "by extension");
	aSortBySize	=new KRadioAction(i18n("By Size"), 0, this, SLOT(slotBySize()), actionCollection, "by size");
	aSortByDate	=new KRadioAction(i18n("By Date"), 0, this, SLOT(slotByDate()), actionCollection, "by date");
	aSortByDirName	=new KRadioAction(i18n("By Path && Name"), 0, this, SLOT(slotByDirName()), actionCollection, "by dir name");
	aSortByName->setExclusiveGroup("sort mode");
	aSortByType->setExclusiveGroup("sort mode");
	aSortBySize->setExclusiveGroup("sort mode");
	aSortByDate->setExclusiveGroup("sort mode");
	aSortByDirName->setExclusiveGroup("sort mode");
	aSortByName->setChecked(true);

	//
	aOpenWithGimp	=new KAction(i18n("Open with &Gimp"), "gimp", 0, this, SLOT(slotGimp()), actionCollection, "Open with Gimp");
	aEditWithShowFoto	=new KAction(i18n("Edit with &showFoto"), "showfoto", KShortcut(CTRL+Key_E), this, SLOT(slotShowFoto()), actionCollection, "Edit with showFoto");
// 	aOpenWithKhexedit=new KAction(i18n("Open with &Khexedit"), "khexedit", 0, this, SLOT(slotKhexedit()), actionCollection, "Open with Khexedit");
	aOpenWith	=new KAction(i18n("&Other..."), 0, this, SLOT(slotOpenWith()), actionCollection, "Open with");

	//aDirCut		=new KAction(i18n("Cut"), "editcut", 0, this, SLOT(slotDirCut()),actionCollection , "editdircut");

	aFilesMoveTo	=new KAction(i18n("Move File to..."), 0, this, SLOT(slotFilesMoveTo()),actionCollection , "moveFilesTo");
	aFilesCopyTo	=new KAction(i18n("Copy File to..."), 0, this, SLOT(slotFilesCopyTo()),actionCollection , "copyFilesTo");
	aFilesMoveToLast=new KAction(i18n("Move File to Last Directory"), KShortcut(SHIFT+Key_F12),
								this, SLOT(slotFilesMoveToLast()),actionCollection , "moveFilesToLast");
	aFilesCopyToLast=new KAction(i18n("Copy File to Last Directory"), Key_F12,
								this, SLOT(slotFilesCopyToLast()),actionCollection , "copyFilesToLast");
	aFilesMoveToLast->setEnabled(false);
	aFilesCopyToLast->setEnabled(false);

#ifdef HAVE_LIBKEXIF
	aDisplayExifInformation	=new KAction(i18n("Exif Information"), 0, this, SLOT(slotDisplayExifInformation()), actionCollection, "files_Display_Exif_Information");
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
	aDisplayExifInformation = NULL;
#endif /* HAVE_LIBKEXIF */

	//
	aEXIF_Orientation_normal=new KToggleAction(i18n("Top Left"), 0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation normal");
	aEXIF_Orientation_hflip=new KToggleAction(i18n("Top Right"), 0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation hflip");
	//aEXIF_Orientation_rot180=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation ");
	aEXIF_Orientation_vflip=new KToggleAction(i18n("Bottom Left"), 0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation vflip");
	//aEXIF_Orientation_rot90hflip=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation ");
	aEXIF_Orientation_rot90=new KToggleAction(i18n("Right Top"),0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation rot90");
	//aEXIF_Orientation_rot90vflip=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation ");
	aEXIF_Orientation_rot270=new KToggleAction(i18n("Left Bottom"), 0, this, SLOT(slotEXIFOrientation()), actionCollection, "EXIF orientation rot270");

	a_regenEXIFThumb=new KAction(i18n("(Re)generate EXIF Thumbnail"), "thumbnail", 0, this, SLOT(generateEXIFThumbnails()), actionCollection, "Regenerate EXIF thumbnail" );

	a_regen=new KAction(i18n("Regenerate Thumbnail"), 0, this, SLOT(forceGenerateThumbnails()), actionCollection, "Regenerate thumbnail" );

	///
	if(mw->getImageViewer()==NULL) {kdWarning()<<"pb in imagelistview: ImageViewer is NULL!!!"<<endl; return;}
	connect(mw->getImageViewer(), SIGNAL(askForPreviousImage()), this, SLOT(previous()));
	connect(mw->getImageViewer(), SIGNAL(askForNextImage()), this, SLOT(next()));
	connect(mw->getImageViewer(), SIGNAL(askForFirstImage()), this, SLOT(first()));
	connect(mw->getImageViewer(), SIGNAL(askForLastImage()), this, SLOT(last()));
}

void
ImageListView::initMenu(KActionCollection *actionCollection)
{
	this->actionCollection=actionCollection;

	KActionMenu *actionSortMenu = new KActionMenu( i18n("Sort"), actionCollection, "view_sort" );
	actionSortMenu->insert(aSortByName);
	actionSortMenu->insert(aSortByType);
	actionSortMenu->insert(aSortBySize);
	actionSortMenu->insert(aSortByDate);
	actionSortMenu->insert(new KActionSeparator());
	actionSortMenu->insert(aSortByDirName);

	//actionCollection->action("create_new_items")->plug(m_popupEmpty);
	actionCollection->action("editpaste")->plug(m_popupEmpty);

	m_popupEmpty->insertSeparator();
	actionCollection->action("view_icons")->plug(m_popupEmpty);
	actionSortMenu->plug(m_popupEmpty);

	m_popupEmpty->insertSeparator();
	aSelect->plug(m_popupEmpty);
	aUnselectAll->plug(m_popupEmpty);
	aInvertSelection->plug(m_popupEmpty);

	/////////////
	/////////////
	m_popup_openWith = new KPopupMenu ();
	m_popup->insertItem(i18n("Open With"), m_popup_openWith);
	actionCollection->action("Edit with showFoto")->plug(m_popup);
// 	m_popup->insertSeparator();
// 	actionCollection->action("editcopy")->plug(m_popup);

	///////////
	m_popup->insertSeparator();
	aCopyActions = new KActionMenu( i18n("Copy"), 0, actionCollection, "Copy files actions" );
	aCopyActions->plug(m_popup);
	aCopyActions->popupMenu()->insertTitle(i18n("Copy"), 1);
	aCopyActions->insert(aFilesCopyToLast);
	aCopyActions->insert(aFilesCopyTo);


	aMoveActions = new KActionMenu( i18n("Move"), 0, actionCollection, "Move files actions" );
	aMoveActions->plug(m_popup);
	aMoveActions->popupMenu()->insertTitle(i18n("Move"), 1);
	aMoveActions->insert(aFilesMoveToLast);
	aMoveActions->insert(aFilesMoveTo);

// 	m_popup->insertSeparator();
	aRename->plug(m_popup);
	aTrash->plug(m_popup);
	aDelete->plug(m_popup);


	////////
	m_popup->insertSeparator();
	KActionMenu *aEXIF = new KActionMenu( i18n("EXIF"), 0, actionCollection, "EXIF actions" );
	aEXIF->popupMenu()->insertTitle(i18n("Exif Information"));
	////////

	aEXIF_Orientation = new KActionMenu( i18n("Orientation"), "rotate", actionCollection, "EXIF orientation" );
	aEXIF_Orientation->insert(aEXIF_Orientation_normal);
	aEXIF_Orientation->insert(aEXIF_Orientation_hflip);
	aEXIF_Orientation->insert(aEXIF_Orientation_vflip);
	aEXIF_Orientation->insert(aEXIF_Orientation_rot90);
	aEXIF_Orientation->insert(aEXIF_Orientation_rot270);

	aEXIF->insert(aEXIF_Orientation);

	////////
	aEXIF->insert(a_regenEXIFThumb);
	a_regenEXIFThumb->setEnabled(false);

	////////
	if(aDisplayExifInformation)
	{
		aEXIF->insert(new KActionSeparator());
		aDisplayExifInformation->plug(aEXIF->popupMenu());
	}

	aEXIF->plug(m_popup);
	a_regen->plug(m_popup);	a_regen->setEnabled(false);

	////////
	m_popup->insertSeparator();
	aImageInfo->plug(m_popup);
	if(mw->getCategoryDBManager()->isConnected()) aCategoryProperties->plug(m_popup);
	aFileProperties->plug(m_popup);
}




KPopupMenu*
ImageListView::popupOpenWith()
{
	popup(currentItem(), QPoint());
	return m_popup_openWith;
}

void
ImageListView::popup(QIconViewItem *item, const QPoint &)
{
	m_popup_openWith->clear();
	m_popup_openWith->disconnect();

	if(!item)
	{
		m_popup_openWith->setEnabled(false);
		return;
	}
	m_popup_openWith->setEnabled(true);

	if(((FileIconItem*)item)->mimetype().left(5) == QString::fromLatin1("image"))
	{
		actionCollection->action("Open with Gimp")->plug(m_popup_openWith);
// 		actionCollection->action("Edit with showFoto")->plug(m_popup_openWith);
		actionCollection->action("Edit with showFoto")->setEnabled(true);
		m_popup_openWith->insertSeparator ();
	}
	else
	{
		actionCollection->action("Edit with showFoto")->setEnabled(false);
	}
	//
	m_offerList = KTrader::self()->query(((FileIconItem*)item)->mimetype(), "Type == 'Application'");
	for (uint i=0; i < m_offerList.count(); i++)
	{
		m_popup_openWith->insertItem(SmallIcon(m_offerList[i]->icon()), m_offerList[i]->name(), i+1);
	}
	if(m_offerList.size()!=0)
		m_popup_openWith->insertSeparator ();

	actionCollection->action("Open with")->plug(m_popup_openWith);
	connect(m_popup_openWith, SIGNAL(activated(int)),
		this, SLOT(slotRun(int)));
	mouseIsPressed=false;
}

void
ImageListView::slotRun(int  id)
{
	if((uint)id>=1 && (uint)id<=m_offerList.size())
	{
		KURL::List list;
		for ( FileIconItem *item = firstItem(); item; item = item->nextItem() )
		{
			if ( item->isSelected() )
				list << item->getURL();
		}
		if (KRun::run(*m_offerList[id-1], list)==0)
		{
			KMessageBox::error(this, "<qt>"+i18n("Error while running %1.").arg((*m_offerList[id-1]).name())+"</qt>");
		}
	}
}


bool
ImageListView::doPreload()
{
	return preload_;
}

void
ImageListView::setPreload(bool p)
{
	this->preload_=p;
}

bool
ImageListView::doLoop()
{
	return loop_;
}

void
ImageListView::setLoop(bool loop)
{
	this->loop_=loop;
}

bool
ImageListView::doRandom()
{
	return random_;
}

void
ImageListView::setRandom(bool ran)
{
	this->random_=ran;
}


void
ImageListView::setShowToolTips(bool s)
{
	sToolTips_=s;
	if(getShowToolTips() && !toolTips)
	{
		toolTips = new KToolTip(viewport(), this);
	}
	if(toolTips)
		toolTips->setShow(getShowToolTips());

}

bool
ImageListView::getShowToolTips()
{
	return sToolTips_;
}

FileIconItem*
ImageListView::itemAt(const QPoint pos)
{
	return (FileIconItem*)KIconView::findItem(pos);
}


void
ImageListView::setThumbnailSize(bool refresh)
{
	mw->slotStop();
	if(aIconSmall->isChecked())
	{
		setThumbnailSize(0, refresh);
	}
	else
	if(aIconMed->isChecked ())
	{
		setThumbnailSize(1, refresh);
	}
	else
	if(aIconBig->isChecked ())
	{
		setThumbnailSize(2, refresh);
	}
	else
	if(aIconTiny->isChecked())
	{
		setThumbnailSize(3, refresh);
	}
	else
	if(refresh) mw->slotRefresh ();
}


void
ImageListView::setThumbnailSize(const int newSize, bool refresh)
{
	QSize size;
	switch(newSize)
	{
		case 0 : size = QSize(80, 60); break;
		case 1 : size = QSize(128, 96); break;
		case 2 : size = QSize(160, 120); break;
		case 3 : size = QSize(64, 64); break;
		default : size = QSize(128, 96); break;
	}
	setThumbnailSize(size, refresh);
}

void
ImageListView::setThumbnailSize(const QSize newSize, bool )
{
	currentIconSize = new QSize(newSize);
	il->setThumbnailSize(*currentIconSize);

	setUpdatesEnabled(false);

	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		i->setHasPreview (false);
		if(!mw->preview())
			i->setPixmap(i->fileInfo()->pixmap(
				getCurrentIconSize().width() / 2));
		else
			i->calcRect();
	}

	setUpdatesEnabled(true);
	slotUpdate();
	arrangeItemsInGrid();
	ensureItemVisible(currentItem());
}


QSize
ImageListView::getCurrentIconSize()
{
	if(!mw->preview())
		return (*currentIconSize/2);
	else
		return *currentIconSize;
}

void
ImageListView::slotUpdate ()
{
}

void
ImageListView::slotResetThumbnail()
{
 	stopLoading();
	setUpdatesEnabled(false);
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		item->setPixmap(item->fileInfo()->pixmap(getCurrentIconSize().width()/2));
	}
	setUpdatesEnabled(true);
}


void
ImageListView::slotRename()
{
	FileIconItem *item = currentItem();
	if(item)
	{
		bool ok;
		QString oldName = item->text(0);
		QString oldFullPath = item->fullName();
		const QString newName(KInputDialog::getText(i18n("Rename %1:").arg(oldName),
					i18n("Enter new name:"),
					oldName,
					 &ok, mw->getImageViewer()).stripWhiteSpace());
		if(ok && !newName.isEmpty())
		{
			item->setText(newName);
			emit fileIconRenamed(oldFullPath, item->fullName());
		}
	}
}


void
ImageListView::refresh ()
{
	mw->slotRefresh();
}


QString
ImageListView::currentItemName()
{
	if(currentItem())
		return currentItem()->text();
	else
		return QString();
}


void
ImageListView::setCurrentItemName(const QString& itemName, bool select)
{
	 KIconView::setCurrentItem(findItem(itemName));
	 if(currentItem())
	 {
	 	KIconView::setSelected(currentItem(), select, false);
		KIconView::ensureItemVisible (currentItem());
		kapp->processEvents();
	 	if(select) currentItem()->setSelected(true);
	}
}

bool
ImageListView::hasImages()
{
	return KIconView::count()!=0;
}

bool
ImageListView::hasImageSelected()
{
	if(!hasImages()) return false;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected() && i->isImage())
			return true;
	}
	return false;
}

bool
ImageListView::hasSelection()
{
	if(!hasImages()) return false;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected() )
			return true;
	}
	return false;
}

int
ImageListView::countSelected()
{
	int number=0;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
		if(i->isSelected()) number++;
	return number;
}


bool
ImageListView::hasOnlyOneImageSelected()
{
	int number=0;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected())
			number++;
		if(number>=2) return false;

	}
	return true;
}


FileIconItem*
ImageListView::firstSelected()
{
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected())
			return i;
	}
	return NULL;
}



void
ImageListView::selectionChanged()
{
	int nbrSel=selectedURLs().count();
	mw->setHasImageSelected(nbrSel>0);
	if(nbrSel>1)
		emit sigSetMessage(i18n("%n selected file", "%n selected files", nbrSel));
	else
		emit sigSetMessage(i18n("Ready"));

#ifdef HAVE_KIPI
	if(mw->pluginManager()) mw->pluginManager()->selectionChanged(hasImageSelected());
#endif /* HAVE_KIPI */

	bool isReadonly = true;
	bool isFile = true;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
	{
		if( !i->isSelected()) continue;
		if( !(isReadonly = isReadonly&&!i->isMovable())) break;
		if( !(isFile = isFile&&i->getProtocol() == QString::fromLatin1("file"))) break;
	}
	aRename->setEnabled(!isReadonly);
	aDelete->setEnabled(!isReadonly);
	aFilesMoveTo->setEnabled(!isReadonly);
	aFilesMoveToLast->setEnabled(!isReadonly && !mw->getLastDestDir().isEmpty());
	aFilesCopyTo->setEnabled(nbrSel>0);
	aShred->setEnabled(!isReadonly);
	aTrash->setEnabled(!isReadonly);
	aEditWithShowFoto->setEnabled(!isReadonly);
	a_regen->setEnabled(!isReadonly);
	aEXIF_Orientation->setEnabled(!isReadonly);
	aCategoryProperties->setEnabled(isFile);
}

void
ImageListView::slotLoadFirst(bool force, bool forceEXIF)
{
	if (mw->preview() && count()>0 && !isLoadingThumbnail)
	{
		stopLoading();

		mw->slotReset ();
		imageLoading = firstItem();
		if(count()==1)
		{
			if(!imageLoading->hasPreview() && imageLoading->isImage())
			{
				isLoadingThumbnail=true;
				QFileInfo fi(imageLoading->fullName());
				il->loadMiniImage (fi, true, force, forceEXIF);
				stopLoading ();
			}
			else imageLoading=NULL;
		}
		else
		{
			int nbr=0;
			while(   imageLoading &&
				( (imageLoading->hasPreview() || !imageLoading->isImage())) ||
				  (forceEXIF && ! imageLoading->isSelected()))
			{
				if(imageLoading->isImage()) nbr++;
				imageLoading=imageLoading->nextItem ();

			}
			mw->slotPreviewDone (nbr);
			if(imageLoading)
			{
				actionCollection->action("Regenerate thumbnail")->setEnabled(false);
				actionCollection->action("Regenerate EXIF thumbnail")->setEnabled(false);
				isLoadingThumbnail=true;
				slotLoadNext (force, forceEXIF);
			}
		}
		if(!imageLoading) mw->slotDone();
	}
}

void
ImageListView::slotLoadFirst(FileIconItem *item)
{
	if (mw->preview())
	{

		mw->slotReset (false);
		imageLoading = item;
		if(imageLoading)
		{
			isLoadingThumbnail=true;
			slotLoadNext ();
		}
		else
			mw->slotDone ();
	}
}

void
ImageListView::slotLoadNext (bool force, bool forceEXIF)
{
	if(!isLoadingThumbnail) return;
	if(imageLoading)
	{
		QString ext = QFileInfo(imageLoading->fullName()).extension(true).lower();
		while(! imageLoading->isImage()
			|| imageLoading->hasPreview()
			|| ext == QString::fromLatin1("psd") || ext == QString::fromLatin1("svgz")|| ext == QString::fromLatin1("svg")
			|| forceEXIF && ! imageLoading->isSelected())
		{
			imageLoading = imageLoading->nextItem ();
			if(!imageLoading)break;
			ext = QFileInfo(imageLoading->fullName()).extension(true).lower();
		}
	}
	if(imageLoading)
	{
		QFileInfo fi(imageLoading->fullName());
		il->loadMiniImage (fi, true, force, forceEXIF);
	}
	else
	{
		stopLoading();
	}
}


void
ImageListView::slotSetPixmap (const QPixmap& pm, const QFileInfo& imgFile, bool success, bool force, bool forceEXIF)
{
	if(!isLoadingThumbnail) return;
	nbrTh++;
	if(imageLoading)
	{
		if(imgFile.absFilePath() != imageLoading->fullName())
			imageLoading = findItem (imgFile.absFilePath(), true);
		if(imageLoading)
		{
			imageLoading->setPixmap (pm, success);
			if((force || forceEXIF) && imageLoading->isSelected()) reload();
		}
		mw->slotPreviewDone ();
		repaint(imageLoading);
		kapp->processEvents();
		if(imageLoading) imageLoading = imageLoading->nextItem ();
	}
	if (imageLoading)
		slotLoadNext (force, forceEXIF);
	else
		stopLoading ();
}

void
ImageListView::stopLoading ()
{
	il->stopLoading (true);
	imageLoading=NULL;
	isLoadingThumbnail=false;
	mw->slotDone();
	nbrTh=0;
}


void
ImageListView::next ()
{
	if(!hasImages()) return;

	FileIconItem *item=NULL;
	if(doRandom())
	{
		srand(time(NULL));
		while(item==NULL)
		{
			int x = (int)(rand()/(RAND_MAX+1.0)*contentsWidth());
			int y = (int)(rand()/(RAND_MAX+1.0)*contentsHeight());
			item=(FileIconItem *)KIconView::findItem((const QPoint&)QPoint(x,y));
		}
	}
	else
	{
		item=currentItem();
		if(!item)
			item=firstItem();
		else
			item=item->nextItem();
	}

	if(item)
	{
		while(item && ! (item->isImage() || item->mimetype().left(5) == QString::fromLatin1("video")) )
			item=item->nextItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		//KIconView::setSelected (item, true, false);
	 		item ->setSelected (true);
			if(dscr) slotImageInfo();

			return;
		}
	}
	if(doLoop())
	{
		first();
		/* FIXME wrap ?
		if(!mw->fullScreen())
			first();
		else
				   getDirectoryView()->goToNextDir();
		*/
	}
}


void
ImageListView::previous ()
{
	if(!hasImages())
		return;

	FileIconItem *item=NULL;
	if(doRandom())
	{
		srand(time(NULL));
		while(item==NULL)
		{
			int x = (int)(rand()/(RAND_MAX+1.0)*contentsWidth());
			int y = (int)(rand()/(RAND_MAX+1.0)*contentsHeight());
			item=(FileIconItem *)KIconView::findItem((const QPoint&)QPoint(x,y));
		}
	}
	else
	{
		item=currentItem();
		if(!item)
			item=firstItem();
		else
			item=item->prevItem();
	}
	if(item)
	{
		while(item && ! (item->isImage() || item->mimetype().left(5) == QString::fromLatin1("video")) )
			item=item->prevItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		KIconView::setSelected (item, true, false);
	 		item ->setSelected (true);
			if(dscr)slotImageInfo();
		}
		else if(doLoop()) last();
	}
	else if(doLoop()) last();
}

void
ImageListView::contentsMousePressEvent (QMouseEvent * e)
{
	KIconView::contentsMousePressEvent (e);
	mouseIsPressed=true;

	if (e->button () == RightButton)
	{
		int nbS = countSelected();
		if(nbS != 0)
		{
			//FileIconItem *si = firstSelected();
			//si->setSelected (true);
			if(nbS == 1)
				m_popup->changeTitle(1, currentItem()->iconPixmap(), currentItem()->text());
			else
				m_popup->changeTitle(1, SmallIcon("editcopy"), i18n("%1 selected files").arg(nbS));

			popup(currentItem(), e->globalPos ());
			m_popup->exec(e->globalPos ());
		}
		else
			m_popupEmpty->exec (e->globalPos ());
	}
}

void
ImageListView::contentsMouseReleaseEvent (QMouseEvent * e)
{
	mouseIsPressed=false;
	if (e->button () == LeftButton)
	{
		int nbs=0;
		for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
		{
			if (item->isSelected () )
			{
				nbs++;
				if(nbs==2) break;
			}
		}
		if(nbs!=1)
		{
			KIconView::contentsMouseReleaseEvent ( e );
			return;
		}
	}
	mousePress(e);
}


void
ImageListView::mousePress (QMouseEvent * e)
{
	FileIconItem *si = firstSelected();
#ifndef Q_WS_WIN
	if (e->button () == MidButton)
	{
		contentsMouseDoubleClickEvent(e);
	}
	else
#endif
	if (e->button () == LeftButton)
	{
		KIconView::contentsMouseReleaseEvent ( e );
		if(!KGlobalSettings::singleClick())
		{
			if(si)
			{
				si->setSelected (true);
				if(dscr) slotImageInfo();
			}
		}
		else
		{
			if(!si) return;
			QString dirName=si->fullName();
			if(si->getType() == QString::fromLatin1("directory") )
			{
				curIt=NULL;
				KApplication::restoreOverrideCursor ();
				mw->openDir(dirName);
			}
			else
			{
				si->setSelected (true);
			}
		}
	}
	mouseIsPressed=false;
}

void
ImageListView::contentsMouseDoubleClickEvent ( QMouseEvent * e)
{
	if (!currentItem () || e->button () == RightButton) return;
	if( currentItem()->isImage())
	{
		mw->slotFullScreen ();
		currentItem ()->setSelected (true);
	}
	else
	if(currentItem()->getType() == QString::fromLatin1("directory"))
	{
		curIt=NULL;KApplication::restoreOverrideCursor ();
		mw->openDir(QDir::cleanDirPath(currentItem()->fullName()));
	}
	else
		KRun::runURL(currentItem()->getURL(), currentItem()->fileInfo()->mimetype());
}

//////////////////////////////////////////////////////////////////////////////

QString
ImageListView::getCurrentKey()
{
	switch(sortMode)
	{
		case 0:return "name";
		case 1:return "type";
		case 2:return "size";
		case 3:return "date";
		case 4:return "dirname";
		default:return "name";
	}
}


void
ImageListView::sort()
{
	KIconView::sort();
}


void
ImageListView::slotByName()
{
	sortMode=0;
	FileIconItem *i;
	for (i=firstItem(); i; i = i->nextItem() )
		i->setKey("name");
	sort();
}

void
ImageListView::slotByDirName()
{
	sortMode=4;
	FileIconItem *i;
	for (i=firstItem(); i; i = i->nextItem() )
		i->setKey("dirname");
	sort();
}


void
ImageListView::slotByExtension()
{
	sortMode=1;
	FileIconItem *i;
	for (i=firstItem(); i; i = i->nextItem() )
		i->setKey("type");
	sort();
}


void
ImageListView::slotBySize()
{
	sortMode=2;
	FileIconItem *i;
	for (i=firstItem(); i; i = i->nextItem() )
		i->setKey("size");
	sort();
}


void
ImageListView::slotByDate()
{
	sortMode=3;
	FileIconItem *i;
	for (i=firstItem(); i; i = i->nextItem() )
		i->setKey("date");
	sort();
}

//////////////////////////////////////////////////////////////////////////////

QDragObject *
ImageListView::dragObject ()
{
	if ( !currentItem() )
		return 0;
	QPoint orig = viewportToContents( viewport()->mapFromGlobal( QCursor::pos() ) );
	QtFileIconDrag *drag = new QtFileIconDrag( this, "ImageListView::dragObject()" );
	drag->setPixmap( *currentItem()->pixmap(),
				QPoint( currentItem()->pixmapRect().width() / 2,
					currentItem()->pixmapRect().height() / 2 ) );
	m_currentDragItemAreMovable = true;
	for ( FileIconItem *item = firstItem(); item; item = item->nextItem() )
	{
		if ( item->isSelected() )
		{
			QIconDragItem id;

			QCString curl = item->getURL().url().utf8();
			id.setData(curl);
			drag->append( id,
				QRect(	item->pixmapRect( false ).x() - orig.x(),
					item->pixmapRect( false ).y() - orig.y(),
					item->pixmapRect().width(), item->pixmapRect().height() ),
				QRect(	item->textRect( false ).x() - orig.x(),
					item->textRect( false ).y() - orig.y(),
					item->textRect().width(), item->textRect().height() ),
				item->getURL().url());
			m_currentDragItemAreMovable = m_currentDragItemAreMovable && item->isMovable();
		}
	}
	return drag;
}


void
ImageListView::slotWallpaper ()
{
	FileIconItem *item = currentItem ();
	if (!item) return;
	currentItem ()->setWallpaper ();
}

void
ImageListView::slotCategoryProperties()
{
#ifdef WANT_LIBKEXIDB
	if(!currentItem()) return;
	KApplication::setOverrideCursor (waitCursor); // this might take time
	mw->setEnabled(false);

	QStringList image_path_list = selectedItemsPath();
	QPtrList<ImageEntry> imageEntryList = mw->getCategoryDBManager()->getImageEntries(image_path_list);
	mw->setEnabled(true);
	CategoriesImageProperty catimgprop(this, mw->getCategoryDBManager(), imageEntryList, image_path_list.count());
	KApplication::restoreOverrideCursor ();

	////////
	if(!image_path_list.isEmpty() && catimgprop.exec())
	{
		mw->setEnabled(false);
		KApplication::setOverrideCursor (waitCursor); // this might take time
		mw->getCategoryDBManager()->updateImageInformations
		(
			imageEntryList,

			catimgprop.getComment(),
			catimgprop.getNote(),
			catimgprop.getDate_begin(),
			catimgprop.getDate_end(),
			catimgprop.getRemovedCategories(),
			catimgprop.getAddedCategories()
		);

		for(ImageEntry *image = imageEntryList.first(); image; image=imageEntryList.next())
		{
			image_path_list.remove(image->getName());
		}
		mw->getCategoryDBManager()->addImageInformations
		(
			image_path_list,

			catimgprop.getComment(),
			catimgprop.getNote(),
			catimgprop.getDate_begin(),
			catimgprop.getDate_end(),
			catimgprop.getAddedCategories()
		);

		mw->setEnabled(true);
		KApplication::restoreOverrideCursor ();
	}
	//KMessageBox::sorry(mw, "<qt>"+i18n("TODO: Image Category Properties")+"</qt>");
#endif /* #ifdef WANT_LIBKEXIDB */
}

void
ImageListView::slotFileProperty()
{
	if(!currentItem())
		return;
	KApplication::setOverrideCursor (waitCursor);
	//
	KFileItemList itemList;
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		if (item->isSelected ())
			itemList.append(item->fileInfo());
	}
	KPropertiesDialog *prop	= new KPropertiesDialog( itemList,
		mw->getImageViewer(), "KPropertiesDialog",
		true, false);
	//
	if(itemList.count()==1)
	{
		KEXIFPropsPlugin *exifProp=NULL;
		if(showMeta() && currentItem()->mimetype() == QString::fromLatin1("image/jpeg"))
		{
			exifProp= new KEXIFPropsPlugin(prop, currentItem()->fullName());
			prop->insertPlugin(exifProp);
		}
		//
		if(currentItem()->fileInfo()->mimetype().right(9)!="directory")
		{
			QFile::Offset big = 0x501400;// 5MB
			QFile qfile( currentItem()->fullName() );
			if(showHexa() && qfile.size() < big)
			{
#ifndef Q_WS_WIN
				KHexeditPropsPlugin *hexeditProp = new KHexeditPropsPlugin(prop, currentItem()->fullName());
				prop->insertPlugin(hexeditProp);
#endif
			}
		}
	}
	//
	KApplication::restoreOverrideCursor ();
	if(prop->exec())
	{
		//prop->applyChanges();
	}
}



void
ImageListView::slotImageInfo()
{
 	if(!currentItem()) return;

	KApplication::setOverrideCursor (waitCursor);
	if(!dscr)
	{
		dscr = new Describe(mw->getImageViewer(), currentItem()->fullName(), "ImageInfo");
		connect(dscr, SIGNAL(close()), SLOT(slotDescribeClose()));
	}
	else
		dscr->setImageFile(currentItem()->fullName());
	KApplication::restoreOverrideCursor ();
	dscr->show();
}

void
ImageListView::slotDescribeClose()
{
	delete(dscr); dscr=NULL;
}

// void
// ImageListView::slotKhexedit()
// {
// 	FileIconItem *item = currentItem ();
// 	if (!item)
// 		return;
// 	KRun::run("khexedit",
// 		KURL::List(QStringList(item->getURL().url())));
// }

void
ImageListView::slotShowFoto()
{
	KURL::List list;
	for ( FileIconItem *item = firstItem(); item; item = item->nextItem() )
	{
		if ( item->isSelected() )
			list << item->getURL();
	}
	if (list.isEmpty()) return;
	if (KRun::run(KStandardDirs::findExe("showfoto"), list, "showfoto", "showfoto")==0)
	{
		KMessageBox::error(this, "<qt>"+i18n("Error while running showFoto.<br>Please check \"showfoto\" on your system.")+"</qt>");
	}
}

void
ImageListView::slotGimp ()
{
	KURL::List list;
	for ( FileIconItem *item = firstItem(); item; item = item->nextItem() )
	{
		if ( item->isSelected() )
		list << item->getURL();
	}
	if (list.isEmpty()) return;
	if (KRun::run(getgimpPath(), list, "gimp", "gimp")==0)
	{
		KMessageBox::error(this, "<qt>"+i18n("Error while running Gimp.<br>Please check \"gimp-remote\" on your system.")+"</qt>");
	}
}


void
ImageListView::slotEndGimp (KProcess *proc)
{
	if(proc->exitStatus()!=0)
	{
		FileIconItem* item = currentItem ();
		if (!item)
			return;
		KRun::run("gimp", item->getURL());
	}
}



void
ImageListView::slotSupprimmer ()
{
	KURL::List list;
	QPtrList < FileIconItem > iconList;
	FileIconItem *nextItem = 0;
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		if (item->isSelected ())
		{
			nextItem = item->nextItem ();
			if (item->text (3) == QString::fromLatin1("file"))
			{
				list.append(item->getURL());
			}
			else
			{
				iconList.append(item);
			}
		}
	}
	if(!list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(mw, KonqOperations::DEL, list);
#endif
	}

	for ( FileIconItem * item = iconList.first(); item; item = iconList.next() )
		item->suppression(false);


	if (nextItem)
	{
		KIconView::setCurrentItem (nextItem);
		nextItem->setSelected (true);
		ensureItemVisible (currentItem());
	}

	emit fileIconsDeleted(list);
}


void
ImageListView::deletionDone( KIO::Job *job)
{
	if(job->error()!=0) job->showErrorDialog();
	refresh();
}


void
ImageListView::slotMoveToTrash()
{
	FileIconItem *nextItem = NULL;
	QPtrList < FileIconItem > iconList;
	KURL::List list;
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		if (item->isSelected ())
		{
			nextItem = item->nextItem ();
			if (item->text (3) == QString::fromLatin1("file"))
			{
				list.append(item->getURL());
			}
			else
			{
				iconList.append(item);
			}
		}
	}
	if(!list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(mw, KonqOperations::TRASH, list);
#endif
	}
	for ( FileIconItem * item = iconList.first(); item; item = iconList.next() )
		item->moveToTrash();

	if (nextItem)
	{
		KIconView::setCurrentItem (nextItem);
		nextItem->setSelected (true);
		ensureItemVisible (currentItem());
	}

	emit fileIconsDeleted(list);
}


void
ImageListView::slotShred()
{
	KURL::List list;
	QPtrList < FileIconItem > iconList;
	FileIconItem *nextItem = 0;
	for (FileIconItem * item = firstItem ();
	    item != 0; item =  item->nextItem ())
	{
		if (item->isSelected ())
		{
			nextItem = item->nextItem ();
			if (item->text (3) == QString::fromLatin1("file"))
			{
				list.append(item->getURL());
			}
			else
			{
				iconList.append(item);
			}
		}
	}
	if(!list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(mw, KonqOperations::SHRED, list);
#endif
	}
	for ( FileIconItem * item = iconList.first(); item; item = iconList.next() )
		item->shred();

	if (nextItem)
	{
		KIconView::setCurrentItem (nextItem);
		nextItem->setSelected (true);
		ensureItemVisible (currentItem());
	}
}



void ImageListView::first ()
{
	if(!hasImages())
	{
		mw->setEmptyImage();
		return;
	}
	FileIconItem *item=firstItem();
	while(item && !item->isImage())
		item=item->nextItem ();
	if(!item) {mw->setEmptyImage(); return;}
	KIconView::ensureItemVisible (item);
	KIconView::setCurrentItem (item);
	KIconView::setSelected (item, true, false);
	item ->setSelected (true);
	if(dscr)slotImageInfo();

}


void ImageListView::last ()
{
	if(!hasImages())
		return;

	FileIconItem *item=lastItem();
	while(item && !item->isImage())
		item=item->prevItem ();
	if(!item) return;
	KIconView::ensureItemVisible (item);
	KIconView::setCurrentItem (item);
	KIconView::setSelected (item, true, false);
	item ->setSelected (true);
	if(dscr)slotImageInfo();
}


FileIconItem*
ImageListView::findItem (const QString& text, bool fullname)
{
	for (FileIconItem *i=firstItem(); i; i=i->nextItem())
	{
		if(fullname && QDir::convertSeparators(i->fullName())==QDir::convertSeparators(text))
			return i;
		else
		if(i->text()==text)
			return i;
	}
	return NULL;
}

void ImageListView::slotOpenWith()
{
	FileIconItem *item = currentItem ();
	if (!item)
		return;
	if(mw->fullScreen())
		mw->slotFullScreen();
	KURL::List url(item->getURL());
	KOpenWithDlg dialog(url, mw);
	if(dialog.exec()!=0)
		KRun::run(dialog.text(),
			item->getURL());
}






void
ImageListView::slotUnselectAll()
{
	clearSelection ();
}

void
ImageListView::slotInvertSelection()
{
	KIconView::invertSelection () ;
}

void
ImageListView::slotSelectAll()
{
	KIconView::selectAll(true);
}


void
ImageListView::setPreloadIm(bool prel)
{
	preload_=prel;
}
bool
ImageListView::preloadIm()
{
	return preload_;
}


bool
ImageListView::showMeta()
{
	return sMeta_;
}
void
ImageListView::setShowMeta(bool sMeta)
{
	sMeta_=sMeta;
}
bool
ImageListView::showHexa()
{
	return sHexa_;
}
void
ImageListView::setShowHexa(bool sHexa)
{
	sHexa_=sHexa;
}

bool
ImageListView::getShowMimeType()
{
	return sMimeType_;
}
void
ImageListView::setShowMimeType(bool s)
{
	sMimeType_=s;
}
bool
ImageListView::getShowSize()
{
	return sSize_;
}
void
ImageListView::setShowSize(bool s)
{
	sSize_=s;
}
bool
ImageListView::getShowDate()
{
	return sDate_;
}
void
ImageListView::setShowDate(bool s)
{
	sDate_=s;
}
bool
ImageListView::getShowDimension()
{
	return sDimension_;
}
void
ImageListView::setShowDimension(bool s)
{
	sDimension_=s;
}
bool
ImageListView::getShowCategoryInfo()
{
	return m_sCategoryInfo;
}
void
ImageListView::setShowCategoryInfo(bool s)
{
	m_sCategoryInfo=s;
}


void
ImageListView::setItemTextPos ( ItemTextPos pos )
{
	if(itemTextPos()==pos)
		return;
	if(pos==Bottom)
	{
		setGridX(gridX()-200+10);
		setWordWrapIconText (true);
}
	else
	{
		setGridX(gridX()+200-10);
		setWordWrapIconText (true);
	}
	KIconView::setItemTextPos(pos);
}


void
ImageListView::slotFilesMoveToLast()
{
	if(mw->getLastDestDir().isEmpty())
	{
		slotFilesMoveTo();
		return;
	}


	QStringList uris;
	for (FileIconItem* item = firstItem (); item != 0; item =  item->nextItem ())
		if (item->isSelected () )
			uris.append(QUriDrag::localFileToUri(item->fullName()));
	if(!uris.isEmpty())
			mw->moveFilesTo(uris, mw->getLastDestDir());
}

void
ImageListView::slotFilesMoveTo()
{
	QStringList uris;
	for (FileIconItem* item = firstItem (); item != 0; item =  item->nextItem ())
		if (item->isSelected () )
			uris.append(QUriDrag::localFileToUri(item->fullName()));

	if(!uris.isEmpty())
	{
		QString destDir=KFileDialog::getExistingDirectory(!mw->getLastDestDir().isEmpty()?mw->getLastDestDir():mw->currentURL(),
				mw, i18n("Move Selected Files To"));
		if(!destDir.isEmpty())
		{
			mw->setLastDestDir(destDir);
			mw->moveFilesTo(uris, destDir+"/");
		}
	}
}

void
ImageListView::slotFilesCopyToLast()
{
	if(mw->getLastDestDir().isEmpty())
	{
		slotFilesCopyTo();
		return;
	}


	QStringList uris;
	for (FileIconItem* item = firstItem (); item != 0; item =  item->nextItem ())
		if (item->isSelected () )
			uris.append(QUriDrag::localFileToUri(item->fullName()));
	if(!uris.isEmpty())
			mw->copyFilesTo(uris, mw->getLastDestDir());
}

void
ImageListView::slotFilesCopyTo()
{
	QStringList uris;
	for (FileIconItem* item = firstItem (); item != 0; item = item->nextItem())
	{
		if (item->isSelected () )
		{
			uris.append(QUriDrag::localFileToUri(item->fullName()));
		}
	}
	if(!uris.isEmpty())
	{
		QString destDir=KFileDialog::getExistingDirectory(!mw->getLastDestDir().isEmpty()?mw->getLastDestDir():mw->currentURL(),
					mw, i18n("Copy Selected Files To"));
		if(!destDir.isEmpty())
		{
			mw->setLastDestDir(destDir);
			mw->copyFilesTo(uris, destDir+"/");
		}
	}
}

void
ImageListView::highlight(QIconViewItem *item)
{
	if (curIt) onViewport();
	if (!item || ! iconEffect->hasEffect(0,1))
	{
		if(KGlobalSettings::changeCursorOverIcon())
			KApplication::restoreOverrideCursor ();
		return;
	}
	if(KGlobalSettings::changeCursorOverIcon()) KApplication::setOverrideCursor (KCursor::handCursor());
	if(mouseIsPressed) { curIt=NULL; return; }

	curIt = (FileIconItem *)item;
	if(!curIt->isSelectable()){ curIt=NULL; return; }

	setUpdatesEnabled( false );
	delete(currentIconItem);
	currentIconItem = new QPixmap(*(curIt->pixmap()));
	currentIconItemName = curIt->fullName();
	currentIconItemHasPreview = curIt->hasPreview();
	curIt->setPixmap(iconEffect->apply(*curIt->pixmap(),0,1), curIt->hasPreview());
	setUpdatesEnabled( true );
	repaintItem(curIt);
}


void
ImageListView::onViewport()
{
	if(KGlobalSettings::changeCursorOverIcon()) KApplication::restoreOverrideCursor ();
	if(curIt==NULL) return;
	if(!curIt->isSelectable()) { curIt=NULL; return; }
	if(currentIconItemName != curIt->fullName() ) { curIt=NULL; return; }
	if(currentIconItemHasPreview != curIt->hasPreview()) { curIt=NULL; return; }

	setUpdatesEnabled( false );
	curIt->setPixmap(*currentIconItem, curIt->hasPreview());

	setUpdatesEnabled( true );
	repaintItem(curIt);
	curIt=NULL;
}

void
ImageListView::leaveEvent(QEvent *e)
{
	KIconView::leaveEvent(e);
	onViewport();
}

FileIconItem*
ImageListView::firstItem ()
{
	return (FileIconItem *) KIconView::firstItem ();
}

FileIconItem*
ImageListView::currentItem()
{
	return (FileIconItem *)KIconView::currentItem ();
}

FileIconItem*
ImageListView::lastItem()
{
	return (FileIconItem *)KIconView::lastItem ();
}


/** functions for Digikam::AlbumItemHandler*/
QStringList
ImageListView::allItems()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
	{
		if((it->getType() == QString::fromLatin1("file") || it->getType() == QString::fromLatin1("filealbum"))) itemList.append(it->text());
	}

	return itemList;
}

KURL::List
ImageListView::allItemsURL()
{
	KURL::List itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if(it->isImage()) itemList.append(it->getURL());
	return itemList;
}

QStringList
ImageListView::selectedItems()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if (it->isSelected() && (it->getType() == QString::fromLatin1("file") || it->getType() == "filealbum") )
			itemList.append(it->text());
	return itemList;
}


KURL::List
ImageListView::selectedURLs()
{
	KURL::List list;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if (it->isSelected())
			list.append(it->getURL());
	return list;
}

KURL::List
ImageListView::selectedImageURLs()
{
	KURL::List list;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if (it->isSelected() && it->isImage())
			list.append(it->getURL());
	return list;
}


QStringList
ImageListView::allItemsPath()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if((it->getType() == QString::fromLatin1("file") || it->getType() == QString::fromLatin1("filealbum")))
			itemList.append(it->fullName());
	return itemList;
}

QStringList
ImageListView::selectedItemsPath()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if (it->isSelected())
			itemList.append(it->fullName());
	return itemList;
}

void
ImageListView::refreshItems(const QStringList& )
{
	refresh();
}

void
ImageListView::reload()
{
	mw->getImageViewer()->reload();
	mw->getImageMetaInfo()->reload();
}

void
ImageListView::load(FileIconItem* item)
{
	QString itemType=QString::null, itemMimetype=QString::null;
	KURL itemKURL;
	bool itemIsImage = false;
	if(!item)
	{
		mw->getImageViewer()->loadImage(QString::null);
		mw->getImageMetaInfo()->setURL(KURL(), QString::null);
		m_OSDWidget->hide();
		return;
	}
	else
	{
		itemType = item->getType();
		itemKURL = item->getURL();
		itemIsImage = item->isImage();
		itemMimetype = item->mimetype();

	}
	if(itemIsImage || itemMimetype.left(5) == QString::fromLatin1("video"))
	{
		mw->getViewer()->openURL(itemKURL, itemMimetype);
		mw->getImageMetaInfo()->setURL(itemKURL, itemMimetype);
	}
	else
	{
		mw->getViewer()->openURL(KURL(), QString::null);
		if(itemType != "directory")
			mw->getImageMetaInfo()->setURL(itemKURL,itemMimetype);
		else
			mw->getImageMetaInfo()->setURL(KURL(), QString::null);
		m_OSDWidget->hide();
	}

	if(mw->fullScreen())
		updateOSD();
}

void
ImageListView::load(const QString& path)
{
	KURL url; url.setPath(path);
	mw->getViewer()->openURL(url, KMimeType::findByPath(path, 0, true )->name());
	mw->getImageMetaInfo()->setURL(url, KMimeType::findByPath(path, 0, true )->name());
}


void
ImageListView::setgimpPath(const QString& gimp)
{
	gimpPath_ = gimp;
}
QString
ImageListView::getgimpPath()
{
	return gimpPath_;
}

void
ImageListView::slotDisplayExifInformation()
{
#ifdef HAVE_LIBKEXIF
#if LIBKEXIF_VERSION < 020
	KExif kexif(this);
#else
	KExifDialog kexif(this);
#endif
	if (kexif.loadFile(currentItem()->fullName()))
		kexif.exec();
	else
		KMessageBox::sorry(this,
						   i18n("This item has no Exif Information."));
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */

}


KIO::Job*
ImageListView::removeThumbnails(bool allCurrentItems)
{
	KURL::List listIm = allCurrentItems?allItemsURL():selectedURLs();
	KURL::List listTh;
	KURL thURL;
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
	{
		if(QFileInfo(il->thumbnailPath((*it).path())).exists())
		{
			thURL.setPath(il->thumbnailPath((*it).path()));
			listTh.append(thURL);
		}
		if(QFileInfo(QDir::homeDirPath()+"/.showimg/cache/"+(*it).path()).exists())
		{
			thURL.setPath(QDir::homeDirPath()+"/.showimg/cache/"+(*it).path());
			listTh.append(thURL);
		}
	}
	return KIO::del( listTh );
}

void
ImageListView::forceGenerateThumbnails()
{
	KIO::Job *job = removeThumbnails();
	connect(job, SIGNAL(result( KIO::Job *)), this, SLOT(forceGenerateThumbnails__( KIO::Job *)));
}

void
ImageListView::forceGenerateThumbnails__(KIO::Job *job)
{
	if(job) if (job->error()==0)
		{
			stopLoading();
			slotLoadFirst(true);
		}
		else
			job->showErrorDialog();
}


void
ImageListView::generateEXIFThumbnails()
{
	KIO::Job *job = removeThumbnails();
	connect(job, SIGNAL(result( KIO::Job *)), this, SLOT(generateEXIFThumbnails__( KIO::Job *)));
}
void
ImageListView::generateEXIFThumbnails__(KIO::Job *job)
{
	if(job) if (job->error()==0)
		{
			stopLoading();
			slotLoadFirst(true, true);
		}
		else
			job->showErrorDialog();
}

bool
ImageListView::currentDragItemAreMovable()
{
	return m_currentDragItemAreMovable;
}

void
ImageListView::slotEXIFOrientation()
{
	ImageLoader::Orientation orient=ImageLoader::NOT_AVAILABLE;
	if(aEXIF_Orientation_normal->isChecked())
	{
		orient=ImageLoader::NORMAL;
		aEXIF_Orientation_normal->setChecked(false);
	}
	else
	if(aEXIF_Orientation_hflip->isChecked())
	{
		orient=ImageLoader::HFLIP;
		aEXIF_Orientation_hflip->setChecked(false);
	}
	else
	if(aEXIF_Orientation_vflip->isChecked())
	{
		orient=ImageLoader::VFLIP;
		aEXIF_Orientation_vflip->setChecked(false);
	}
	else
	if(aEXIF_Orientation_rot90->isChecked())
	{
		orient=ImageLoader::ROT_90;
		aEXIF_Orientation_rot90->setChecked(false);
	}
	else
	if(aEXIF_Orientation_rot270->isChecked())
	{
		orient=ImageLoader::ROT_270;
		aEXIF_Orientation_rot270->setChecked(false);
	}

	if (orient != ImageLoader::NOT_AVAILABLE)
		if (ImageLoader::setEXIFOrientation(currentItem()->fullName(), orient))
			reload();
}

void
ImageListView::toggleShowHideOSD(bool showOSD)
{
	if(showOSD)
		updateOSD();
	else
		m_OSDWidget->hide();
}


ShowimgOSD*
ImageListView::getOSD()
{
	return m_OSDWidget;
}


void
ImageListView::updateOSD()
{
	QRect toberepainted(m_OSDWidget->geometry());
	QFileInfo info(mw->getImageMetaInfo()->getURL().path());
	m_OSDWidget->setTexts(info.fileName(), info.dirPath(),
						  mw->getImageMetaInfo()->getDimensions(), mw->getImageMetaInfo()->getComments(),
					      mw->getImageMetaInfo()->getDatetime().toString(),
                          mw->getImageMetaInfo()->toString());
    m_OSDWidget->show();
    mw->getImageViewer()->repaint(toberepainted); kapp->processEvents();
}

void
ImageListView::updateDestDirTitle(const QString& dir)
{
    aCopyActions->popupMenu()->changeTitle(1, dir);
    aMoveActions->popupMenu()->changeTitle(1, dir);

   	aFilesMoveToLast->setEnabled(true);
	aFilesCopyToLast->setEnabled(true);

}


#include "imagelistview.moc"
