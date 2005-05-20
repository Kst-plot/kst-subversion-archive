/***************************************************************************
                          mainwindow.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
                           (C) 2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "showimg_export.h"
#include "history_action.h"

// KDE
#include <kmdimainfrm.h>
#include <kdockwindow.h>
#include <kbookmarkmanager.h>
#include <kdockwidget.h>
#include <kio/job.h>

class Directory;
class DirectoryView;
class ImageListView;
class ImageListViewSimple;
class DirectoryView;
class ImageViewer;
class HistoryAction;
class ListItem;
class ImageSimilarityData;
class RenameSeries;
class ImageMetaInfo;
class CDArchive;
class FormatConversion;
class ListItemView;
class KSideBar;
class Tools;

#ifdef HAVE_KIPI
class KIPIPluginManager;
#endif /* HAVE_KIPI */

#ifdef WANT_LIBKEXIDB
class CategoryListItem;
class CategoryDBManager;
class CategoryView;
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
class CDArchiveView;
#endif /* SHOWIMG_NO_CDARCHIVE */

class QKeyEvent;
class QFile;
class QSize;
class QDate;
class QDateTime;
class QStringList;
class QString;
class QCloseEvent;
class QImage;
class QListViewItem;
class QProgressBar;

class KDockMainWindow;
class KBookmarkOwner;
class KProcess;
class KDockWidget;
class KAction;
class KToggleAction;
class KRadioAction;
class KAccel;
class KActionMenu;
class KBookmarkMenu;
class KScanDialog;
class KProgress;
class KProgressDialog;
class KActionCollection;
class KURLCompletion;
class KHistoryCombo;
class KSqueezedTextLabel;
class KMdiChildView;
class KMdiToolViewAccessor;


class SHOWIMGCORE_EXPORT MainWindow:public KDockMainWindow, public KBookmarkOwner
{
	Q_OBJECT
public:
	MainWindow (const QString& pic=QString::null,
				bool fs_mode=false, bool fs_mode_force=false,
				bool runSlideshow=false, int slideshowT=-1);
	~MainWindow();

	ListItem* findDir(QString);
	void nextDir (ListItem *r);

	/**
		open a directory given its fullname
	*/
	bool openDir (const QString& dir, bool updateHistory_=true, bool loadThumbnails=true);

	/**
		diplay the txt in the status bar
	*/
	void setMessage (const QString& txt);

	/**
		set the number of item in the file view in the statusbar
	*/
	void setNbrItems (int nbr);

	/**
		set the zoom of displayed image in the statusbar
	*/
	void setZoom (float zoom);

	/**
		set the Imagename of displayed image in the statusbar
	*/
	void setImagename (const QString& name);

	/**
		set the Imagetype of displayed image in the statusbar
	*/
	void setImagetype (const QString& type);

	/**
		set the Imagetype of displayed image in the statusbar
	*/
	void setImageIndex (int index);

	/**
		set the Dimension of displayed image in the statusbar
	*/
	void setDim (const QSize& size, float dpi=0);

	/**
		set the Size  of displayed image in the statusbar
	*/
	void setSize (int size);

	/**
		set the Date  of displayed image in the statusbar
	*/
	void setDate (QDateTime *date);
	void setDate (const QString& date);

	/**
	*/
	void setActionsEnabled(bool enable);

	/**
		set the imageViewer to an empty image
	*/
	void setEmptyImage();

	/**
		@return true if icons preview is enabled
	*/
	bool preview();

	//
	/**
	 * This function is called if the user selects a bookmark.  It will
	 * open up the bookmark in a default fashion unless you override it.
	 */
	void openBookmarkURL(const QString& _url);

	/**
	 * This function is called whenever the user wants to add the
	 * current page to the bookmarks list.  The title will become the
	 * "name" of the bookmark.  You must overload this function if you
	 * wish to give your users the ability to add bookmarks.
	 *
	 * @return the title of the current page.
	 */
	QString currentTitle() const;

	 /**
	 * This function is called whenever the user wants to add the
	 * current page to the bookmarks list.  The URL will become the URL
	 * of the bookmark.  You must overload this function if you wish to
	 * give your users the ability to add bookmarks.
	 *
	 * @return the URL of the current page.
	 */
	QString currentURL() const;

	/**
	* add the url in the groupText group of the bookmark
	*/
	void addToBookmark(const QString& groupText, const QString& url);

	//
	/**
		@return true if in fullscreen mode
	*/
	bool fullScreen();

	/**
		copy uris to the specified directory
	*/
	void copyFilesTo(const QStringList& uris, const QString& dest);

	/**
		move uris to the specified directory
	*/
	void moveFilesTo(const QStringList& uris, const QString& dest);

	void startWatchDir();
	void stopWatchDir();

	QString getcdromPath();

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

//------------------------------------------------------------------------------
	inline ImageViewer* getImageViewer() {return iv;}
	inline ImageListView* getImageListView() {return imageList;}
	inline DirectoryView* getDirectoryView() {return dirView;}
#ifdef WANT_LIBKEXIDB
	CategoryDBManager* getCategoryManager();
	inline CategoryView* getCategoryView(){return catView;}
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	inline CDArchiveView* getCDArchiveView(){return cdarcView;}
#endif /* SHOWIMG_NO_CDARCHIVE */

#ifdef HAVE_KIPI
	 KIPIPluginManager* pluginManager();
#endif /*HAVE_KIPI*/

	Tools* getToolManager();

	//------------------------------------------------------------------------------
	/**
		Utility method: shows message box.
	*/
	void showUnableOpenDirectoryError(const QString& dir);

	void updateSelections(ListItem *item);

	//------------------------------------------------------------------------------
	QString getLastDestDir();
	void setLastDestDir(const QString& path);

	QString getCurrentDir() const;
	void setCurrentDir(const QString& dir);

	static QString getFileName(QString *fullName);
	static QString getFileExt(QString *fullName);
	static QString getFullName(QString *fullName);
	static QString getFullPath(QString *fullName);

	//------------------------------------------------------------------------------
	MainWindow* instance();

public slots:
	void clearCache();
	void clearCacheRec();
	void updateCache();

	void setHasImageSelected(bool selected);

	void slotFullScreen (); //switch to full screen and windows mode
	void slotSlideShow ();  //launch the slide show
	void slotSlideShow (int time);  //launch the slide show
	void slotPreview ();	  //show/hide thes thumbertails
	void slotRefresh (bool forceRefresh=false);	 //refresh the image list
	void slotRefresh (const QString& dir);	//refresh a specifique dir

	void slotNewWindow();
	void slotUndo();
	void slotRedo();

	void slotOpenLocation();

	void slotTODO ();
	void slotcopy ();
	void slotcopyPixmap ();
	void slotcut ();
	void slotpaste ();
	void slotStop();

	void configureKey();
	void configureToolbars();
	void configureShowImg();
	void slotShowTips();

	void slotForward();
	void slotBack();
	void goHome();
	void goUp();

	void slotAddImage (int number=1);
	void slotRemoveImage();
	void slotPreviewDone (int number=1);
	void slotReset (bool init=true);
	void slotDone ();
	void slotRemoveImage (int val);

	void slotDirChange (const QString& path);
	void slotDirChange_created(const QString& path);
	void slotDirChange_deleted(const QString& path);

	void slotEditFileType();

	void slotArrangement();
	void slotTxtPos();

	void changeDirectory(QString);
	void changeDirectory();

	void backMenuActivated(int);
	void forwardMenuActivated(int);
	void slotForwardAboutToShow();
	void slotBackAboutToShow();

	bool queryClose();
	bool closeAppl();

	void slotDisplayNBImg();

	void setZoom ( const QString&);

	void switchToInterface();

signals:
	void toggleFullscreen(bool infs);
	void lastDestDirChanged(const QString&);

private slots:
	void escapePressed();
	void spacePressed();
	void updateWindowActions();
	void deleteTempDirectoriesDone( KIO::Job *job );

private:
	void setupActions();
	void setupStatusBar();

	void init();
	void initSimpleView(const QString& dir);

	void closeEvent( QCloseEvent* );

	void clearCacheRec(const QString& fromDir);
	KURL::List updateCache(const QString &fromDir);

	void updateHistory();
	void go( int steps );

	void createActions();
	void createMenus();
	void createStatusbar();
	void createMainView();
	void createHideShowAction(KDockWidget* dock) ;

	void setLayout(int layout);
	void deleteTempDirectories();

public:
	ImageMetaInfo *imageMetaInfo;
private:
	KProgress *progress;
	int total;
	int done;
	bool inFullScreen;
	int  openDirType;
	bool showSP, startFS;
	bool showToolbar, showStatusbar;
	bool hasimageselected;
	bool inInterface;
	bool requestingClose;
	int nbrItems; //number of item
	int imageIndex; //current item index
	bool deleteTempDirectoriesDone_;
	int slideshowType, slideshowTime;
#ifdef Q_WS_WIN
	QString m_myPicturesDirPath;
#endif

//------------------------------------------------------------------------------
	ImageViewer *iv;
	ImageListView *imageList;
	DirectoryView *dirView;
	ListItemView *currentListItemView;
#ifdef WANT_LIBKEXIDB
	CategoryView *catView;
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
	CDArchiveView *cdarcView;
#endif /* SHOWIMG_NO_CDARCHIVE */
	ImageListViewSimple *imageListSimple;

	int m_sideBar_id_dirView, m_sideBar_id_catView, m_sideBar_id_cdarcView;

#ifdef HAVE_KIPI
	 KIPIPluginManager *m_pluginManager;
#endif /* HAVE_KIPI */

	Tools *m_tools;
//------------------------------------------------------------------------------
/*	KMdiChildView *mdiChildView_iv;
	KMdiToolViewAccessor
			*navToolWindow_il, *navToolWindow_dir, *navToolWindow_imi,
			*navToolWindow_cat, *navToolWindow_cdarc;
*/
	MainWindow *m_mw;

	KSideBar	*m_sideBar;
	KDockWidget *dockIV;
	KDockWidget *dockDir;
	KDockWidget *dockIL;
	KDockWidget *dockIMI;

//------------------------------------------------------------------------------
	Directory *root;
	CDArchive *cdArchiveRoot;
#ifdef WANT_LIBKEXIDB
	CategoryListItem *categoryRoot;
#endif /* WANT_LIBKEXIDB */
//------------------------------------------------------------------------------

	KProgressDialog *pdCache;
	QLabel *label;

	QSize oldSize;
	QPoint oldPos;
	QTimer *timer;

	KToolBar *mainToolbar, *pathToolbar;

	KSqueezedTextLabel *SB_NAME_Label, *SB_DATE_Label;

	KConfig *m_config;

	QColor bgColor;
	QString openDirname;

	QString cdromPath;

	QString currentDir_;
	QString m_lastDestDir;

	QLabel *lnbrItems, *lZoom, *lImagename, *lImagetype, *lDim, *lSize, *lDate;

//------------------------------------------------------------------------------
	KBookmarkMenu *mBookMenu;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
	KActionCollection *m_actions;
	KAccel* m_accel;

	KAction *aCut, *aCopy, *aPaste,  *aCopyPixmap,
	      *aNewWindow,
	      *aQuickPrint,
	      *aOpenLocation,
	      *aQuit,*aClose,
	      *aUndo,*aRedo,
	      *aEditType,
	      *aConfigureKey,*aConfigureToolbars,*aConfigureShowImg,
	      *aReloadDir,*aStop,

	      *aDirBookmark,

	      *aGoHome, *aGoUp, *aGo,

	      *aClearCache, *aClearCacheRec, *aUpdateCache,

	      *aTime;

	HistoryAction *aBack, *aForward;
	HistoryEntryList m_lstHistory;
	KHistoryCombo *m_URLHistory;
	KURLCompletion *m_URLHistoryCompletion;

	KComboBox *m_zoomCombo;

	KToggleAction *aSlideshow,
	      *aPreview,
	      *aFullScreen;

	KRadioAction
	      *aIconSmall, *aIconMed, *aIconBig,
	      *aArrangementLR, *aArrangementTB,
	      *aArrangementB, *aArrangementR;

	KActionMenu* abookmarkmenu;

	QPtrList<KAction> m_windowListActions;

};

#endif
