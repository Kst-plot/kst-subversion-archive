/***************************************************************************
                          imagelistview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

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

#include "imagelistview.h"

//-----------------------------------------------------------------------------
#define IMAGELISTVIEW_DEBUG          0
#define IMAGELISTVIEW_DEBUG_LOAD     0
//-----------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "albumimagefileiconitem.h"
#include "categorydbmanager.h"
#include "categoryimageproperties.h"
#include "describe.h"
#include "directoryview.h"
#include "dirfileiconitem.h"
#include "imagefileiconitem.h"
#include "imageloader.h"
#include "imagemetainfo.h"
#include "imageviewer.h"
#include "kexifpropsplugin.h"
#include "mainwindow.h"
#include "osd.h"
#include "showimg_common.h"
#include <showimgdb/imageentry.h>
#include "viewer.h"
#include "tools.h"

#ifndef Q_WS_WIN
#include "khexeditpropsplugin.h"
#endif

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */

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
#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kfileiconview.h>
#include <kfileitem.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <kio/job.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kopenwith.h>
#include <kpixmapio.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kpropsdlg.h>
#include <kstandarddirs.h>
#include <kurlrequesterdlg.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

#if  KDE_VERSION >= 0x30200
#include <kinputdialog.h>
#endif

// QT
#include <qdragobject.h>
#include <qevent.h>
#include <qiconview.h>
#include <qkeycode.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qmime.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextcodec.h>


//-----------------------------------------------------------------------------

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
	return 
		e->provides("application/x-qiconlist" ) ||
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
			QWidget        * a_p_parent,
			const QString&   a_name)
	:KIconView (a_p_parent, a_name.ascii()),

	m_p_curIt(NULL),
	m_sortMode(0),
	m_p_currentIconItem(NULL),
	m_p_dscr(NULL),
	m_loop(true),
	m_preload(true),
	m_random(false),
	m_isLoadingThumbnail(false),
	m_p_toolTips(NULL),
	m_mouseIsPressed(false),
	m_currentDragItemAreMovable(false)
{
	m_p_popup = new KPopupMenu (); m_p_popup->insertTitle("", 1);
	m_p_popup_openWith = new KPopupMenu ();
	m_p_popupEmpty = new KPopupMenu ();

	m_p_il = new ImageLoader (this);

	m_p_iconEffect = new KIconEffect();

	m_p_OSDWidget = new ShowimgOSD(getMainWindow()->getImageViewer());
	m_p_OSDWidget->setDuration(5*1000);
	m_p_OSDWidget->setDrawShadow(false);

	setResizeMode (Adjust);
	setWordWrapIconText(true);
	setSelectionMode (Extended);
	setItemsMovable ( true );
	setItemTextPos( Bottom );
	setSpacing(5);

	connect(this, SIGNAL(selectionChanged()),
			SLOT(selectionChanged()));
	connect(this, SIGNAL(onItem(QIconViewItem * )),
			SLOT(highlight(QIconViewItem * )));
	connect(this, SIGNAL(onViewport()),
			SLOT(onViewport()));
	connect(this, SIGNAL(contextMenuRequested(QIconViewItem *, const QPoint &)),
			SLOT(popup(QIconViewItem *, const QPoint &)));


	connect(getMainWindow(), SIGNAL(lastDestURLChanged(const KURL&)),
			this, SLOT(updateDestURLTitle(const KURL&)));
	connect(getMainWindow(), SIGNAL(toggleFullscreen(bool)), 
			SLOT(toggleShowHideOSD(bool)));
}

ImageListView::~ImageListView()
{
}


//-----------------------------------------------------------------------------

void
ImageListView::readConfig(KConfig *config)
{
	config->setGroup("Options");
	m_p_il->setStoreThumbnails(config->readBoolEntry("storeth", true));
	m_p_il->setShowFrame(config->readBoolEntry("showFrame", true));

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
		case 0  : m_p_iconSmall_action->setChecked(true); break;
		case 1  : m_p_iconMed_action->setChecked(true); break;
		case 2  : m_p_iconBig_action->setChecked(true); break;
		case 3  : m_p_iconTiny_action->setChecked(true); break;
		default : m_p_iconMed_action->setChecked(true); break;
	};
	setThumbnailSize(icSize);

	config->setGroup("Slideshow");
	setLoop(config->readBoolEntry("loop", false));

	config->setGroup("confirm");
	m_p_il->setUseEXIF(getMainWindow()->getImageViewer()->useEXIF());

	config->setGroup("Paths");
	setgimpPath(config->readPathEntry("gimpPath", "gimp-remote -n"));

	config->setGroup("OSD");
	QFont m_font(font());
	m_p_OSDWidget->initOSD(config->readBoolEntry("showOSD", true), config->readBoolEntry("OSDOnTop", false),  config->readFontEntry("OSDFont", &m_font), config->readBoolEntry("showFilename", true), config->readBoolEntry("showFullpath", false), config->readBoolEntry("showDimensions", true), config->readBoolEntry("showComments", true), config->readBoolEntry("showDatetime", true), config->readBoolEntry("showExif", false) );

}

void
ImageListView::writeConfig(KConfig *config)
{
	config->setGroup("Options");
	config->writeEntry( "storeth", m_p_il->getStoreThumbnails() );
	config->writeEntry( "showFrame", m_p_il->getShowFrame() );
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
	config->writeEntry( "gimpPath", getgimpPath() );

	config->setGroup("Icons");
	int icSize=0;
	if(m_p_iconSmall_action->isChecked())     icSize=0;
	else if (m_p_iconMed_action->isChecked()) icSize=1;
	else if (m_p_iconBig_action->isChecked()) icSize=2;
	else if (m_p_iconTiny_action->isChecked()) icSize=3;
	else icSize=1;
	config->writeEntry("size", icSize);

	config->setGroup("OSD");
	config->writeEntry("showOSD", m_p_OSDWidget->getShowOSD());
	config->writeEntry("OSDOnTop", m_p_OSDWidget->getOSDOnTop());
	config->writeEntry("OSDFont", m_p_OSDWidget->font());
	config->writeEntry("showFilename", m_p_OSDWidget->getOSDShowFilename());
	config->writeEntry("showFullpath", m_p_OSDWidget->getOSDShowFullpath());
	config->writeEntry("showDimensions", m_p_OSDWidget->getOSDShowDimensions());
	config->writeEntry("showComments", m_p_OSDWidget->getOSDShowComments());
	config->writeEntry("showDatetime", m_p_OSDWidget->getOSDShowDatetime());
	config->writeEntry("showExif", m_p_OSDWidget->getOSDShowExif());
}



//-----------------------------------------------------------------------------

void
ImageListView::initActions(KActionCollection *a_p_actionCollection)
{
	m_p_actionCollection=a_p_actionCollection;

	m_p_rename_action = new KAction(i18n("Rename File"), "item_rename", 
		KShortcut(Key_F2), this, SLOT(slotRename()),
		m_p_actionCollection , "rename");
	
	m_p_delete_action = new KAction(i18n("Delete File"), "editdelete", 
		KShortcut(SHIFT+Key_Delete), this, SLOT(slotSupprimmer()), 
		m_p_actionCollection, "editdelete");
	
	m_p_trash_action = new KAction(i18n("Move File to Trash"), "edittrash", 
		KShortcut(Key_Delete), this, SLOT(slotMoveToTrash()), 
		m_p_actionCollection, "edittrash");
	
	m_p_shred_action = new KAction(i18n("Shred File"), "editshred", 
		KShortcut(SHIFT+CTRL+Key_Delete), this, SLOT(slotShred()), 
		m_p_actionCollection, "editshred");

	m_p_fileProperties_action = new KAction(i18n("Properties"), "info", 
		0, this, SLOT(slotFileProperty()), 
		m_p_actionCollection, "Properties");
	
	m_p_categoryProperties_action = new KAction(i18n("Category Properties"), "kexi_kexi", 
		0, this, SLOT(slotCategoryProperties()), 
		m_p_actionCollection, "ImageCategoryProperties");
	
	m_p_imageInfo_action = new KAction(i18n("Image Info"), 
		0, this, SLOT(slotImageInfo()), 
		m_p_actionCollection, "Image Info");

	m_p_select_action = new KAction(i18n("Select All"), 
		KStdAccel::shortcut(KStdAccel::SelectAll) , this, SLOT(slotSelectAll()), 
		m_p_actionCollection, "SelectAll");
	
	m_p_unselectAll_action = new KAction(i18n("Unselect All"),
		0, this , SLOT(slotUnselectAll()), 
		m_p_actionCollection, "Unselect All");
	
	m_p_invertSelection_action = new KAction(i18n("Invert Selection"), 
		KShortcut(CTRL+Key_I),this , SLOT(slotInvertSelection()), 
		m_p_actionCollection, "Invert Selection");

	//------------------------------------------------------------------------------
	m_p_iconTiny_action	= new KRadioAction(i18n("Tiny Icons"), "tinythumbnails", 0,
					this, SLOT(setThumbnailSize()), m_p_actionCollection, "Tiny Icons");
	m_p_iconSmall_action	= new KRadioAction(i18n("Small Icons"), "smallthumbnails", 0,
					this, SLOT(setThumbnailSize()), m_p_actionCollection, "Small Icons");
	m_p_iconMed_action	= new KRadioAction(i18n("Medium Icons"), "medthumbnails", 0,
					this, SLOT(setThumbnailSize()), m_p_actionCollection, "Medium Icons");
	m_p_iconBig_action	= new KRadioAction(i18n("Large Icons"), "largethumbnails", 0,
					this, SLOT(setThumbnailSize()), m_p_actionCollection, "Big Icons");
	m_p_iconTiny_action->setExclusiveGroup("IconSize");
	m_p_iconSmall_action->setExclusiveGroup("IconSize");
	m_p_iconMed_action->setExclusiveGroup("IconSize");
	m_p_iconBig_action->setExclusiveGroup("IconSize");
	
	KActionMenu *actionIconSize = new KActionMenu( i18n("Icon Size"), "view_icon", 
		m_p_actionCollection, "view_icons" );
	actionIconSize->insert(m_p_iconTiny_action);
	actionIconSize->insert(m_p_iconSmall_action);
	actionIconSize->insert(m_p_iconMed_action);
	actionIconSize->insert(m_p_iconBig_action);


	//
	m_p_sortByName_action = new KRadioAction(i18n("By Name"), 
		0, this, SLOT(slotByName()), 
		m_p_actionCollection, "by name");

	m_p_sortByType_action = new KRadioAction(i18n("By Type"), 
		0, this, SLOT(slotByExtension()), 
		m_p_actionCollection, "by extension");

	m_p_sortBySize_action = new KRadioAction(i18n("By Size"), 
		0, this, SLOT(slotBySize()), 
		m_p_actionCollection, "by size");
	
	m_p_sortByDate_action = new KRadioAction(i18n("By Date"), 
		0, this, SLOT(slotByDate()), 
		m_p_actionCollection, "by date");
	
	m_p_sortByDirName_action = new KRadioAction(i18n("By Path && Name"), 
		0, this, SLOT(slotByDirName()), 
		m_p_actionCollection, "by dir name");
	
	m_p_sortByName_action->setExclusiveGroup("sort mode");
	m_p_sortByType_action->setExclusiveGroup("sort mode");
	m_p_sortBySize_action->setExclusiveGroup("sort mode");
	m_p_sortByDate_action->setExclusiveGroup("sort mode");
	m_p_sortByDirName_action->setExclusiveGroup("sort mode");
	m_p_sortByName_action->setChecked(true);

	//
	m_p_openWithGimp_action	= new KAction(i18n("Open with &Gimp"), "gimp", 
		0, this, SLOT(slotGimp()), 
		m_p_actionCollection, "Open with Gimp");
	
	m_p_editWithShowFoto_action = new KAction(i18n("Edit with &showFoto"), "showfoto", 
		KShortcut(CTRL+Key_E), this, SLOT(slotShowFoto()), 
		m_p_actionCollection, "Edit with showFoto");

	m_p_openWith_action = new KAction(i18n("&Other..."), 
		0, this, SLOT(slotOpenWith()), 
		m_p_actionCollection, "Open with");

	m_p_filesMoveTo_action = new KAction(i18n("Move File to..."), 
		0, this, SLOT(slotFilesMoveTo()),
		m_p_actionCollection , "moveFilesTo");
	
	m_p_filesCopyTo_action = new KAction(i18n("Copy File to..."), 
		0, this, SLOT(slotFilesCopyTo()),
		m_p_actionCollection , "copyFilesTo");
	
	m_p_filesMoveToLast_action = new KAction(i18n("Move File to Last Directory"), 
		KShortcut(SHIFT+Key_F12), this, SLOT(slotFilesMoveToLast()),
		m_p_actionCollection , "moveFilesToLast");
	
	m_p_filesCopyToLast_action = new KAction(i18n("Copy File to Last Directory"), 
		Key_F12, this, SLOT(slotFilesCopyToLast()),
		m_p_actionCollection , "copyFilesToLast");
	
	m_p_filesMoveToLast_action->setEnabled(false);
	m_p_filesCopyToLast_action->setEnabled(false);

#ifdef HAVE_LIBEXIV2
	m_p_displayExifInformation_action = new KAction(i18n("Exif Information"), 
		0, this, SLOT(slotDisplayExifInformation()), 
		m_p_actionCollection, "files_Display_Exif_Information");
#else
	m_p_displayExifInformation_action = NULL;
#endif /* HAVE_LIBEXIV2 */

	//
	m_p_EXIF_Orientation_normal_action = new KToggleAction(i18n("Top Left"), 
		0, this, SLOT(slotEXIFOrientation()), 
		m_p_actionCollection, "EXIF orientation normal");
	
	m_p_EXIF_Orientation_hflip_action = new KToggleAction(i18n("Top Right"), 
		0, this, SLOT(slotEXIFOrientation()), 
		m_p_actionCollection, "EXIF orientation hflip");
	
	//aEXIF_Orientation_rot180=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), m_p_actionCollection, "EXIF orientation ");
	
	m_p_EXIF_Orientation_vflip_action = new KToggleAction(i18n("Bottom Left"), 
		0, this, SLOT(slotEXIFOrientation()), 
		m_p_actionCollection, "EXIF orientation vflip");
	
	//aEXIF_Orientation_rot90hflip=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), m_p_actionCollection, "EXIF orientation ");
	
	m_p_EXIF_Orientation_rot90_action = new KToggleAction(i18n("Right Top"),
		0, this, SLOT(slotEXIFOrientation()), 
		m_p_actionCollection, "EXIF orientation rot90");
	
	//aEXIF_Orientation_rot90vflip=new KToggleAction(i18n("normal"),0, this, SLOT(slotEXIFOrientation()), m_p_actionCollection, "EXIF orientation ");
	
	m_p_EXIF_Orientation_rot270_action = new KToggleAction(i18n("Left Bottom"), 
		0, this, SLOT(slotEXIFOrientation()), 
		m_p_actionCollection, "EXIF orientation rot270");

	m_p_regenEXIFThumb_action = new KAction(i18n("(Re)generate EXIF Thumbnail"), "thumbnail", 
		0, this, SLOT(generateEXIFThumbnails()), 
		m_p_actionCollection, "Regenerate EXIF thumbnail" );

	m_p_regen_action = new KAction(i18n("Regenerate Thumbnail"), 
		0, this, SLOT(forceGenerateThumbnails()), 
		m_p_actionCollection, "Regenerate thumbnail" );

	///
	if(getMainWindow()->getImageViewer()==NULL) {MYWARNING<<"pb in imagelistview: ImageViewer is NULL!!!"<<endl; return;}
	connect(getMainWindow()->getImageViewer(), SIGNAL(askForPreviousImage()), 
		this, SLOT(previous()));
	connect(getMainWindow()->getImageViewer(), SIGNAL(askForNextImage()), 
		this, SLOT(next()));
	connect(getMainWindow()->getImageViewer(), SIGNAL(askForFirstImage()), 
		this, SLOT(first()));
	connect(getMainWindow()->getImageViewer(), SIGNAL(askForLastImage()), 
		this, SLOT(last()));
}

void
ImageListView::initMenu(KActionCollection *a_p_actionCollection)
{
	m_p_actionCollection=a_p_actionCollection;

	KActionMenu *actionSortMenu = new KActionMenu( i18n("Sort"), m_p_actionCollection, "view_sort" );
	actionSortMenu->insert(m_p_sortByName_action);
	actionSortMenu->insert(m_p_sortByType_action);
	actionSortMenu->insert(m_p_sortBySize_action);
	actionSortMenu->insert(m_p_sortByDate_action);
	actionSortMenu->insert(new KActionSeparator());
	actionSortMenu->insert(m_p_sortByDirName_action);

	m_p_actionCollection->action("editpaste")->plug(m_p_popupEmpty);

	m_p_popupEmpty->insertSeparator();
	m_p_actionCollection->action("view_icons")->plug(m_p_popupEmpty);
	actionSortMenu->plug(m_p_popupEmpty);

	m_p_popupEmpty->insertSeparator();
	m_p_select_action->plug(m_p_popupEmpty);
	m_p_unselectAll_action->plug(m_p_popupEmpty);
	m_p_invertSelection_action->plug(m_p_popupEmpty);

	//------------
	//------------
	m_p_popup_openWith = new KPopupMenu ();
	m_p_popup->insertItem(i18n("Open With"), m_p_popup_openWith);
	m_p_actionCollection->action("Edit with showFoto")->plug(m_p_popup);

	///////////
	m_p_popup->insertSeparator();
	m_p_copy_actionMenu = new KActionMenu( i18n("Copy"), 0, m_p_actionCollection, "Copy files actions" );
	m_p_copy_actionMenu->plug(m_p_popup);
	m_p_copy_actionMenu->popupMenu()->insertTitle(i18n("Copy"), 1);
	m_p_copy_actionMenu->insert(m_p_filesCopyToLast_action);
	m_p_copy_actionMenu->insert(m_p_filesCopyTo_action);


	m_p_move_actionMenu = new KActionMenu( i18n("Move"), 0, m_p_actionCollection, "Move files actions" );
	m_p_move_actionMenu->plug(m_p_popup);
	m_p_move_actionMenu->popupMenu()->insertTitle(i18n("Move"), 1);
	m_p_move_actionMenu->insert(m_p_filesMoveToLast_action);
	m_p_move_actionMenu->insert(m_p_filesMoveTo_action);

	m_p_rename_action->plug(m_p_popup);
	m_p_trash_action->plug(m_p_popup);
	m_p_delete_action->plug(m_p_popup);


	////////
	m_p_popup->insertSeparator();
	KActionMenu *aEXIF = new KActionMenu( i18n("EXIF"), 0, m_p_actionCollection, "EXIF actions" );
	aEXIF->popupMenu()->insertTitle(i18n("Exif Information"));
	
	////////
	m_p_EXIF_Orientation_actionMenu = new KActionMenu( i18n("Orientation"), "rotate", m_p_actionCollection, "EXIF orientation" );
	m_p_EXIF_Orientation_actionMenu->insert(m_p_EXIF_Orientation_normal_action);
	m_p_EXIF_Orientation_actionMenu->insert(m_p_EXIF_Orientation_hflip_action);
	m_p_EXIF_Orientation_actionMenu->insert(m_p_EXIF_Orientation_vflip_action);
	m_p_EXIF_Orientation_actionMenu->insert(m_p_EXIF_Orientation_rot90_action);
	m_p_EXIF_Orientation_actionMenu->insert(m_p_EXIF_Orientation_rot270_action);

	aEXIF->insert(m_p_EXIF_Orientation_actionMenu);

	////////
	aEXIF->insert(m_p_regenEXIFThumb_action);
	m_p_regenEXIFThumb_action->setEnabled(false);

	////////
	if(m_p_displayExifInformation_action)
	{
		aEXIF->insert(new KActionSeparator());
		m_p_displayExifInformation_action->plug(aEXIF->popupMenu());
	}

	aEXIF->plug(m_p_popup);
	m_p_regen_action->plug(m_p_popup);	m_p_regen_action->setEnabled(false);

	////////
	m_p_popup->insertSeparator();
	m_p_imageInfo_action->plug(m_p_popup);
#ifdef HAVE_KIPI
	if(getMainWindow()->getCategoryDBManager()->isConnected()) m_p_categoryProperties_action->plug(m_p_popup);
#endif
	m_p_fileProperties_action->plug(m_p_popup);
}



//-----------------------------------------------------------------------------



KPopupMenu*
ImageListView::popupOpenWith()
{
	popup(currentItem(), QPoint());
	return m_p_popup_openWith;
}

void
ImageListView::popup(QIconViewItem *a_p_item, const QPoint &)
{
	m_p_popup_openWith->clear();
	m_p_popup_openWith->disconnect();

	FileIconItem * a_p_file_icon_item = dynamic_cast<FileIconItem*>(a_p_item);
	
	if(!a_p_file_icon_item)
	{
		m_p_popup_openWith->setEnabled(false);
		return;
	}
	m_p_popup_openWith->setEnabled(true);

	if(a_p_file_icon_item->isImage())
	{
		m_p_actionCollection->action("Open with Gimp")->plug(m_p_popup_openWith);
		m_p_actionCollection->action("Edit with showFoto")->setEnabled(true);
		m_p_popup_openWith->insertSeparator ();
	}
	else
	{
		m_p_actionCollection->action("Edit with showFoto")->setEnabled(false);
	}
	//
	m_offerList = KTrader::self()->query(a_p_file_icon_item->mimetype(), "Type == 'Application'");
	for (uint i=0; i < m_offerList.count(); i++)
	{
		m_p_popup_openWith->insertItem(SmallIcon(m_offerList[i]->icon()), m_offerList[i]->name(), i+1);
	}
	if(m_offerList.size()!=0)
		m_p_popup_openWith->insertSeparator ();

	m_p_actionCollection->action("Open with")->plug(m_p_popup_openWith);
	connect(m_p_popup_openWith, SIGNAL(activated(int)),
		this, SLOT(slotRun(int)));
		
	m_mouseIsPressed=false;
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
			KMessageBox::error(getMainWindow(), 
				"<qt>"+i18n("Error while running %1.").arg((*m_offerList[id-1]).name())+"</qt>");
		}
	}
}



//-----------------------------------------------------------------------------

bool
ImageListView::doPreload()
{
	return m_preload;
}

void
ImageListView::setPreload(bool p)
{
	m_preload=p;
}

bool
ImageListView::doLoop()
{
	return m_loop;
}

void
ImageListView::setLoop(bool loop)
{
	m_loop=loop;
}

bool
ImageListView::doRandom()
{
	return m_random;
}

void
ImageListView::setRandom(bool ran)
{
	m_random=ran;
}


void
ImageListView::setShowToolTips(bool s)
{
	m_sToolTips=s;
	if(getShowToolTips() && !m_p_toolTips)
	{
		m_p_toolTips = new KToolTip(viewport(), this);
	}
	if(m_p_toolTips)
		m_p_toolTips->setShow(getShowToolTips());

}

bool
ImageListView::getShowToolTips()
{
	return m_sToolTips;
}

FileIconItem*
ImageListView::itemAt(const QPoint pos)
{
	return dynamic_cast<FileIconItem*>(KIconView::findItem(pos));
}


void
ImageListView::setThumbnailSize(bool refresh)
{
	getMainWindow()->slotStop();
	if(m_p_iconSmall_action->isChecked())
	{
		setThumbnailSize(0, refresh);
	}
	else
	if(m_p_iconMed_action->isChecked ())
	{
		setThumbnailSize(1, refresh);
	}
	else
	if(m_p_iconBig_action->isChecked ())
	{
		setThumbnailSize(2, refresh);
	}
	else
	if(m_p_iconTiny_action->isChecked())
	{
		setThumbnailSize(3, refresh);
	}
	else
	if(refresh) getMainWindow()->slotRefresh ();
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
	m_p_currentIconSize = new QSize(newSize);
	m_p_il->setThumbnailSize(*m_p_currentIconSize);

// 	setUpdatesEnabled(false);

	for (FileIconItem *i = firstItem(); i; i=i->nextItem() )
	{
		i->setHasPreview (false);
		if(!getMainWindow()->preview())
			i->setPixmap(i->fileInfo()->pixmap(
				getCurrentIconSize().width() / 2));
		else
			i->calcRect();
	}

// 	setUpdatesEnabled(true);
	slotUpdate();
	arrangeItemsInGrid();
	ensureItemVisible(currentItem());
}


QSize
ImageListView::getCurrentIconSize()
{
	if(!getMainWindow()->preview())
		return (*m_p_currentIconSize/2);
	else
		return *m_p_currentIconSize;
}

void
ImageListView::slotUpdate ()
{
	KIconView::slotUpdate () ;
}

void
ImageListView::slotResetThumbnail()
{
 	stopLoading();
// 	setUpdatesEnabled(false);
	for (FileIconItem * item = firstItem (); item != 0; item = item->nextItem ())
	{
		item->setPixmap(item->fileInfo()->pixmap(getCurrentIconSize().width()/2));
	}
// 	setUpdatesEnabled(true);
}



//-----------------------------------------------------------------------------

void
ImageListView::slotRename()
{
#if KDE_IS_VERSION(3,3,0)
	FileIconItem *item = currentItem();
	if(item)
	{
		bool ok;
		QString oldName = item->text(0);
		QString oldFullPath = item->fullName();
		const QString newName(KInputDialog::getText(i18n("Rename %1:").arg(oldName),
					i18n("Enter new name:"),
					oldName,
					 &ok, getMainWindow()->getImageViewer()).stripWhiteSpace());
		if(ok && !newName.isEmpty())
		{
			item->setText(newName);
			emit fileIconRenamed(oldFullPath, item->fullName());
		}
	}
#endif
}


void
ImageListView::refresh ()
{
	getMainWindow()->slotRefresh();
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
// 		kapp->processEvents();
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
	getMainWindow()->setHasImageSelected(nbrSel>0);
	if(nbrSel>1)
		emit sigSetMessage(i18n("%n selected file", "%n selected files", nbrSel));
	else
		emit sigSetMessage(i18n("Ready"));

#ifdef HAVE_KIPI
	if(getMainWindow()->pluginManager()) getMainWindow()->pluginManager()->selectionChanged(hasImageSelected());
#endif /* HAVE_KIPI */

	bool isReadonly = true;
	bool isFile = true;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
	{
		if( !i->isSelected()) 
			continue;
		if( !(
			isReadonly = 
				(isReadonly &&
				 ! i->isMovable()
				)
			)
		)
			break;
			
		if( !(
			isFile = 
				(isFile &&
				i->getProtocol() == QString::fromLatin1("file")
				)
			)
		) 
			break;
	}
	m_p_rename_action->setEnabled(!isReadonly);
	m_p_delete_action->setEnabled(!isReadonly);
	m_p_filesMoveTo_action->setEnabled(!isReadonly);
	m_p_filesMoveToLast_action->setEnabled(!isReadonly && !getMainWindow()->getLastDestURL().isEmpty());
	m_p_filesCopyTo_action->setEnabled(nbrSel>0);
	m_p_shred_action->setEnabled(!isReadonly);
	m_p_trash_action->setEnabled(!isReadonly);
	m_p_editWithShowFoto_action->setEnabled(!isReadonly);
	m_p_regen_action->setEnabled(!isReadonly);
	m_p_EXIF_Orientation_actionMenu->setEnabled(!isReadonly);
	m_p_categoryProperties_action->setEnabled(isFile);
}

void
ImageListView::slotLoadFirst(bool force, bool forceEXIF)
{
	if (getMainWindow()->preview() && count()>0 && !m_isLoadingThumbnail)
	{
		stopLoading();

		getMainWindow()->slotReset ();
		m_p_imageLoading = firstItem();
		if(count()==1)
		{
			if(!m_p_imageLoading->hasPreview() && m_p_imageLoading->isImage())
			{
				m_isLoadingThumbnail=true;
				QFileInfo fi(m_p_imageLoading->fullName());
				m_p_il->loadMiniImage (fi, true, force, forceEXIF);
				stopLoading ();
			}
			else m_p_imageLoading=NULL;
		}
		else
		{
			int nbr=0;
			while(   m_p_imageLoading &&
				 
				( 
					!m_p_imageLoading->getURL().isLocalFile() ||
					m_p_imageLoading->hasPreview() || 
					!m_p_imageLoading->isImage()   ||
				        (forceEXIF && ! m_p_imageLoading->isSelected())
				)
			)
			{
				if(m_p_imageLoading->isImage()) nbr++;
				m_p_imageLoading=m_p_imageLoading->nextItem ();

			}
			getMainWindow()->slotPreviewDone (nbr);
			if(m_p_imageLoading)
			{
				m_p_actionCollection->action("Regenerate thumbnail")->setEnabled(false);
				m_p_actionCollection->action("Regenerate EXIF thumbnail")->setEnabled(false);
				m_isLoadingThumbnail=true;
				slotLoadNext (force, forceEXIF);
			}
		}
		if(!m_p_imageLoading) getMainWindow()->slotDone();
	}
}

void
ImageListView::slotLoadFirst(FileIconItem *item)
{
	if (getMainWindow()->preview())
	{

		getMainWindow()->slotReset (false);
		m_p_imageLoading = item;
		if(m_p_imageLoading)
		{
			m_isLoadingThumbnail=true;
			slotLoadNext ();
		}
		else
			getMainWindow()->slotDone ();
	}
}

void
ImageListView::slotLoadNext (bool force, bool forceEXIF)
{
	if(!m_isLoadingThumbnail) return;
	if(m_p_imageLoading)
	{
		QString ext = QFileInfo(m_p_imageLoading->fullName()).extension(true).lower();
		while(! m_p_imageLoading->isImage() ||
		      !m_p_imageLoading->getURL().isLocalFile() 
			|| m_p_imageLoading->hasPreview()
			|| ext == QString::fromLatin1("psd") || ext == QString::fromLatin1("svgz")|| ext == QString::fromLatin1("svg")
			|| forceEXIF && ! m_p_imageLoading->isSelected())
		{
			m_p_imageLoading = m_p_imageLoading->nextItem ();
			if(!m_p_imageLoading)break;
			ext = QFileInfo(m_p_imageLoading->fullName()).extension(true).lower();
		}
	}
	if(m_p_imageLoading)
	{
		QFileInfo fi(m_p_imageLoading->fullName());
		m_p_il->loadMiniImage (fi, true, force, forceEXIF);
	}
	else
	{
		stopLoading();
	}
}


void
ImageListView::slotSetPixmap (const QPixmap& pm, const QFileInfo& imgFile, bool success, bool force, bool forceEXIF)
{
	if(!m_isLoadingThumbnail) return;
	m_nbrTh++;
	if(m_p_imageLoading)
	{
		if(imgFile.absFilePath() != m_p_imageLoading->fullName())
			m_p_imageLoading = findItem (imgFile.absFilePath(), true);
		if(m_p_imageLoading)
		{
			m_p_imageLoading->setPixmap (pm, success);
			if((force || forceEXIF) && m_p_imageLoading->isSelected()) reload();
		}
		getMainWindow()->slotPreviewDone ();
		//repaint(m_p_imageLoading);
		//FIXME not update all the time !
		//kapp->processEvents();
		if(m_p_imageLoading)
			m_p_imageLoading = m_p_imageLoading->nextItem ();
	}
	if (m_p_imageLoading)
		slotLoadNext (force, forceEXIF);
	else
		stopLoading ();
}

void
ImageListView::stopLoading ()
{
	m_p_il->stopLoading (true);
	m_p_imageLoading=NULL;
	m_isLoadingThumbnail=false;
	getMainWindow()->slotDone();
	m_nbrTh=0;
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
			item=dynamic_cast<FileIconItem *>(KIconView::findItem((const QPoint&)QPoint(x,y)));
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
		while(item && ! (item->isImage() || Tools::isVideo(item->getURL(), item->mimetype())) )
			item=item->nextItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		//KIconView::setSelected (item, true, false);
	 		item ->setSelected (true);
			if(m_p_dscr) slotImageInfo();

			return;
		}
	}
	if(doLoop())
	{
		first();
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
			item=dynamic_cast<FileIconItem *>(KIconView::findItem((const QPoint&)QPoint(x,y)));
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
		while(item && ! (item->isImage() || Tools::isVideo(item->getURL(), item->mimetype())) )
			item=item->prevItem ();
		if(item)
		{
	 		KIconView::ensureItemVisible (item);
	 		KIconView::setCurrentItem (item);
	 		KIconView::setSelected (item, true, false);
	 		item ->setSelected (true);
			if(m_p_dscr)slotImageInfo();
		}
		else if(doLoop()) last();
	}
	else if(doLoop()) last();
}

void
ImageListView::contentsMousePressEvent (QMouseEvent * e)
{
	KIconView::contentsMousePressEvent (e);
	m_mouseIsPressed=true;

	if (e->button () == RightButton)
	{
		int nbS = countSelected();
		if(nbS != 0)
		{
			//FileIconItem *si = firstSelected();
			//si->setSelected (true);
			if(nbS == 1)
				m_p_popup->changeTitle(1, currentItem()->iconPixmap(), currentItem()->text());
			else
				m_p_popup->changeTitle(1, SmallIcon("editcopy"), i18n("%1 selected files").arg(nbS));

			popup(currentItem(), e->globalPos ());
			m_p_popup->exec(e->globalPos ());
		}
		else
			m_p_popupEmpty->exec (e->globalPos ());
	}
}

void
ImageListView::contentsMouseReleaseEvent (QMouseEvent * e)
{
	m_mouseIsPressed=false;
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
	FileIconItem *l_p_selected_item = firstSelected();
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
			if(l_p_selected_item)
			{
				l_p_selected_item->setSelected (true);
				if(m_p_dscr) slotImageInfo();
			}
		}
		else
		{
			if(!l_p_selected_item) return;
 			if(dynamic_cast<DirFileIconItem*>(l_p_selected_item) )
			{
				m_p_curIt=NULL;
				KApplication::restoreOverrideCursor ();
				getMainWindow()->openURL(l_p_selected_item->getURL());
			}
			else
			{
				l_p_selected_item->setSelected (true);
			}
		}
	}
	m_mouseIsPressed=false;
}

void
ImageListView::contentsMouseDoubleClickEvent ( QMouseEvent * e)
{
	if (!currentItem () || e->button () == RightButton) return;
	if( currentItem()->isImage())
	{
		getMainWindow()->slotFullScreen ();
		currentItem ()->setSelected (true);
	}
	else
	if(dynamic_cast<DirFileIconItem*>(currentItem()) )
	{
		m_p_curIt=NULL;
		KApplication::restoreOverrideCursor ();
		getMainWindow()->openURL(currentItem()->getURL());
	}
	else
		KRun::runURL(currentItem()->getURL(), currentItem()->fileInfo()->mimetype());
}

//----------------------------------------------------------------------------

QString
ImageListView::getCurrentKey()
{
	switch(m_sortMode)
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
ImageListView::slotByName()
{
	m_sortMode=0;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		i->setKey("name");
	sort();
}

void
ImageListView::slotByDirName()
{
	m_sortMode=4;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		i->setKey("dirname");
	sort();
}


void
ImageListView::slotByExtension()
{
	m_sortMode=1;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		i->setKey("type");
	sort();
}


void
ImageListView::slotBySize()
{
	m_sortMode=2;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		i->setKey("size");
	sort();
}


void
ImageListView::slotByDate()
{
	m_sortMode=3;
	for (FileIconItem *i=firstItem(); i; i = i->nextItem() )
		i->setKey("date");
	sort();
}

//----------------------------------------------------------------------------

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



//----------------------------------------------------------------------------

void
ImageListView::slotWallpaper ()
{
	FileIconItem *item = currentItem ();
	if (!item) return;
	currentItem ()->setWallpaper ();
}


//----------------------------------------------------------------------------

void
ImageListView::slotCategoryProperties()
{
#ifdef WANT_LIBKEXIDB
	if(!currentItem()) return;
	KApplication::setOverrideCursor (waitCursor); // this might take time
	getMainWindow()->setEnabled(false);

	QStringList image_path_list = selectedItemsPath();
	QPtrList<ImageEntry> imageEntryList = getMainWindow()->getCategoryDBManager()->getImageEntries(image_path_list);
	getMainWindow()->setEnabled(true);
	CategoriesImageProperty catimgprop(this, getMainWindow()->getCategoryDBManager(), imageEntryList, image_path_list.count());
	KApplication::restoreOverrideCursor ();

	////////
	if(!image_path_list.isEmpty() && catimgprop.exec())
	{
		getMainWindow()->setEnabled(false);
		KApplication::setOverrideCursor (waitCursor); // this might take time
		getMainWindow()->getCategoryDBManager()->updateImageInformations
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
		getMainWindow()->getCategoryDBManager()->addImageInformations
		(
			image_path_list,

			catimgprop.getComment(),
			catimgprop.getNote(),
			catimgprop.getDate_begin(),
			catimgprop.getDate_end(),
			catimgprop.getAddedCategories()
		);

		getMainWindow()->setEnabled(true);
		KApplication::restoreOverrideCursor ();
	}
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
	KPropertiesDialog l_prop( itemList,
		getMainWindow()->getImageViewer(), "KPropertiesDialog",
		true, false);
	//
	if(itemList.count()==1)
	{
		KEXIFPropsPlugin * exifProp=NULL;
		if(
			showMeta() && 
			Tools::isJPEG(currentItem()->fullName())
		)
		{
			exifProp = new KEXIFPropsPlugin(&l_prop, currentItem()->fullName());
			l_prop.insertPlugin(exifProp);
		}
		
		//
		if(currentItem()->mimetype() != "inode/directory")
		{
			QFile::Offset big = 0x501400;// 5MB
			QFile qfile( currentItem()->fullName() );
			if(showHexa() && qfile.size() < big)
			{
#ifndef Q_WS_WIN
				KHexeditPropsPlugin *hexeditProp = new KHexeditPropsPlugin(&l_prop, currentItem()->fullName());
				l_prop.insertPlugin(hexeditProp);
#endif
			}
		}
	}
	//
	KApplication::restoreOverrideCursor ();
	if(l_prop.exec())
	{
		//prop->applyChanges();
	}
}



void
ImageListView::slotImageInfo()
{
 	if(!currentItem()) return;

	KApplication::setOverrideCursor (waitCursor);
	if(!m_p_dscr)
	{
		m_p_dscr = new Describe(getMainWindow()->getImageViewer(), currentItem()->fullName(), "ImageInfo");
		connect(m_p_dscr, SIGNAL(close()), SLOT(slotDescribeClose()));
	}
	else
		m_p_dscr->setImageFile(currentItem()->fullName());
	KApplication::restoreOverrideCursor ();
	m_p_dscr->show();
}

void
ImageListView::slotDescribeClose()
{
	delete(m_p_dscr); m_p_dscr=NULL;
}


//----------------------------------------------------------------------------

void
ImageListView::slotShowFoto()
{
		QString l_exe_path = KStandardDirs::findExe("showfoto");
		if ( 
			l_exe_path.isNull() ||
			KRun::run(l_exe_path, selectedImageURLs(), "showfoto", "showfoto") == 0
		)
		{
			KMessageBox::error(getMainWindow(), "<qt>"+i18n("Error while running showFoto.<br>Please check \"showfoto\" on your system.")+"</qt>");
		}
}

void
ImageListView::slotGimp ()
{
	if (KRun::run(getgimpPath(), selectedImageURLs(), "gimp", "gimp") == 0)
	{
		KMessageBox::error(this, "<qt>"+i18n("Error while running Gimp.<br>Please check \"gimp-remote\" on your system.")+"</qt>");
	}
}


void
ImageListView::slotEndGimp (KProcess *proc)
{
}




//----------------------------------------------------------------------------

void
ImageListView::slotSupprimmer ()
{
	KURL::List l_list = selectedURLs();
	if(!l_list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(getMainWindow(), 
			KonqOperations::DEL, l_list);
		emit fileIconsDeleted(l_list);
#endif
	}
	else
	{
		KMessageBox::information(getMainWindow(),
			i18n("Nothing to delete."));
	}

}


void
ImageListView::deletionDone( KIO::Job *job)
{
}


void
ImageListView::slotMoveToTrash()
{
	KURL::List l_list = selectedURLs();
	if(!l_list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(getMainWindow(), 
			KonqOperations::TRASH, l_list);
		emit fileIconsDeleted(l_list);
#endif
	}
	else
	{
		KMessageBox::information(getMainWindow(),
			i18n("Nothing to move to trash."));
	}
}


void
ImageListView::slotShred()
{
	KURL::List l_list = selectedURLs();
	if(!l_list.empty())
	{
#ifndef Q_WS_WIN
		KonqOperations::del(getMainWindow(),
			 KonqOperations::SHRED, l_list);
#endif
		emit fileIconsDeleted(l_list);
	}
	else
	{
		KMessageBox::information(getMainWindow(),
			i18n("Nothing to move to shred."));
	}
}




//----------------------------------------------------------------------------

void ImageListView::first ()
{
	if(!hasImages())
	{
		getMainWindow()->setEmptyImage();
		return;
	}
	
	
	FileIconItem *item=firstItem();
	while(item && !item->isImage())
		item=item->nextItem ();	
	if(!item) 
	{
		getMainWindow()->setEmptyImage(); 
	}
	else
	{
		KIconView::ensureItemVisible (item);
		KIconView::setCurrentItem (item);
		KIconView::setSelected (item, true, false);
		item ->setSelected (true);
		if(m_p_dscr)
			slotImageInfo();
	}
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
	if(m_p_dscr)
		slotImageInfo();
}


FileIconItem*
ImageListView::findItem (const QString& a_text, bool a_is_fullname)
{
	for (FileIconItem *i=firstItem(); i; i=i->nextItem())
	{
		if(
			a_is_fullname && 
			QDir::convertSeparators(i->fullName()) == QDir::convertSeparators(a_text)
		)
			return i;
		else
		if(i->text()==a_text)
			return i;
	}
	return NULL;
}

void ImageListView::slotOpenWith()
{
	FileIconItem *item = currentItem ();
	if (!item)
		return;
	if(getMainWindow()->fullScreen())
		getMainWindow()->slotFullScreen();
	KURL::List url(item->getURL());
	KOpenWithDlg dialog(url, getMainWindow());
	if(dialog.exec()!=0)
		KRun::run(dialog.text(),
			item->getURL());
}



//----------------------------------------------------------------------------





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
	m_preload=prel;
}
bool
ImageListView::preloadIm()
{
	return m_preload;
}


bool
ImageListView::showMeta()
{
	return m_sMeta;
}
void
ImageListView::setShowMeta(bool sMeta)
{
	m_sMeta=sMeta;
}
bool
ImageListView::showHexa()
{
	return m_sHexa;
}
void
ImageListView::setShowHexa(bool sHexa)
{
	m_sHexa=sHexa;
}

bool
ImageListView::getShowMimeType()
{
	return m_sMimeType;
}
void
ImageListView::setShowMimeType(bool s)
{
	m_sMimeType=s;
}
bool
ImageListView::getShowSize()
{
	return m_sSize;
}
void
ImageListView::setShowSize(bool s)
{
	m_sSize=s;
}
bool
ImageListView::getShowDate()
{
	return m_sDate;
}
void
ImageListView::setShowDate(bool s)
{
	m_sDate=s;
}
bool
ImageListView::getShowDimension()
{
	return m_sDimension;
}
void
ImageListView::setShowDimension(bool s)
{
	m_sDimension=s;
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



//----------------------------------------------------------------------------

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


//----------------------------------------------------------------------------


void
ImageListView::slotFilesMoveToLast()
{
	if(getMainWindow()->getLastDestURL().isEmpty())
	{
		slotFilesMoveTo();
		return;
	}
	getMainWindow()->moveFilesTo(selectedURLs(), getMainWindow()->getLastDestURL());
}

void
ImageListView::slotFilesMoveTo()
{
	KURL::List l_list = selectedURLs();
	if(!l_list.isEmpty())
	{
		QString destDir = 
			KFileDialog::getExistingDirectory(
				!getMainWindow()->getLastDestURL().isEmpty()
					?getMainWindow()->getLastDestURL().url()
					:getMainWindow()->getCurrentURL().url(),
				getMainWindow(), 
				i18n("Move Selected Files To")
			);
		if(!destDir.isEmpty())
		{
			KURL l_url;
				l_url.setPath(destDir);
				getMainWindow()->setLastDestURL(l_url);
			getMainWindow()->moveFilesTo(l_list, l_url);
		}
	}
}

void
ImageListView::slotFilesCopyToLast()
{
	if(getMainWindow()->getLastDestURL().isEmpty())
	{
		slotFilesCopyTo();
		return;
	}
	getMainWindow()->copyFilesTo(selectedURLs(), getMainWindow()->getLastDestURL());
}

void
ImageListView::slotFilesCopyTo()
{
	KURL::List l_list = selectedURLs();
	if(!l_list.isEmpty())
	{
		QString destDir = 
			KFileDialog::getExistingDirectory(
				!getMainWindow()->getLastDestURL().isEmpty()
					?getMainWindow()->getLastDestURL().url()
					:getMainWindow()->getCurrentURL().url(),
				getMainWindow(), 
				i18n("Copy Selected Files To")
			);
		if(!destDir.isEmpty())
		{
			KURL l_url;
				l_url.setPath(destDir);
				getMainWindow()->setLastDestURL(l_url);
			getMainWindow()->copyFilesTo(l_list, l_url);
		}
	}
}

//-----------------------------------------------------------------------------

void
ImageListView::highlight(QIconViewItem *item)
{
	if (m_p_curIt) onViewport();
	if (!item || ! m_p_iconEffect->hasEffect(0,1))
	{
		if(KGlobalSettings::changeCursorOverIcon())
			KApplication::restoreOverrideCursor ();
		return;
	}
	if(KGlobalSettings::changeCursorOverIcon()) KApplication::setOverrideCursor (KCursor::handCursor());
	if(m_mouseIsPressed) { m_p_curIt=NULL; return; }

	m_p_curIt = dynamic_cast<FileIconItem *>(item);
	if(!m_p_curIt->isSelectable()){ m_p_curIt=NULL; return; }

	setUpdatesEnabled( false );
	delete(m_p_currentIconItem);
	m_p_currentIconItem = new QPixmap(*(m_p_curIt->pixmap()));
	m_currentIconItemName = m_p_curIt->fullName();
	m_currentIconItemHasPreview = m_p_curIt->hasPreview();
	m_p_curIt->setPixmap(m_p_iconEffect->apply(*m_p_curIt->pixmap(),0,1), m_p_curIt->hasPreview());
	setUpdatesEnabled( true );
	repaintItem(m_p_curIt);
}


void
ImageListView::onViewport()
{
	if(KGlobalSettings::changeCursorOverIcon()) KApplication::restoreOverrideCursor ();
	if(m_p_curIt==NULL) return;
	if(!m_p_curIt->isSelectable()) { m_p_curIt=NULL; return; }
	if(m_currentIconItemName != m_p_curIt->fullName() ) { m_p_curIt=NULL; return; }
	if(m_currentIconItemHasPreview != m_p_curIt->hasPreview()) { m_p_curIt=NULL; return; }

	setUpdatesEnabled( false );
	m_p_curIt->setPixmap(*m_p_currentIconItem, m_p_curIt->hasPreview());
	setUpdatesEnabled( true );

	repaintItem(m_p_curIt);
	m_p_curIt=NULL;
}

void
ImageListView::leaveEvent(QEvent *e)
{
	KIconView::leaveEvent(e);
	onViewport();
}


//-----------------------------------------------------------------------------

FileIconItem*
ImageListView::firstItem ()
{
	return dynamic_cast<FileIconItem *>(KIconView::firstItem ());
}

FileIconItem*
ImageListView::currentItem()
{
	return dynamic_cast<FileIconItem *>(KIconView::currentItem ());
}

FileIconItem*
ImageListView::lastItem()
{
	return dynamic_cast<FileIconItem *>(KIconView::lastItem ());
}


/** functions for Digikam::AlbumItemHandler*/
QStringList
ImageListView::allItems()
{
	QStringList itemList;
	for (FileIconItem *it = firstItem(); it; it = it->nextItem())
	{
		if(
			dynamic_cast<ImageFileIconItem*>(it) ||
			dynamic_cast<AlbumImageFileIconItem*>(it)
		)
		itemList.append(it->text());
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
		if (
			it->isSelected() &&
 			(
			dynamic_cast<ImageFileIconItem*>(it) ||
			dynamic_cast<AlbumImageFileIconItem*>(it)
			)
		)
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
		if (
			dynamic_cast<ImageFileIconItem*>(it) ||
			dynamic_cast<AlbumImageFileIconItem*>(it)
		)
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


//-----------------------------------------------------------------------------

void
ImageListView::refreshItems(const QStringList& )
{
	refresh();
}

void
ImageListView::reload()
{
	getMainWindow()->getImageViewer()->reload();
	getMainWindow()->getImageMetaInfo()->reload();
}

void
ImageListView::load(FileIconItem* a_p_item)
{
#if IMAGELISTVIEW_DEBUG_LOAD > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	if(!a_p_item)
	{
#if IMAGELISTVIEW_DEBUG_LOAD > 1
		MYDEBUG<<"!a_p_item"<<endl;
#endif
		getMainWindow()->getImageViewer()->openURL(QString::null);
		getMainWindow()->getImageMetaInfo()->setURL(KURL(), QString::null);
		m_p_OSDWidget->hide();
	}
	else
	{
		QString
			itemType=QString::null,
			itemMimetype=QString::null;
		KURL itemKURL;
		bool itemIsImage = false;

		itemKURL = a_p_item->getURL();
		itemIsImage = a_p_item->isImage();
		itemMimetype = a_p_item->mimetype();

		if(
			itemIsImage ||
			Tools::isVideo( itemKURL.path())
		)
		{
#if IMAGELISTVIEW_DEBUG_LOAD > 1
			MYDEBUG<<"itemIsImage || ListItemView::isVideo( itemKURL)"<<endl;
#endif
			getMainWindow()->getViewer()->openURL(itemKURL, itemMimetype);
			getMainWindow()->getImageMetaInfo()->setURL(itemKURL, itemMimetype);
		}
		else
		{
#if IMAGELISTVIEW_DEBUG_LOAD > 1
			MYDEBUG<<"! itemIsImage || ListItemView::isVideo( itemKURL)"<<endl;
#endif
			getMainWindow()->getViewer()->openURL(KURL(), QString::null);
	// 		if(itemType != "directory")
			if(!dynamic_cast<DirFileIconItem*>(a_p_item))
				getMainWindow()->getImageMetaInfo()->setURL(itemKURL,itemMimetype);
			else
				getMainWindow()->getImageMetaInfo()->setURL(KURL(), QString::null);
			m_p_OSDWidget->hide();
		}

		if(getMainWindow()->fullScreen())
			updateOSD();
	}

#if IMAGELISTVIEW_DEBUG_LOAD > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void
ImageListView::load(const QString& path)
{
#if IMAGELISTVIEW_DEBUG_LOAD > 0
	MYDEBUG<<"BEGIN "<<path<<endl;
#endif
	KURL url;
	url.setPath(path);
	getMainWindow()->getViewer()->openURL(url, KMimeType::findByPath(path, 0, true )->name());
	getMainWindow()->getImageMetaInfo()->setURL(url, KMimeType::findByPath(path, 0, true )->name());

#if IMAGELISTVIEW_DEBUG_LOAD > 0
	MYDEBUG<<"END "<<path<<endl;
#endif
}



//-----------------------------------------------------------------------------

void
ImageListView::setgimpPath(const QString& gimp)
{
	m_gimpPath = gimp;
}
QString
ImageListView::getgimpPath()
{
	return m_gimpPath;
}


//-----------------------------------------------------------------------------

void
ImageListView::slotDisplayExifInformation()
{
#ifdef HAVE_LIBEXIV2
/*
	KExifDialog kexif(this);
	if (kexif.loadFile(currentItem()->fullName()))
		kexif.exec();
	else
		KMessageBox::sorry(getMainWindow(),
						   i18n("This item has no Exif Information."));
*/
#endif /* HAVE_LIBEXIV2 */

}



//-----------------------------------------------------------------------------

KIO::Job*
ImageListView::removeThumbnails(bool allCurrentItems)
{
	KURL::List listIm = allCurrentItems?allItemsURL():selectedURLs();
	KURL::List listTh;
	KURL thURL;
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
	{
		if(QFileInfo(m_p_il->thumbnailPath((*it).path())).exists())
		{
			thURL.setPath(m_p_il->thumbnailPath((*it).path()));
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
	connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(forceGenerateThumbnails__( KIO::Job * )));
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
	connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(generateEXIFThumbnails__( KIO::Job * )));
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
	if(m_p_EXIF_Orientation_normal_action->isChecked())
	{
		orient=ImageLoader::NORMAL;
		m_p_EXIF_Orientation_normal_action->setChecked(false);
	}
	else
	if(m_p_EXIF_Orientation_hflip_action->isChecked())
	{
		orient=ImageLoader::HFLIP;
		m_p_EXIF_Orientation_hflip_action->setChecked(false);
	}
	else
	if(m_p_EXIF_Orientation_vflip_action->isChecked())
	{
		orient=ImageLoader::VFLIP;
		m_p_EXIF_Orientation_vflip_action->setChecked(false);
	}
	else
	if(m_p_EXIF_Orientation_rot90_action->isChecked())
	{
		orient=ImageLoader::ROT_90;
		m_p_EXIF_Orientation_rot90_action->setChecked(false);
	}
	else
	if(m_p_EXIF_Orientation_rot270_action->isChecked())
	{
		orient=ImageLoader::ROT_270;
		m_p_EXIF_Orientation_rot270_action->setChecked(false);
	}

	if (orient != ImageLoader::NOT_AVAILABLE)
		if (ImageLoader::setEXIFOrientation(currentItem()->fullName(), orient))
			reload();
}


//-----------------------------------------------------------------------------

void
ImageListView::toggleShowHideOSD(bool showOSD)
{
	if(showOSD)
		updateOSD();
	else
		m_p_OSDWidget->hide();
}


ShowimgOSD*
ImageListView::getOSD()
{
	return m_p_OSDWidget;
}


void
ImageListView::updateOSD()
{
	QRect toberepainted(m_p_OSDWidget->geometry());
	QFileInfo info(getMainWindow()->getImageMetaInfo()->getURL().path());
	m_p_OSDWidget->setTexts(info.fileName(), info.dirPath(),
						  getMainWindow()->getImageMetaInfo()->getDimensions(), getMainWindow()->getImageMetaInfo()->getComments(),
					      getMainWindow()->getImageMetaInfo()->getDatetime().toString(),
                          getMainWindow()->getImageMetaInfo()->toString());
    m_p_OSDWidget->show();
    getMainWindow()->getImageViewer()->repaint(toberepainted);
//     kapp->processEvents();
}

void
ImageListView::updateDestURLTitle(const KURL& a_url)
{
	m_p_copy_actionMenu->popupMenu()->changeTitle(1, a_url.url());
	m_p_move_actionMenu->popupMenu()->changeTitle(1, a_url.url());

   	m_p_filesMoveToLast_action->setEnabled(true);
	m_p_filesCopyToLast_action->setEnabled(true);

}


//-----------------------------------------------------------------------------

MainWindow * ImageListView::getMainWindow()
{
	return &MAINWINDOW;
}

#include "imagelistview.moc"
