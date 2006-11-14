/*****************************************************************************
                          imagelistview.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
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

#ifndef IMAGELISTVIEW_H
#define IMAGELISTVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kfileiconview.h>
#include <ktrader.h>
#include <kurl.h>

// QT
#include <qtooltip.h>

class Describe;
class FileIconItem;
class ImageListView;
class ImageLoader;
class MainWindow;
class ShowimgOSD;

class KAction;
class KActionMenu;
class KFileViewItem;
class KIconEffect;
class KPopupMenu;
class KProcess;
class KRadioAction;
class KToggleAction;
class KToolTip;

class QFileInfo;
class QIconViewItem;
class QListViewItem;




//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class ImageListView : public KIconView
{
	Q_OBJECT
//---------------------------------------------------------------------------
public:
	ImageListView (QWidget *a_p_parent, const QString& a_name);
	virtual ~ImageListView();

	void initMenu(KActionCollection *m_p_actionCollection);
	void initActions(KActionCollection *m_p_actionCollection);

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

	virtual void setItemTextPos ( ItemTextPos pos );

	bool doPreload();
	void setPreload(bool p);

	bool doLoop();
	void setLoop(bool loop);

	bool doRandom();
	void setRandom(bool ran);

	bool showMeta();
	void setShowMeta(bool sMeta);
	bool showHexa();
	void setShowHexa(bool sHexa);
	bool getShowMimeType();
	void setShowMimeType(bool s);
	bool getShowSize();
	void setShowSize(bool s);
	bool getShowDate();
	void setShowDate(bool s);
	bool getShowDimension();
	void setShowDimension(bool s);
	bool getShowToolTips();
	void setShowToolTips(bool s);
	bool getShowCategoryInfo();
	void setShowCategoryInfo(bool s);

	void setgimpPath(const QString& gimp);
	QString getgimpPath();

	/**
		return true if it have to show ALL files
	*/
	void setPreloadIm(bool prel);
	bool preloadIm();

	void load(FileIconItem* item);
	void load(const QString& path);
	void reload();


	FileIconItem* firstItem ();
	FileIconItem* currentItem();
	FileIconItem* lastItem();
	FileIconItem* findItem (const QString& text, bool fullname=false);
	FileIconItem* itemAt(const QPoint pos);

	void refresh ();
	void stopLoading ();

	void setThumbnailSize(const QSize newSize, bool refresh=true);
	void setThumbnailSize(const int newSize, bool refresh=true);

	QString getCurrentKey();

	KPopupMenu* popupOpenWith();

	QString currentItemName();
	void setCurrentItemName(const QString& itemName, bool select=true);
	bool hasSelection();
	bool hasImages();
	bool hasImageSelected();
	bool hasOnlyOneImageSelected();
	int countSelected();
	FileIconItem* firstSelected();

	QSize getCurrentIconSize();

	QStringList allItems();
	KURL::List allItemsURL();
	QStringList selectedItems();
	KURL::List selectedURLs();
	KURL::List selectedImageURLs();
	QStringList allItemsPath();
	QStringList selectedItemsPath();
	void refreshItems(const QStringList& itemList);

	bool currentDragItemAreMovable();

	ShowimgOSD* getOSD();
	inline ImageLoader * getImageLoader(){return m_p_il;};

//---------------------------------------------------------------------------
public slots:
	void setThumbnailSize(bool refresh=true);

	void slotByName();
	void slotByExtension();
	void slotBySize();
	void slotByDate();
	void slotByDirName();

	void slotWallpaper();
// 	void slotKhexedit();
	void slotGimp();
	void slotEndGimp(KProcess *proc);
	void slotShowFoto();

	void slotSupprimmer ();
	void slotMoveToTrash();
	void slotShred();

	void slotFilesMoveToLast();
	void slotFilesMoveTo();
	void slotFilesCopyToLast();
	void slotFilesCopyTo();

	void slotFileProperty();
	void slotImageInfo();
	void slotCategoryProperties();

	void next ();
	void previous ();
	void first ();
	void last ();

	void slotOpenWith();
	void slotRename();

	void slotSetPixmap (const QPixmap& pm, const QFileInfo& imgFile, bool success, bool force, bool forceEXIF=false);

	void slotLoadFirst (bool force=false, bool forceEXIF=false);
	void slotLoadFirst(FileIconItem *item);
	void slotLoadNext (bool force=false, bool forceEXIF=false);
	void slotResetThumbnail();

	void slotInvertSelection();
	void slotUnselectAll();
	void slotSelectAll();
	virtual void slotUpdate ();

	void selectionChanged();

	void slotDisplayExifInformation();

	KIO::Job* removeThumbnails(bool allCurrentItems=false);
	void forceGenerateThumbnails();
	void generateEXIFThumbnails();

	FileIconItem *getCurrentItem() const{return m_p_curIt;};
	void setCurrentItem(FileIconItem *a_p_item){m_p_curIt=a_p_item;};

	//--------------------------------------------------------------------------
signals:
	void loadFinish();

	void fileIconRenamed(const QString& oldFullPath, const QString& newFullPath);
	void fileIconsDeleted(const KURL::List& list);

	void sigSetMessage(const QString&);

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
protected:
	QDragObject* dragObject();
	void leaveEvent (QEvent *e) ;

	void mousePress (QMouseEvent * e);
	void contentsMouseReleaseEvent ( QMouseEvent * e );
	void contentsMousePressEvent ( QMouseEvent * e );
	void contentsMouseDoubleClickEvent ( QMouseEvent * e );

	void updateOSD();
	MainWindow * getMainWindow();
		
		
	//--------------------------------------------------------------------------
protected slots:
	void deletionDone( KIO::Job *);
	void highlight(QIconViewItem *item);
	void onViewport();
	void slotDescribeClose();

	void toggleShowHideOSD(bool show);

	void popup(QIconViewItem *item, const QPoint &point);
	void slotRun(int id);

	void forceGenerateThumbnails__( KIO::Job *job=NULL);
	void generateEXIFThumbnails__( KIO::Job *job=NULL);

	void slotEXIFOrientation();

	void updateDestURLTitle(const KURL& a_url);
	
//---------------------------------------------------------------------------
private:
	ImageLoader 
		* m_p_il;

	KAction
			*m_p_rename_action,*m_p_trash_action,*m_p_shred_action,*m_p_delete_action,
			*m_p_fileProperties_action,*m_p_imageProperties_action,*m_p_categoryProperties_action,
			*m_p_select_action,*m_p_unselect_action,*m_p_unselectAll_action,*m_p_invertSelection_action,
			*m_p_filesMoveTo_action, *m_p_filesCopyTo_action, *m_p_filesMoveToLast_action, *m_p_filesCopyToLast_action,
			*m_p_openWithGimp_action, *m_p_editWithShowFoto_action, *m_p_openWithKhexedit_action, *m_p_openWith_action,
			*m_p_regenEXIFThumb_action,
			*m_p_regen_action,
			*m_p_imageInfo_action,
			*m_p_displayExifInformation_action;

	KToggleAction
			*m_p_EXIF_Orientation_normal_action, *m_p_EXIF_Orientation_hflip_action,
			*m_p_EXIF_Orientation_vflip_action, *m_p_EXIF_Orientation_rot90_action,
			*m_p_EXIF_Orientation_rot270_action;

	KRadioAction
			*m_p_sortBySize_action,*m_p_sortByDirName_action,*m_p_sortByType_action,*m_p_sortByName_action, *m_p_sortByDate_action,
			*m_p_iconTiny_action, *m_p_iconSmall_action, *m_p_iconMed_action, *m_p_iconBig_action;

	KPopupMenu
			*m_p_popup, *m_p_popupEmpty, *m_p_popup_openWith ;

	KActionMenu
			*m_p_copy_actionMenu, *m_p_move_actionMenu, *m_p_EXIF_Orientation_actionMenu;

	//--------------------------------------------------------------------------
	ShowimgOSD 
		* m_p_OSDWidget;
	int 
		m_sortMode,
		m_nbrTh;

	QSize 
		* m_p_currentIconSize;

	QPixmap 
		* m_p_currentIconItem;
	
	QString 
		m_currentIconItemName,
		m_lastDestDir,
		m_gimpPath;

	KIconEffect 
		* m_p_iconEffect;

	Describe 
		* m_p_dscr;

	bool 
		m_loop, //loop icons
		m_preload, //preload
		m_random, //random
		m_sMeta, m_sHexa,
		m_sMimeType, m_sSize, m_sDate, m_sDimension, m_sCategoryInfo,
		m_sToolTips,
		m_isLoadingThumbnail,
		m_mouseIsPressed,
		m_currentDragItemAreMovable,
		m_currentIconItemHasPreview,
		m_inFullScreen,
		m_preview;

	FileIconItem 
		* m_p_imageLoading,
		* m_p_curIt;

	KTrader::OfferList 
		m_offerList;

	KActionCollection  
		* m_p_actionCollection;

	KToolTip 
		* m_p_toolTips;


};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


class QtFileIconDrag : public QIconDrag
{
    Q_OBJECT

public:
    QtFileIconDrag( QWidget * dragSource, const char* name = 0 );

    const char* format( int i ) const;
    QByteArray encodedData( const char* mime ) const;
    static bool canDecode( QMimeSource* e );
    void append( const QIconDragItem &item, const QRect &pr, const QRect &tr, const QString &url );

private:
    QStringList urls;
};


#endif
