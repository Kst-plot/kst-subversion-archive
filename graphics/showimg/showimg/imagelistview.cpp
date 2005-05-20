/***************************************************************************
                          imagelistview.cpp  -  description
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

#include "imagelistview.h"

// Local
#include "imageloader.h"
#include "directoryview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "osd.h"

#include "kexifpropsplugin.h"
#ifndef Q_WS_WIN
#include "khexeditpropsplugin.h"
#endif
#include "qtfileicondrag.h"
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
	#warning LIBKEXIF_VERSION >= 020
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
#include <kglobalsettings.h>
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
#include <qevent.h>
#include <qmime.h>
#include <qiconview.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qpopupmenu.h>
#include <qdragobject.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qtextcodec.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

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
	if(!m_show) return;

	FileIconItem *item = imageList->itemAt(imageList->viewportToContents(pos));
	if(!item) return;
	QRect r = item->pixmapRect(FALSE);
	r.moveTopLeft(imageList->contentsToViewport(r.topLeft()));
	if ( !r.isValid() )	return;
	QString t = "<font size=\"-1\">"+item->toolTipStr()+"</font>";
	if (!t.isEmpty()) tip(r, t);
}


/////////////////////////
ImageListView::ImageListView (
			QWidget * parent,
			const QString& name,
			MainWindow * mw)
	:KIconView (parent, name.ascii()),

	curIt(NULL),
	sortMode(0),
	currentIconItem(NULL),
	dscr(NULL),
	loop_(TRUE),
	preload_(TRUE),
	random_(FALSE),
	isLoadingThumbnail(FALSE),
	toolTips(NULL),
	mouseIsPressed(FALSE),
	m_currentDragItemAreMovable(FALSE)
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
	setWordWrapIconText(TRUE);
	setSelectionMode (Extended);
	setItemsMovable ( TRUE );
	setItemTextPos( Bottom );
	setArrangement(LeftToRight);
	setSpacing(5);

	iconEffect=new KIconEffect();

	m_OSDWidget = new ShowimgOSD(mw->getImageViewer());
	m_OSDWidget->setDuration(5*1000);
	m_OSDWidget->setDrawShadow(FALSE);
	connect(mw, SIGNAL(toggleFullscreen(bool)), this, SLOT(toggleShowHideOSD(bool)));

}

ImageListView::~ImageListView()
{
}

void
ImageListView::readConfig(KConfig *config)
{
	config->setGroup("Options");
	il->setStoreThumbnails(config->readBoolEntry("storeth", TRUE));
	il->setShowFrame(config->readBoolEntry("showFrame", TRUE));

	setWordWrapIconText(config->readBoolEntry("WordWrapIconText", TRUE));
	setShowMimeType(config->readBoolEntry("ShowMimeType", FALSE));
	setShowSize(config->readBoolEntry("ShowSize", TRUE));
	setShowDate(config->readBoolEntry("ShowDate", TRUE));
	setShowDimension(config->readBoolEntry("ShowDimension", FALSE));
	setShowToolTips(config->readBoolEntry("ShowToolTips", TRUE));
	//
	setPreloadIm(config->readBoolEntry("preloadIm", TRUE));
	setShowMeta(config->readBoolEntry("showMeta", TRUE));
	setShowHexa(config->readBoolEntry("showHexa", TRUE));

	config->setGroup("Icons");
	int icSize = config->readNumEntry("size", 1);
	switch(icSize)
	{
		case 0  : aIconSmall->setChecked(true); break;
		case 1  : aIconMed->setChecked(true); break;
		case 2  : aIconBig->setChecked(true); break;
		default : aIconMed->setChecked(true); break;
	};
	setThumbnailSize(icSize);

	config->setGroup("Slideshow");
	setLoop(config->readBoolEntry("loop", FALSE));

	config->setGroup("confirm");
	il->setUseEXIF(mw->getImageViewer()->useEXIF());

	config->setGroup("Paths");
	setgimpPath(config->readPathEntry("gimpPath", "gimp-remote -n"));

	config->setGroup("OSD");
	QFont m_font(font());
	m_OSDWidget->initOSD(config->readBoolEntry("showOSD", TRUE), config->readBoolEntry("OSDOnTop", false),  config->readFontEntry("OSDFont", &m_font), config->readBoolEntry("showFilename", TRUE), config->readBoolEntry("showFullpath", FALSE), config->readBoolEntry("showDimensions", TRUE), config->readBoolEntry("showComments", TRUE), config->readBoolEntry("showDatetime", TRUE), config->readBoolEntry("showExif", FALSE) );

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
	config->writeEntry("ShowToolTips", getShowToolTips());

	config->setGroup("Slideshow");
	config->writeEntry( "loop", doLoop() );

	config->setGroup("Paths");
	config->writeEntry( "gimpPath", getgimpPath() );

	config->setGroup("Icons");
	int icSize=0;
	if(aIconSmall->isChecked())     icSize=0;
	else if (aIconMed->isChecked()) icSize=1;
	else if (aIconBig->isChecked()) icSize=2;
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
		m_popup_openWith->setEnabled(FALSE);
		return;
	}
	m_popup_openWith->setEnabled(TRUE);

	if(((FileIconItem*)item)->mimetype().left(5)=="image")
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
	mouseIsPressed=FALSE;
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
		default : size = QSize(128, 96); break;
	}
	setThumbnailSize(size, refresh);
}

void
ImageListView::setThumbnailSize(const QSize newSize, bool )
{
	currentIconSize = new QSize(newSize);
	il->setThumbnailSize(*currentIconSize);

	setUpdatesEnabled(FALSE);

	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		i->setHasPreview (FALSE);
		if(!mw->preview())
			i->setPixmap(i->fileInfo()->pixmap(
				getCurrentIconSize().width() / 2));
		else
			i->calcRect();
	}

	setUpdatesEnabled(TRUE);
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
	repaint(); repaintContents();
	KIconView::slotUpdate();
	kapp->processEvents();
}

void
ImageListView::slotResetThumbnail()
{
 	stopLoading();
	setUpdatesEnabled(FALSE);
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		item->setPixmap(item->fileInfo()->pixmap(getCurrentIconSize().width()/2));
	}
	setUpdatesEnabled(TRUE);
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
ImageListView::initActions(KActionCollection *actionCollection)
{
	aRename		=new KAction(i18n("Rename File..."), "item_rename", KShortcut(Key_F2), this, SLOT(slotRename()),actionCollection , "rename");
	aDelete		=new KAction(i18n("Delete File"), "editdelete", KShortcut(SHIFT+Key_Delete), this, SLOT(slotSupprimmer()), actionCollection, "editdelete");
	aTrash		=new KAction(i18n("Move File to Trash"), "edittrash", KShortcut(Key_Delete), this, SLOT(slotMoveToTrash()), actionCollection, "edittrash");
	aShred		=new KAction(i18n("Shred File"), "editshred", KShortcut(SHIFT+CTRL+Key_Delete), this, SLOT(slotShred()), actionCollection, "editshred");

	aFileProperties	=new KAction(i18n("Properties"), "info", 0, this, SLOT(slotFileProperty()), actionCollection, "Properties");
	aImageInfo	=new KAction(i18n("Image Info"), 0, this, SLOT(slotImageInfo()), actionCollection, "Image Info");

	aSelect		=new KAction(i18n("Select All"), KStdAccel::shortcut(KStdAccel::SelectAll) , this, SLOT(slotSelectAll()), actionCollection, "SelectAll");
	aUnselectAll	=new KAction(i18n("Unselect All"),0, this , SLOT(slotUnselectAll()), actionCollection, "Unselect All");
	aInvertSelection=new KAction(i18n("Invert Selection"), KShortcut(CTRL+Key_I),this , SLOT(slotInvertSelection()), actionCollection, "Invert Selection");

	//------------------------------------------------------------------------------
	aIconSmall	= new KRadioAction(i18n("Small Icons"), "smallthumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Small Icons");
	aIconMed	= new KRadioAction(i18n("Medium Icons"), "medthumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Medium Icons");
	aIconBig	= new KRadioAction(i18n("Large Icons"), "largethumbnails", 0,
					this, SLOT(setThumbnailSize()), actionCollection, "Big Icons");
	aIconSmall->setExclusiveGroup("IconSize");
	aIconMed->setExclusiveGroup("IconSize"); //aIconMed->setChecked(true);
	aIconBig->setExclusiveGroup("IconSize");
	KActionMenu *actionIconSize = new KActionMenu( i18n("Icon Size"), "medthumbnails", actionCollection, "view_icons" );
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
	aSortByName->setChecked(TRUE);

	//
	aOpenWithGimp	=new KAction(i18n("Open with &Gimp"), "gimp", 0, this, SLOT(slotGimp()), actionCollection, "Open with Gimp");
	aEditWithShowFoto	=new KAction(i18n("Edit with &showFoto"), "showfoto", KShortcut(CTRL+Key_E), this, SLOT(slotShowFoto()), actionCollection, "Edit with showFoto");
// 	aOpenWithKhexedit=new KAction(i18n("Open with &Khexedit"), "khexedit", 0, this, SLOT(slotKhexedit()), actionCollection, "Open with Khexedit");
	aOpenWith	=new KAction(i18n("&Other..."), 0, this, SLOT(slotOpenWith()), actionCollection, "Open with");

	//aDirCut		=new KAction(i18n("Cut"), "editcut", 0, this, SLOT(slotDirCut()),actionCollection , "editdircut");

	aFilesMoveTo	=new KAction(i18n("Move File to..."), 0, this, SLOT(slotFilesMoveTo()),actionCollection , "moveFilesTo");
	aFilesCopyTo	=new KAction(i18n("Copy File to..."), 0, this, SLOT(slotFilesCopyTo()),actionCollection , "copyFilesTo");
	aFilesMoveToLast=new KAction(i18n("Move File to Last Directory"), KShortcut(Key_Shift+Key_F12),
								this, SLOT(slotFilesMoveToLast()),actionCollection , "moveFilesToLast");
	aFilesCopyToLast=new KAction(i18n("Copy File to Last Directory"), Key_F12,
								this, SLOT(slotFilesCopyToLast()),actionCollection , "copyFilesToLast");

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
	aCopyActions->popupMenu()->insertTitle(i18n("Choose Folder"), 1);
	aCopyActions->insert(aFilesCopyToLast);
	aCopyActions->insert(aFilesCopyTo);


	aMoveActions = new KActionMenu( i18n("Move"), 0, actionCollection, "Move files actions" );
	aMoveActions->plug(m_popup);
	aMoveActions->popupMenu()->insertTitle(i18n("Choose Folder"), 1);
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

	KActionMenu *aEXIF_Orientation = new KActionMenu( i18n("Orientation"), "rotate", actionCollection, "EXIF orientation" );
	aEXIF_Orientation->insert(aEXIF_Orientation_normal);
	aEXIF_Orientation->insert(aEXIF_Orientation_hflip);
	aEXIF_Orientation->insert(aEXIF_Orientation_vflip);
	aEXIF_Orientation->insert(aEXIF_Orientation_rot90);
	aEXIF_Orientation->insert(aEXIF_Orientation_rot270);

	aEXIF->insert(aEXIF_Orientation);

	////////
	aEXIF->insert(a_regenEXIFThumb);
	a_regenEXIFThumb->setEnabled(FALSE);

	////////
	if(aDisplayExifInformation)
	{
		aEXIF->insert(new KActionSeparator());
		aDisplayExifInformation->plug(aEXIF->popupMenu());
	}

	aEXIF->plug(m_popup);
	a_regen->plug(m_popup);	a_regen->setEnabled(FALSE);

	////////
	m_popup->insertSeparator();
	aImageInfo->plug(m_popup);
	aFileProperties->plug(m_popup);
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
	 	KIconView::setSelected(currentItem(), select, FALSE);
		KIconView::ensureItemVisible (currentItem());
		kapp->processEvents();
	 	if(select) currentItem()->setSelected(TRUE);
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
	if(!hasImages()) return FALSE;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected() && i->isImage())
			return TRUE;
	}
	return FALSE;
}

bool
ImageListView::hasSelection()
{
	if(!hasImages()) return FALSE;
	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		if(i->isSelected() )
			return TRUE;
	}
	return FALSE;
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
		if(number>=2) return FALSE;

	}
	return TRUE;
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
		mw->setMessage(i18n("%n selected file", "%n selected files", nbrSel));
	else
		mw->setMessage(i18n("Ready"));

#ifdef HAVE_KIPI
	if(mw->pluginManager()) mw->pluginManager()->selectionChanged(hasImageSelected());
#endif /* HAVE_KIPI */

	bool isReadonly = TRUE;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		if( !(isReadonly = isReadonly&&!i->isMovable())) break;
	aRename->setEnabled(!isReadonly);
	aDelete->setEnabled(!isReadonly);
	aFilesMoveTo->setEnabled(!isReadonly);
	aFilesCopyTo->setEnabled(nbrSel>0);
	aShred->setEnabled(!isReadonly);
	aTrash->setEnabled(!isReadonly);
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
				isLoadingThumbnail=TRUE;
				QFileInfo *fi = new QFileInfo(imageLoading->fullName());
				il->loadMiniImage (fi, TRUE, force, forceEXIF);
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
				actionCollection->action("Regenerate thumbnail")->setEnabled(FALSE);
				actionCollection->action("Regenerate EXIF thumbnail")->setEnabled(FALSE);
				isLoadingThumbnail=TRUE;
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

		mw->slotReset (FALSE);
		imageLoading = item;
		if(imageLoading)
		{
			isLoadingThumbnail=TRUE;
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
		while(! imageLoading->isImage()
			|| imageLoading->hasPreview()
			|| QFileInfo(imageLoading->fullName()).extension(TRUE).lower()=="psd"
			|| forceEXIF && ! imageLoading->isSelected())
		{
			imageLoading = imageLoading->nextItem ();
			if(!imageLoading)break;
		}
	}
	if(imageLoading)
	{
		QFileInfo *fi = new QFileInfo(imageLoading->fullName());
		il->loadMiniImage (fi, TRUE, force, forceEXIF);
	}
	else
	{
		stopLoading();
	}
}


void
ImageListView::slotSetPixmap (const QPixmap pm, QFileInfo *imgFile, bool success, bool force, bool forceEXIF)
{
	if(!isLoadingThumbnail) return;
	nbrTh++;
	if(imageLoading)
	{
		if(imgFile->absFilePath() != imageLoading->fullName())
			imageLoading = findItem (imgFile->absFilePath(), TRUE);
		if(imageLoading)
		{
			imageLoading->setPixmap (pm, success);
			if((force || forceEXIF) && imageLoading->isSelected()) reload();
		}
		mw->slotPreviewDone ();
		repaint(imageLoading); kapp->processEvents();
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
	il->stopLoading (TRUE);
	imageLoading=NULL;
	isLoadingThumbnail=FALSE;
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
		while(item && !item->isImage())
			item=item->nextItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		KIconView::setSelected (item, TRUE, FALSE);
	 		item ->setSelected (TRUE);
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
		while(item && !item->isImage())
			item=item->prevItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		KIconView::setSelected (item, TRUE, FALSE);
	 		item ->setSelected (TRUE);
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
	mouseIsPressed=TRUE;

	if (e->button () == RightButton)
	{
		int nbS = countSelected();
		if(nbS != 0)
		{
			//FileIconItem *si = firstSelected();
			//si->setSelected (TRUE);
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
	mouseIsPressed=FALSE;
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
				si->setSelected (TRUE);
				if(dscr)slotImageInfo();
			}
		}
		else
		{
			if(!si) return;
			QString dirName=si->fullName();
			if(si->fileInfo()->mimetype().right(9)=="directory")
			{
				curIt=NULL;
				KApplication::restoreOverrideCursor ();
				mw->openDir(dirName);
			}
			else
			{
				si->setSelected (TRUE);
			}
		}
	}
	mouseIsPressed=FALSE;
}

void
ImageListView::contentsMouseDoubleClickEvent ( QMouseEvent * e)
{
	if (!currentItem () || e->button () == RightButton) return;
	if( currentItem()->isImage())
	{
		mw->slotFullScreen ();
		currentItem ()->setSelected (TRUE);
	}
	else
	if(currentItem()->fileInfo()->mimetype().right(9)=="directory")
	{
		curIt=NULL;KApplication::restoreOverrideCursor ();
		mw->openDir(QDir::cleanDirPath(((FileIconItem*)currentItem())->fullName()));
	}
	else
		KRun::runURL(currentItem()->getURL(), currentItem()->fileInfo()->mimetype());
}

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
	m_currentDragItemAreMovable = TRUE;
	for ( FileIconItem *item = firstItem(); item; item = item->nextItem() )
	{
		if ( item->isSelected() )
		{
			QIconDragItem id;

			QCString curl = item->getURL().url().utf8();
			id.setData(curl);
			drag->append( id,
				QRect(	item->pixmapRect( FALSE ).x() - orig.x(),
					item->pixmapRect( FALSE ).y() - orig.y(),
					item->pixmapRect().width(), item->pixmapRect().height() ),
				QRect(	item->textRect( FALSE ).x() - orig.x(),
					item->textRect( FALSE ).y() - orig.y(),
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
	if (!item)
		return;
	currentItem ()->setWallpaper ();
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
		TRUE, FALSE);
	//
	if(itemList.count()==1)
	{
		KEXIFPropsPlugin *exifProp=NULL;
		if(showMeta() && currentItem()->mimetype()=="image/jpeg")
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
			if (item->text (3) == "file")
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
		item->suppression(FALSE);


	if (nextItem)
	{
		KIconView::setCurrentItem (nextItem);
		nextItem->setSelected (TRUE);
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
			if (item->text (3) == "file")
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
		nextItem->setSelected (TRUE);
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
			if (item->text (3) == "file")
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
		nextItem->setSelected (TRUE);
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
	KIconView::setSelected (item, TRUE, FALSE);
	item ->setSelected (TRUE);
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
	KIconView::setSelected (item, TRUE, FALSE);
	item ->setSelected (TRUE);
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
	KIconView::selectAll(TRUE);
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


void
ImageListView::setItemTextPos ( ItemTextPos pos )
{
	if(itemTextPos()==pos)
		return;
	if(pos==Bottom)
	{
		setGridX(gridX()-200+10);
		setWordWrapIconText (TRUE);
}
	else
	{
		setGridX(gridX()+200-10);
		setWordWrapIconText (TRUE);
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
			mw->moveFilesTo(uris, mw->getLastDestDir()+"/");
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
			mw->copyFilesTo(uris, mw->getLastDestDir()+"/");
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

	setUpdatesEnabled( FALSE );
	delete(currentIconItem);
	currentIconItem = new QPixmap(*(curIt->pixmap()));
	currentIconItemName = curIt->fullName();
	currentIconItemHasPreview = curIt->hasPreview();
	curIt->setPixmap(iconEffect->apply(*curIt->pixmap(),0,1), curIt->hasPreview());
	setUpdatesEnabled( TRUE );
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

	setUpdatesEnabled( FALSE );
	curIt->setPixmap(*currentIconItem, curIt->hasPreview());

	setUpdatesEnabled( TRUE );
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
		if((it->getType() == "file" || it->getType() == "filealbum")) itemList.append(it->text());
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
		if (it->isSelected() && (it->getType() == "file" || it->getType() == "filealbum") )
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
		if((it->getType() == "file" || it->getType() == "filealbum"))
			itemList.append(it->fullName());
	return itemList;
}

QStringList
ImageListView::selectedItemsPath()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
		if (it->isSelected())
			if((it->getType() == "file" || it->getType() == "filealbum")) itemList.append(it->fullName());
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
		mw->imageMetaInfo->reload();

}

void
ImageListView::load(FileIconItem* item)
{
	if(!item)
	{
		mw->getImageViewer()->loadImage(QString::null);
		mw->imageMetaInfo->setURL(KURL(), QString::null);
		m_OSDWidget->hide();
		return;
	}
	if(item->isImage())
	{
		mw->getImageViewer()->loadImage(item->fullName(), item->index());
		mw->imageMetaInfo->setURL(item->getURL(), item->mimetype());
	}
	else
	{
		mw->getImageViewer()->loadImage(QString::null);
		if(item->getType() != "dir")
			mw->imageMetaInfo->setURL(item->getURL(), item->mimetype());
		else
			mw->imageMetaInfo->setURL(KURL(), QString::null);
		m_OSDWidget->hide();
	}

	if(mw->fullScreen())
	{
		updateOSD();
	}
}

void
ImageListView::load(const QString& path)
{
	mw->getImageViewer()->loadImage(path);
	KURL url; url.setPath(path);
	mw->imageMetaInfo->setURL(url, KMimeType::findByPath(path, 0, TRUE )->name());
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
			slotLoadFirst(TRUE);
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
			slotLoadFirst(TRUE, TRUE);
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
		aEXIF_Orientation_normal->setChecked(FALSE);
	}
	else
	if(aEXIF_Orientation_hflip->isChecked())
	{
		orient=ImageLoader::HFLIP;
		aEXIF_Orientation_hflip->setChecked(FALSE);
	}
	else
	if(aEXIF_Orientation_vflip->isChecked())
	{
		orient=ImageLoader::VFLIP;
		aEXIF_Orientation_vflip->setChecked(FALSE);
	}
	else
	if(aEXIF_Orientation_rot90->isChecked())
	{
		orient=ImageLoader::ROT_90;
		aEXIF_Orientation_rot90->setChecked(FALSE);
	}
	else
	if(aEXIF_Orientation_rot270->isChecked())
	{
		orient=ImageLoader::ROT_270;
		aEXIF_Orientation_rot270->setChecked(FALSE);
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
	QFileInfo info(mw->imageMetaInfo->getURL().path());
	m_OSDWidget->setTexts(info.fileName(), info.dirPath(),
						  mw->imageMetaInfo->getDimensions(), mw->imageMetaInfo->getComments(),
					      mw->imageMetaInfo->getDatetime().toString(),
                          mw->imageMetaInfo->toString());
    m_OSDWidget->show();
    mw->getImageViewer()->repaint(toberepainted); kapp->processEvents();
}

void
ImageListView::updateDestDirTitle(const QString& dir)
{
    aCopyActions->popupMenu()->changeTitle(1, dir);
    aMoveActions->popupMenu()->changeTitle(1, dir);
}


#include "imagelistview.moc"
