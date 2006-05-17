/***************************************************************************
                          imagelistview.h  -  description
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

#ifndef IMAGELISTVIEW_H
#define IMAGELISTVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "describe.h"

// KDE
#include <kfileiconview.h>
#include <ktrader.h>
#include <kurl.h>

// QT
#include <qtooltip.h>

class FileIconItem;
class MainWindow;
class ImageLoader;
class ImageListView;
class ShowimgOSD;
class KToolTip;

class KProcess;
class KPopupMenu;
class KFileViewItem;
class KAction;
class KIconEffect;
class KToggleAction;
class KRadioAction;
class KActionMenu;

class QListViewItem;
class QIconViewItem;
class QFileInfo;




//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class ImageListView : public KIconView
{
	Q_OBJECT
//---------------------------------------------------------------------------
public:
	ImageListView (QWidget *parent, const QString& name, MainWindow *mw);
	virtual ~ImageListView();

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);

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

	void sort();
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

	//--------------------------------------------------------------------------
public:
	bool inFullScreen;
	bool preview;
	ImageLoader * il;
	FileIconItem *curIt;


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

	void updateDestDirTitle(const QString& dir);

//---------------------------------------------------------------------------
private:
	KAction
			*aRename,*aTrash,*aShred,*aDelete,
			*aFileProperties,*aImageProperties,*aCategoryProperties,
			*aSelect,*aUnselect,*aUnselectAll,*aInvertSelection,
			*aFilesMoveTo, *aFilesCopyTo, *aFilesMoveToLast, *aFilesCopyToLast,
			*aOpenWithGimp, *aEditWithShowFoto, *aOpenWithKhexedit, *aOpenWith,
			*a_regenEXIFThumb,
			*a_regen,
			*aImageInfo,
			*aDisplayExifInformation;

	KToggleAction
			*aEXIF_Orientation_normal, *aEXIF_Orientation_hflip,
			*aEXIF_Orientation_vflip, *aEXIF_Orientation_rot90,
			*aEXIF_Orientation_rot270;

	KRadioAction
			*aSortBySize,*aSortByDirName,*aSortByType,*aSortByName, *aSortByDate,
			*aIconTiny, *aIconSmall, *aIconMed, *aIconBig;

	KPopupMenu
			*m_popup, *m_popupEmpty, *m_popup_openWith ;

	KActionMenu
			*aCopyActions, *aMoveActions, *aEXIF_Orientation;

	//--------------------------------------------------------------------------
	ShowimgOSD *m_OSDWidget;
	int sortMode;

	MainWindow *mw;

	QSize *currentIconSize;

	QPixmap *currentIconItem;
	QString currentIconItemName;
	bool currentIconItemHasPreview;
	KIconEffect *iconEffect;

	QString lastDestDir;

	Describe *dscr;

	int nbrTh;

	bool loop_; //loop icons
	bool preload_; //preload
	bool random_; //random

	bool sMeta_, sHexa_;
	bool sMimeType_, sSize_, sDate_, sDimension_, m_sCategoryInfo;
	bool sToolTips_;
	QString gimpPath_;

	FileIconItem *imageLoading;
	bool isLoadingThumbnail;

	KTrader::OfferList m_offerList;
	KActionCollection *actionCollection;

	KToolTip *toolTips;

	bool mouseIsPressed;
	bool m_currentDragItemAreMovable;

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
