/***************************************************************************
                          mainwindow.cp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult,
                               2003 by OGINO Tomonori,
                               2004 by Guillaume Duhamel
                               2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           ogino@nn.iij4u.or.jp
                           guillaume.duhamel@univ-rouen.fr
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

#include "mainwindow.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>

// Local
#include "mybookmarkmanager.h"
#include "confshowimg.h"
#include "imageviewer.h"
#include "extract.h"
#include "imagelistview.h"
#include "directoryview.h"
#include "directory.h"
#include "desktoplistitem.h"
#include "displaycompare.h"
#include "compressedfileitem.h"
#include "history_action.h"
#include "imagefileiconitem.h"
#include "imageloader.h"
#include "imagemetainfo.h"
#include "ksidebar.h"
#include "imagelistviewsimple.h"
#include "tools.h"
#include "kstartuplogo.h"
#include "osd.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#else
#warning no HAVE_KIPI
#endif /* HAVE_KIPI */

#ifdef WANT_LIBKEXIDB
#include "categoryview.h"
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
#include "cdarchive.h"
#include "cdarchiveview.h"
#endif

#ifdef WANT_DIGIKAMIMAGEEFFECT
#include <digikam/imageplugin.h>
#include "imageeditor/imagepluginloader.h"
#endif

// Qt
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmemarray.h>
#include <qdict.h>
#include <qkeycode.h>
#include <qdir.h>
#include <qtimer.h>
#include <qptrvector.h>
#include <qdatetime.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qdockarea.h>
#include <qtextcodec.h>
#include <qlayout.h>
#include <qvbox.h>

// KDE
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kio/job.h>
#include <kaboutapplication.h>
#include <ktoolbarbutton.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kglobalsettings.h>
#include <kurlrequesterdlg.h>
#include <kdialogbase.h>
#include <kdockwidget.h>
#include <kcursor.h>
#include <kurldrag.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kwin.h>
#include <ktoolbar.h>
#include <kapplication.h>
#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include <kbookmarkbar.h>
#include <kbookmark.h>
#include <kaccel.h>
#include <kstatusbar.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kimageeffect.h>
#include <kprinter.h>
#include <kmenubar.h>
#include <kprogress.h>
#include <kfilemetainfo.h>
#include <kprocess.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kcompletion.h>
#include <kurlcompletion.h>
#include <kurldrag.h>
#include <kio/netaccess.h>
#include <ktip.h>
#include <ksqueezedtextlabel.h>
#include <ktabwidget.h>
#include <kpixmapio.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#ifdef Q_WS_WIN
# include "win_utils.h"
#endif

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

const char* CONFIG_IMAGEVIEWER_GROUP = "imageviewer widget";
const char* CONFIG_DOCK_GROUP =        "dock showimg 20050103";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


MainWindow::MainWindow (const QString& pic,
			bool fs_mode, bool fs_mode_force,
			bool runSlideshow, int slideshowT)
	: KDockMainWindow (0, "ShowImg MainFrame"), KBookmarkOwner(),

	total(0),
	done(0),
	inFullScreen(false),
	inInterface(false),
	requestingClose(false),
	nbrItems(0),
	imageIndex(0),
	deleteTempDirectoriesDone_(false),

	iv(NULL),
	imageList(NULL),
	dirView(NULL),

#ifdef HAVE_KIPI
	m_pluginManager(NULL),
#endif /* HAVE_KIPI */
	m_tools(NULL),
	m_mw(NULL),
	timer(NULL),

	m_config(NULL),

	m_actions(NULL),
	m_zoomCombo(NULL),

	aFullScreen(NULL)
{
// 	MYDEBUG<<endl;
	if(pic.isEmpty() || pic.isNull())
	{
		MYDEBUG<<endl;
		init();
		show();

		m_config->setGroup("Options");
		openDirType = m_config->readNumEntry("openDirType", 0);
		openDirname = m_config->readPathEntry("openDirname", QDir::homeDirPath());

		inInterface=true;
		if(openDirType==0)
#ifdef Q_WS_WIN
			openDir(desktopDirPath());
#else
			openDir(QDir::homeDirPath());
#endif
		else
			openDir(openDirname);
		setHasImageSelected(getImageListView()->hasImages());
	}
	else
	if (QFileInfo (pic).isDir() || QString(pic).right (3) == "sia")
	{
		MYDEBUG<<endl;
		init();
		show();

		inInterface=true;
		openDir(QDir (pic).absPath());
		if(runSlideshow)
		{
			if(slideshowT < 0)
				slideshowT = m_config->readNumEntry("time", 2);
			slotSlideShow(slideshowT);
		}
		else
		if(fs_mode==true && fs_mode_force==true)
		{
			MYDEBUG<<endl;
			getImageListView()->first();
			slotFullScreen();
		}
		setHasImageSelected(getImageListView()->hasImages());
	}
	else
	if (Extract::canExtract(pic))
	{
		init();
		show();

		QString rep = QDir (pic).absPath ();
		int pos = rep.findRev ("/");

		inInterface=true;
		openDir(rep.left (pos));
		setHasImageSelected(true);

		getImageListView()->setCurrentItemName(QFileInfo(pic).fileName());
	}
	else
	{
		MYDEBUG<<endl;

		m_config = KGlobal::config();
		startFS = m_config->readBoolEntry("startFS", true);
		MYDEBUG<<startFS<<" " <<fs_mode<<" "<<fs_mode_force<<endl;
		if(ListItemView::isImage(pic) &&
				 ( (startFS && fs_mode_force==false) ||
				   (fs_mode==true && fs_mode_force==true)))
		{
			MYDEBUG<<endl;

			inInterface=false;
			initSimpleView(QDir(pic).absPath());
			showFullScreen();
			if(runSlideshow)
			{
				if(slideshowT < 0)
					slideshowT = m_config->readNumEntry("time", 2);
				imageListSimple->startSlideshow(slideshowT);
			}
		}
		else
		{
			MYDEBUG<<pic<<endl;

			inInterface=true;
			init();
			show();
			if(!pic.isEmpty())
			{
				MYDEBUG<<endl;
				openDir(QDir(pic).absPath(), true, false);
				getDirectoryView()->setLoadThumbnails(true);
			}
			else
			{
				MYDEBUG<<endl;
				openDir(QDir::homeDirPath());
			}
			if(runSlideshow)
			{
				if(slideshowT < 0)
					slideshowT = m_config->readNumEntry("time", 2);
				slotSlideShow(slideshowT);
			}
		}
	}
	MYDEBUG<<endl;
}


MainWindow::~MainWindow ()
{
}


void
MainWindow::init()
{
	m_config = KGlobal::config();
	KGlobal::iconLoader()->addAppDir("showimg");
	//------------------------------------------------------------------------------
	createStatusbar();
	createMainView();
	createActions();
	createMenus();

	//------------------------------------------------------------------------------
	readConfig(m_config);

	//------------------------------------------------------------------------------
	KMainWindow::createGUI("showimgui.rc", false);
	KMainWindow::setStandardToolBarMenuEnabled(true);
	KMainWindow::applyMainWindowSettings(m_config);
	KMainWindow::restoreWindowSize(m_config);

	//------------------------------------------------------------------------------
#ifdef WANT_DIGIKAMIMAGEEFFECT
	//ImagePluginLoader *m_ImagePluginsLoader = new ImagePluginLoader(this);
	(void) new ImagePluginLoader(this);
    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
    }
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            plugin->setEnabledSelectionActions(true);
        }
    }
#endif

#ifdef HAVE_KIPI
	// Load Plugins
	m_pluginManager = new KIPIPluginManager(this);
	pluginManager()->loadPlugins();
#endif /* HAVE_KIPI */
#ifndef WANT_LIBKEXIDB
	toolBar("CatViewToolBar")->hide();
#endif /* WANT_LIBKEXIDB */

}


void
MainWindow::initSimpleView(const QString& dir)
{

	(void)new KActionMenu( i18n("&Go"), actionCollection(), "action go");

	iv = new ImageViewer (this); KCursor::setAutoHideCursor(iv, true);
	setView(iv);
	getImageViewer()->initActions(actionCollection());
	getImageViewer()->readConfig(m_config, CONFIG_IMAGEVIEWER_GROUP);
	getImageViewer()->initMenu(actionCollection());

	imageListSimple = new ImageListViewSimple(this, dir, getImageViewer());
	imageListSimple->initActions(actionCollection());
	imageListSimple->readConfig(m_config);

	///
	(void)new KAction(i18n("Next"), KShortcut(Key_Space), imageListSimple, SLOT(next()), actionCollection(), "simple interface next" );
	(void)new KAction(i18n("Switch to interface"), "window_fullscreen", KShortcut(Key_F), this, SLOT(switchToInterface()), actionCollection(), "Switch to interface" );
	(void)new KAction(i18n("Quit"), "quit", KShortcut(Key_Escape), this, SLOT(escapePressed()), actionCollection(), "Quit simple interface");

	///
	createGUI("showimgui.rc", false);

	///
	leftDock()->hide(); rightDock()->hide(); topDock()->hide();	bottomDock()->hide();
	menuBar()->hide(); statusBar()->hide();
	toolBar("locationToolBar")->hide(); toolBar("mainToolBar")->hide();	toolBar("viewToolBar")->hide();

	setGeometry(0,0,KApplication::desktop()->width(), KApplication::desktop()->height());
	imageListSimple->load();
	inFullScreen=false;
	inInterface=false;
	requestingClose=false;
	hasimageselected=true;
}

#define SB_ITEMS    1
#define SB_SCALE    2
#define SB_NAME     3
#define SB_TYPE     4
#define SB_IMG_SIZE 5
#define SB_BYTES    6
#define SB_DATE     7
#define SB_MSG      8
#define SB_PATH     9

void
MainWindow::createStatusbar()
{
	statusBar()->insertItem(QString::null, SB_MSG, 1,true);		statusBar()->setItemAlignment(SB_MSG, AlignTop);
	statusBar()->insertItem(QString::null, SB_ITEMS, 0,true);		statusBar()->setItemAlignment(SB_ITEMS, AlignTop|AlignHCenter);
	statusBar()->insertItem(QString::null, SB_SCALE, 0,true);		statusBar()->setItemAlignment(SB_SCALE, AlignTop|AlignHCenter);

	SB_NAME_Label = new KSqueezedTextLabel (statusBar());
	SB_NAME_Label->setAlignment( int( KSqueezedTextLabel::AlignCenter ) );
	statusBar()->addWidget (SB_NAME_Label, 2, true);

	statusBar()->insertItem(QString::null, SB_TYPE, 0,true);		statusBar()->setItemAlignment(SB_TYPE, AlignTop|AlignHCenter);
	statusBar()->insertItem(QString::null, SB_IMG_SIZE, 1,true);	statusBar()->setItemAlignment(SB_IMG_SIZE, AlignTop|AlignHCenter);
	statusBar()->insertItem(QString::null, SB_BYTES, 1,true);		statusBar()->setItemAlignment(SB_BYTES, AlignTop|AlignHCenter);

	SB_DATE_Label = new KSqueezedTextLabel (statusBar());
	SB_DATE_Label->setAlignment( int( KSqueezedTextLabel::AlignCenter ) );
	statusBar()->addWidget (SB_DATE_Label, 2, true);

	progress = new KProgress (statusBar(), "QProgressBar de chargement des images de MainWindow");
	progress->setCenterIndicator (true);
	statusBar()->addWidget (progress, 1, false);
	progress->hide();

}

void
MainWindow::createMainView()
{
// 	MYDEBUG<<endl;

	manager()->setSplitterOpaqueResize(true);

	//
	dockIV = createDockWidget ("Preview", SmallIcon("image"),0L, i18n("Preview"), i18n("Preview"));
	iv = new ImageViewer (dockIV, this, "ImageViewer");
	dockIV->setWidget( iv );
	KCursor::setAutoHideCursor(iv, true);
	QWhatsThis::add(iv, i18n( "Image preview" ) );
	dockIV->setToolTipString(i18n("Image preview"));
	setView (dockIV);

	dockIL = createDockWidget ("image listview dock ", SmallIcon("view_icon"), 0L, i18n("Image Icon Viewer"), i18n("Image Icon Viewer"));
	imageList = new ImageListView (dockIL, "ImageList", this);
	QWhatsThis::add(imageList, i18n( "Images of the selected directories" ) );
	dockIL->setWidget(imageList);
	dockIL->setToolTipString(i18n("Images in the selected directories"));


	dockIMI = createDockWidget ("Image Meta Data", SmallIcon("info"), 0L, i18n("Image Meta Data"), i18n("Image Meta Data"));
	imageMetaInfo = new ImageMetaInfo(dockIMI);
	dockIMI->setWidget( imageMetaInfo );

	//
	/////////////////////////////////////////////////////////////////////////////////
	m_sideBar = new KSideBar(this, "my sidebar");

	dockDir = createDockWidget ("Tree View", SmallIcon("folder"), 0L, i18n("Tree View"), i18n("Tree View"));
	dockDir->setWidget( m_sideBar );

	QVBox* vbox_dir=new QVBox(this, "navToolWindow_dir QVBox");
	KToolBar *tb_dir =new KToolBar(vbox_dir, "fileViewToolBar", true); tb_dir->setIconSize(KIcon::SizeSmall);
	dirView = new DirectoryView (vbox_dir, this, "Directory View");
	m_sideBar_id_dirView = m_sideBar->addTab(vbox_dir, SmallIcon("folder"), i18n("Tree View"));

#ifdef WANT_LIBKEXIDB
	QVBox* vbox_cat=new QVBox(this, "navToolWindow_cat QVBox");
	KToolBar *tb_cat =new KToolBar(vbox_cat, "CatViewToolBar", true); tb_cat->setIconSize(KIcon::SizeSmall);
	catView = new CategoryView (vbox_cat, this, "CategoryView DirectoryView");
	m_sideBar_id_catView = m_sideBar->addTab(vbox_cat, SmallIcon("kexi_kexi"), i18n("Category View"));
#endif /* WANT_LIBKEXIDB
	   */
	QVBox* vbox_cdarc=new QVBox(this, "navToolWindow_cdarcQVBox");
	KToolBar *tb_cdarc=new KToolBar(vbox_cdarc, "CdarcViewToolBar", true); tb_cdarc->setIconSize(KIcon::SizeSmall);
	cdarcView = new CDArchiveView (vbox_cdarc, this, "CDArchiveView DirectoryView");
	m_sideBar_id_cdarcView = m_sideBar->addTab(vbox_cdarc, SmallIcon("cdimage"), i18n("CD Archive View"));

	dockDir->manualDock(dockIV, KDockWidget::DockLeft, 35);
	dockIL->manualDock(dockDir, KDockWidget::DockBottom, 50);
	dockIMI->manualDock(dockIV, KDockWidget::DockCenter, 35);


	//------------------------------------------------------------------------------
	m_sideBar->switchToTab(m_sideBar_id_dirView);
	currentListItemView = dirView;

	//------------------------------------------------------------------------------
#ifdef Q_WS_WIN
	m_myPicturesDirPath = myPicturesDirPath();
	root = new SpecialListItem( getDirectoryView(), getImageViewer(), getImageListView(), this, desktopDirPath(), "desktop", i18n("Desktop") );
	root = new SpecialListItem( getDirectoryView(), getImageViewer(), getImageListView(), this, myDocumentsDirPath(), "kword", i18n("My Documents") );
//	root = new SpecialListItem( dirView, iv, imageList, this, myPicturesDirPath(), "images", i18n("My Pictures") );
	for (QPtrListIterator<QFileInfo> drivesIt(*QDir::drives()); drivesIt.current(); ++drivesIt) {
		new Directory (getDirectoryView(), getImageViewer(), getImageListView(), this, drivesIt.current()->absFilePath());
	}
#else
	root = new Directory (this);
	root->setOpen (true);
#endif

#ifndef SHOWIMG_NO_CDARCHIVE
	cdArchiveRoot = new CDArchive (this);
	cdArchiveRoot->setOpen (true);
#endif

#ifdef WANT_LIBKEXIDB
	getCategoryView()->createRootCategory();
#endif

	m_tools = new Tools(this);
}

void
MainWindow::createActions()
{
	m_actions = actionCollection();

	aBack		=new HistoryAction(i18n("Back"), "back", KStdAccel::shortcut(KStdAccel::Back).keyCodeQt(), this, SLOT(slotBack()), m_actions, "back");
	connect(aBack->popupMenu(), SIGNAL(activated(int)), this, SLOT(backMenuActivated(int)));
	connect(aBack->popupMenu(), SIGNAL( aboutToShow() ), SLOT( slotBackAboutToShow() ) );
	aBack->setEnabled(false);

	aForward	=new HistoryAction(i18n("Forward"), "forward", KStdAccel::shortcut(KStdAccel::Forward).keyCodeQt() , this, SLOT(slotForward()), m_actions, "forward");
	connect(aForward->popupMenu(), SIGNAL(activated(int)), this, SLOT(forwardMenuActivated(int)));
	connect(aForward->popupMenu(), SIGNAL( aboutToShow() ), SLOT( slotForwardAboutToShow() ) );
	aForward->setEnabled(false);

	//------------------------------------------------------------------------------
	aCut		=new KAction(i18n("Cut"), "editcut", KStdAccel::shortcut(KStdAccel::Cut), this, SLOT(slotcut()), m_actions , "editcut");
	aCut->setEnabled(false);
	aCopy		=new KAction(i18n("Copy"), "editcopy", KStdAccel::shortcut(KStdAccel::Copy), this, SLOT(slotcopy()), m_actions, "editcopy");
	aCopyPixmap	=new KAction(i18n("Copy Image"), 0, this, SLOT(slotcopyPixmap()), m_actions, "editcopypixmap");
	aPaste		=new KAction(i18n("Paste"), "editpaste", KStdAccel::shortcut(KStdAccel::Paste), this, SLOT(slotpaste()), m_actions, "editpaste" );

	//------------------------------------------------------------------------------
	aGoHome		=new KAction(i18n("Go to &Home Directory"), "gohome", KStdAccel::shortcut(KStdAccel::Home), this, SLOT(goHome()), m_actions, "goHome");
	aGoUp		=new KAction(i18n("Go Up"), "up", KStdAccel::shortcut(KStdAccel::Up), this, SLOT(goUp()), m_actions , "goUp");


	KActionMenu *actionGo = new KActionMenu( i18n("&Go"), m_actions, "action go");
	actionGo->insert(aGoUp);
	actionGo->insert(aGoHome);
	//FIXME add actions
// 	actionGo->insert(new KActionSeparator());
	//actionGo->insert(aPreviousDir);
	//actionGo->insert(aNextDir);
	//actionGo->insert(new KActionSeparator());
// 	actionGo->insert(aPrevious);
// 	actionGo->insert(aNext);
// 	actionGo->insert(aFirst);
// 	actionGo->insert(aLast);


	//------------------------------------------------------------------------------
	aNewWindow	=new KAction(i18n("New Window"), "window_new", KStdAccel::shortcut(KStdAccel::New), this, SLOT(slotNewWindow()), m_actions, "window_new" );

	aOpenLocation	=new KAction(i18n("Open Location..."), "fileopen", KStdAccel::shortcut(KStdAccel::Open), this, SLOT(slotOpenLocation()), m_actions, "fileopen" );

	aQuit = KStdAction::quit( kapp, SLOT (closeAllWindows()), actionCollection() );
	aClose		=new KAction(i18n("Close"), "fileclose", KStdAccel::shortcut(KStdAccel::Close), this, SLOT(close()), m_actions , "close");

	//------------------------------------------------------------------------------
// 	aUndo		=new KAction(i18n("Undo"), "undo", KStdAccel::shortcut(KStdAccel::Undo), this, SLOT(slotUndo()), m_actions , "undo");
// 	aUndo->setEnabled(false);
// 	aRedo		=new KAction(i18n("Redo"), "redo", KStdAccel::shortcut(KStdAccel::Redo), this, SLOT(slotRedo()), m_actions , "redo");
// 	aRedo->setEnabled(false);

	//------------------------------------------------------------------------------
	aEditType	=new KAction(i18n("Edit File Type"), 0, this, SLOT(slotEditFileType()), m_actions, "Edit File Type");

	aConfigureKey =     KStdAction::keyBindings(this, SLOT(configureKey()),  actionCollection() );
	aConfigureToolbars= KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection() );
	aConfigureShowImg = KStdAction::preferences(this, SLOT(configureShowImg()), actionCollection() );
	// the tip action
	(void)KStdAction::tipOfDay(this, SLOT(slotShowTips()), actionCollection(), "help_showimgtipofday");

	//------------------------------------------------------------------------------
	aSlideshow	=new KToggleAction(i18n("&Slide Show"), "run", 0, this, SLOT(slotSlideShow()), m_actions, "Slideshow");

	//------------------------------------------------------------------------------
	aReloadDir	=new KAction(i18n("Refresh"), "reload", KStdAccel::shortcut(KStdAccel::Reload), this, SLOT(slotRefresh()), m_actions, "Refresh");
	aPreview	=new KToggleAction(i18n("Toggle Thumbnails"), "thumbnail", 0, this, SLOT(slotPreview()), m_actions, "Preview");
	aStop		=new KAction(i18n("Stop"), "stop", 0, this, SLOT(slotStop()), m_actions, "Stop");
	aStop->setEnabled(false);


	//------------------------------------------------------------------------------
	KShortcut sc_fs(CTRL+Key_F); sc_fs.append(KKeySequence((const KKey&)(CTRL+Key_Return)));
	aFullScreen=new KToggleAction(i18n("Full Screen"), "window_fullscreen", sc_fs, this, SLOT(slotFullScreen()), m_actions, "FullScreen" );
	aFullScreen->setChecked(false);

	//------------------------------------------------------------------------------
	aUpdateCache	=new KAction(i18n("Recursively &Update Current Cached Thumbnails"), 0, this, SLOT(updateCache()), m_actions , "updateCache");
	aClearCacheRec	=new KAction(i18n("Recursively Remove Current Cached Thumbnails"),  0, this, SLOT(clearCacheRec()), m_actions , "clearCacheRec");
	aClearCache	=new KAction(i18n("&Remove Current Cached Thumbnails"),  0, this, SLOT(clearCache()), m_actions , "clearCache");
	KActionMenu *actionMaint = new KActionMenu( i18n("Main&tenance"), m_actions, "tools_maint" );
	actionMaint->insert(aUpdateCache);
	actionMaint->insert(aClearCacheRec);
	actionMaint->insert(aClearCache);

	//------------------------------------------------------------------------------
	abookmarkmenu = 	new KActionMenu(i18n("&Bookmark"), m_actions, "bookm");
	mBookMenu=		new KBookmarkMenu(MyBookmarkManager::self(), this, abookmarkmenu->popupMenu(),  m_actions, true);

	//------------------------------------------------------------------------------
	m_URLHistory=new KHistoryCombo(this);
	m_URLHistory->setDuplicatesEnabled(false);
	m_URLHistory->setAutoDeleteCompletionObject(true);

	m_URLHistoryCompletion=new KURLCompletion(KURLCompletion::DirCompletion);
	m_URLHistory->setCompletionObject(m_URLHistoryCompletion);
	m_URLHistoryCompletion->setDir("file:/");

	KWidgetAction* comboAction=new KWidgetAction( m_URLHistory, i18n("Location Bar"), 0, 0, 0, m_actions, "location_url");
	comboAction->setShortcutConfigurable(false);
	comboAction->setAutoSized(true);

	(void)new KAction( i18n("Clear Location Bar"), "locationbar_erase", 0, m_URLHistory, SLOT(clearEdit()), m_actions, "clear_location");

	QLabel* m_urlLabel=new QLabel(i18n("L&ocation:"), this, "kde toolbar widget");
	(void)new KWidgetAction( m_urlLabel, i18n("L&ocation:"), 0, 0, 0, m_actions, "location_label");
	m_urlLabel->setBuddy(m_URLHistory);
	aGo     =       new KAction(i18n("Go"), "key_enter", 0, this, SLOT(changeDirectory()), actionCollection(), "location_go");

	connect(m_URLHistory, SIGNAL(returnPressed()), this, SLOT(changeDirectory()));

	//------------------------------------------------------------------------------
	m_zoomCombo=new KComboBox(/*this*/);
	m_zoomCombo->insertItem("10 %"); m_zoomCombo->insertItem("25 %"); m_zoomCombo->insertItem("33 %");
	m_zoomCombo->insertItem("50 %"); m_zoomCombo->insertItem("67 %"); m_zoomCombo->insertItem("75 %");
	m_zoomCombo->insertItem("100 %"); m_zoomCombo->insertItem("150 %"); m_zoomCombo->insertItem("200 %");
	m_zoomCombo->insertItem("300 %"); m_zoomCombo->insertItem("400 %"); m_zoomCombo->insertItem("600 %");
	m_zoomCombo->insertItem("800 %"); m_zoomCombo->insertItem("1000 %"); m_zoomCombo->insertItem("1200 %");
	m_zoomCombo->insertItem("1600 %"); m_zoomCombo->setEditable ( true );
	m_zoomCombo->setInsertionPolicy ( QComboBox::NoInsertion );
	m_zoomCombo->setDuplicatesEnabled( false );
	KWidgetAction* zoomComboAction=new KWidgetAction( m_zoomCombo, i18n("Zoom"),
			0,
			0, 0,
			m_actions, "zoomComboAction");
	zoomComboAction->setShortcutConfigurable(false);
	zoomComboAction->setAutoSized(false);
	connect(m_zoomCombo, SIGNAL(activated ( const QString& )), this, SLOT(setZoom( const QString& )));

	//------------------------------------------------------------------------------
 	// Non configurable stop-fullscreen accel
	QAccel* accel=m_actions->accel();
	accel->connectItem(accel->insertItem(Key_Escape), this, SLOT(escapePressed()));
	accel->connectItem(accel->insertItem(Key_Space), this, SLOT(spacePressed()));

	// Dock related
	connect(manager(), SIGNAL(change()), this, SLOT(updateWindowActions()) );

	//------------------------------------------------------------------------------
	aTime = new KAction(QString::null, (const KShortcut&)0,
						this, SLOT(slotDisplayNBImg()), m_actions, "time");

	//------------------------------------------------------------------------------
	m_windowListActions.setAutoDelete(true);
	updateWindowActions();
	//------------------------------------------------------------------------------

#ifdef WANT_LIBKEXIDB
	connect(getImageListView(), SIGNAL(fileIconRenamed(const QString&, const QString&)),
			getCategoryView(), SLOT(fileIconRenamed(const QString&, const QString&)) );
	connect(getImageListView(), SIGNAL(fileIconsDeleted(const KURL::list&)),
			getCategoryView(), SLOT(fileIconsDeleted(const KURL::list&)) );

	connect(getDirectoryView(), SIGNAL(renameListItemDone(const KURL&, const KURL&)),
			getCategoryView(), SLOT(directoryRenamed(const KURL&, const KURL&)) );
	connect(getDirectoryView(), SIGNAL(moveFilesDone(const KURL::List&, const KURL&)),
			getCategoryView(), SLOT(filesMoved(const KURL::List&, const KURL&)) );
#endif /* WANT_LIBKEXIDB */

	//------------------------------------------------------------------------------
	getImageListView()->initActions(m_actions);
	getDirectoryView()->initActions(m_actions);
	getImageViewer()->initActions(m_actions);
	getToolManager()->initActions(m_actions);
#ifdef WANT_LIBKEXIDB
	getCategoryView()->initActions(m_actions);
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	getCDArchiveView()->initActions(m_actions);
#endif /* SHOWIMG_NO_CDARCHIVE */

}


void
MainWindow::createMenus()
{
//	MYDEBUG<<endl;
	getImageListView()->initMenu(m_actions);
	getDirectoryView()->initMenu(m_actions);
	getImageViewer()->initMenu(m_actions);
#ifdef WANT_LIBKEXIDB
	catView->initMenu(m_actions);
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	cdarcView->initMenu(m_actions);
#endif /* SHOWIMG_NO_CDARCHIVE */
//	MYDEBUG<<endl;

}

void
MainWindow::readConfig(KConfig *config)
{
	getImageViewer()->readConfig(config, CONFIG_IMAGEVIEWER_GROUP);
	getImageListView()->readConfig(config);
	getDirectoryView()->readConfig(config);
	getToolManager()->readConfig(config);

	//------------------------------------------------------------------------------
// 	MYDEBUG<<endl;
	config->setGroup("Slideshow");
	slideshowTime=config->readNumEntry("time", 2);
	slideshowType=config->readNumEntry("type", 0);

	config->setGroup("Options");
	aPreview->setChecked(config->readBoolEntry("preview", true));
	aTime->setText(i18n("1 image seen", "%n images seen", getImageViewer()->getNbImg()));

	openDirType=config->readNumEntry("openDirType", 0);
	openDirname=config->readPathEntry("openDirname", QDir::homeDirPath());
	showSP=config->readBoolEntry("showSP", true);
	startFS=config->readBoolEntry("startFS", true);
	showToolbar=config->readBoolEntry("showToolbar", false);
	showStatusbar=config->readBoolEntry("showStatusbar", false);

	config->setGroup("Paths");
	cdromPath = config->readPathEntry("cdromPath", "/mnt/cdrom");

	timer = new QTimer (this);
	if(slideshowType == 0 )
		connect (timer, SIGNAL (timeout ()), getImageListView(), SLOT (next ()));
	else
		connect (timer, SIGNAL (timeout ()), getImageListView(), SLOT (previous()));

	config->setGroup("TipOfDay");
	if(config->readBoolEntry("RunOnStart", true))
		slotShowTips();

	readDockConfig (config, CONFIG_DOCK_GROUP);

// 	slotIconSize();
// 	MYDEBUG<<endl;
}

void
MainWindow::writeConfig(KConfig *config)
{
	if(!inInterface) return;

	config->setGroup("Options");
	config->writeEntry( "preview", aPreview->isChecked() );
	config->writeEntry( "openDirType", openDirType );
	if(openDirType==1) openDirname=getCurrentDir();
	config->writePathEntry( "openDirname",  openDirname);
	config->writeEntry( "showSP",  showSP);
	config->writeEntry( "startFS",  startFS);
	config->writeEntry( "showToolbar",  showToolbar);
	config->writeEntry( "showStatusbar",  showStatusbar);

	config->setGroup("Paths");
	config->writeEntry( "cdromPath", getcdromPath() );

	config->setGroup("Slideshow");
	config->writeEntry( "time", slideshowTime );
	config->writeEntry( "type", slideshowType );

	KMainWindow::saveMainWindowSettings(config);
	KMainWindow::saveWindowSize(config);
	writeDockConfig(config, CONFIG_DOCK_GROUP);

	config->sync();
}

void
MainWindow::createHideShowAction(KDockWidget* dock) {
	QString caption;
	if (dock->mayBeHide()) {
		caption=i18n("Hide %1").arg(dock->caption());
	} else {
		caption=i18n("Show %1").arg(dock->caption());
	}

	KAction* action=new KAction(caption, 0, dock, SLOT(changeHideShowState()), actionCollection() );
	if (dock->icon()) {
		action->setIconSet( QIconSet(*dock->icon()) );
	}
	m_windowListActions.append(action);
		}


void
MainWindow::updateSelections(ListItem *item)
{
	setUpdatesEnabled(false);

	if(currentListItemView && item)
	{
		if(item->isSelected() && currentListItemView !=  item->getListItemView() && !item->getListItemView()->isDropping())
		{
			currentListItemView->clearSelection();
			currentListItemView=item->getListItemView();
		}
	}
#ifdef WANT_LIBKEXIDB
	getCategoryView()->updateActions(item);
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	getCDArchiveView()->updateActions(item);
#endif /* SHOWIMG_NO_CDARCHIVE */
	getDirectoryView()->updateActions(item);

	setUpdatesEnabled(true);
}


//------------------------------------------------------------------------------
void
MainWindow::updateWindowActions()
{
	unplugActionList("winlist");
	m_windowListActions.clear();
	createHideShowAction(dockIL);
	createHideShowAction(dockIV);
	createHideShowAction(dockDir);
	createHideShowAction(dockIMI);
	plugActionList("winlist", m_windowListActions);
}


void
MainWindow::setActionsEnabled(bool enable)
{
	int count=m_actions->count();
	for (int pos=0;pos<count;++pos)
		m_actions->action(pos)->setEnabled(enable);

}

void
MainWindow::setMessage(const QString& msg)
{
	statusBar()->changeItem(msg, SB_MSG);
}


void
MainWindow::setImageIndex (int index)
{
	imageIndex=index;
	setNbrItems(nbrItems);
}

void
MainWindow::setNbrItems (int nbr)
{
	nbrItems=nbr;

	QString msg;
	if(nbr==0)
		msg =i18n("no item");
	else
		if(imageIndex>=0&&!getDirectoryView()->showAllFile()&&!getDirectoryView()->showDir())
		msg =i18n("%2/%n item", "%2/%n items", nbrItems).arg(imageIndex+1);
	else
		msg =i18n("%n item", "%n items", nbrItems);

	statusBar()->changeItem(" "+msg+" ", SB_ITEMS);
}

void
MainWindow::setZoom (float zoom)
{
	if(!m_zoomCombo) return;

	QString nb; nb.setNum(zoom,'f',0);
	statusBar()->changeItem(QString(" %1% ").arg(nb), SB_SCALE );
	m_zoomCombo->disconnect( SIGNAL( activated ( const QString& ) ) );
	m_zoomCombo->setCurrentText(nb + " %");
	connect(m_zoomCombo, SIGNAL(activated ( const QString& )), this, SLOT(setZoom( const QString& )));
}

void
MainWindow::setZoom ( const QString& val)
{
	QRegExp reg("(\\d*)");
	reg.search(val);
	QStringList list = reg.capturedTexts();
	bool ok;
	float v = QString(list[1]).toInt(&ok);
	if(ok)
		getImageViewer()->setZoomValue(v/100);
}

void
MainWindow::setImagename (const QString& name)
{
	SB_NAME_Label->setText(name);
}

void
MainWindow::setImagetype (const QString& type)
{
	statusBar()->changeItem(" "+type.upper()+" ", SB_TYPE );
}


void
MainWindow::setDim (const QSize& size, float dpi)
{
	if(!size.isEmpty())
	{
		QString msg;
		statusBar()->changeItem(i18n("%1 x %2 (%3 dpi) ")
				.arg(size.width())
				.arg(size.height())
				.arg((int)(dpi!=0?ceil(dpi):72)),
			SB_IMG_SIZE );
	}
	else
		statusBar()->changeItem(QString::null, SB_IMG_SIZE );
}

void
MainWindow::setSize (int size)
{
	if(size>=0)
	{
		statusBar()->changeItem(QString("%1").arg(KGlobal::locale()->formatNumber(size, 0)), SB_BYTES);
	}
	else
		statusBar()->changeItem(QString::null, SB_BYTES);
}

void
MainWindow::setDate (QDateTime *date)
{
	if(!date)
		setDate(QString::null);
	else
		setDate(KGlobal::locale()->formatDateTime(*date, false));
}
void
MainWindow::setDate (const QString& date)
{
	SB_DATE_Label->setText(date);
}


void
MainWindow::changeDirectory()
{
	KURL url(m_URLHistoryCompletion->replacedPath(m_URLHistory->currentText()));
	if (!url.protocol().compare("http") || !url.protocol().compare("ftp"))
	{
		QString tmpFile = locateLocal("tmp", "showimg-net/");
		tmpFile += url.filename();
		if( KIO::NetAccess::download( url, tmpFile, this ) )
		{
			openDir(tmpFile);
		}
		else
		{
			openDir(getCurrentDir());
		}
	}
	else
	{
		openDir(url.path());
	}
}


void
MainWindow::forwardMenuActivated(int item)
{
	go(aForward->popupMenu()->indexOf(item)+1);
}

bool
MainWindow::preview()
{
	return aPreview->isChecked();
}


QString
MainWindow::getFileName(QString *fullName)
{
		int debut = fullName->findRev ("/");
		int fin = fullName->findRev (".");
		return fullName->mid(debut+1, fin-debut-1);
}
QString
MainWindow::getFileExt(QString *fullName)
{
		int pos = fullName->findRev (".");
		return  fullName->right (fullName->length () - pos-1);
}
QString
MainWindow::getFullName(QString *fullName)
{
		int pos = fullName->findRev ("/");
		return fullName->right (fullName->length () - pos-1);
}
QString
MainWindow::getFullPath(QString *fullName)
{
		int pos = fullName->findRev ("/");
		return fullName->left(pos+1);
}


ListItem*
MainWindow::findDir(QString dir)
{
	if(QFileInfo(dir).isDir()&&!dir.endsWith("/")) dir+="/";
	ListItem *item=NULL;
	item = getDirectoryView()->getDir(dir);
	if(!item) item = getCDArchiveView()->getCDArchiveItem(dir);
	return item;
}


bool
MainWindow::openDir (const QString& dir, bool updateHistory_, bool loadThumbnails)
{
	if(!inInterface) return false;

	QString picName;
	QString _dir_;

	if(ListItemView::isImage(dir))
	{
		picName=QFileInfo(dir).fileName();
		_dir_=QFileInfo(dir).dirPath(true);
	}
	else
	{
		_dir_ = QDir::convertSeparators(dir);
	}

	ListItem *ssrep = NULL;
	ListItem *f_ssrep = NULL;
	QFileInfo info( _dir_ );
#ifdef Q_WS_WIN
	QStringList list = QStringList::split('/', info.absFilePath().mid(3) /* omit drive letter */ );
#else
	QStringList list = QStringList::split('/', info.absFilePath() );
#endif
	if(info.exists()
#ifndef SHOWIMG_NO_CDARCHIVE
		&& !_dir_.startsWith(CDArchive_ROOTPATH)
		&& !_dir_.startsWith(CDArchive::CDArchive_TEMP_ROOTPATH())
#endif
	)
	{
		ssrep = root;
		bool found = false;
#ifdef Q_WS_WIN //for win32 it can be "Desktop" root item:
		if (_dir_==desktopDirPath())
		{
			//nothing to do
			found = true;
		}
		if (!found) {
			//try to find a drive for this dir
			while (ssrep && !_dir_.startsWith( QDir::convertSeparators( ssrep->fullName() ) ))
			{
				ssrep = ssrep->nextSibling();
			}
			if (!ssrep)
				return false;
		}
#endif
		if (!found)
		{
			QStringList::iterator it;
			for ( it = list.begin(); it != list.end(); ++it )
			{
				f_ssrep = ssrep->find ( *it );
				if ( ! f_ssrep )
					f_ssrep = new Directory((Directory*)ssrep, *it, this);
				ssrep = f_ssrep;
				ssrep->setOpen(true);
			}
		}
	}
	else
	{
#ifndef SHOWIMG_NO_CDARCHIVE
		if( cdArchiveRoot &&
			(_dir_.startsWith(CDArchive::CDArchive_TEMP_ROOTPATH()) || _dir_.startsWith(CDArchive_ROOTPATH)))
		{
			ssrep = cdArchiveRoot->find(_dir_);
		}
#endif
		if(ssrep)
			ssrep->setOpen(true);
	}

	if (ssrep)
	{
		if(getDirectoryView()) getDirectoryView()->setLoadThumbnails(loadThumbnails);
		if(info.absFilePath () != QFileInfo(getCurrentDir()).absFilePath())
		{

			getDirectoryView()->clearSelection();
			getDirectoryView()->slotShowItem(ssrep);
			getDirectoryView()->setCurrentItem(ssrep);
			getDirectoryView()->setSelected(ssrep, true);

			setCaption(_dir_);
			setCurrentDir(_dir_);

			m_sideBar->switchToTab(m_sideBar_id_dirView);
		}
		if(updateHistory_)
		{
			updateHistory();
		}
		if(!picName.isEmpty()) getImageListView()->setCurrentItemName(picName);
		return true;
	}
	else
	{
		showUnableOpenDirectoryError(dir);
		return false;
	}
}

void
MainWindow::nextDir (ListItem *r)
{
	if (r == 0)
	{
		return;
	}

	if(!r->fullName ())
	{
		return;
	}
	ListItem *dir = r;
	while (dir != 0)
	{
		if (dir->getType() != "Directory")
		{
			dir = 0;
		}
		else
		{
			if (dir->isSelected ())
			{
				dir->unLoad ();
				dir->load ();
			}

			if (dir->firstChild ())
			{
				nextDir (dir->firstChild ());
			}
		}
		dir = dir->itemBelow ();
	}
}



void
MainWindow::slotRefresh (const QString& dir)
{
	setMessage(i18n("Refreshing..."));

	QString res = QString(dir);
	int  pos = res.find ("/");
	ListItem *ssrep;

	res = res.right (res.length () - pos - 1);
	pos = res.find ("/");

	ssrep = root;
	while (pos != -1)
	{
		ssrep = ssrep->find (res.left (pos));
		res = res.right (res.length () - pos - 1);

		if (ssrep)
			pos = res.find ("/");
		else
			break;
	}
	ssrep = ssrep->find (res);
	if (ssrep && ssrep->isSelected ())
	{
		ssrep->refresh();
	}

	setMessage(i18n("Ready"));
}


void
MainWindow::slotRefresh (bool forceRefresh)
{
	setMessage(i18n("Refreshing..."));
	getImageListView()->stopLoading();

	QPtrList<ListItem> list;
	QListViewItemIterator it (root);
	for (; it.current (); ++it)
	{
		if (it.current ()->isSelected ())
			list.append((ListItem*)(it.current()));
	}
	for ( ListItem *dir=list.first(); dir != 0; dir=list.next() )
	{
		if(!forceRefresh)
		{
			dir->refresh(false);
		}
		else
		{
			dir->unLoad();
			dir->load();
		}
	}
	getImageListView()->reload();
	getImageListView()->slotLoadFirst();
	setMessage(i18n("Ready"));
}



void
MainWindow::setHasImageSelected(bool selected)
{
	hasimageselected=selected;

	aCopy->setEnabled(selected);
	aCopyPixmap->setEnabled(selected);
	aEditType->setEnabled(selected);
	aFullScreen->setEnabled(selected);

	getImageViewer()->selectionChanged(selected);

	FileIconItem* si = getImageListView()->firstSelected();
	if(si)
	{
		if(si->getType() != "file" && si->getType() != "dir")
		{
			aPaste->setEnabled(false);
				//
			if(si->getType()=="filealbum")
			{
				m_actions->action("editdelete")->setText(i18n("Remove From Album"));
			}
			else
			if(si->getType()=="zip")
			{
				m_actions->action("editdelete")->setText(i18n("Remove From Archive"));
			}
			else
			{
				m_actions->action("editdelete")->setEnabled(false);
			}
		}
		else
		{
			m_actions->action("editdelete")->setText(i18n("Delete File"));
		}
		if(!getImageListView()->hasOnlyOneImageSelected())
		{
			aEditType->setEnabled(false);
			m_actions->action("EXIF orientation")->setEnabled(false);
		}
		#ifdef HAVE_LIBKEXIF
		else
		{
			m_actions->action("EXIF actions")->setEnabled(si->mimetype()=="image/jpeg");
			m_actions->action("EXIF orientation")->setEnabled(si->mimetype()=="image/jpeg");
		}
		#endif
	}




}


void
MainWindow::slotPreview ()
{
	getImageListView()->setThumbnailSize(false);
	if(aPreview->isChecked())
	{
		getImageListView()->slotLoadFirst();

		m_actions->action("Regenerate EXIF thumbnail")->setEnabled(true);
		m_actions->action("Regenerate thumbnail")->setEnabled(true);
	}
	else
	{
		slotStop();
		getImageListView()->slotResetThumbnail();

		m_actions->action("Regenerate EXIF thumbnail")->setEnabled(false);
		m_actions->action("Regenerate thumbnail")->setEnabled(false);
	}
}

void
MainWindow::slotSlideShow(int time)
{
	slideshowTime = time;
	slotSlideShow();
}

void
MainWindow::slotSlideShow ()
{
	if(!timer) return;
#ifdef HAVE_KIPI
	if (pluginManager() && pluginManager()->action("SlideShow...") && aSlideshow->isChecked())
	{
		pluginManager()->action("SlideShow...")->activate();
		aSlideshow->setChecked(false);
		return;
	}
#endif /* HAVE_KIPI */
	MYDEBUG<<endl;
	if (!timer->isActive())
	{
		MYDEBUG<<endl;
		if(!getImageListView()->hasImageSelected())
		{
			MYDEBUG<<endl;
			getImageListView()->first();
			if(!getImageListView()->hasImageSelected())
			{
				MYDEBUG<<endl;
				aSlideshow->setChecked(false);
				return;
			}
		}
		MYDEBUG<<endl;
		KApplication::setOverrideCursor (KCursor::blankCursor());
		timer->start (slideshowTime*1000, false);
		aSlideshow->setChecked(false);
		if(!inFullScreen) slotFullScreen();
	}
	else
		timer->stop ();
}


void
MainWindow::escapePressed()
{
	if(!inInterface)
	{
		requestingClose=true;
		hide();
		if(queryClose()) kapp->quit();
		return;
	}

	if(fullScreen())
	{
		slotFullScreen();
	}
	else
	{
		slotStop();
	}
}


void
MainWindow::spacePressed()
{
	if(fullScreen())
	{
		if(!getImageViewer()->scrollDown ())
			getImageListView()->next();
	}
}


void
MainWindow::switchToInterface()
{
	MYDEBUG<<endl;
	if(inInterface) return;

	hide();

	QString currentPath = imageListSimple->currentAbsImagePath();
	iv->deleteLater(); iv=NULL;
	imageListSimple->deleteLater(); imageListSimple=NULL;

	KStartupLogo* logo=NULL;
	m_config = KGlobal::config();
	m_config->setGroup("Options");
	if( m_config->readBoolEntry("showSP", true))
	{
		logo = new KStartupLogo();
		logo->show();
	}
	m_mw = new MainWindow(currentPath, false, true);
	if(logo)
	{
		logo->hide();
		delete logo;
	}
	MYDEBUG<<endl;
	//m_mw->getDirectoryView()->setLoadThumbnails(false);
// 	m_mw->openDir(currentPath, true, false);
// 	m_mw->getDirectoryView()->setLoadThumbnails(true);
	MYDEBUG<<endl;

	inInterface=true;
	finalizeGUI(this);
}


void
MainWindow::closeEvent( QCloseEvent* e)
{
	requestingClose=true;
	KMainWindow::closeEvent(e);
}


bool
MainWindow::closeAppl()
{
	requestingClose=true;
	return true;
}


bool
MainWindow::queryClose()
{
//	MYDEBUG<<endl;
	if(deleteTempDirectoriesDone_)
	{
//		MYDEBUG<<endl;
		if(inFullScreen) slotFullScreen();
		if(getImageViewer()) getImageViewer()->writeConfig(m_config, CONFIG_IMAGEVIEWER_GROUP);
		if(getImageListView()) getImageListView()->writeConfig(m_config);
		if(getDirectoryView()) getDirectoryView()->writeConfig(m_config);
		if(getToolManager()) getToolManager()->writeConfig(m_config);
		writeConfig(m_config);

		m_config->sync();
		return true;
	}
	else
	{
		deleteTempDirectories();
		return false;
	}
}


void
MainWindow::slotFullScreen()
{
	if(!inInterface) return;
	if(!getImageViewer()->hasImage()) return;
	hide();
	setUpdatesEnabled(false);

	if (!inFullScreen )
	{
		inFullScreen = true;

		writeDockConfig (m_config, CONFIG_DOCK_GROUP);

		makeDockInvisible(dockIL);
		makeDockInvisible(dockDir);
		makeDockInvisible(dockIMI);

		leftDock()->hide(); rightDock()->hide();

		toolBar("locationToolBar")->hide(); menuBar()->hide();
		if(!showToolbar)
		{
			toolBar("mainToolBar")->hide(); toolBar("viewToolBar")->hide();
			topDock()->hide();
		}
		if(!showStatusbar)
		{
			statusBar()->hide();
			bottomDock()->hide();
		}

		aBack->setEnabled(false);
		aForward->setEnabled(false);
		aGoHome->setEnabled(false);
		aGoUp->setEnabled(false);
		aGo->setEnabled(false);
		aPreview->setEnabled(false);

		getImageViewer()->setBackgroundColor(QColor("black"));

		setUpdatesEnabled(true);
		showFullScreen();
		KWin::setState(winId(), NET::StaysOnTop);
		getImageViewer()->setFocus();
		KWin::raiseWindow(winId());
		kapp->processEvents();
		emit toggleFullscreen(inFullScreen);
	}
	else
	{
		inFullScreen = false;
		emit toggleFullscreen(inFullScreen);

		getImageViewer()->setBackgroundColor(bgColor);

		topDock()->show();
		menuBar()->show();

		bottomDock()->show();
		toolBar("mainToolBar")->show();
		toolBar("viewToolBar")->show();
		toolBar("locationToolBar")->show();

		statusBar()->show();

		///
		readDockConfig (m_config, CONFIG_DOCK_GROUP);

		///
		aBack->setEnabled(true);
		aForward->setEnabled(true);
		aGoHome->setEnabled(true);
		aGoUp->setEnabled(true);
		aGo->setEnabled(true);
		aPreview->setEnabled(true);
		aBack->setEnabled(true);
		aForward->setEnabled(true);
		aGoHome->setEnabled(true);
		aGo->setEnabled(true);
		aGoUp->setEnabled(true);

		if(timer->isActive()) {timer->stop(); aSlideshow->setChecked(false); KApplication::restoreOverrideCursor ();}

		getDirectoryView()->setLoadThumbnails(true);

		KWin::setState(winId(), NET::Normal);
		setUpdatesEnabled(true);
		showNormal();
	}
	aFullScreen->setChecked(inFullScreen);
}


void
MainWindow::slotAddImage (int number)
{
	total+=number;
	setNbrItems(total);
	progress->setTotalSteps (total);
}


void
MainWindow::slotRemoveImage ()
{
	total--;setNbrItems(total);
	progress->setTotalSteps (total);
}


void
MainWindow::slotPreviewDone (int number)
{
	done+=number;
	progress->setProgress (done);
}


void
MainWindow::slotReset (bool init)
{
	aStop->setEnabled(true);
	if(init)
	{
		done = 0;
		progress->setProgress (-1);
	}
	if(total>=1) progress->show();
}


void
MainWindow::slotDone()
{
	aStop->setEnabled(false);
	done = total;
	progress->hide();

	if(aPreview->isChecked())
	{
		m_actions->action("Regenerate thumbnail")->setEnabled(true);
		m_actions->action("Regenerate EXIF thumbnail")->setEnabled(true);
	}
}


void
MainWindow::slotRemoveImage (int val)
{
	total -= val;setNbrItems(total);
	progress->setTotalSteps (total);
	progress->setProgress (total);
}


void
MainWindow::slotDirChange (const QString& path)
{
	MYDEBUG<<path<<endl;
	if(QFileInfo(path).isDir())
	{
		ListItem* d = findDir(path);
		if(d)
		{
			if(d->refresh())
			{
				MYDEBUG<<path<<endl;
				getImageListView()->slotLoadFirst();
			}
		}
	}
}


void
MainWindow::slotDirChange_created(const QString& path)
{
	MYDEBUG<<path<<endl;
}


void
MainWindow::slotDirChange_deleted(const QString& path)
{
	MYDEBUG<<path<<endl;
	getDirectoryView()->removeDir(path);
}


void
MainWindow::slotTODO ()
{
	KMessageBox::sorry(this,
		i18n("Not yet implemented.\nSorry ;("),
		i18n("Functionality"));
}

void
MainWindow::slotcopyPixmap()
{
	KApplication::setOverrideCursor (waitCursor); // this might take time
	KApplication::clipboard()->setPixmap(getImageViewer()->getPixmap());
	KApplication::restoreOverrideCursor ();
}

void
MainWindow::slotcopy ()
{
	QString files, name;
	KURL::List list;

	for (FileIconItem* item = getImageListView()->firstItem(); item != 0; item = item->nextItem())
	{
		if (item->isSelected () )
		{
			list.append(item->getURL());
		}
	}
	QClipboard *cb = KApplication::clipboard();
	cb->setData(new KURLDrag(list, this, "MainWindow"));
}


void
MainWindow::slotcut ()
{
	slotTODO ();
}

void
MainWindow::slotpaste ()
{
	KURL::List uris;
	if(KURLDrag::decode(KApplication::clipboard()->data(), uris))
	{
		if(!uris.isEmpty())
			getDirectoryView()->copy(uris.toStringList(), currentTitle());
	}
}


void
MainWindow::slotNewWindow()
{
	(void)new MainWindow(getCurrentDir().ascii());
}


void
MainWindow::slotOpenLocation()
{
	QString destDir=KFileDialog::getExistingDirectory(getCurrentDir(),
								this,
								i18n("Open Location"));

	if(destDir.isEmpty())
		return;

	if(!QFileInfo(destDir).exists())
	{
		KMessageBox::error(this, "<qt>"+i18n("The directory '<b>%1</b>' does not exist").arg(destDir)+"</qt>");
		return;
	}
	openDir(destDir);
	changeDirectory(destDir);
}


void
MainWindow::slotUndo()
{
	slotTODO ();
}


void
MainWindow::slotRedo()
{
	slotTODO ();
}



void
MainWindow::slotEditFileType()
{
	if(!getImageListView()->currentItem())
		return;
#ifndef Q_WS_WIN
	KonqOperations::editMimeType( getImageListView()->currentItem()->mimetype());
#endif
}


void
MainWindow::slotStop()
{
	getImageListView()->stopLoading ();
	slotDone();
}


void
MainWindow::slotShowTips()
{
  KTipDialog::showTip( this, "showimg/tips", true );
}


void
MainWindow::configureKey()
{
	KKeyDialog::configure(m_actions, this);
}


void
MainWindow::configureToolbars()
{
	KMainWindow::configureToolbars();
}


void
MainWindow::configureShowImg()
{
	ConfShowImg conf(this);
	conf.initColor(getImageViewer()->bgColor(), getImageViewer()->toGrayscale());
	conf.initFiling(openDirType, openDirname, showSP, startFS);
	conf.initMiscellaneous(getImageViewer()->smooth(), getDirectoryView()->loadFirstImage(), getDirectoryView()->showHiddenDir(), getDirectoryView()->showHiddenFile(), getDirectoryView()->showDir(), getDirectoryView()->showAllFile(), getImageListView()->preloadIm());
	conf.initThumbnails(getImageListView()->il->getStoreThumbnails(), getImageListView()->il->getShowFrame(), getImageViewer()->useEXIF(), getImageListView()->wordWrapIconText(), getImageListView()->getShowMimeType(), getImageListView()->getShowSize(), getImageListView()->getShowDate(), getImageListView()->getShowDimension(), getImageListView()->getShowToolTips());
	conf.initSlideshow(slideshowType, slideshowTime);
	conf.initFullscreen(showToolbar, showStatusbar);
	conf.initOSD(getImageListView()->getOSD()->getShowOSD(), getImageListView()->getOSD()->getOSDOnTop(), getImageListView()->getOSD()->font(),  getImageListView()->getOSD()->getOSDShowFilename(), getImageListView()->getOSD()->getOSDShowFullpath(), getImageListView()->getOSD()->getOSDShowDimensions(), getImageListView()->getOSD()->getOSDShowComments(), getImageListView()->getOSD()->getOSDShowDatetime(), getImageListView()->getOSD()->getOSDShowExif());
	conf.initProperties(getImageListView()->showMeta(), getImageListView()->showHexa());
	conf.initPaths(getcdromPath(), getImageListView()->getgimpPath(), getToolManager()->getConvertPath(), getToolManager()->getJpegtranPath());
	conf.initImagePosition(getImageViewer()->getImagePosition());

	if(conf.exec())
	{
		openDirType=conf.getOpenDirType();
		openDirname=conf.getOpenDir();
		showSP=conf.checkshowSP();
		startFS=conf.checkstartFS();
		getImageViewer()->setUseEXIF(conf.getUseEXIF());
		showToolbar=conf.getShowToolbar();
		showStatusbar=conf.getShowStatusbar();

		getImageListView()->setShowMimeType(conf.getShowMimeType());
		getImageListView()->setShowSize(conf.getShowSize());
		getImageListView()->setShowDate(conf.getShowDate());
		getImageListView()->setShowDimension(conf.getShowDimension());
		getImageListView()->setWordWrapIconText(conf.getWordWrapIconText());
		getImageListView()->setShowToolTips(conf.getShowTooltips());

		getDirectoryView()->setShowHiddenDir(conf.getShowHiddenDir());
		getDirectoryView()->setShowHiddenFile(conf.getShowHiddenFile());
		getDirectoryView()->setShowDir(conf.getShowDir());
		getDirectoryView()->setLoadFirstImage(conf.getLoadFirstImage());
		getDirectoryView()->setShowAllFile(conf.getShowAll());

		getImageListView()->setPreloadIm(conf.getPreloadIm());
		getImageListView()->setRandom(conf.getSlideshowType()==2);
		getImageListView()->setShowMeta(conf.getShowMeta());
		getImageListView()->setShowHexa(conf.getShowHexa());

		getImageViewer()->setBackgroundColor(conf.getColor());
		getImageViewer()->setToGrayscale(conf.getGrayscale());
		getImageViewer()->setSmooth(conf.getSmooth());

		slideshowTime=conf.getSlideshowTime();
		slideshowType=conf.getSlideshowType();
		delete(timer); timer = new QTimer (this);
		if(slideshowType == 0 )
			connect (timer, SIGNAL (timeout ()), getImageListView(), SLOT (next ()));
		else
			connect (timer, SIGNAL (timeout ()), getImageListView(), SLOT (previous()));


		getImageListView()->il->setStoreThumbnails(conf.getStoreth());
		getImageListView()->il->setShowFrame(conf.getShowFrame());
		getImageListView()->il->setUseEXIF(getImageViewer()->useEXIF());
		getImageListView()->getOSD()->initOSD(conf.getShowOSD(), conf.getOSDOnTop(), conf.getOSDFont(), conf.getOSDShowFilename(), conf.getOSDShowFullpath(), conf.getOSDShowDimensions(), conf.getOSDShowComments(), conf.getOSDShowDatetime(), conf.getOSDShowExif());

		setLayout(conf.getLayout());

#ifdef HAVE_KIPI
		conf.applyPlugins();
		pluginManager()->loadPlugins();
#endif /* HAVE_KIPI */

		getImageListView()->selectionChanged();
		getDirectoryView()->slotSelectionChanged();

		cdromPath = conf.getcdromPath();
		getImageListView()->setgimpPath(conf.getgimpPath());
		getToolManager()->setConvertPath(conf.getconvertPath());
		getToolManager()->setJpegtranPath(conf.getjpegtranPath());

		getImageViewer()->setImagePosition(conf.getImagePosition());

		slotRefresh(true);
	}
}


void
MainWindow::setLayout(int layout)
{
	switch(layout)
	{
		case 1:
				dockDir->manualDock(dockIV, KDockWidget::DockLeft, 35);
				dockIL->manualDock(dockDir, KDockWidget::DockBottom, 35);
				//aArrangementB->setChecked(true);
				//aArrangementLR->setChecked(true);
			break;
		case 2:
				dockDir->manualDock(dockIV, KDockWidget::DockTop, 35);
				dockIL->manualDock(dockDir, KDockWidget::DockRight, 50);
				//aArrangementB->setChecked(true);
				//aArrangementLR->setChecked(true);
			break;
		case 3:
				dockIL->manualDock(dockIV, KDockWidget::DockRight, 35);
				dockDir->manualDock(dockIV, KDockWidget::DockTop, 35);
				//aArrangementB->setChecked(true);
				//aArrangementLR->setChecked(true);
			break;
		case 4:
				dockDir->manualDock(dockIV, KDockWidget::DockLeft, 35);
				dockIL->manualDock(dockIV, KDockWidget::DockTop, 10);
				//aArrangementB->setChecked(true);
				//aArrangementTB->setChecked(true);
			break;
		default:
			return;
			break;
	}
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
	bookmarkmanager
*/
void
MainWindow::openBookmarkURL(const QString& _url)
{
	KURL url(_url);
	if(!url.isLocalFile())
	{
		KMessageBox::error(this, "<qt>"+i18n("The directory '<b>%1</b>' is not local").arg(url.url())+"</qt>");
		return;
	}
	else
	if(!QFileInfo(url.path()).exists())
	{
		KMessageBox::error(this, "<qt>"+i18n("The directory '<b>%1</b>' does not exist").arg(url.url())+"</qt>");
		return;
	}
	openDir(url.path());
}


QString
MainWindow::currentTitle() const
{
	return getCurrentDir();
}


QString
MainWindow::currentURL() const
{
	return "file:"+getCurrentDir();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::addToBookmark(const QString& groupText, const QString& url)
{
	KBookmarkGroup root = MyBookmarkManager::self()->root();
	KBookmark bookm;
	bool bmAlbumExists = false;
	for(bookm = root.first(); !bookm.isNull(); bookm = root.next(bookm))
	{
		if(bookm.text() == groupText)
		{
			bmAlbumExists=true;
			break;
		}
	}
	KBookmarkGroup bmg;
	if(!bmAlbumExists)
	{
		bmg = MyBookmarkManager::self()->root().createNewFolder(MyBookmarkManager::self(), groupText);
		MyBookmarkManager::self()->root().moveItem(bmg, KBookmarkGroup());
	}
	else
	{
		bmg=bookm.toGroup();
	}
	bmg.addBookmark(MyBookmarkManager::self(), url, url, KMimeType::iconForURL(url));
	MyBookmarkManager::self()->emitChanged(root);
}


void
MainWindow::changeDirectory(QString dir_)
{
	QString dir(QDir::convertSeparators(dir_));
	setCaption(dir);
	setCurrentDir(dir);
	updateHistory();
}


void
MainWindow::backMenuActivated(int item)
{
	go(-(aBack->popupMenu()->indexOf(item)+1));
}


void
MainWindow::slotBack()
{
	go(-1);
}


void
MainWindow::slotForward()
{
	go(+1);
}


void MainWindow::go( int steps )
{
	updateHistory();

	int newPos = m_lstHistory.at() + steps;
	HistoryEntry* l = m_lstHistory.at( newPos );
	if(openDir(l->filePath, false))
	{
		aBack->setEnabled( m_lstHistory.at() > 0 );
		aForward->setEnabled( m_lstHistory.at() != ((int)m_lstHistory.count())-1 );
	}
}


void
MainWindow::updateHistory()
{
	KURL url;
	url.setPath(getCurrentDir());

	m_URLHistoryCompletion->addItem(url.prettyURL(/*0, KURL::StripFileProtocol*/));
	m_URLHistory->setEditText(url.prettyURL(/*0, KURL::StripFileProtocol*/));
	m_URLHistory->addToHistory(url.prettyURL(/*0, KURL::StripFileProtocol*/));

	HistoryEntry * current = m_lstHistory.current();
	HistoryEntry* newEntry = new HistoryEntry;
	newEntry->filePath = url.path();
	if (current && current->filePath == newEntry->filePath)
	{
		delete newEntry;
		return;
	}
	if (current)
	{
		m_lstHistory.at( m_lstHistory.count() - 1 ); // go to last one
		for ( ; m_lstHistory.current() != current ; )
		{
			m_lstHistory.removeLast();
		}
	}
	m_lstHistory.append(newEntry);
	aBack->setEnabled( m_lstHistory.at() > 0 );
	aForward->setEnabled( m_lstHistory.at() != ((int)m_lstHistory.count())-1 );
}


void
MainWindow::slotForwardAboutToShow()
{
	aForward->popupMenu()->clear();
	HistoryAction::fillHistoryPopup( m_lstHistory, aForward->popupMenu(), false, true );
}


void
MainWindow::slotBackAboutToShow()
{
	aBack->popupMenu()->clear();
	HistoryAction::fillHistoryPopup( m_lstHistory, aBack->popupMenu(), true, false );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::slotArrangement()
{
	if(aArrangementLR->isChecked())
		getImageListView()->setArrangement(ImageListView::LeftToRight);
	else
		getImageListView()->setArrangement(ImageListView::TopToBottom);
	getImageListView()->setThumbnailSize(false);
}

void
MainWindow::slotTxtPos()
{
	if(aArrangementR->isChecked())
		getImageListView()->setItemTextPos(ImageListView::Right);
	else
		getImageListView()->setItemTextPos(ImageListView::Bottom);
}


bool
MainWindow::fullScreen()
{
	return inFullScreen;
}


void
MainWindow::setEmptyImage()
{
	getImageViewer()->loadImage();
}


void
MainWindow::copyFilesTo(const QStringList& uris, const QString& dest)
{
	getDirectoryView()->copy(uris, dest);
}


void
MainWindow::moveFilesTo(const QStringList& uris, const QString& dest)
{
	getDirectoryView()->move(uris, dest);
}


void
MainWindow::startWatchDir()
{
	getDirectoryView()->startWatchDir();
}


void
MainWindow::stopWatchDir()
{
	getDirectoryView()->stopWatchDir();
}


void
MainWindow::goHome()
{
	openDir(QDir::homeDirPath());
}


void
MainWindow::goUp()
{
	QDir dir(getCurrentDir());dir.cdUp();
	openDir(dir.path());
}


void
MainWindow::clearCache()
{
	getImageListView()->removeThumbnails(true);
}


void
MainWindow::clearCacheRec()
{
	clearCacheRec(getCurrentDir());
}


void
MainWindow::clearCacheRec(const QString& fromDir)
{
#ifndef Q_WS_WIN
	KonqOperations::del(this, KonqOperations::DEL, getImageListView()->il->clearThumbnailDir(fromDir));
#else
	(void)KIO::del(getImageListView()->il->clearThumbnailDir(fromDir));
#endif
}


void
MainWindow::updateCache()
{
	KURL::List list = getImageListView()->il->updateThumbnailDir(getCurrentDir());

	pdCache=new KProgressDialog (this, "Thumbnail",
				i18n("Thumbnail progress"),
				QString::null,
				true);
	pdCache->setLabel(i18n("Updating in progress..."));
	pdCache->progressBar()->setTotalSteps(2);
	pdCache->progressBar()->setProgress(2);
	pdCache->show();
	pdCache->adjustSize();
	list += updateCache(getCurrentDir());
	pdCache->close();
	delete(pdCache);

	//
#ifndef Q_WS_WIN
	KonqOperations::del(this, KonqOperations::DEL, list);
#else
	(void)KIO::del( list );
#endif
}


KURL::List
MainWindow::updateCache(const QString& fromDir)
{
	pdCache->setLabel(
		"<qt>"
		+i18n("Updating in progress for:<center>%1</center>")
			.arg(fromDir)
		+"</qt>"
		);
	kapp->processEvents();

	QDir d(QDir::homeDirPath()+"/.showimg/cache/"+fromDir);
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *dlist = d.entryInfoList();

	if(!dlist) return KURL::List();

	int len=QString(QDir::homeDirPath()+"/.showimg/cache").length();
	KURL::List list;
	QFileInfoListIterator it( *dlist );
	QFileInfo *fi;
	KURL m_url;
	while ( (fi = it.current()) != 0 )
	{
		QString fCache=fi->absFilePath();
		QString orgFile=fCache.right(fCache.length()-len);
		if(fi->isDir() && !fromDir.startsWith(orgFile))
		{
			list += updateCache(orgFile);
		}
		else
		{
			if(!QFileInfo(orgFile).exists() && QFileInfo(orgFile).extension(false)!="dat")
			{
				m_url.setPath(fCache);
				list.append(m_url);

				m_url.setPath(fCache+".dat");
				list.append(m_url);

			}
		}
		++it;
	}
	return list;

}


void
MainWindow::slotDisplayNBImg()
{
	aTime->setText(i18n("1 image seen", "%n images seen", getImageViewer()->getNbImg()));
	KMessageBox::information(this, "<qt>" + i18n("You have already seen <b>1</b> image.", "You have already seen <b>%n</b> images.", getImageViewer()->getNbImg()) + "</qt>");
}


void
MainWindow::setCurrentDir(const QString& dir)
{
	currentDir_ = dir;
	if(QFileInfo(currentDir_).isDir() && !currentDir_.endsWith(QChar(QDir::separator())))
			currentDir_+=QDir::separator();

}


QString
MainWindow::getCurrentDir() const
{
	return currentDir_;
}


QString
MainWindow::getcdromPath()
{
	return cdromPath;
}


#ifdef HAVE_KIPI
KIPIPluginManager*
MainWindow::pluginManager()
{
	return m_pluginManager;
}
#endif /*HAVE_KIPI*/


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::deleteTempDirectories()
{
    hide();

    KURL::List list;
    KURL url;

    if(KStandardDirs::exists(locateLocal("tmp", "showimg-cpr/")))
    {
    	url.setPath(locateLocal("tmp", "showimg-cpr/"));
    	list.append(url);
    }
    if(KStandardDirs::exists(locateLocal("tmp", "showimg-arc/")))
    {
	    url.setPath(locateLocal("tmp", "showimg-arc/"));
	    list.append(url);
    }
    if(KStandardDirs::exists(locateLocal("tmp", "showimg-net/")))
    {
	    url.setPath(locateLocal("tmp", "showimg-net/") );
	    list.append(url);
    }
    KIO::DeleteJob *job = KIO::del( list );
    connect(job, SIGNAL(result( KIO::Job *)), this, SLOT(deleteTempDirectoriesDone( KIO::Job *)));

}

void
MainWindow::deleteTempDirectoriesDone( KIO::Job *job)
{
	if(job)
	if(job->error()==0)
	{
		//pas d'erreurs
	}
	else
	{	//erreur
		kdWarning()<<job->errorText()<<endl;
	}
	deleteTempDirectoriesDone_=true;
	close();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::showUnableOpenDirectoryError(const QString& dir)
{
	KMessageBox::error(this, "<qt>"+i18n("Unable to open the directory <b>%1</b>").arg(QDir::convertSeparators(dir))+"</qt>");
}

QString
MainWindow::getLastDestDir()
{
	return m_lastDestDir;
}

void
MainWindow::setLastDestDir(const QString& path)
{
	m_lastDestDir=path;
	emit lastDestDirChanged(m_lastDestDir);
}


//------------------------------------------------------------------------------
Tools*
MainWindow::getToolManager()
{
	return m_tools;
}

#ifdef WANT_LIBKEXIDB
CategoryDBManager*
MainWindow::getCategoryManager()
{
	return getCategoryView()->getCategoryDBManager();
}
#endif

MainWindow *
MainWindow::instance()
{
	if(m_mw)
		return m_mw;
	else
		return this;
}

#include "mainwindow.moc"
