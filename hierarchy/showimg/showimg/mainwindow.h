/*****************************************************************************
                          mainwindow.h
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
                           (C) 2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "showimg_export.h"
#include "history_action.h"
#include "viewer.h"

// KDE
#include <kdeversion.h>
#if KDE_IS_VERSION(3,3,0)
#include <kmdimainfrm.h>
#endif
#include <kdockwindow.h>
#include <kbookmarkmanager.h>
#include <kdockwidget.h>
#include <kio/job.h>

class CDArchive;
class Directory;
class DirectoryView;
class DirectoryView;
class FormatConversion;
class HistoryAction;
class ImageListView;
class ImageListViewSimple;
class ImageMetaInfo;
class ImageMetaInfoView;
class ImageSimilarityData;
class ImageViewer;
class KSideBar;
class ListItem;
class ListItemView;
class RenameSeries;
class Tools;
class Viewer;

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

// QT
#include <qdict.h>
class QCloseEvent;
class QDate;
class QDateTime;
class QFile;
class QImage;
class QKeyEvent;
class QListViewItem;
class QProgressBar;
class QSize;
class QString;
class QStringList;

class KAccel;
class KAction;
class KActionCollection;
class KActionMenu;
class KBookmarkMenu;
class KBookmarkOwner;
class KDockMainWindow;
class KDockWidget;
class KHistoryCombo;
class KMdiChildView;
class KMdiToolViewAccessor;
class KParts::ReadOnlyPart;
class KProcess;
class KProgress;
class KProgressDialog;
class KRadioAction;
class KScanDialog;
class KSqueezedTextLabel;
class KToggleAction;
class KURLCompletion;

#define MAINWINDOW MainWindow::getInstance()


class SHOWIMGCORE_EXPORT MainWindow :
#if ! KDE_IS_VERSION(3,3,0)
	public KDockMainWindow,
#else
	public KParts::DockMainWindow,
#endif
	public KBookmarkOwner
{
	Q_OBJECT
public:

	MainWindow (
		const QString& pic=QString::null,
		bool fs_mode=false,
		bool fs_mode_force=false,
		bool runSlideshow=false,
		int slideshowT=-1);

	virtual ~MainWindow();


	ListItem* findDir(const QString &);
	void nextDir (ListItem *r);

	/**
		open a directory given its fullname
	*/
	bool openDir (const QString& dir, bool updateHistory_=true, bool loadThumbnails=true);

	/**
		open a directory given its fullname
	*/
	bool openURL (const KURL& a_url);

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
	void setDate (const QDateTime& a_date);
	void setDate (const QString& a_date);

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
	void copyFilesTo(const KURL::List& a_list, const KURL& a_dest);

	/**
		move uris to the specified directory
	*/
	void moveFilesTo(const KURL::List& a_list, const KURL& a_dest);

	QString getcdromPath();

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

//------------------------------------------------------------------------------
	inline Viewer* getViewer()          {return m_p_viewer;}
	inline const Viewer* getViewer() const   {return m_p_viewer;}

	inline ImageViewer* getImageViewer()        {return m_p_iv;}
	inline const ImageViewer* getImageViewer() const {return m_p_iv;}

	inline ImageListView* getImageListView()        {return m_p_imageList;}
	inline const ImageListView* getImageListView() const {return m_p_imageList;}

	inline DirectoryView* getDirectoryView()       {return m_p_dirView;}
	inline const DirectoryView* getDirectoryView()const {return m_p_dirView;}

#ifdef WANT_LIBKEXIDB
	const CategoryDBManager* getCategoryDBManager()const;
	CategoryDBManager* getCategoryDBManager();

	inline CategoryView* getCategoryView()   {return m_p_catView;}
	inline const  CategoryView* getCategoryView() const {return m_p_catView;}

	void setEnabledCategories(bool enable);
	bool getEnabledCategories();
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
	inline CDArchiveView* getCDArchiveView(){return m_cdarcView;}
#endif /* SHOWIMG_NO_CDARCHIVE */

#ifdef HAVE_KIPI
	 KIPIPluginManager* pluginManager();
#endif /*HAVE_KIPI*/

	Tools* getToolManager();

	ImageMetaInfo* getImageMetaInfo();

	//------------------------------------------------------------------------------
	/**
		Utility method: shows message box.
	*/
	void showUnableOpenDirectoryError(const KURL& a_url);

	void updateSelections(ListItem *item);

	//------------------------------------------------------------------------------
	KURL getLastDestURL();
	void setLastDestURL(const KURL& a_url);

	QString getCurrentDir() const;
	KURL    getCurrentURL() const;
	void setCurrentURL(const KURL & a_url);

	static QString getFileName(QString *fullName);
	static QString getFileExt(QString *fullName);
	static QString getFullName(QString *fullName);
	static QString getFullPath(QString *fullName);

	void updateDB(QDict<QString>& renamedFile);


	//------------------------------------------------------------------------------
	static MainWindow &getInstance();

public slots:
	void clearCache();
	void clearCacheRec();
	void updateCache();
	void removeObsololeteFilesOfTheDatabase();

	void setHasImageSelected(bool selected);

	/**
	diplay the txt in the status bar
	*/
	void slotSetStatusBarText (const QString& txt);



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
	void slotDone (int numberOfItems);
	void slotRemoveImage (int val);

	void slotDirChange (const QString& path);
	void slotDirChange_created(const QString& path);
	void slotDirChange_deleted(const QString& path);

	void slotEditFileType();

	void slotArrangement();
	void slotTxtPos();

	void changeURL(const KURL& a_url);
	void changeURL();

	void backMenuActivated(int);
	void forwardMenuActivated(int);
	void slotForwardAboutToShow();
	void slotBackAboutToShow();

	bool queryClose();
	bool closeAppl();

	void slotDisplayNBImg();

	void setZoom ( const QString&);

	void switchToInterface();
	void switchToSimpleUI();
	void switchToFullUI();

	void updateGUI (Viewer::AvailableViewers current);

	int getTotal();
	void saveNumberOfImages();
	void restoreNumberOfImages();

signals:
	void toggleFullscreen(bool infs);
	void lastDestURLChanged(const KURL&);

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

	bool initMovieViewer();
	void initAvailableMovieViewer();
	QStringList getAvailableMovieViewer();
	void setCurrentAvailableMovieViewerIndex(int current);
	int getCurrentAvailableMovieViewerIndex();

	bool initSVGViewer();

private:
	ImageMetaInfoView *m_p_imageMetaInfo;
	KProgress         *m_p_statusbarProgress;
	QDateTime          m_statusbarProgressLastUpdate;
	int
		m_total,
		m_done,
		m_savedNumberOfImages,
		m_openDirType,
		m_nbrItems, //number of item
		m_imageIndex, //current item index
		m_slideshowType, m_slideshowTime;

	bool
		m_inFullScreen,
		m_showSP, m_startFS,
		m_showToolbar, m_showStatusbar,
		m_hasimageselected,
		m_inInterface,
		m_requestingClose,
		m_deleteTempDirectoriesDone;
#ifdef Q_WS_WIN
	QString m_myPicturesDirPath;
#endif

//------------------------------------------------------------------------------
	ImageViewer          *m_p_iv;
	KParts::ReadOnlyPart *m_p_movieViewer;
	KParts::ReadOnlyPart *m_p_SVGViewer;
	Viewer               *m_p_viewer;
	KParts::PartManager  *m_p_partManager;
	QStringList           m_availableMovieViewer;
	int                   m_currentAvailableMovieViewerIndex;


	ImageListView *m_p_imageList;
	DirectoryView *m_p_dirView;
	ListItemView  *m_p_currentListItemView;
#ifdef WANT_LIBKEXIDB
	CategoryView  *m_p_catView;
	bool           m_categoriesEnabled;
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
	CDArchiveView       *m_cdarcView;
#endif /* SHOWIMG_NO_CDARCHIVE */
	ImageListViewSimple *m_imageListSimple;

	int m_sideBar_id_dirView, m_sideBar_id_catView, m_sideBar_id_cdarcView;

#ifdef HAVE_KIPI
	 KIPIPluginManager *m_p_pluginManager;
#endif /* HAVE_KIPI */

	Tools *m_p_tools;

//------------------------------------------------------------------------------
	static MainWindow *m_singleton_mw ;

	KSideBar	*m_sideBar;
	KDockWidget *m_dockIV, *m_dockDir, *m_dockIL, *m_dockIMI;

//------------------------------------------------------------------------------
// 	Directory *m_root_dir;
	CDArchive *m_root_cdarchive;

//------------------------------------------------------------------------------
	KProgressDialog        *m_pdCache;

	QTimer                 *m_timer;

	KSqueezedTextLabel     *m_p_SB_NAME_Label,
	                       *m_p_SB_DATE_Label;

	KConfig                *m_p_config;

	QColor                 m_bgColor;
	QString                m_openDirname;

	QString                m_cdromPath;

	KURL
	                       m_currentURL,
	                       m_lastDestURL;

//------------------------------------------------------------------------------
	KBookmarkMenu         *m_p_bookMenu;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
	KActionCollection     *m_p_actions;

	KToolBar
#ifdef WANT_LIBKEXIDB
		*m_p_tb_cat,
#endif
		*m_p_tb_dir,
		*m_p_tb_cdarc;

	KAction
		*m_p_cut_action, *m_p_copy_action, *m_p_paste_action,  *m_p_copyPixmap_action,
		*m_p_newWindow_action,
		*m_p_openLocation_action,
		*m_p_quit_action, *m_p_close_action,
		*m_p_undo_action, *m_p_redo_action,
		*m_p_editType_action,
		*m_p_configureKey_action, *m_p_configureToolbars_action, *m_p_configureShowImg_action,
		*m_p_reloadDir_action,*m_p_stop_action,

		*m_p_dirBookmark_action,

		*m_p_goHome_action, *m_p_goUp_action, *m_p_go_action,

		*m_p_clearCache_action, *m_p_clearCacheRec_action, *m_p_updateCache_action, *m_p_updateDB_action,

		*m_p_time_action;

	HistoryAction    *m_p_back_action, *m_p_forward_action;
	HistoryEntryList  m_lstHistory_list;
	KHistoryCombo    *m_p_URLHistory_combo;
	KURLCompletion   *m_p_URLHistoryCompletion;

	KComboBox        *m_p_zoomCombo;

	KToggleAction
		*m_p_slideshow_action,
		*m_p_preview_action,
		*m_p_fullScreen_action;

	KRadioAction
	      *m_p_iconSmall_action, *m_p_iconMed_action, *m_p_iconBig_action,
	      *m_p_arrangementLR_action, *m_p_arrangementTB_action,
	      *m_p_arrangementB_action, *m_p_arrangementR_action;

	KActionMenu *m_p_bookmarkmenu;

	QString      m_xmluifile;

	QPtrList<KAction> m_windowListActions;
};

#endif
