/*****************************************************************************
                          mainwindow.cp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
                               2003 by OGINO Tomonori
                               2004 by Guillaume Duhamel
                               2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           ogino@nn.iij4u.or.jp
                           guillaume.duhamel@univ-rouen.fr
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

#include "mainwindow.h"

//-----------------------------------------------------------------------------
#define MAINWINDOW_DEBUG           0
#define MAINWINDOW_DEBUG_GUI       0
#define MAINWINDOW_DEBUG_CHANGEURL 0
//-----------------------------------------------------------------------------

// System
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Local
#include "albumimagefileiconitem.h"
#include "compressedfileitem.h"
#include "compressedimagefileiconitem.h"
#include "confshowimg.h"
#include "desktoplistitem.h"
#include "directory.h"
#include "directoryview.h"
#include "dirfileiconitem.h"
#include "displaycompare.h"
#include "extract.h"
#include "history_action.h"
#include "imagefileiconitem.h"
#include "imagelistview.h"
#include "imagelistviewsimple.h"
#include "imageloader.h"
#include "imagemetainfo.h"
#include "imageviewer.h"
#include "ksidebar.h"
#include "kstartuplogo.h"
#include "osd.h"
#include "showimg_common.h"
#include "tools.h"
#include "viewer.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#else
#warning no HAVE_KIPI
#endif /* HAVE_KIPI */

#ifdef WANT_LIBKEXIDB
#include "categoryview.h"
#include "categorydbmanager.h"
#endif /* WANT_LIBKEXIDB */

#ifndef SHOWIMG_NO_CDARCHIVE
#include "cdarchive.h"
#include "cdarchiveview.h"
#endif

// Qt
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdict.h>
#include <qdir.h>
#include <qdockarea.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qmemarray.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qptrvector.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>

// KDE
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kbookmarkbar.h>
#include <kbookmark.h>
#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdialogbase.h>
#include <kdockwidget.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kpixmapio.h>
#include <kpopupmenu.h>
#include <kprinter.h>
#include <kprocess.h>
#include <kprogress.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <ktip.h>
#include <ktoolbarbutton.h>
#include <ktoolbar.h>
#include <kurlcompletion.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kwin.h>

#if KDE_IS_VERSION(3,3,0)
#include <ktabwidget.h>
#endif

#ifndef Q_WS_WIN
#include <konq_operations.h>
#include "win_utils.h"
#endif

const char* CONFIG_IMAGEVIEWER_GROUP    = "imageviewer widget";
const char* CONFIG_DOCK_GROUP           = "dock showimg 20050103";

MainWindow * MainWindow::m_singleton_mw = 0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class ShowImgBookmarkManager : public KBookmarkManager
{
public:
	/**
		a bookmark manager
	*/
	ShowImgBookmarkManager( const QString& bookmarksFile = QString::null,
		bool bImportDesktopFiles = true );

	static ShowImgBookmarkManager* self();

};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

MainWindow::MainWindow (const QString& pic,
			bool fs_mode, 
			bool fs_mode_force,
			bool runSlideshow, 
			int slideshowT)
	:
#if KDE_IS_VERSION(3,3,0)
	KParts::DockMainWindow(0, "ShowImg MainFrame"),
#else
	KDockMainWindow (0, "ShowImg MainFrame"),
#endif
	KBookmarkOwner(),

	m_total(0),
	m_done(0),
	m_inFullScreen(false),
	m_inInterface(false),
	m_requestingClose(false),
	m_nbrItems(0),
	m_imageIndex(0),
	m_deleteTempDirectoriesDone(false),

	m_p_iv(NULL), m_p_movieViewer(NULL), m_p_SVGViewer(NULL),
	m_p_viewer(NULL),
	m_p_imageList(NULL),
	m_p_dirView(NULL),

#ifdef WANT_LIBKEXIDB
	m_p_catView(NULL),
#endif

#ifdef HAVE_KIPI
	m_p_pluginManager(NULL),
#endif /* HAVE_KIPI */
	m_p_tools(NULL),

	m_dockIV(0),m_dockDir(0),m_dockIL(0),m_dockIMI(0),

	m_timer(NULL),

	m_p_config(NULL),

	m_p_actions(NULL),
	m_p_zoomCombo(NULL),

	m_p_fullScreen_action(NULL),
	
	m_p_SB_NAME_Label(NULL),
	m_p_SB_DATE_Label(NULL)
{
#if MAINWINDOW_DEBUG > 0
	MYDEBUG
		<< pic << " "
		<< fs_mode << " "
		<< fs_mode_force << " "
		<< runSlideshow << " "
		<< slideshowT << " "
	<<endl;
#endif
	m_singleton_mw = this;
	if(pic.isEmpty() || pic.isNull())
	{
		init();
		show();

		m_inInterface=true;
		if(m_openDirType==0 || ! QFileInfo( m_openDirname ).exists())
#ifdef Q_WS_WIN
			openURL(desktopDirPath());
#else
			openURL(QDir::homeDirPath());
#endif
		else
		{
			openURL(m_openDirname);
		}
		setHasImageSelected(getImageListView()->hasImages());
	}
	else
	if (
		QFileInfo(pic).isDir() || 
		QString(pic).right (3) == QString::fromLatin1("sia")
	)
	{
		init();
		show();

		m_inInterface=true;
		openURL(QDir (pic).absPath());
		if(runSlideshow)
		{
			if(slideshowT < 0)
				slideshowT = m_p_config->readNumEntry("time", 2);
			slotSlideShow(slideshowT);
		}
		else
		if(fs_mode==true && fs_mode_force==true)
		{
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

		m_inInterface=true;
		openURL(QDir (pic).absPath ());
		setHasImageSelected(true);
	}
	else
	{
		m_p_config = KGlobal::config();
		m_startFS = m_p_config->readBoolEntry("m_startFS", true);
		if(Tools::isImage(pic) &&
				 ( (m_startFS && fs_mode_force==false) ||
				   (fs_mode==true && fs_mode_force==true)))
		{
			m_inInterface=false;
			initSimpleView(QDir(pic).absPath());
			showFullScreen();
			if(runSlideshow)
			{
				if(slideshowT < 0)
					slideshowT = m_p_config->readNumEntry("time", 2);
				m_imageListSimple->startSlideshow(slideshowT);
			}
		}
		else
		{
			m_inInterface=true;
			init();
			show();
			if(!pic.isEmpty() && QFileInfo( pic ).exists())
			{
				openURL(QDir(pic).absPath());
				getDirectoryView()->setLoadThumbnails(true);
			}
			else
			{
				openURL(QDir::homeDirPath());
			}
			if(runSlideshow)
			{
				if(slideshowT < 0)
					slideshowT = m_p_config->readNumEntry("time", 2);
				slotSlideShow(slideshowT);
			}
		}
	}
}


MainWindow::~MainWindow ()
{
	if (m_p_movieViewer)
		m_p_movieViewer->closeURL();
}


void
MainWindow::init()
{

	m_p_config = KGlobal::config();

	//------------------------------------------------------------------------------
	createStatusbar();
	createMainView();
	createActions();
	createMenus();

	readConfig(m_p_config);
	setXMLFile(m_xmluifile);
	createGUI(0L);
	readDockConfig (m_p_config);

	//------------------------------------------------------------------------------
	applyMainWindowSettings(m_p_config);
	setStandardToolBarMenuEnabled(true);
	restoreWindowSize(m_p_config);

	//------------------------------------------------------------------------------
#ifdef HAVE_KIPI
	// Load Plugins
	m_p_pluginManager = new KIPIPluginManager(this);
	pluginManager()->loadPlugins();
#endif /* HAVE_KIPI */
#ifndef WANT_LIBKEXIDB
	toolBar("CatViewToolBar")->hide();
#else
	if(getCategoryView()->isConnected() && getEnabledCategories())
		getCategoryView()->createRoot();
	else
	{
		m_sideBar->removeTab(m_sideBar_id_catView);
		getCategoryView()->setEnabled(false);
		toolBar("CatViewToolBar")->hide();

		m_p_actions->action("ImageCategoryProperties")->setEnabled(false);
		m_p_updateDB_action->setEnabled(false);
	}
#endif /* WANT_LIBKEXIDB */

}


void
MainWindow::initSimpleView(const QString& dir)
{

	(void)new KActionMenu( i18n("&Go"), actionCollection(), "action go");

	m_p_iv = new ImageViewer (this); KCursor::setAutoHideCursor(m_p_iv, true);
	getImageViewer()->initActions(actionCollection());
	getImageViewer()->readConfig(m_p_config, CONFIG_IMAGEVIEWER_GROUP);

	m_imageListSimple = new ImageListViewSimple(this, dir, getImageViewer());
	m_imageListSimple->initActions(actionCollection());
	m_imageListSimple->readConfig(m_p_config);

	///
	(void)new KAction(i18n("Next"), KShortcut(Key_Space), m_imageListSimple, SLOT(next()), actionCollection(), "simple interface next" );
	(void)new KAction(i18n("Switch to interface"), "window_fullscreen", KShortcut(Key_F), this, SLOT(switchToInterface()), actionCollection(), "Simple Interface Switch to interface" );
	(void)new KAction(i18n("Quit"), "quit", KShortcut(Key_Escape), this, SLOT(escapePressed()), actionCollection(), "Simple Interface Quit");

	getImageViewer()->initMenu(actionCollection());
	setView(m_p_iv);

	///
 	createGUI(0L);

	///
	leftDock()->hide(); rightDock()->hide(); topDock()->hide();	bottomDock()->hide();
	menuBar()->hide(); statusBar()->hide();
	toolBar("locationToolBar")->hide(); toolBar("mainToolBar")->hide();	toolBar("viewToolBar")->hide();

	setGeometry(0,0,KApplication::desktop()->width(), KApplication::desktop()->height());
	m_imageListSimple->load();
	m_inFullScreen=false;
	m_inInterface=false;
	m_requestingClose=false;
	m_hasimageselected=true;
}

bool
MainWindow::initMovieViewer()
{
	if(m_availableMovieViewer.isEmpty())
		initAvailableMovieViewer();
	if(getCurrentAvailableMovieViewerIndex()<0)
		return false;

	m_p_partManager = new KParts::PartManager(this, "KParts::PartManager");

	KTrader::OfferList offers = KTrader::self()->query("video/avi" );
	KLibFactory *factory = 0;
	KTrader::OfferList::Iterator l_it(offers.begin());
	for( ; l_it != offers.end(); ++l_it)
	{
		KService::Ptr ptr = (*l_it);

		QString name    = ptr->name();
		QString comment = ptr->comment();
		QString library = ptr->library();

		factory = KLibLoader::self()->factory( ptr->library().ascii() );
		if (factory &&
			library == m_availableMovieViewer[m_currentAvailableMovieViewerIndex]
		/*"libkaffeinepart"*/ /*"libkplayerpart"*/ /*"libkmplayerpart"*/ /*"libkaboodlepart"*/)
		{
			m_p_movieViewer =
				static_cast<KParts::ReadOnlyPart *>(
					factory->create(this,
							ptr->name().ascii(),
							"KParts::ReadOnlyPart"));
 			break;
		}
	}
	return m_p_movieViewer!=NULL;
}

void
MainWindow::initAvailableMovieViewer()
{
	KTrader::OfferList offers = KTrader::self()->query("video/avi" );
	KTrader::OfferList::Iterator l_it(offers.begin());
	for( ; l_it != offers.end(); ++l_it)
	{
		KService::Ptr ptr = (*l_it);
		QString library = ptr->library();
		if(
			(library.contains("kaffeinepart")||
			library.contains("kmplayerpart")||
			library.contains("kaboodlepart")||
			library.contains("kplayerpart"))
			&& ! m_availableMovieViewer.contains(library)
			)
			m_availableMovieViewer.append(library.stripWhiteSpace());
	}
}

QStringList
MainWindow::getAvailableMovieViewer()
{
	if(m_availableMovieViewer.isEmpty())
		initAvailableMovieViewer();
	return m_availableMovieViewer;
}
int
MainWindow::getCurrentAvailableMovieViewerIndex()
{
	return m_currentAvailableMovieViewerIndex;
}

void
MainWindow::setCurrentAvailableMovieViewerIndex(int current)
{
	m_currentAvailableMovieViewerIndex = current;
}

bool
MainWindow::initSVGViewer()
{
	KTrader::OfferList offers = KTrader::self()->query("image/svg+xml" );
	KLibFactory *factory = 0;
	KTrader::OfferList::Iterator l_it(offers.begin());
	for( ; l_it != offers.end(); ++l_it)
	{
		KService::Ptr ptr = (*l_it);

		QString name    = ptr->name();
		QString comment = ptr->comment();
		QString library = ptr->library();

		factory = KLibLoader::self()->factory( ptr->library().ascii() );
		if (factory && library == QString::fromLatin1("libksvgplugin") )
		{
			m_p_SVGViewer =
				static_cast<KParts::ReadOnlyPart *>(
					factory->create(this,
							ptr->name().ascii(),
							"KParts::ReadOnlyPart"));

			break;
		}
	}
	return m_p_SVGViewer!=NULL;
}

void
MainWindow::updateGUI (Viewer::AvailableViewers a_current)
{
#if MAINWINDOW_DEBUG_GUI > 0
	MYDEBUG<<"Viewer::AvailableViewers a_current="<<a_current<<endl;
#endif
	KApplication::setOverrideCursor (waitCursor); // this might take time

#if KDE_IS_VERSION(3,3,0)
	if(a_current == Viewer::AV_MovieViewer)
	{
#if MAINWINDOW_DEBUG_GUI > 0
		MYDEBUG<<endl;
#endif
		if(!m_p_movieViewer)
		{
			if(initMovieViewer())
				m_p_viewer->setMovieViewer(m_p_movieViewer);
		}
		createGUI(m_p_movieViewer);
		delete(m_p_SVGViewer); m_p_SVGViewer=NULL;m_p_viewer->setSVGViewer(m_p_SVGViewer);
	}
	else
	if(a_current == Viewer::AV_SVGViewer)
	{
#if MAINWINDOW_DEBUG_GUI > 0
		MYDEBUG<<endl;
#endif
		if(!m_p_SVGViewer)
		{
			if(initSVGViewer())
				m_p_viewer->setSVGViewer(m_p_SVGViewer);
		}
		createGUI(m_p_SVGViewer);
		delete(m_p_movieViewer); m_p_movieViewer=NULL;m_p_viewer->setMovieViewer(m_p_movieViewer);
	}
	else
#endif
	{
#if MAINWINDOW_DEBUG_GUI > 0
		MYDEBUG<<endl;
#endif
		createGUI(0L);
		delete(m_p_movieViewer); m_p_movieViewer=NULL;m_p_viewer->setMovieViewer(m_p_movieViewer);
		delete(m_p_SVGViewer); m_p_SVGViewer=NULL;m_p_viewer->setSVGViewer(m_p_SVGViewer);
	}

	applyMainWindowSettings( KGlobal::config(), "MainWindow" );
	KApplication::restoreOverrideCursor ();
	if(m_inFullScreen)
	{
		menuBar()->hide();
		toolBar("locationToolBar")->hide();
		if(!m_showToolbar)
		{
			toolBar("mainToolBar")->hide();
			toolBar("viewToolBar")->hide();
			topDock()->hide();
		}
		if(!m_showStatusbar)
		{
			statusBar()->hide();
			bottomDock()->hide();
		}
	}
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
	statusBar()->insertItem(QString::null, SB_MSG, 2,true);		statusBar()->setItemAlignment(SB_MSG, AlignTop);
	statusBar()->insertItem(QString::null, SB_ITEMS, 0,true);		statusBar()->setItemAlignment(SB_ITEMS, AlignTop|AlignHCenter);

	m_p_statusbarProgress = new KProgress (statusBar(), "QProgressBar de chargement des images de MainWindow");
	m_p_statusbarProgress->setCenterIndicator (true);
	statusBar()->addWidget (m_p_statusbarProgress, 1, true);
	m_statusbarProgressLastUpdate = QDateTime::currentDateTime ();

	statusBar()->insertItem(QString::null, SB_SCALE, 0,true);		statusBar()->setItemAlignment(SB_SCALE, AlignTop|AlignHCenter);

	m_p_SB_NAME_Label = new KSqueezedTextLabel (statusBar());
	m_p_SB_NAME_Label->setAlignment( int( KSqueezedTextLabel::AlignCenter ) );
	statusBar()->addWidget (m_p_SB_NAME_Label, 2, true);

	statusBar()->insertItem(QString::null, SB_TYPE, 0,true);		statusBar()->setItemAlignment(SB_TYPE, AlignTop|AlignHCenter);
	statusBar()->insertItem(QString::null, SB_IMG_SIZE, 1,true);	statusBar()->setItemAlignment(SB_IMG_SIZE, AlignTop|AlignHCenter);
	statusBar()->insertItem(QString::null, SB_BYTES, 1,true);		statusBar()->setItemAlignment(SB_BYTES, AlignTop|AlignHCenter);

	m_p_SB_DATE_Label = new KSqueezedTextLabel (statusBar());
	m_p_SB_DATE_Label->setAlignment( int( KSqueezedTextLabel::AlignCenter ) );
	statusBar()->addWidget (m_p_SB_DATE_Label, 2, true);
}

void
MainWindow::createMainView()
{
	m_dockIV = createDockWidget ("Preview", BarIcon("image"),0L, i18n("Preview"), i18n("Preview"));
	m_p_viewer = new Viewer(m_dockIV);
	m_p_iv = new ImageViewer (m_p_viewer, "ImageViewer");
	m_p_viewer->setImageViewer(m_p_iv);
	m_p_viewer->setVisibleImageViewer();
	if(m_dockIV)
	{
		m_dockIV->setWidget( m_p_viewer );
		m_dockIV->setToolTipString(i18n("Image Preview"));
	}
	KCursor::setAutoHideCursor(m_p_iv, true);
	QWhatsThis::add(m_p_iv, i18n( "Image Preview" ) );


	//////
	m_dockIL = createDockWidget ("image listview dock ", SmallIcon("view_icon"), 0L, i18n("Image Icon Viewer"), i18n("Image Icon Viewer"));
	m_p_imageList = new ImageListView (m_dockIL, "ImageList");
	QWhatsThis::add(m_p_imageList, i18n( "Images of the selected directories" ) );
	if(m_dockIL)
	{
		m_dockIL->setWidget(m_p_imageList);
		m_dockIL->setToolTipString(i18n("Images in the selected directories"));
	}


	m_dockIMI = createDockWidget ("Image Meta Data", SmallIcon("info"), 0L, i18n("Image Meta Data"), i18n("Image Meta Data"));
	m_p_imageMetaInfo = new ImageMetaInfoView(m_dockIMI);
	if(m_dockIMI)
	{
		m_dockIMI->setWidget( m_p_imageMetaInfo );
	}

	//
	/////////////////////////////////////////////////////////////////////////////////
	m_sideBar = new KSideBar(this, "my sidebar");
#if KDE_IS_VERSION(3,3,0)
	m_dockDir = createDockWidget ("Tree View", SmallIcon("folder"), 0L, i18n("Tree View"), i18n("Tree View"));
	if(m_dockDir)
	{
		m_dockDir->setWidget( m_sideBar );
	}
	QVBox* vbox_dir=new QVBox(this, "navToolWindow_dir QVBox");
	m_p_tb_dir =new KToolBar(vbox_dir, "fileViewToolBar", true); m_p_tb_dir->setIconSize(KIcon::SizeSmall);
	m_p_dirView = new DirectoryView (vbox_dir, "Directory View");
	m_sideBar_id_dirView = m_sideBar->addTab(vbox_dir, SmallIcon("folder"), i18n("Tree View"));
#else
	m_dockDir = createDockWidget ("Tree View", BarIcon("folder", 16),0L, i18n("Tree View"), i18n("Tree View"));
	m_p_dirView = new DirectoryView (m_dockDir, "Directory View");
	QWhatsThis::add(m_p_dirView, i18n( "List of directories" ) );
	m_dockDir->setWidget( m_p_dirView );
	m_dockDir->setToolTipString(i18n("The directory tree"));
#endif

#ifdef WANT_LIBKEXIDB
	QVBox* vbox_cat=new QVBox(this, "navToolWindow_cat QVBox");
	m_p_tb_cat =new KToolBar(vbox_cat, "CatViewToolBar", true); m_p_tb_cat->setIconSize(KIcon::SizeSmall);
	m_p_catView = new CategoryView (vbox_cat, "CategoryView DirectoryView");
	m_sideBar_id_catView = m_sideBar->addTab(vbox_cat, SmallIcon("kexi_kexi"), i18n("Category View"));
#endif /* WANT_LIBKEXIDB
	   */
	QVBox* vbox_cdarc=new QVBox(this, "navToolWindow_cdarcQVBox");
	m_p_tb_cdarc=new KToolBar(vbox_cdarc, "CdarcViewToolBar", true); m_p_tb_cdarc->setIconSize(KIcon::SizeSmall);
	m_cdarcView = new CDArchiveView (vbox_cdarc, "CDArchiveView DirectoryView");
	m_sideBar_id_cdarcView = m_sideBar->addTab(vbox_cdarc, SmallIcon("cdimage"), i18n("CD Archive View"));

	//------------------------------------------------------------------------------
#ifdef Q_WS_WIN
	m_myPicturesDirPath = myPicturesDirPath();
// 	//FIXME m_root_dir = new SpecialListItem( getDirectoryView(), getImageViewer(), getImageListView(), this, desktopDirPath(), "desktop", i18n("Desktop") );
	//FIXME m_root_dir = new SpecialListItem( getDirectoryView(), getImageViewer(), getImageListView(), this, myDocumentsDirPath(), "kword", i18n("My Documents") );
	for (QPtrListIterator<QFileInfo> drivesIt(*QDir::drives()); drivesIt.current(); ++drivesIt) {
		new Directory (getDirectoryView(), getImageViewer(), getImageListView(), this, drivesIt.current()->absFilePath());
	}
#else
	getDirectoryView()->createRoot();
	//getDirectoryView()->getRoot()->setOpen(true) ;
#endif

#ifndef SHOWIMG_NO_CDARCHIVE
	getCDArchiveView()->createRoot();
#endif

//------------------------------------------------------------------------------
	m_sideBar->switchToTab(m_sideBar_id_dirView);
	m_p_currentListItemView = m_p_dirView;

	m_p_tools = new Tools();
	getImageViewer()->setMainWindow(this);

//------------------------------------------------------------------------------
	setView(m_dockIV); // central widget in a KDE mainwindow
	setMainDockWidget( m_dockIV); // master dockwidget

	manager()->setSplitterOpaqueResize(true);
	m_dockDir->manualDock(m_dockIV, KDockWidget::DockLeft, 35);
	m_dockIL->manualDock(m_dockDir, KDockWidget::DockBottom, 50);
	m_dockIMI->manualDock(m_dockIV, KDockWidget::DockCenter, 35);
}

void
MainWindow::createActions()
{
	m_p_actions = actionCollection();

	m_p_back_action		=new HistoryAction(i18n("Back"), "back", KStdAccel::shortcut(KStdAccel::Back).keyCodeQt(), this, SLOT(slotBack()), m_p_actions, "back");
	connect(m_p_back_action->popupMenu(), SIGNAL(activated(int)), this, SLOT(backMenuActivated(int)));
	connect(m_p_back_action->popupMenu(), SIGNAL( aboutToShow() ), SLOT( slotBackAboutToShow() ) );
	m_p_back_action->setEnabled(false);

	m_p_forward_action	=new HistoryAction(i18n("Forward"), "forward", KStdAccel::shortcut(KStdAccel::Forward).keyCodeQt() , this, SLOT(slotForward()), m_p_actions, "forward");
	connect(m_p_forward_action->popupMenu(), SIGNAL(activated(int)), this, SLOT(forwardMenuActivated(int)));
	connect(m_p_forward_action->popupMenu(), SIGNAL( aboutToShow() ), SLOT( slotForwardAboutToShow() ) );
	m_p_forward_action->setEnabled(false);

	//------------------------------------------------------------------------------
	m_p_cut_action		=new KAction(i18n("Cut"), "editcut", KStdAccel::shortcut(KStdAccel::Cut), this, SLOT(slotcut()), m_p_actions , "editcut");
	m_p_cut_action->setEnabled(false);
	m_p_copy_action		=new KAction(i18n("Copy"), "editcopy", KStdAccel::shortcut(KStdAccel::Copy), this, SLOT(slotcopy()), m_p_actions, "editcopy");
	m_p_copyPixmap_action	=new KAction(i18n("Copy Image"), 0, this, SLOT(slotcopyPixmap()), m_p_actions, "editcopypixmap");
	m_p_paste_action		=new KAction(i18n("Paste"), "editpaste", KStdAccel::shortcut(KStdAccel::Paste), this, SLOT(slotpaste()), m_p_actions, "editpaste" );

	//------------------------------------------------------------------------------
	m_p_goHome_action		=new KAction(i18n("Go to &Home Directory"), "gohome", KStdAccel::shortcut(KStdAccel::Home), this, SLOT(goHome()), m_p_actions, "goHome");
	m_p_goUp_action		=new KAction(i18n("Go Up"), "up", KStdAccel::shortcut(KStdAccel::Up), this, SLOT(goUp()), m_p_actions , "goUp");


	KActionMenu *actionGo = new KActionMenu( i18n("&Go"), m_p_actions, "action go");
	actionGo->insert(m_p_goUp_action);
	actionGo->insert(m_p_goHome_action);

	//------------------------------------------------------------------------------
	m_p_newWindow_action	=new KAction(i18n("New Window"), "window_new", KStdAccel::shortcut(KStdAccel::New), this, SLOT(slotNewWindow()), m_p_actions, "window_new" );

	m_p_openLocation_action	=new KAction(i18n("Open Location..."), "fileopen", KStdAccel::shortcut(KStdAccel::Open), this, SLOT(slotOpenLocation()), m_p_actions, "fileopen" );

	m_p_quit_action = KStdAction::quit( kapp, SLOT (closeAllWindows()), actionCollection() );
	m_p_close_action		=new KAction(i18n("Close"), "fileclose", KStdAccel::shortcut(KStdAccel::Close), this, SLOT(close()), m_p_actions , "close");

	//------------------------------------------------------------------------------
// 	m_p_undo_action		=new KAction(i18n("Undo"), "undo", KStdAccel::shortcut(KStdAccel::Undo), this, SLOT(slotUndo()), m_p_actions , "undo");
// 	m_p_undo_action->setEnabled(false);
// 	m_p_redo_action		=new KAction(i18n("Redo"), "redo", KStdAccel::shortcut(KStdAccel::Redo), this, SLOT(slotRedo()), m_p_actions , "redo");
// 	m_p_redo_action->setEnabled(false);

	//------------------------------------------------------------------------------
	m_p_editType_action	=new KAction(i18n("Edit File Type"), 0, this, SLOT(slotEditFileType()), m_p_actions, "Edit File Type");

	m_p_configureKey_action =     KStdAction::keyBindings(this, SLOT(configureKey()),  actionCollection() );
	m_p_configureToolbars_action= KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection() );
	m_p_configureShowImg_action = KStdAction::preferences(this, SLOT(configureShowImg()), actionCollection() );

	KActionMenu *actionInterface = new KActionMenu( i18n("Interface"), m_p_actions, "configure_interface" );
	actionInterface->insert(new KAction(i18n("Full Interface"), 0, this, SLOT(switchToFullUI()), m_p_actions , "simple ui"));
	actionInterface->insert(new KAction(i18n("Simple Interface"), 0, this, SLOT(switchToSimpleUI()), m_p_actions , "full ui"));

	// the tip action
	(void)KStdAction::tipOfDay(this, SLOT(slotShowTips()), actionCollection(), "help_showimgtipofday");

	//------------------------------------------------------------------------------
	m_p_slideshow_action	=new KToggleAction(i18n("&Slide Show"), "run", 0, this, SLOT(slotSlideShow()), m_p_actions, "Slideshow");

	//------------------------------------------------------------------------------
	m_p_reloadDir_action	=new KAction(i18n("Refresh"), "reload", KStdAccel::shortcut(KStdAccel::Reload), this, SLOT(slotRefresh()), m_p_actions, "Refresh");
	m_p_preview_action	=new KToggleAction(i18n("Toggle Thumbnails"), "thumbnail", 0, this, SLOT(slotPreview()), m_p_actions, "Preview");
	m_p_stop_action		=new KAction(i18n("Stop"), "stop", 0, this, SLOT(slotStop()), m_p_actions, "Stop");
	m_p_stop_action->setEnabled(false);


	//------------------------------------------------------------------------------
	KShortcut sc_fs(CTRL+Key_F); sc_fs.append(KKeySequence((const KKey&)(CTRL+Key_Return)));
	m_p_fullScreen_action=new KToggleAction(i18n("Full Screen"), "window_fullscreen", sc_fs, this, SLOT(slotFullScreen()), m_p_actions, "FullScreen" );
	m_p_fullScreen_action->setChecked(false);

	//------------------------------------------------------------------------------
	m_p_updateCache_action	=new KAction(i18n("Recursively &Update Current Cached Thumbnails"), 0, this, SLOT(updateCache()), m_p_actions , "updateCache");
	m_p_clearCacheRec_action	=new KAction(i18n("Recursively Remove Current Cached Thumbnails"),  0, this, SLOT(clearCacheRec()), m_p_actions , "clearCacheRec");
	m_p_clearCache_action	=new KAction(i18n("&Remove Current Cached Thumbnails"),  0, this, SLOT(clearCache()), m_p_actions , "clearCache");
	m_p_updateDB_action	=new KAction(i18n("Update &Database"), 0, this, SLOT(removeObsololeteFilesOfTheDatabase()), m_p_actions , "updateDatabase");
	KActionMenu *actionMaint = new KActionMenu( i18n("Main&tenance"), m_p_actions, "tools_maint" );
	actionMaint->insert(m_p_updateCache_action);
	actionMaint->insert(m_p_clearCacheRec_action);
	actionMaint->insert(m_p_clearCache_action);
	actionMaint->insert(m_p_updateDB_action);

	//------------------------------------------------------------------------------
	m_p_bookmarkmenu = 	new KActionMenu(i18n("&Bookmark"), m_p_actions, "bookm");
	m_p_bookMenu=		new KBookmarkMenu(ShowImgBookmarkManager::self(), this, m_p_bookmarkmenu->popupMenu(),  m_p_actions, true);

	//------------------------------------------------------------------------------
	m_p_URLHistory_combo=new KHistoryCombo(this);
	m_p_URLHistory_combo->setDuplicatesEnabled(false);
	m_p_URLHistory_combo->setAutoDeleteCompletionObject(true);

	m_p_URLHistoryCompletion=new KURLCompletion(KURLCompletion::DirCompletion);
	m_p_URLHistory_combo->setCompletionObject(m_p_URLHistoryCompletion);
//	m_p_URLHistoryCompletion->setDir("file:/");

	KWidgetAction* comboAction=new KWidgetAction( m_p_URLHistory_combo, i18n("Location Bar"), 0, 0, 0, m_p_actions, "location_url");
	comboAction->setShortcutConfigurable(false);
	comboAction->setAutoSized(true);

	(void)new KAction( i18n("Clear Location Bar"), "locationbar_erase", 0, m_p_URLHistory_combo, SLOT(clearEdit()), m_p_actions, "clear_location");

	QLabel* m_urlLabel=new QLabel(i18n("L&ocation:"), this, "kde toolbar widget");
	(void)new KWidgetAction( m_urlLabel, i18n("L&ocation:"), 0, 0, 0, m_p_actions, "location_label");
	m_urlLabel->setBuddy(m_p_URLHistory_combo);
	m_p_go_action     =       new KAction(i18n("Go"), "key_enter", 0, this, SLOT(changeURL()), actionCollection(), "location_go");

	connect(m_p_URLHistory_combo, SIGNAL(returnPressed()), this, SLOT(changeURL()));

	//------------------------------------------------------------------------------
	m_p_zoomCombo=new KComboBox(/*this*/);
	m_p_zoomCombo->insertItem("10 %"); m_p_zoomCombo->insertItem("25 %"); m_p_zoomCombo->insertItem("33 %");
	m_p_zoomCombo->insertItem("50 %"); m_p_zoomCombo->insertItem("67 %"); m_p_zoomCombo->insertItem("75 %");
	m_p_zoomCombo->insertItem("100 %"); m_p_zoomCombo->insertItem("150 %"); m_p_zoomCombo->insertItem("200 %");
	m_p_zoomCombo->insertItem("300 %"); m_p_zoomCombo->insertItem("400 %"); m_p_zoomCombo->insertItem("600 %");
	m_p_zoomCombo->insertItem("800 %"); m_p_zoomCombo->insertItem("1000 %"); m_p_zoomCombo->insertItem("1200 %");
	m_p_zoomCombo->insertItem("1600 %"); m_p_zoomCombo->setEditable ( true );
	m_p_zoomCombo->setInsertionPolicy ( QComboBox::NoInsertion );
	m_p_zoomCombo->setDuplicatesEnabled( false );
	KWidgetAction* zoomComboAction=new KWidgetAction( m_p_zoomCombo, i18n("Zoom"),
			0,
			0, 0,
			m_p_actions, "zoomComboAction");
	zoomComboAction->setShortcutConfigurable(false);
	zoomComboAction->setAutoSized(false);
	connect(m_p_zoomCombo, SIGNAL(activated ( const QString& )), this, SLOT(setZoom( const QString& )));

	//------------------------------------------------------------------------------
 	// Non configurable stop-fullscreen accel
	QAccel* accel=m_p_actions->accel();
	accel->connectItem(accel->insertItem(Key_Escape), this, SLOT(escapePressed()));
	accel->connectItem(accel->insertItem(Key_Space), this, SLOT(spacePressed()));

	// Dock related
	connect(manager(), SIGNAL(change()), this, SLOT(updateWindowActions()) );

	//------------------------------------------------------------------------------
	m_p_time_action = new KAction(QString::null, (const KShortcut&)0,
						this, SLOT(slotDisplayNBImg()), m_p_actions, "time");

	//------------------------------------------------------------------------------
	m_windowListActions.setAutoDelete(true);
	updateWindowActions();
	//------------------------------------------------------------------------------

#ifdef WANT_LIBKEXIDB
	connect(getImageListView(), SIGNAL(fileIconRenamed(const QString&, const QString&)),
			getCategoryView(), SLOT(fileIconRenamed(const QString&, const QString&)) );
	//FIXME connect(getImageListView(), SIGNAL(fileIconsDeleted(const KURL::l_list&)),
	//		getCategoryView(), SLOT(fileIconsDeleted(const KURL::l_list&)) );

	connect(getDirectoryView(), SIGNAL(renameListItemDone(const KURL&, const KURL&)),
			getCategoryView(), SLOT(directoryRenamed(const KURL&, const KURL&)) );
	connect(getDirectoryView(), SIGNAL(moveFilesDone(const KURL::List&, const KURL&)),
			getCategoryView(), SLOT(filesMoved(const KURL::List&, const KURL&)) );
#endif /* WANT_LIBKEXIDB */

	connect(getImageViewer(), SIGNAL(sigSetMessage(const QString&)),
			this, SLOT(slotSetStatusBarText(const QString&)));
	connect(getImageListView(), SIGNAL(sigSetMessage(const QString&)),
			this, SLOT(slotSetStatusBarText(const QString&)));

	//------------------------------------------------------------------------------
	getImageListView()->initActions(m_p_actions);
	getDirectoryView()->initActions(m_p_actions);
	getImageViewer()->initActions(m_p_actions);
	getToolManager()->initActions(m_p_actions);
#ifdef WANT_LIBKEXIDB
	getCategoryView()->initActions(m_p_actions);
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	getCDArchiveView()->initActions(m_p_actions);
#endif /* SHOWIMG_NO_CDARCHIVE */

}


void
MainWindow::createMenus()
{
	getImageListView()->initMenu(m_p_actions);
	getDirectoryView()->initMenu(m_p_actions);
	getImageViewer()->initMenu(m_p_actions);
#ifdef WANT_LIBKEXIDB
	m_p_catView->initMenu(m_p_actions);
#endif /* WANT_LIBKEXIDB */
#ifndef SHOWIMG_NO_CDARCHIVE
	m_cdarcView->initMenu(m_p_actions);
#endif /* SHOWIMG_NO_CDARCHIVE */

}

void
MainWindow::readConfig(KConfig *config)
{
	getImageViewer()->readConfig(config, CONFIG_IMAGEVIEWER_GROUP);
	getImageListView()->readConfig(config);
	getDirectoryView()->readConfig(config);
	getToolManager()->readConfig(config);

	//------------------------------------------------------------------------------
#ifdef WANT_LIBKEXIDB
	config->setGroup("Categories");
	setEnabledCategories(config->readBoolEntry("enable", true));
	getCategoryView()->readConfig(config);
#endif /* WANT_LIBKEXIDB */

	//------------------------------------------------------------------------------
	config->setGroup("Slideshow");
	m_slideshowTime=config->readNumEntry("time", 2);
	m_slideshowType=config->readNumEntry("type", 0);

	//------------------------------------------------------------------------------
	config->setGroup("Options");
	m_xmluifile=config->readEntry("xmluifile", "showimgsimpleui.rc");
	m_p_preview_action->setChecked(config->readBoolEntry("preview", true));
	m_p_time_action->setText(i18n("1 image seen", "%n images seen", getImageViewer()->getNbImg()));
	m_openDirType=config->readNumEntry("m_openDirType", 0);
	m_openDirname=config->readPathEntry("m_openDirname", QDir::homeDirPath());
	m_showSP=config->readBoolEntry("m_showSP", true);
	m_startFS=config->readBoolEntry("m_startFS", true);
	m_showToolbar=config->readBoolEntry("m_showToolbar", false);
	m_showStatusbar=config->readBoolEntry("m_showStatusbar", false);
	setCurrentAvailableMovieViewerIndex(config->readNumEntry("movieViewer", 0));

	//------------------------------------------------------------------------------
	config->setGroup("Paths");
	m_cdromPath = config->readPathEntry("m_cdromPath", "/mnt/cdrom");

	m_timer = new QTimer (this);
	if(m_slideshowType == 0 )
		connect (m_timer, SIGNAL (timeout ()), getImageListView(), SLOT (next ()));
	else
		connect (m_timer, SIGNAL (timeout ()), getImageListView(), SLOT (previous()));

	//------------------------------------------------------------------------------
	config->setGroup("TipOfDay");
	if(config->readBoolEntry("RunOnStart", true))
		slotShowTips();

	//------------------------------------------------------------------------------
//	readDockConfig (config/*, CONFIG_DOCK_GROUP*/);
}

void
MainWindow::writeConfig(KConfig *config)
{
	if(!m_inInterface) return;

#ifdef WANT_LIBKEXIDB
	config->setGroup("Category View");
	config->writeEntry("enable", getEnabledCategories());
#endif /* WANT_LIBKEXIDB */

	config->setGroup("Options");
	config->writeEntry( "xmluifile", m_xmluifile );
	config->writeEntry( "preview", m_p_preview_action->isChecked() );
	config->writeEntry( "m_openDirType", m_openDirType );
	if(m_openDirType==1) m_openDirname=getCurrentDir();
	config->writePathEntry( "m_openDirname",  m_openDirname);
	config->writeEntry( "m_showSP",  m_showSP);
	config->writeEntry( "m_startFS",  m_startFS);
	config->writeEntry( "m_showToolbar",  m_showToolbar);
	config->writeEntry( "m_showStatusbar",  m_showStatusbar);
	config->writeEntry( "movieViewer", getCurrentAvailableMovieViewerIndex());

	config->setGroup("Paths");
	config->writeEntry( "m_cdromPath", getcdromPath() );

	config->setGroup("Slideshow");
	config->writeEntry( "time", m_slideshowTime );
	config->writeEntry( "type", m_slideshowType );

	KMainWindow::saveMainWindowSettings(config);
	KMainWindow::saveWindowSize(config);
	writeDockConfig(config/*, CONFIG_DOCK_GROUP*/);

	config->sync();
}

void
MainWindow::createHideShowAction(KDockWidget* dock)
{
	if(!dock)
	{
		return;
	}
	QString caption;
	if (dock->mayBeHide())
	{
		caption=i18n("Hide %1").arg(dock->caption());
	} else
	{
		caption=i18n("Show %1").arg(dock->caption());
	}

	KAction* action=new KAction(caption, 0, dock, SLOT(changeHideShowState()), actionCollection() );
	if (dock->icon())
	{
		action->setIconSet( QIconSet(*dock->icon()) );
	}
	m_windowListActions.append(action);
}


void
MainWindow::updateSelections(ListItem *item)
{
	setUpdatesEnabled(false);

	if(
		m_p_currentListItemView &&
		item
	)
	{
		if(
			item->isSelected() &&
			m_p_currentListItemView != item->getListItemView() &&
			!item->getListItemView()->isDropping()
		)
		{
			m_p_currentListItemView->clearSelection();
			m_p_currentListItemView=item->getListItemView();
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
	createHideShowAction(m_dockIL);
	createHideShowAction(m_dockIV);
	createHideShowAction(m_dockDir);
	createHideShowAction(m_dockIMI);
	plugActionList("winlist", m_windowListActions);
}


void
MainWindow::setActionsEnabled(bool enable)
{
	int count=m_p_actions->count();
	for (int pos=0;pos<count;++pos)
		m_p_actions->action(pos)->setEnabled(enable);

}

void
MainWindow::slotSetStatusBarText(const QString& msg)
{
	statusBar()->changeItem(msg, SB_MSG);
}


void
MainWindow::setImageIndex (int index)
{
	m_imageIndex=index;
	setNbrItems(m_nbrItems);
}

void
MainWindow::setNbrItems (int nbr)
{
	m_nbrItems = nbr;

	QString msg;
	if(nbr==0)
		msg =i18n("no item");
	else
		if(m_imageIndex>=0&&!getDirectoryView()->showAllFile()&&!getDirectoryView()->showDir())
		msg =i18n("%2/%n item", "%2/%n items", m_nbrItems).arg(m_imageIndex+1);
	else
		msg =i18n("%n item", "%n items", m_nbrItems);

	statusBar()->changeItem(' '+msg+' ', SB_ITEMS);
}

void
MainWindow::setZoom (float zoom)
{
	if(!m_p_zoomCombo) return;

	QString nb; nb.setNum(zoom,'f',0);
	statusBar()->changeItem(QString(" %1% ").arg(nb), SB_SCALE );
	m_p_zoomCombo->disconnect( SIGNAL( activated ( const QString& ) ) );
	m_p_zoomCombo->setCurrentText(nb + " %");
	connect(m_p_zoomCombo, SIGNAL(activated ( const QString& )), this, SLOT(setZoom( const QString& )));
}

void
MainWindow::setZoom ( const QString& val)
{
	QRegExp reg("(\\d*)");
	reg.search(val);
	QStringList l_list = reg.capturedTexts();
	bool ok;
	float v = QString(l_list[1]).toInt(&ok);
	if(ok)
		getImageViewer()->setZoomValue(v/100);
}

void
MainWindow::setImagename (const QString& a_name)
{
	if(m_p_SB_NAME_Label)
		m_p_SB_NAME_Label->setText(a_name);
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
MainWindow::setDate (const QDateTime& a_date)
{
	setDate(KGlobal::locale()->formatDateTime(a_date, false));
}
void
MainWindow::setDate (const QString& a_date)
{
	if(m_p_SB_DATE_Label)
		m_p_SB_DATE_Label->setText(a_date);
}



void
MainWindow::forwardMenuActivated(int item)
{
	go(m_p_forward_action->popupMenu()->indexOf(item)+1);
}

bool
MainWindow::preview()
{
	return m_p_preview_action->isChecked();
}


QString
MainWindow::getFileName(QString *fullName)
{
		int debut = fullName->findRev ('/');
		int fin = fullName->findRev ('.');
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
MainWindow::findDir(const QString & a_dir)
{
	QString l_dir(a_dir);
	if(QFileInfo(l_dir).isDir() && !l_dir.endsWith("/")) l_dir+="/";
	ListItem *l_p_item=NULL;
	l_p_item = getDirectoryView()->getDir(l_dir);
	if(!l_p_item) l_p_item = getCDArchiveView()->getCDArchiveItem(l_dir);
	return l_p_item;
}


bool
MainWindow::openURL (const KURL& a_url)
{
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG
		<< "a_url="<< a_url.url() << " "
	<<endl;
#endif
	if(getDirectoryView()->isManagedURL(a_url))
	{
		m_sideBar->switchToTab(m_sideBar_id_dirView);
		getDirectoryView()->openURL(a_url);
	}
#ifdef WANT_LIBKEXIDB
	else
	if(getCategoryView()->isManagedURL(a_url))
	{
		m_sideBar->switchToTab(m_sideBar_id_catView);
		getCategoryView()->openURL(a_url);
	}
#endif
#ifndef SHOWIMG_NO_CDARCHIVE
	else
	if(getCDArchiveView()->isManagedURL(a_url))
	{
		m_sideBar->switchToTab(m_sideBar_id_cdarcView);
		getCDArchiveView()->openURL(a_url);
	}
#endif
	else
		KMessageBox::error(this, "<qt>"+i18n("The URL '<b>%1</b>' is not managed").arg(a_url.url())+"</qt>");
	
	return true;
}

void
MainWindow::changeURL()
{
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG	<<endl;
#endif
	KURL l_url(m_p_URLHistoryCompletion->replacedPath(m_p_URLHistory_combo->currentText()));
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG <<"l_url="<<l_url.url()<<endl;
#endif
	openURL(l_url);
}



bool
MainWindow::openDir (const QString& dir, bool a_updateHistory/*=true*/, bool a_loadThumbnails/*=true*/)
{
/*
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG
		<< "dir="<< dir << " "
		<< "a_updateHistory="<< a_updateHistory << " "
		<< "a_loadThumbnails="<< a_loadThumbnails << " "
	<<endl;
#endif
	if(!m_inInterface) return false;

	QString l_picName;
	QString l_dir;
	KURL l_url;
	l_url.setPath(dir);

	if(Tools::isImage(dir))
	{
		l_picName=QFileInfo(dir).fileName();
		l_dir=QFileInfo(dir).dirPath(true);
	}
	else
	{
		l_dir = QDir::convertSeparators(dir);
	}

	ListItem *l_p_ssrep = NULL;
	QFileInfo info( l_dir );
#ifdef Q_WS_WIN
	QStringList l_list = QStringList::split('/', info.absFilePath().mid(3)  ); // omit drive letter 
#else
	QStringList l_list = QStringList::split('/', info.absFilePath() );
#endif
	if(info.exists()
#ifndef SHOWIMG_NO_CDARCHIVE
		&& !l_dir.startsWith(CDArchive_ROOTPATH)
		&& !l_dir.startsWith(CDArchive::CDArchive_TEMP_ROOTPATH())
#endif
	)
	{
		l_p_ssrep = getDirectoryView()->getRoot();
		bool l_is_found = false;
#ifdef Q_WS_WIN //for win32 l_it can be "Desktop" m_root_dir item:
		if (l_dir==desktopDirPath())
		{
			//nothing to do
			l_is_found = true;
		}
		if (!l_is_found)
		{
			//try to find a drive for this dir
			while (l_p_ssrep && !l_dir.startsWith( QDir::convertSeparators( l_p_ssrep->fullName() ) ))
			{
				l_p_ssrep = l_p_ssrep->nextSibling();
			}
			if (!l_p_ssrep)
				return false;
		}
#endif

		if (!l_is_found)
		{
			l_p_ssrep = getDirectoryView()->openURL(l_url);
		}
	}
	else
	{
#ifndef SHOWIMG_NO_CDARCHIVE
		if( m_root_cdarchive &&
			(l_dir.startsWith(CDArchive::CDArchive_TEMP_ROOTPATH()) || l_dir.startsWith(CDArchive_ROOTPATH)))
		{
			l_p_ssrep = m_root_cdarchive->find(l_dir);
		}
#endif
		if(l_p_ssrep)
			l_p_ssrep->setOpen(true);
	}

	if (l_p_ssrep)
	{
		if(getDirectoryView())
			getDirectoryView()->setLoadThumbnails(a_loadThumbnails);
		getDirectoryView()->clearSelection();
		getDirectoryView()->slotShowItem(l_p_ssrep);
		getDirectoryView()->setCurrentItem(l_p_ssrep);
		getDirectoryView()->setSelected(l_p_ssrep, true);

		setCaption(l_dir);
		setCurrentURL(l_p_ssrep->getURL());

		m_sideBar->switchToTab(m_sideBar_id_dirView);

		if(a_updateHistory)
			updateHistory();
		if(!l_picName.isEmpty())
			getImageListView()->setCurrentItemName(l_picName);
		return true;
	}
	else
	{
		showUnableOpenDirectoryError(l_url);
		return false;
	}
	*/
	return false;
}

void
MainWindow::nextDir (ListItem *a_p_item)
{
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG	<<endl;
#endif
	if (a_p_item == 0)
	{
		return;
	}

	if(!a_p_item->fullName ())
	{
		return;
	}
	ListItem *l_p_item = a_p_item;
	while (l_p_item != 0)
	{
		if (!dynamic_cast<Directory*>(l_p_item))
		{
			l_p_item = 0;
		}
		else
		{
			if (l_p_item->isSelected ())
			{
				l_p_item->unLoad();
				l_p_item->load();
			}

			if (l_p_item->firstChild ())
			{
				nextDir (l_p_item->firstChild ());
			}
		}
		l_p_item = l_p_item->itemBelow ();
	}
}



void
MainWindow::slotRefresh (const QString& dir)
{
	QString res = QString(dir);
	int  pos = res.find ("/");
	ListItem *l_p_ssrep;

	res = res.right (res.length () - pos - 1);
	pos = res.find ("/");

	l_p_ssrep = getDirectoryView()->getRoot();
	while (pos != -1)
	{
		l_p_ssrep = l_p_ssrep->find (res.left (pos));
		res = res.right (res.length () - pos - 1);

		if (l_p_ssrep)
			pos = res.find ("/");
		else
			break;
	}
	l_p_ssrep = l_p_ssrep->find (res);
	if (l_p_ssrep && l_p_ssrep->isSelected ())
	{
		l_p_ssrep->refresh();
	}
}


void
MainWindow::slotRefresh (bool forceRefresh)
{
	getImageListView()->stopLoading();

	QPtrList<ListItem> l_list;
	QListViewItemIterator l_it (getDirectoryView()->getRoot());
	for (; l_it.current (); ++l_it)
	{
		if (l_it.current ()->isSelected ())
			l_list.append(dynamic_cast<ListItem*>((l_it.current())));
	}
	bool directoryRefreshed = false;
	for ( ListItem *dir=l_list.first(); dir != 0; dir=l_list.next() )
	{
		if(!forceRefresh)
		{
			directoryRefreshed = directoryRefreshed || dir->refresh(false);
		}
		else
		{
			dir->unLoad();
			dir->load();
		}
	}

	//------------------------------------------------
//	getDirectoryView()->slotRefresh();

#ifdef WANT_LIBKEXIDB
	getCategoryView()->slotRefresh();
#endif
	//------------------------------------------------
	getImageListView()->reload();
	if(directoryRefreshed)
	{
		getImageListView()->sort();
		getImageListView()->slotLoadFirst();
	}
}



void
MainWindow::setHasImageSelected(bool selected)
{
	m_hasimageselected=selected;

	m_p_copy_action->setEnabled(selected);
	m_p_copyPixmap_action->setEnabled(selected);
	m_p_editType_action->setEnabled(selected);
	m_p_fullScreen_action->setEnabled(selected);

	FileIconItem* l_p_selected_item = getImageListView()->firstSelected();
	if(l_p_selected_item)
	{
		m_p_fullScreen_action->setEnabled(
			Tools::isImage(l_p_selected_item->fullName()) ||
			Tools::isVideo(l_p_selected_item->fullName())
			);
		if(
			!dynamic_cast<ImageFileIconItem*>(l_p_selected_item) &&
			!dynamic_cast<DirFileIconItem*>(l_p_selected_item)
		)
		{
			m_p_paste_action->setEnabled(false);
			if(dynamic_cast<AlbumImageFileIconItem*>(l_p_selected_item))
			{
				m_p_actions->action("editdelete")->setText(i18n("Remove From Album"));
			}
			else
				if(dynamic_cast<CompressedImageFileIconItem*>(l_p_selected_item))
			{
				m_p_actions->action("editdelete")->setText(i18n("Remove From Archive"));
			}
			else
			{
				m_p_actions->action("editdelete")->setEnabled(false);
			}
		}
		else
		{
			m_p_actions->action("editdelete")->setText(i18n("Delete File"));
		}

		if(!getImageListView()->hasOnlyOneImageSelected())
		{
			m_p_editType_action->setEnabled(false);
			m_p_actions->action("EXIF orientation")->setEnabled(false);
		}
#ifdef HAVE_LIBEXIV2
		else
		{
			bool l_is_jpeg = Tools::isJPEG(l_p_selected_item->getURL(), l_p_selected_item->mimetype());
			m_p_actions->action("EXIF actions")->setEnabled(l_is_jpeg);
			m_p_actions->action("EXIF orientation")->setEnabled(l_is_jpeg);
		}
#endif /* HAVE_LIBEXIV2 */
		m_p_actions->action("Regenerate thumbnail")->setEnabled(l_p_selected_item->isImage());
	}
}


void
MainWindow::slotPreview ()
{
	getImageListView()->setThumbnailSize(false);
	if(m_p_preview_action->isChecked())
	{
		getImageListView()->slotLoadFirst();

		m_p_actions->action("Regenerate EXIF thumbnail")->setEnabled(true);
		m_p_actions->action("Regenerate thumbnail")->setEnabled(true);
	}
	else
	{
		slotStop();
		getImageListView()->slotResetThumbnail();

		m_p_actions->action("Regenerate EXIF thumbnail")->setEnabled(false);
		m_p_actions->action("Regenerate thumbnail")->setEnabled(false);
	}
}

void
MainWindow::slotSlideShow(int a_time)
{
	m_slideshowTime = a_time;
	slotSlideShow();
}

void
MainWindow::slotSlideShow ()
{
	if(!m_timer) return;
#ifdef HAVE_KIPI
	if (pluginManager() && pluginManager()->action("SlideShow...") && m_p_slideshow_action->isChecked())
	{
		pluginManager()->action("SlideShow...")->activate();
		m_p_slideshow_action->setChecked(false);
		return;
	}
#endif /* HAVE_KIPI */
	if (!m_timer->isActive())
	{
		if(!getImageListView()->hasImageSelected())
		{
			getImageListView()->first();
			if(!getImageListView()->hasImageSelected())
			{
				m_p_slideshow_action->setChecked(false);
				return;
			}
		}
		KApplication::setOverrideCursor (KCursor::blankCursor());
		m_timer->start (m_slideshowTime*1000, false);
		m_p_slideshow_action->setChecked(false);
		if(!m_inFullScreen) slotFullScreen();
	}
	else
		m_timer->stop ();
}


void
MainWindow::escapePressed()
{
	if(!m_inInterface)
	{
		m_requestingClose=true;
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
MainWindow::switchToSimpleUI()
{
	m_xmluifile="showimgsimpleui.rc";
	KMessageBox::information(this, i18n("Interface configuration has changed, you need to restart to take effect."));}

void
MainWindow::switchToFullUI()
{
	m_xmluifile="showimgui.rc";
	KMessageBox::information(this, i18n("Interface configuration has changed, you need to restart to take effect."));
}

void
MainWindow::switchToInterface()
{
	if(m_inInterface) 
		return;

	hide();

	QString currentPath = m_imageListSimple->currentAbsImagePath();
	m_p_iv->deleteLater(); m_p_iv=NULL;
	m_imageListSimple->deleteLater(); m_imageListSimple=NULL;

	KStartupLogo* logo=NULL;
	m_p_config = KGlobal::config();
	m_p_config->setGroup("Options");
	if( m_p_config->readBoolEntry("m_showSP", true))
	{
		logo = new KStartupLogo();
		logo->show();
	}
	m_singleton_mw = new MainWindow(currentPath, false, true);
	if(logo)
	{
		logo->hide();
		delete logo;
	}
	m_inInterface=true;
	finalizeGUI(this);
}


void
MainWindow::closeEvent( QCloseEvent* e)
{
	m_requestingClose=true;
	KMainWindow::closeEvent(e);
}


bool
MainWindow::closeAppl()
{
	m_requestingClose=true;
	return true;
}


bool
MainWindow::queryClose()
{
	if(m_deleteTempDirectoriesDone)
	{
		if(m_inFullScreen) slotFullScreen();
		if(getImageViewer()) getImageViewer()->writeConfig(m_p_config, CONFIG_IMAGEVIEWER_GROUP);
		if(getImageListView()) getImageListView()->writeConfig(m_p_config);
		if(getDirectoryView()) getDirectoryView()->writeConfig(m_p_config);
		if(getToolManager()) getToolManager()->writeConfig(m_p_config);
#ifdef WANT_LIBKEXIDB
		if(getCategoryView()) getCategoryView()->writeConfig(m_p_config);
#endif /* WANT_LIBKEXIDB */
		writeConfig(m_p_config);

		m_p_config->sync();
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
	if(!m_inInterface) return;
	hide();
	setUpdatesEnabled(false);

	if (!m_inFullScreen )
	{
		m_inFullScreen = true;

		writeDockConfig (m_p_config/*, CONFIG_DOCK_GROUP*/);

		makeDockInvisible(m_dockIL);
		makeDockInvisible(m_dockDir);
		makeDockInvisible(m_dockIMI);
		leftDock()->hide(); rightDock()->hide();

		toolBar("locationToolBar")->hide();
		menuBar()->hide();
		if(!m_showToolbar)
		{
			toolBar("mainToolBar")->hide(); toolBar("viewToolBar")->hide();
			topDock()->hide();
		}
		if(!m_showStatusbar)
		{
			statusBar()->hide();
			bottomDock()->hide();
		}

		m_p_back_action->setEnabled(false);
		m_p_forward_action->setEnabled(false);
		m_p_goHome_action->setEnabled(false);
		m_p_goUp_action->setEnabled(false);
		m_p_go_action->setEnabled(false);
		m_p_preview_action->setEnabled(false);

		getImageViewer()->setBackgroundColor(QColor("black"));

		setUpdatesEnabled(true);
		showFullScreen();
		KWin::setState(winId(), NET::StaysOnTop);
		getImageViewer()->setFocus();
#if KDE_IS_VERSION(3,3,0)
		KWin::raiseWindow(winId());
#endif
// 		kapp->processEvents();
		emit toggleFullscreen(m_inFullScreen);
	}
	else
	{
		m_inFullScreen = false;
		emit toggleFullscreen(m_inFullScreen);

		getImageViewer()->setBackgroundColor(m_bgColor);

		topDock()->show();
		menuBar()->show();

		bottomDock()->show();
		toolBar("mainToolBar")->show();
		toolBar("viewToolBar")->show();
		toolBar("locationToolBar")->show();

		statusBar()->show();

		///
		readDockConfig (m_p_config/*, CONFIG_DOCK_GROUP*/);

		///
		m_p_back_action->setEnabled(true);
		m_p_forward_action->setEnabled(true);
		m_p_goHome_action->setEnabled(true);
		m_p_goUp_action->setEnabled(true);
		m_p_go_action->setEnabled(true);
		m_p_preview_action->setEnabled(true);
		m_p_back_action->setEnabled(true);
		m_p_forward_action->setEnabled(true);
		m_p_goHome_action->setEnabled(true);
		m_p_go_action->setEnabled(true);
		m_p_goUp_action->setEnabled(true);

		if(m_timer->isActive()) {m_timer->stop(); m_p_slideshow_action->setChecked(false); KApplication::restoreOverrideCursor ();}

		getDirectoryView()->setLoadThumbnails(true);

		KWin::setState(winId(), NET::Normal);
		setUpdatesEnabled(true);
		showNormal();
	}
	m_p_fullScreen_action->setChecked(m_inFullScreen);
}

////////////////////////////////////////////////////////////////////////////////

void
MainWindow::saveNumberOfImages()
{
	m_savedNumberOfImages = m_total;
}

void
MainWindow::restoreNumberOfImages()
{
/*	m_total = m_savedNumberOfImages;
	setNbrItems(m_total);
	m_p_statusbarProgress->setTotalSteps (m_total);
	m_done=m_total;*/
}

int
MainWindow::getTotal()
{
	return m_total;
}

void
MainWindow::slotAddImage (int number)
{
	m_total+=number;
	setNbrItems(m_total);
	m_p_statusbarProgress->setTotalSteps (m_total);
}
void
MainWindow::slotRemoveImage ()
{
	m_total--;
	setNbrItems(m_total);
	m_p_statusbarProgress->setTotalSteps (m_total);
	m_p_statusbarProgress->setProgress (m_total);
}
void
MainWindow::slotRemoveImage (int val)
{
	m_total -= val;
	setNbrItems(m_total);
	m_p_statusbarProgress->setTotalSteps (m_total);
	m_p_statusbarProgress->setProgress (m_total);
}
void
MainWindow::slotPreviewDone (int number)
{
	m_done += number;
	m_p_statusbarProgress->setProgress (m_done);

	if(m_statusbarProgressLastUpdate.time().msecsTo(QDateTime::currentDateTime().time()) >= 500)
	{
		m_statusbarProgressLastUpdate=QDateTime::currentDateTime ();
		//kapp->processEvents();
	}
}
void
MainWindow::slotReset (bool init)
{
	m_p_stop_action->setEnabled(true);
	if(init)
	{
		m_done = 1;
		m_p_statusbarProgress->setProgress (-1);
	}
	m_p_statusbarProgress->show();
}

void
MainWindow::slotDone(int numberOfItems)
{
	slotRemoveImage(numberOfItems);
	slotDone();
}

void
MainWindow::slotDone()
{
	m_p_stop_action->setEnabled(false);
	m_total = getImageListView()->allItemsURL().count(); setNbrItems(m_total);
	m_done = m_total;
	m_p_statusbarProgress->hide();

	if(m_p_preview_action->isChecked())
	{
		m_p_actions->action("Regenerate thumbnail")->setEnabled(true);
		m_p_actions->action("Regenerate EXIF thumbnail")->setEnabled(true);
	}
}

//////////////////////////////////////////////////////////////////////////////

void
MainWindow::slotDirChange (const QString& a_path)
{
	if(QFileInfo(a_path).isDir())
	{
		ListItem* l_p_item = findDir(a_path);
		if(l_p_item)
		{
			bool hasChanged = l_p_item->refresh();
			if(hasChanged)
				getImageListView()->slotLoadFirst();
		}
	}
}


void
MainWindow::slotDirChange_created(const QString& /*path*/)
{
}

void
MainWindow::slotDirChange_deleted(const QString& path)
{
	getDirectoryView()->removeDir(path);
}

//////////////////////////////////////////////////////////////////////////////

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
	KApplication::clipboard()->setData(
		new KURLDrag(
			getImageListView()->selectedURLs(), 
			this, 
			"MainWindow"
			)
		);
}


void
MainWindow::slotcut ()
{
	slotTODO ();
}

void
MainWindow::slotpaste ()
{
	KURL::List l_list;
	if(KURLDrag::decode(KApplication::clipboard()->data(), l_list))
	{
		if(!l_list.isEmpty())
			getDirectoryView()->copy(l_list, getCurrentURL());
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
	QString l_destDir = 
		KFileDialog::getExistingDirectory(getCurrentDir(),
							this,
							i18n("Open Location"));

	if(l_destDir.isEmpty())
		return;

	if(!QFileInfo(l_destDir).exists())
	{
		KMessageBox::error(this, "<qt>"+i18n("The directory '<b>%1</b>' does not exist").arg(l_destDir)+"</qt>");
		return;
	}
	
	openURL(l_destDir);
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
	KKeyDialog::configure(m_p_actions, this);
}


void
MainWindow::configureToolbars()
{
#if KDE_IS_VERSION(3,3,0)
	KMainWindow::configureToolbars();
#endif
}


void
MainWindow::configureShowImg()
{
	ConfShowImg conf(this);
	conf.initColor(getImageViewer()->bgColor(), getImageViewer()->toGrayscale());

	conf.initFiling(m_openDirType, m_openDirname, m_showSP, m_startFS);

	conf.initMiscellaneous(getImageViewer()->smooth(), getDirectoryView()->loadFirstImage(), getDirectoryView()->showHiddenDir(), getDirectoryView()->showHiddenFile(), getDirectoryView()->showDir(), getDirectoryView()->showAllFile(), getImageListView()->preloadIm(), getDirectoryView()->getShowCompressedFiles());

	conf.initThumbnails(getImageListView()->getImageLoader()->getStoreThumbnails(), getImageListView()->getImageLoader()->getShowFrame(), getImageViewer()->useEXIF(), getImageListView()->wordWrapIconText(), getImageListView()->getShowMimeType(), getImageListView()->getShowSize(), getImageListView()->getShowDate(), getImageListView()->getShowDimension(), getImageListView()->getShowCategoryInfo(), getImageListView()->getShowToolTips());

	conf.initSlideshow(m_slideshowType, m_slideshowTime);

	conf.initFullscreen(m_showToolbar, m_showStatusbar);

	conf.initOSD(getImageListView()->getOSD()->getShowOSD(), getImageListView()->getOSD()->getOSDOnTop(), getImageListView()->getOSD()->font(),  getImageListView()->getOSD()->getOSDShowFilename(), getImageListView()->getOSD()->getOSDShowFullpath(), getImageListView()->getOSD()->getOSDShowDimensions(), getImageListView()->getOSD()->getOSDShowComments(), getImageListView()->getOSD()->getOSDShowDatetime(), getImageListView()->getOSD()->getOSDShowExif());

	conf.initProperties(getImageListView()->showMeta(), getImageListView()->showHexa());

	conf.initPaths(getcdromPath(), getImageListView()->getgimpPath(), getToolManager()->getConvertPath(), getToolManager()->getJpegtranPath(), getDirectoryView()->getUnrarPath());

	conf.initImagePosition(getImageViewer()->getImagePosition());

	conf.initVideo(getDirectoryView()->getShowVideo(), getAvailableMovieViewer(), getCurrentAvailableMovieViewerIndex());
#ifdef WANT_LIBKEXIDB
	conf.initCategories(getEnabledCategories(), getCategoryView()->getAddAllImages(), getCategoryView()->getType(), getCategoryView()->getSqlitePath(), getCategoryView()->getMysqlUsername(), getCategoryView()->getMysqlPassword(), getCategoryView()->getMysqlHostname());
#endif

	if(conf.exec())
	{
		m_openDirType=conf.getOpenDirType();
		m_openDirname=conf.getOpenDir();
		m_showSP=conf.checkshowSP();
		m_startFS=conf.checkstartFS();
		getImageViewer()->setUseEXIF(conf.getUseEXIF());
		m_showToolbar=conf.getShowToolbar();
		m_showStatusbar=conf.getShowStatusbar();

		getImageListView()->setShowMimeType(conf.getShowMimeType());
		getImageListView()->setShowSize(conf.getShowSize());
		getImageListView()->setShowDate(conf.getShowDate());
		getImageListView()->setShowDimension(conf.getShowDimension());
		getImageListView()->setWordWrapIconText(conf.getWordWrapIconText());
		getImageListView()->setShowToolTips(conf.getShowTooltips());
		getImageListView()->setShowCategoryInfo(conf.getShowCategoryInfo());

		getDirectoryView()->setShowHiddenDir(conf.getShowHiddenDir());
		getDirectoryView()->setShowHiddenFile(conf.getShowHiddenFile());
		getDirectoryView()->setShowDir(conf.getShowDir());
		getDirectoryView()->setLoadFirstImage(conf.getLoadFirstImage());
		getDirectoryView()->setShowAllFile(conf.getShowAll());
		getDirectoryView()->setShowVideo(conf.getVideoEnabled());
		getDirectoryView()->setShowCompressedFiles(conf.getShowArchives());

		getImageListView()->setPreloadIm(conf.getPreloadIm());
		getImageListView()->setRandom(conf.getSlideshowType()==2);
		getImageListView()->setShowMeta(conf.getShowMeta());
		getImageListView()->setShowHexa(conf.getShowHexa());

		getImageViewer()->setBackgroundColor(conf.getColor());
		getImageViewer()->setToGrayscale(conf.getGrayscale());
		getImageViewer()->setSmooth(conf.getSmooth());

		m_slideshowTime=conf.getSlideshowTime();
		m_slideshowType=conf.getSlideshowType();
		delete(m_timer); m_timer = new QTimer (this);
		if(m_slideshowType == 0 )
			connect (m_timer, SIGNAL (timeout ()), getImageListView(), SLOT (next ()));
		else
			connect (m_timer, SIGNAL (timeout ()), getImageListView(), SLOT (previous()));


		getImageListView()->getImageLoader()->setStoreThumbnails(conf.getStoreth());
		getImageListView()->getImageLoader()->setShowFrame(conf.getShowFrame());
		getImageListView()->getImageLoader()->setUseEXIF(getImageViewer()->useEXIF());
		getImageListView()->getOSD()->initOSD(conf.getShowOSD(), conf.getOSDOnTop(), conf.getOSDFont(), conf.getOSDShowFilename(), conf.getOSDShowFullpath(), conf.getOSDShowDimensions(), conf.getOSDShowComments(), conf.getOSDShowDatetime(), conf.getOSDShowExif());

		setLayout(conf.getLayout());

#ifdef HAVE_KIPI
		conf.applyPlugins();
		pluginManager()->loadPlugins();
#endif /* HAVE_KIPI */

		getImageListView()->selectionChanged();
		getDirectoryView()->slotSelectionChanged();

		m_cdromPath = conf.getcdromPath();
		getImageListView()->setgimpPath(conf.getgimpPath());
		getToolManager()->setConvertPath(conf.getconvertPath());
		getToolManager()->setJpegtranPath(conf.getjpegtranPath());
		getDirectoryView()->setUnrarPath(conf.getunrarPath());

		getImageViewer()->setImagePosition(conf.getImagePosition());

		getDirectoryView()->setShowVideo(conf.getVideoEnabled()>=0);
		setCurrentAvailableMovieViewerIndex(conf.getVideoEnabled());

#ifdef WANT_LIBKEXIDB
		setEnabledCategories(conf.getCategoriesEnabled());
		getCategoryView()->setAddAllImages(conf.getCategoriesAddAllImages());
		getCategoryView()->setType(conf.getCategoriesType());
		getCategoryView()->setSqlitePath(conf.getCategoriesSqlitePath());
		getCategoryView()->setMysqlUsername(conf.getCategoriesMysqlUsername());
		getCategoryView()->setMysqlPassword(conf.getCategoriesMysqlPassword());
		getCategoryView()->setMysqlHostname(conf.getCategoriesMysqlHostname());
#endif
		slotRefresh(true);
	}
}


void
MainWindow::setLayout(int layout)
{
	if(!m_dockDir || !m_dockIL) return;

	switch(layout)
	{
		case 1:
				m_dockDir->manualDock(m_dockIV, KDockWidget::DockLeft, 35);
				m_dockIL->manualDock(m_dockDir, KDockWidget::DockBottom, 35);
				//m_p_arrangementB_action->setChecked(true);
				//m_p_arrangementLR_action->setChecked(true);
			break;
		case 2:
				m_dockDir->manualDock(m_dockIV, KDockWidget::DockTop, 35);
				m_dockIL->manualDock(m_dockDir, KDockWidget::DockRight, 50);
				//m_p_arrangementB_action->setChecked(true);
				//m_p_arrangementLR_action->setChecked(true);
			break;
		case 3:
				m_dockIL->manualDock(m_dockIV, KDockWidget::DockRight, 35);
				m_dockDir->manualDock(m_dockIV, KDockWidget::DockTop, 35);
				//m_p_arrangementB_action->setChecked(true);
				//m_p_arrangementLR_action->setChecked(true);
			break;
		case 4:
				m_dockDir->manualDock(m_dockIV, KDockWidget::DockLeft, 35);
				m_dockIL->manualDock(m_dockIV, KDockWidget::DockTop, 10);
				//m_p_arrangementB_action->setChecked(true);
				//m_p_arrangementTB_action->setChecked(true);
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
MainWindow::openBookmarkURL(const QString & a_url)
{
	openURL(a_url);
}


QString
MainWindow::currentTitle() const
{
	return getCurrentURL().prettyURL();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::addToBookmark(const QString& groupText, const QString& l_url)
{
	KBookmarkGroup l_root_dir = ShowImgBookmarkManager::self()->root();
	KBookmark bookm;
	bool bmAlbumExists = false;
	for(bookm = l_root_dir.first(); !bookm.isNull(); bookm = l_root_dir.next(bookm))
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
		bmg = ShowImgBookmarkManager::self()->root().createNewFolder(ShowImgBookmarkManager::self(), groupText);
		ShowImgBookmarkManager::self()->root().moveItem(bmg, KBookmarkGroup());
	}
	else
	{
		bmg=bookm.toGroup();
	}
	bmg.addBookmark(ShowImgBookmarkManager::self(), l_url, l_url, KMimeType::iconForURL(l_url));
	ShowImgBookmarkManager::self()->emitChanged(l_root_dir);
}


void
MainWindow::changeURL(const KURL& a_url)
{
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG	<<"a_url="<<a_url.url()<<endl;
#endif
	setCurrentURL(a_url);
	setCaption(currentTitle());
	updateHistory();
}


void
MainWindow::backMenuActivated(int item)
{
	go(-(m_p_back_action->popupMenu()->indexOf(item)+1));
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

	int newPos = m_lstHistory_list.at() + steps;
	HistoryEntry * l_p_hist = m_lstHistory_list.at( newPos );
	if(l_p_hist)
	{
		if(openURL(l_p_hist->fileURL))
		{
			m_p_back_action->setEnabled( m_lstHistory_list.at() > 0 );
			m_p_forward_action->setEnabled( m_lstHistory_list.at() != ((int)m_lstHistory_list.count())-1 );
		}
	}
}


void
MainWindow::updateHistory()
{
#if MAINWINDOW_DEBUG_CHANGEURL > 0
	MYDEBUG<<"getCurrentURL()="<<getCurrentURL()<<endl;
#endif
	KURL l_url(getCurrentURL());

	m_p_URLHistory_combo->setEditText(l_url.url());
	m_p_URLHistoryCompletion->addItem(l_url.url());

	m_p_URLHistory_combo->addToHistory(l_url.url());

	HistoryEntry * l_p_current = m_lstHistory_list.current();
	HistoryEntry * l_p_newEntry = new HistoryEntry;
	l_p_newEntry->fileURL = l_url;
	if (
		l_p_current && 
		l_p_current->fileURL == l_p_newEntry->fileURL
	)
	{
		delete l_p_newEntry;
		return ;//l_p_newEntry;
	}
	if (l_p_current)
	{
		m_lstHistory_list.at( m_lstHistory_list.count() - 1 ); // go to last one
		for ( ; m_lstHistory_list.current() != l_p_current ; )
		{
			m_lstHistory_list.removeLast();
		}
	}
	m_lstHistory_list.append(l_p_newEntry);
	m_p_back_action->setEnabled( m_lstHistory_list.at() > 0 );
	m_p_forward_action->setEnabled( 
		m_lstHistory_list.at() != ((int)m_lstHistory_list.count())-1 
		);
}


void
MainWindow::slotForwardAboutToShow()
{
	m_p_forward_action->popupMenu()->clear();
	HistoryAction::fillHistoryPopup( m_lstHistory_list, m_p_forward_action->popupMenu(), false, true );
}


void
MainWindow::slotBackAboutToShow()
{
	m_p_back_action->popupMenu()->clear();
	HistoryAction::fillHistoryPopup( m_lstHistory_list, m_p_back_action->popupMenu(), true, false );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::slotArrangement()
{
	if(m_p_arrangementLR_action->isChecked())
		getImageListView()->setArrangement(ImageListView::LeftToRight);
	else
		getImageListView()->setArrangement(ImageListView::TopToBottom);
	getImageListView()->setThumbnailSize(false);
}

void
MainWindow::slotTxtPos()
{
	if(m_p_arrangementR_action->isChecked())
		getImageListView()->setItemTextPos(ImageListView::Right);
	else
		getImageListView()->setItemTextPos(ImageListView::Bottom);
}


bool
MainWindow::fullScreen()
{
	return m_inFullScreen;
}


void
MainWindow::setEmptyImage()
{
	getImageViewer()->openURL(NULL);
}


void
MainWindow::copyFilesTo(const KURL::List& a_list, const KURL& a_dest)
{
	getDirectoryView()->copy(a_list, a_dest);
}


void
MainWindow::moveFilesTo(const KURL::List& a_list, const KURL& a_dest)
{
	getDirectoryView()->move(a_list, a_dest);
}




void
MainWindow::goHome()
{
	openURL(QDir::homeDirPath());
}


void
MainWindow::goUp()
{
	openURL(getCurrentURL().upURL());
	
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
	KonqOperations::del(this, KonqOperations::DEL, getImageListView()->getImageLoader()->clearThumbnailDir(fromDir));
#else
	(void)KIO::del(getImageListView()->getImageLoader()->clearThumbnailDir(fromDir));
#endif
}


void
MainWindow::updateCache()
{
	KURL::List l_list = getImageListView()->getImageLoader()->updateThumbnailDir(getCurrentDir());

	m_pdCache=new KProgressDialog (this, "Thumbnail",
				i18n("Updating Thumbnails"),
				QString::null,
				true);
	m_pdCache->setLabel(i18n("Updating in progress..."));
	m_pdCache->progressBar()->setTotalSteps(2);
	m_pdCache->progressBar()->setProgress(2);
	m_pdCache->show();
	m_pdCache->adjustSize();
	l_list += updateCache(getCurrentDir());
	m_pdCache->close();
	delete(m_pdCache);

	//
#ifndef Q_WS_WIN
	KonqOperations::del(this, KonqOperations::DEL, l_list);
#else
	(void)KIO::del( l_list );
#endif
}


KURL::List
MainWindow::updateCache(const QString& fromDir)
{
	m_pdCache->setLabel(
		"<qt>"
		+i18n("Updating in progress for: <center>%1</center>")
			.arg(fromDir)
		+"</qt>"
		);
// 	kapp->processEvents();

	QDir d(QDir::homeDirPath()+"/.showimg/cache/"+fromDir);
	d.setFilter( QDir::All | QDir::Hidden | QDir::NoSymLinks );
	const QFileInfoList *dlist = d.entryInfoList();

	if(!dlist) return KURL::List();

	int len=QString(QDir::homeDirPath()+"/.showimg/cache").length();
	KURL::List l_list;
	QFileInfoListIterator l_it( *dlist );
	QFileInfo *l_p_fi;
	KURL m_url;
	while ( (l_p_fi = l_it.current()) != 0 )
	{
		QString fCache=l_p_fi->absFilePath();
		QString orgFile=fCache.right(fCache.length()-len);
		if(l_p_fi->isDir() && !fromDir.startsWith(orgFile))
		{
			l_list += updateCache(orgFile);
		}
		else
		{
			if(!QFileInfo(orgFile).exists() && QFileInfo(orgFile).extension(false)!="dat")
			{
				m_url.setPath(fCache);
				l_list.append(m_url);

				m_url.setPath(fCache+".dat");
				l_list.append(m_url);

			}
		}
		++l_it;
	}
	return l_list;

}

void
MainWindow::removeObsololeteFilesOfTheDatabase()
{
#ifdef WANT_LIBKEXIDB
	int num = getCategoryView()->removeObsololeteFilesOfTheDatabase();
	if(num>0)
		KMessageBox::information(this, i18n("%1 obsolete image(s) have been removed of the database.").arg(num));
	else
	if(num==0)
		KMessageBox::information(this, i18n("No obsolete file found."));
	else
		KMessageBox::error(this, i18n("Unable to check for obsolete files into database"));
#endif
}

void
MainWindow::slotDisplayNBImg()
{
	m_p_time_action->setText(i18n("1 image seen", "%n images seen", getImageViewer()->getNbImg()));
#ifdef WANT_LIBKEXIDB
	KMessageBox::information(this, "<qt>" + i18n("You have already seen <b>1</b> image.", "You have already seen <b>%1</b> images.\nThere are <b>%2</b> images in your database.").arg(KGlobal::locale()->formatNumber(getImageViewer()->getNbImg(),0)).arg(KGlobal::locale()->formatNumber(getCategoryDBManager()->getNumberOfImages(),0)) + "</qt>");
#else
        KMessageBox::information(this, "<qt>" + i18n("You have already seen <b>1</b> image.", "You have already seen <b>%1</b> images.").arg(KGlobal::locale()->formatNumber(getImageViewer()->getNbImg(),0)));
#endif
}


void
MainWindow::setCurrentURL(const KURL & a_url)
{
	m_currentURL = a_url;
//	if(QFileInfo(m_currentDir).isDir() && !m_currentDir.endsWith(QChar(QDir::separator())))
//			m_currentDir+=QDir::separator();
}


QString
MainWindow::getCurrentDir() const
{
	return getCurrentURL().path();
	//return m_currentDir;
}

KURL
MainWindow::getCurrentURL() const
{
	return m_currentURL;
	//return m_currentDir;
}

QString
MainWindow::currentURL() const
{
	return getCurrentURL().url();
	//return m_currentDir;
}


QString
MainWindow::getcdromPath()
{
	return m_cdromPath;
}


#ifdef HAVE_KIPI
KIPIPluginManager*
MainWindow::pluginManager()
{
	return m_p_pluginManager;
}
#endif /*HAVE_KIPI*/


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::deleteTempDirectories()
{
    hide();

    KURL::List l_list;
    KURL l_url;

    if(KStandardDirs::exists(locateLocal("tmp", "showimg-cpr/")))
    {
    	l_url.setPath(locateLocal("tmp", "showimg-cpr/"));
    	l_list.append(l_url);
    }
    if(KStandardDirs::exists(locateLocal("tmp", "showimg-arc/")))
    {
	    l_url.setPath(locateLocal("tmp", "showimg-arc/"));
	    l_list.append(l_url);
    }
    if(KStandardDirs::exists(locateLocal("tmp", "showimg-net/")))
    {
	    l_url.setPath(locateLocal("tmp", "showimg-net/") );
	    l_list.append(l_url);
    }
    KIO::DeleteJob *job = KIO::del( l_list );
    connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(deleteTempDirectoriesDone( KIO::Job * )));

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
	m_deleteTempDirectoriesDone=true;
	close();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
MainWindow::showUnableOpenDirectoryError(const KURL& a_url)
{
	KMessageBox::sorry(getImageViewer(),
		"<qt>"+i18n("Unable to open the directory <b>%1</b>").arg(a_url.url( ))+"</qt>",
		i18n("Directory does not exist"),
		KMessageBox::Notify);
}

KURL
MainWindow::getLastDestURL()
{
	return m_lastDestURL;
}

void
MainWindow::setLastDestURL(const KURL& a_url)
{
	m_lastDestURL = a_url;
	emit lastDestURLChanged(getLastDestURL());
}


//------------------------------------------------------------------------------
Tools*
MainWindow::getToolManager()
{
	return m_p_tools;
}

#ifdef WANT_LIBKEXIDB
CategoryDBManager*
MainWindow::getCategoryDBManager()
{
	return getCategoryView()->getCategoryDBManager();
}
const CategoryDBManager*
MainWindow::getCategoryDBManager() const
{
        return getCategoryView()->getCategoryDBManager();
}

#endif

ImageMetaInfo*
MainWindow::getImageMetaInfo()
{
	return m_p_imageMetaInfo->getImageMetaInfo();
}

void
MainWindow::updateDB(QDict<QString>& renamedFiles)
{
#ifdef WANT_LIBKEXIDB
	getCategoryView()->renameImage(renamedFiles);
#endif
}

#ifdef WANT_LIBKEXIDB
void
MainWindow::setEnabledCategories(bool enable)
{
	m_categoriesEnabled = enable;
	getCategoryView()->setEnabled(m_categoriesEnabled);
	m_p_tb_cat->setEnabled(m_categoriesEnabled);
}
bool
MainWindow::getEnabledCategories()
{
	return m_categoriesEnabled;
}
#endif

MainWindow &
MainWindow::getInstance()
{
	return *m_singleton_mw;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ShowImgBookmarkManager::ShowImgBookmarkManager( const QString& bookmarksFile,
		bool bImportDesktopFiles)
:KBookmarkManager(bookmarksFile,bImportDesktopFiles )
{
}

ShowImgBookmarkManager*
ShowImgBookmarkManager::self()
{
	QDir dirDest(QDir::homeDirPath () + "/.showimg/");
	if (!dirDest.exists())
		QDir().mkdir(dirDest.absPath ());
	ShowImgBookmarkManager* l_p_manager = dynamic_cast<ShowImgBookmarkManager*>(KBookmarkManager::managerForFile(dirDest.absPath ()+"/bookmark.xml", false));
  if(!l_p_manager)
    l_p_manager=new ShowImgBookmarkManager(dirDest.absPath ()+"/bookmark.xml", false);
  return l_p_manager;
}

#if ! KDE_IS_VERSION(3,3,0)
// hack for moc
namespace KParts
{
	class DockMainWindow;
}
#endif
#include "mainwindow.moc"
