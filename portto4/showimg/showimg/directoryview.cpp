/*****************************************************************************
                          directoryview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
    email                : rgroult@jalix.org
 ****************************************************************************/

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

#include "directoryview.h"

//-----------------------------------------------------------------------------
#define DIRECTORYVIEW_DEBUG               0
#define DIRECTORYVIEW_DEBUG_LOAD          0
#define DIRECTORYVIEW_DEBUG_SELECTEDITEMS 0
#define DIRECTORYVIEW_DEBUG_TREEVIEW      0
#define DIRECTORYVIEW_DEBUG_OPENURL       0
#define DIRECTORYVIEW_DEBUG_COPYMOVE      1
//-----------------------------------------------------------------------------

// Local
#include "album.h"
#include "compressedimagefileiconitem.h"
#include "describealbum.h"
#include "directory.h"
#include "extract.h"
#include "imagefileiconitem.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "krar.h"
#include "listitem.h"
#include "mainwindow.h"
#include "showimg_common.h"
#include "tools.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qclipboard.h>
#include <qdir.h>
#include <qptrlist.h>
#include <qstring.h>

// KDE
#include <kaction.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kdirlister.h>
#include <kdirwatch.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <klineeditdlg.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpopupmenu.h>
#include <kpropertiesdialog.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>

#if KDE_IS_VERSION(3,3,0)
#include <kinputdialog.h>
#endif

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

DirectoryView::DirectoryView (
			QWidget    *a_p_parent,
			const char *a_p_name)
	: ListItemView (a_p_parent, a_p_name),

	m_showCompressedFiles(true),
	m_root_dir(0)
{
	setShowHiddenDir(false);

	/////
	m_p_treeview_lister = new KDirLister();
	m_p_treeview_lister->setMimeFilter(
		  Tools::getDirectoryMimeFilter()
		+ Extract::getArchiveMimeFilter()
		);

	m_p_treeview_lister->setMainWindow( this );
	connect(m_p_treeview_lister, SIGNAL(refreshItems( const KFileItemList&)),
			this, SLOT(treeview_refreshItems( const KFileItemList&)));
	connect(m_p_treeview_lister, SIGNAL(clear(const KURL&)),
			this, SLOT(treeview_cleared(const KURL&)));
	connect(m_p_treeview_lister, SIGNAL(newItems(const KFileItemList&)),
			this, SLOT(treeview_newItem(const KFileItemList&)));
	connect(m_p_treeview_lister, SIGNAL(deleteItem( KFileItem * )),
			this, SLOT(treeview_itemDeleted( KFileItem * )));
	connect(m_p_treeview_lister, SIGNAL(started( const KURL& )),
			this, SLOT(treeview_jobStarted( const KURL& )));
	connect(m_p_treeview_lister, SIGNAL(completed()),
			this, SLOT(treeview_jobCompleted(  )));
	connect(m_p_treeview_lister, SIGNAL(infoMessage( const QString& )),
			getMainWindow(), SLOT(slotSetStatusBarText (const QString& )));


	//
	m_p_selecteditems_lister = new KDirLister();
	QStringList l_selecteditems_mime;
		l_selecteditems_mime+=Tools::getDirectoryMimeFilter();
		l_selecteditems_mime+=Tools::getImageMimeFilter();
		l_selecteditems_mime+=Tools::getVideoMimeFilter();
	m_p_selecteditems_lister->setMimeFilter(l_selecteditems_mime);

	m_p_selecteditems_lister->setMainWindow( this );
	connect(m_p_selecteditems_lister, SIGNAL(refreshItems( const KFileItemList&)),
			this, SLOT(selecteditems_refreshItems( const KFileItemList&)));
	connect(m_p_selecteditems_lister, SIGNAL(clear(const KURL&)),
			this, SLOT(selecteditems_cleared(const KURL&)));
	connect(m_p_selecteditems_lister, SIGNAL(clear()),
			this, SLOT(selecteditems_cleared()));
	connect(m_p_selecteditems_lister, SIGNAL(newItems(const KFileItemList&)),
			this, SLOT(selecteditems_newItem(const KFileItemList&)));
	connect(m_p_selecteditems_lister, SIGNAL(deleteItem( KFileItem * )),
			this, SLOT(selecteditems_itemDeleted( KFileItem * )));
	connect(m_p_selecteditems_lister, SIGNAL(started( const KURL& )),
			this, SLOT(selecteditems_jobStarted( const KURL& )));
	connect(m_p_selecteditems_lister, SIGNAL(completed()),
			this, SLOT(selecteditems_jobCompleted(  )));
	connect(m_p_selecteditems_lister, SIGNAL(infoMessage( const QString& )),
			getMainWindow(), SLOT(slotSetStatusBarText (const QString& )));

	//
	connect(getMainWindow(), SIGNAL(lastDestURLChanged(const KURL&)),
			this, SLOT(updateDestURLTitle(const KURL&)));
}


DirectoryView::~DirectoryView()
{
	delete(m_p_treeview_lister);
	delete(m_p_selecteditems_lister);
}

//-----------------------------------------------------------------------------

void
DirectoryView::readConfig(KConfig *a_p_config)
{
	a_p_config->setGroup("Options");
	setShowHiddenDir(a_p_config->readBoolEntry("showhiddenDir", false));
	setShowHiddenFile(a_p_config->readBoolEntry("showhiddenFile", false));
	setShowDir(a_p_config->readBoolEntry("showDir", true));
	setShowAllFile(a_p_config->readBoolEntry("showallFile", false));
	setLoadFirstImage(a_p_config->readBoolEntry("loadFirstImage", false));
	setShowVideo(a_p_config->readBoolEntry("enable video", true));
	setUnrarPath(a_p_config->readPathEntry("unrarPath", "unrar"));
	setShowCompressedFiles(a_p_config->readBoolEntry("showArchives", true));

	a_p_config->setGroup("DirectoryView");
	setColumnWidth (COLUMN_TYPE, a_p_config->readNumEntry("COLUMN_TYPE", 0 ));
	setColumnWidth (COLUMN_SIZE, a_p_config->readNumEntry("COLUMN_SIZE", 60 ));
	setColumnWidth (COLUMN_SELECT, a_p_config->readNumEntry("COLUMN_SELECT", 3*ListItem::TREE_CHECKBOX_MAXSIZE/2 ));
}

void
DirectoryView::writeConfig(KConfig *a_p_config)
{
	a_p_config->setGroup("Options");
	a_p_config->writeEntry( "showhiddenDir", showHiddenDir() );
	a_p_config->writeEntry( "showhiddenFile", showHiddenFile() );
	a_p_config->writeEntry( "showDir", showDir() );
	a_p_config->writeEntry( "showallFile", showAllFile() );
	a_p_config->writeEntry( "loadFirstImage", loadFirstImage() );
	a_p_config->writeEntry( "enable video", getShowVideo() );
	a_p_config->writeEntry( "unrarPath", getUnrarPath() );
	a_p_config->writeEntry( "showArchives", getShowCompressedFiles() );

	a_p_config->setGroup("DirectoryView");
	a_p_config->writeEntry( "COLUMN_TYPE", columnWidth(COLUMN_TYPE) );
	a_p_config->writeEntry( "COLUMN_SIZE", columnWidth(COLUMN_SIZE) );
	a_p_config->writeEntry( "COLUMN_SELECT", columnWidth(COLUMN_SELECT) );


	a_p_config->sync();
}


void
DirectoryView::initActions(KActionCollection *a_p_actionCollection)
{
	setActionCollection(a_p_actionCollection);

	m_p_dirCopy_action =
			new KAction(i18n("Copy Folder To..."), "editcopy", 0,
						this, SLOT(slotDirCopy()),
						getActionCollection(), "editdircopy");
	m_p_dirMove_action =
			new KAction(i18n("Move Folder To..."), 0,
						this, SLOT(slotDirMove()),
						getActionCollection(), "editdirmove");

	m_p_dirMoveToLast_action =
			new KAction(i18n("Move Folder to Last Directory"), 0,
								this, SLOT(slotDirMoveToLast()),
								getActionCollection(), "moveDirToLast");
	m_p_dirCopyToLast_action =
			new KAction(i18n("Copy Folder to Last Directory"), 0,
								this, SLOT(slotDirCopyToLast()),
								getActionCollection(), "copyDirToLast");

	m_p_dirPasteFiles_action =
			new KAction(i18n("Paste Files"), "editpaste", 0,
						this, SLOT(slotDirPasteFiles()),
						getActionCollection() , "editdirpaste files");
	m_p_dirRecOpen_action =
			new KAction(i18n("Recursively Open"), 0,
						this, SLOT(recursivelyOpen()),
						getActionCollection() , "dirRecOpen");

	m_p_dirRename_action =
			new KAction(i18n("&Rename Item..."), "item_rename", 0,
						this, SLOT(slotRename()),
						getActionCollection() , "editdirrename");

	m_p_dirTrash_action =
			new KAction(i18n("&Move Item to Trash"), "edittrash", 0,
						this, SLOT(slotTrash()),
						getActionCollection() , "editdirtrash");
	m_p_dirDelete_action =
			new KAction(i18n("&Delete"), "editdelete", 0,
						this, SLOT(slotSuppr()),
						getActionCollection() , "editdirdelete");

	m_p_previousDir_action =
			new KAction(i18n("Previous Directory"), "1leftarrow", KShortcut(SHIFT+Key_Space),
						this, SLOT(goToPreviousDir()),
						getActionCollection(), "Previous Directory" );
	m_p_nextDir_action =
			new KAction(i18n("Next Directory"), "1rightarrow", KShortcut(CTRL+Key_Space),
						this, SLOT(goToNextDir()),
						getActionCollection(), "Next Directory");

	m_p_dirInfo_action =
			new KAction(i18n("Describe Directory..."), 0,
						this, SLOT(slotDirInfo()),
						getActionCollection(), "Dir Info");
	m_p_dirProperties_action =
			new KAction(i18n("Properties"), "info", 0,
						this, SLOT(slotDirProperty()),
						getActionCollection(), "Dir Properties");

	m_p_dirNewFolder_action =
			new KAction(i18n("New Directory..."), "folder_new", 0,
						this, SLOT(slotNewDir()),
						getActionCollection(), "editdirnew");
	m_p_dirNewAlbum_action =
			new KAction(i18n("New Album..."), "txt", 0,
						this, SLOT(slotNewAlbum()),
						getActionCollection(), "editalbumnew");

	////////////////////////////////////////////////////////////////////////////
	m_p_detailType_action =
			new KAction(i18n("Show/Hide Type"), 0,
						this, SLOT(slotShowHideDetail_Type()),
						getActionCollection(), "dirview showhide type");
	m_p_detailSize_action =
			new KAction(i18n("Show/Hide Size"), 0,
						this, SLOT(slotShowHideDetail_Size()),
						getActionCollection(), "dirview showhide size");
	m_p_detailSelect_action  =
			new KAction(i18n("Show/Hide Select"), 0,
						this, SLOT(slotShowHideDetail_Select()),
						getActionCollection(), "dirview showhide select");
	KActionMenu *a_p_detailShowHide =
			new KActionMenu( i18n("Directory Details"), "view_tree",
							 getActionCollection(), "dirview details" );
	a_p_detailShowHide->insert(m_p_detailType_action);
	a_p_detailShowHide->insert(m_p_detailSize_action);
	a_p_detailShowHide->insert(m_p_detailSelect_action);


	////////////////////////////////////////////////////////////////////////////
	connect(this, SIGNAL(sigTotalNumberOfFiles(int)),
			getMainWindow(), SLOT(slotAddImage(int)));
	connect(this, SIGNAL(sigHasSeenFile(int)),
			getMainWindow(), SLOT(slotPreviewDone(int)));
	connect(this, SIGNAL(loadingFinished(int)),
			getMainWindow(), SLOT(slotDone(int)));

}

void
DirectoryView::initMenu(KActionCollection *a_p_actionCollection)
{
	setActionCollection(a_p_actionCollection);

	setPopupMenu(new KPopupMenu());
	getPopupMenu()->insertTitle("", 1);
	if(!getActionCollection()) return;

	m_p_newItems_menuaction =
			new KActionMenu( i18n("Create &New"), "filenew",
							 getActionCollection(), "dirview create_new_items" );
	m_p_newItems_menuaction->insert(m_p_dirNewFolder_action);
	m_p_newItems_menuaction->insert(m_p_dirNewAlbum_action);

	m_p_newItems_menuaction->plug(getPopupMenu());
	m_p_dirRecOpen_action->plug(getPopupMenu());

	//
	getPopupMenu()->insertSeparator();
	m_p_copy_menuaction =
			new KActionMenu( i18n("Copy"), 0,
					getActionCollection(), "Copy Folders actions" );
	m_p_copy_menuaction->plug(getPopupMenu());
	m_p_copy_menuaction->popupMenu()->insertTitle(i18n("Copy..."), 1);
	m_p_copy_menuaction->insert(m_p_dirCopyToLast_action);
	m_p_copy_menuaction->insert(m_p_dirCopy_action);

	m_p_move_menuaction =
			new KActionMenu( i18n("Move"), 0,
							 getActionCollection(), "Move Folders actions" );
	m_p_move_menuaction->plug(getPopupMenu());
	m_p_move_menuaction->popupMenu()->insertTitle(i18n("Move..."), 1);
	m_p_move_menuaction->insert(m_p_dirMoveToLast_action);
	m_p_move_menuaction->insert(m_p_dirMove_action);

	//
	m_p_dirRename_action->plug(getPopupMenu());
	m_p_dirTrash_action->plug(getPopupMenu());
	m_p_dirDelete_action->plug(getPopupMenu());
	getPopupMenu()->insertSeparator();
	m_p_dirPasteFiles_action->plug(getPopupMenu());
	getPopupMenu()->insertSeparator();
	m_p_dirInfo_action->plug(getPopupMenu());
	m_p_dirProperties_action->plug(getPopupMenu());
}


void
DirectoryView::updateActions(ListItem *a_p_item)
{
	if(isDropping() || !getActionCollection()) return ;
#if DIRECTORYVIEW_DEBUG > 3
	MYDEBUG<<"BEGIN"<<endl;
#endif
	bool enableAction=true;
	if(!a_p_item)
	{
		getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
 		if(!(dynamic_cast<Directory*>(a_p_item) || dynamic_cast<Album*>(a_p_item)))
			enableAction=false;
	}
	m_p_dirPasteFiles_action->setEnabled(enableAction);
	m_p_dirNewFolder_action->setEnabled(enableAction);
	m_p_dirNewAlbum_action->setEnabled(enableAction);
	m_p_dirRecOpen_action->setEnabled(enableAction);
	m_p_dirCopy_action->setEnabled(enableAction);
	m_p_previousDir_action->setEnabled(enableAction);
	m_p_nextDir_action->setEnabled(enableAction);
	m_p_dirProperties_action->setEnabled(enableAction);
	m_p_newItems_menuaction->setEnabled(enableAction);

	enableAction = a_p_item&&!a_p_item->isReadOnly();
	const bool isRoot =
#ifdef Q_WS_WIN
			QDir( QDir::convertSeparators(a_p_item->fullName()) ).isRoot();
#else
		false;
#endif
	m_p_dirMove_action->setEnabled(enableAction && !isRoot);
	m_p_dirCopyToLast_action->setEnabled(enableAction && !isRoot && !getMainWindow()->getLastDestURL().isEmpty());
	m_p_dirMoveToLast_action->setEnabled(enableAction && !isRoot && !getMainWindow()->getLastDestURL().isEmpty());
	m_p_dirTrash_action->setEnabled(enableAction && !isRoot);
	m_p_dirDelete_action->setEnabled(enableAction && !isRoot);
	m_p_dirPasteFiles_action->setEnabled(enableAction);
	m_p_dirRename_action->setEnabled(enableAction && !isRoot);
	m_p_dirInfo_action->setEnabled(enableAction);
#if DIRECTORYVIEW_DEBUG > 3
	MYDEBUG<<"END"<<endl;
#endif
}

//----------------------------------------------------------------------------

void DirectoryView::treeview_refreshItems( const KFileItemList & a_item_list)
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	QPtrListIterator<KFileItem> it( a_item_list );
	KFileItem * l_p_file_item=NULL;
	while ( (l_p_file_item = it.current()) != 0 )
	{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 2
		MYDEBUG
			<<l_p_file_item->url().prettyURL()
			<<endl;
#endif
		++it;
	}
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::treeview_cleared(const KURL& a_url)
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
	MYDEBUG<<a_url.prettyURL()<<endl;
#endif
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::treeview_newItem(const KFileItemList & a_item_list)
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	QPtrListIterator<KFileItem> it( a_item_list );
	KFileItem * l_p_file_item=NULL;
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 1
	MYDEBUG<<it.count()<<endl;
#endif
	ListItem * l_p_last_dir = NULL;
	while ( (l_p_file_item = it.current()) != 0 )
	{
		KURL l_parent_url = l_p_file_item->url().upURL();
		ListItem * l_p_dir = NULL;
		if(
			!l_p_last_dir ||
			!l_parent_url.equals(l_p_last_dir->getURL(), true) )
		{
			l_p_dir = getDir(l_parent_url);
			l_p_last_dir = l_p_dir;
		}
		else
			l_p_dir = l_p_last_dir;

#if DIRECTORYVIEW_DEBUG_TREEVIEW > 2
		MYDEBUG
			<<l_p_file_item->url().prettyURL()
			<<"-"<< l_p_file_item->url().path()
			<<"-"<< l_p_dir->getURL().prettyURL()
			<<endl;
#endif
		if(l_p_dir)
		{
			l_p_dir->insertListItem(l_p_file_item->url());
		}
		else
		{
			MYDEBUG<<"can't find '"<<l_parent_url.url ()<<"'"<<endl;
		}

		++it;
	}
	sort();

#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::treeview_itemDeleted( KFileItem * a_p_item )
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
	MYDEBUG<<a_p_item->url().prettyURL()<<endl;
#endif

	removeDir(a_p_item->url().path());

#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::treeview_jobStarted( const KURL& a_url )
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
	MYDEBUG<<a_url.path()<<endl;
#endif
	KApplication::setOverrideCursor (waitCursor);
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::treeview_jobCompleted(  )
{
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	openingFinished();
	KApplication::restoreOverrideCursor ();
#if DIRECTORYVIEW_DEBUG_TREEVIEW > 0
	MYDEBUG<<"END"<<endl;
#endif
}

//----------------------------------------------------------------------------

void DirectoryView::selecteditems_refreshItems( const KFileItemList & a_item_list)
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"BEGIN"<<endl;
#endif
	QPtrListIterator<KFileItem> it( a_item_list );
	KFileItem * l_p_file_item=NULL;
	while ( (l_p_file_item = it.current()) != 0 )
	{
		++it;
	}

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_cleared(const KURL& a_url)
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"BEGIN"<<endl;
#endif
	ListItem * l_p_dir = getDir(a_url);
	if(l_p_dir)
	{
		l_p_dir->unLoad();
	}

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_cleared()
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"BEGIN"<<endl;
#endif

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_newItem(const KFileItemList & a_item_list)
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"BEGIN"<<endl;
#endif
	QPtrListIterator<KFileItem> it( a_item_list );
	KFileItem * l_p_file_item=NULL;
	//getImageListView()->setUpdatesEnabled( false );
	loadingIsStarted(NULL, it.count());
	ListItem * l_p_last_dir = NULL;
	while ( (l_p_file_item = it.current()) != 0 )
	{
		KURL l_parent_url = l_p_file_item->url().upURL();
		ListItem * l_p_dir = NULL;
		if(
			!l_p_last_dir ||
			!l_parent_url.equals(l_p_last_dir->getURL(), true) )
		{
			l_p_dir = getDir(l_parent_url);
			l_p_last_dir = l_p_dir;
		}
		else
			l_p_dir = l_p_last_dir;

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 3
		MYDEBUG
			<<l_p_file_item->url().prettyURL()
			<<"-"<< l_p_file_item->url().prettyURL()
			<<"-"<< l_p_dir->getURL().prettyURL ()
		<<endl;
#endif
		if(l_p_dir)
		{
			if(l_p_dir->isSelected())
				l_p_dir->insertIconItem(l_p_file_item->url().path());
		}
		else
		{
			MYDEBUG<<"can't find '"<<l_p_file_item->url()<<"'"<<endl;
		}

		++it;
	}
	if(l_p_last_dir)
	{
		l_p_last_dir->repaint();
		//kapp->processEvents();
	}
	loadingIsFinished(NULL, it.count());

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 1
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_itemDeleted( KFileItem * a_p_item )
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<"BEGIN "<<a_p_item->url().prettyURL()<<endl;
#endif

	KURL l_parent_url = a_p_item->url().upURL();
	ListItem * l_p_item = getDir(l_parent_url);
	if(l_p_item)
	{
		Directory * l_p_dir = dynamic_cast<Directory*>(l_p_item);
		if(l_p_dir)
		{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
			MYDEBUG<<"I'll remove image"<<endl;
#endif
			FileIconItem * l_p_file_icon_item = getImageListView()->findItem (a_p_item->url().path(), /*fullname=*/true);
			l_p_dir->removeIconItem(l_p_file_icon_item);
		}
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
		else
		{
			MYDEBUG<<"Directory not found"<<endl;
		}
#endif
	}
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	else
	{
		MYWARNING<<"unable to 'getDir "<< l_parent_url.url() <<endl;
	}
#endif

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_jobStarted( const KURL& a_url )
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"BEGIN"<<endl;
	MYDEBUG<<a_url.path()<<endl;
#endif
//	KApplication::setOverrideCursor (waitCursor);

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"END"<<endl;
#endif
}

void DirectoryView::selecteditems_jobCompleted(  )
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"BEGIN"<<endl;
#endif
	//repaint();
	//kapp->processEvents();
//	KApplication::restoreOverrideCursor ();

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 2
	MYDEBUG<<"END"<<endl;
#endif
}


//----------------------------------------------------------------------------
void
DirectoryView::refresh(const KURL& a_dir)
{
	m_p_treeview_lister->updateDirectory(a_dir);
	m_p_selecteditems_lister->updateDirectory(a_dir);
}
	
bool DirectoryView::isManagedURL(const KURL& a_url)
{
	return 
		a_url.protocol()=="file" ||
		a_url.protocol()=="sftp" || a_url.protocol()=="ftp" ||
		a_url.protocol()=="media" ;
}

//----------------------------------------------------------------------------

void DirectoryView::openingFinished( Directory * a_p_directory)
{
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
	MYDEBUG << "BEGIN Directory * a_p_directory="<<(a_p_directory?a_p_directory->getURL().url():"NULL")<<endl;
#endif
	if(m_is_openingURL)
	{
		KURL l_parentURL(m_openingURL.upURL());
//		a_url.fileName()
		ListItem * l_p_list_item = getDir(m_openingURL);
		if(l_p_list_item)
		{
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
			MYDEBUG << "l_p_list_item found !" <<endl;
#endif
			m_is_openingURL = false;
			
			clearSelection();
			slotShowItem(l_p_list_item);
			setCurrentItem(l_p_list_item);
			setSelected(l_p_list_item, true);
			l_p_list_item->setOpen(true);
		}
		else
		{
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
			MYDEBUG << "l_p_list_item NOT found !" <<endl;
#endif
		}

	}
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
	MYDEBUG << "END Directory * a_p_directory="<<(a_p_directory?a_p_directory->getURL().url():"NULL")<<endl;
#endif
}


ListItem*
DirectoryView::getDir(const KURL& a_url)
{
#if DIRECTORYVIEW_DEBUG > 0
	MYDEBUG<<"BEGIN a_url="<<a_url.url( )<<endl;
#endif
	if(Tools::isImage(a_url))
		return NULL;

	ListItem *l_p_ssrep = NULL;
	ListItem *l_p_rootItems = firstChild ();
	bool l_is_root_found = false;
	while(
		l_p_rootItems  &&
		!l_p_ssrep
	)
	{
#if DIRECTORYVIEW_DEBUG > 0
		MYDEBUG
			<<"l_p_rootItems->getURL()="<<l_p_rootItems->getURL().url()
			<<" == "<<(l_p_rootItems->getURL().protocol() == a_url.protocol()?"true":"false")
		<<endl;
#endif

		if(
			l_p_rootItems->getURL().protocol() == a_url.protocol() &&
			l_p_rootItems->getURL().host() == a_url.host() &&
			l_p_rootItems->getURL().port() == a_url.port() &&
			l_p_rootItems->getURL().user() == a_url.user() 
		)
		{
			l_is_root_found = true;
			l_p_ssrep = l_p_rootItems->find(a_url);
		}
		l_p_rootItems = l_p_rootItems->nextSibling();
	}
	if(!l_is_root_found)
	{
		createRootForURL(a_url);
		getDir(a_url);
	}
#if DIRECTORYVIEW_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
	return l_p_ssrep;
}


void
DirectoryView::removeDir(const QString& dirfullname)
{
#if DIRECTORYVIEW_DEBUG > 0
	MYDEBUG<<"BEGIN dirfullname="<<dirfullname<<endl;
#endif

	ListItem * l_p_item = getDir(dirfullname);
	if(l_p_item)
	{
		Directory * l_p_dir = dynamic_cast<Directory*>(l_p_item);
		if(l_p_dir)
			l_p_dir->recursivelyDelete ();
#if DIRECTORYVIEW_DEBUG > 0
		else
			MYDEBUG<<"Unable to cast into Directory: dirfullname="<< dirfullname <<endl;
#endif
		delete(l_p_item);
	}
	else
	{
#if DIRECTORYVIEW_DEBUG > 0
		MYDEBUG<<"Listitem not found: dirfullname="<< dirfullname <<endl;
#endif
	}
#if DIRECTORYVIEW_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

//----------------------------------------------------------------------------

ListItem * DirectoryView::openURL(const KURL & a_url)
{
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
	MYDEBUG	<< "a_url="<< a_url.url() << " "<<endl;
#endif
	m_is_openingURL = true;
	m_openingURL    = a_url;

	ListItem * l_p_list_item = getDir(a_url);
	if(!l_p_list_item)
	{
#if DIRECTORYVIEW_DEBUG_OPENURL > 0
		MYDEBUG	<< "Not found "<<endl;
#endif
		//m_is_openingURL = true;
	}
	else
	{
		clearSelection();
		slotShowItem(l_p_list_item);
		setCurrentItem(l_p_list_item);
		setSelected(l_p_list_item, true);

		m_is_openingURL = false;
	}
	return l_p_list_item;
}


//----------------------------------------------------------------------------

void
DirectoryView::slotDirInfo()
{
	if(getClickedItem())
	{
		   (void)DescribeAlbum(getMainWindow(), getClickedItem()->fullName()).exec();
	}
}


void
DirectoryView::slotDirProperty()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	if(getClickedItem())
	{
		KApplication::setOverrideCursor (waitCursor);
		KFileItem l_item_info(KFileItem::Unknown,
				KFileItem::Unknown,
				getClickedItem()->getURL(),
				true);
			KPropertiesDialog prop( &l_item_info,
			getImageViewer(), "KPropertiesDialog",
			true, false);

		KApplication::restoreOverrideCursor ();
		prop.exec();
	}
}


void
DirectoryView::startWatchURL(const KURL & a_url)
{
#if DIRECTORYVIEW_DEBUG_LOAD > 0
		MYDEBUG<<a_url.url()<<endl;
#endif
	if(!m_p_treeview_lister->openURL(a_url, /*_keep=*/true ))
	{
		MYDEBUG<<"Cannot m_p_treeview_lister->openURL"<<endl;
	}
}


void DirectoryView::stopWatchURL(const KURL & a_url)
{
	m_p_treeview_lister->stop(a_url);
}


//----------------------------------------------------------------------------

void DirectoryView::startWatchSelectedItem(const KURL & a_url)
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<endl;
	MYDEBUG<<"BEFORE: "<<m_p_selecteditems_lister->directories()<<endl;
#endif

	if(!m_p_selecteditems_lister->openURL(a_url, /*_keep=*/false, /*_reload=*/ true ))
	{
		MYDEBUG<<"Unable to openURL"<<endl;
	}


// 	KURL l_dirURL;
// 	l_dirURL.setPath(a_fullname);
// 	QFileInfo l_info(a_fullname);
// 	if(l_info.isDir())
// 	{
// #if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
// 		MYDEBUG<<"openURL "<<l_dirURL.prettyURL()<<endl;
// #endif
// 		if(!m_p_selecteditems_lister->openURL(l_dirURL, /*_keep=*/false, /*_reload=*/ true ))
// 		{
// 			MYDEBUG<<"Unable to openURL"<<endl;
// 		}
// 	}
// 	else
// 	if(l_info.isFile())
// 	{
// #if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
// 		MYDEBUG<<"openURL FILE "<<l_dirURL.prettyURL()<<endl;
// #endif
// //		m_p_dirWatch->removeFile(a_dir);
// 	}

#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<"AFTER: "<<m_p_selecteditems_lister->directories()<<endl;
#endif
}

void DirectoryView::stopWatchSelectedItem(const KURL & a_url)
{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<endl;
	MYDEBUG<<"BEFORE: "<<m_p_selecteditems_lister->directories()<<endl;
#endif
	m_p_selecteditems_lister->stop(a_url);

/*
	KURL l_dirURL;
	l_dirURL.setPath(a_fullname);
	QFileInfo l_info(a_fullname);
	if(l_info.isDir())
	{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
		MYDEBUG<<"stop dir "<<l_dirURL.prettyURL()<<endl;
#endif
		m_p_selecteditems_lister->stop(l_dirURL);
	}
	else
	if(l_info.isFile())
	{
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
		MYDEBUG<<"stop file "<<l_dirURL.prettyURL()<<endl;
#endif
//		m_p_dirWatch->removeFile(a_dir);
	}
*/
#if DIRECTORYVIEW_DEBUG_SELECTEDITEMS > 0
	MYDEBUG<<"AFTER: "<<m_p_selecteditems_lister->directories()<<endl;
#endif
}
//----------------------------------------------------------------------------

void
DirectoryView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();

	if ( !KURLDrag::canDecode(event) || !getDropedItem())
	{
		contentsDropEvent_end();
		event->ignore();
	}
	else
	{
		event->acceptAction(true);
// 		getImageListView()->stopLoading();

		KURL::List l_list;
		if (KURLDrag::decode (event, l_list))
		{
			event->acceptAction();
			if(dynamic_cast<Album*>(getDropedItem()))
				(dynamic_cast<Album*>(getDropedItem()))->addURL(l_list);
			else
			if(!dynamic_cast<Directory*>(getDropedItem()))
			{
				contentsDropEvent_end();
				KMessageBox::error (this,
						"<qt>"+i18n("Adding file in '%1' is not yet implemented")
							.arg(dynamic_cast<ListItem*>(getDropedItem())->text(1))+"</qt>",
					 	i18n("File(s) Copy/Move"));
			}
			else
			if(!QFileInfo((dynamic_cast<Directory*>(getDropedItem()))->fullName()).isWritable ())
			{
				contentsDropEvent_end();
				KMessageBox::error (this,
						i18n("The destination directory is not writable"),
						i18n("File(s) Copy/Move"));
			}
			else
			switch (event->action ())
			{
				case QDropEvent::Copy:
				case QDropEvent::Move:
					{
						bool canBeMoved = true;
						if(event->source() == getImageListView())
							canBeMoved = getImageListView()->currentDragItemAreMovable();

						if (event->action () == QDropEvent::Move && canBeMoved )
							move(l_list, getDropedItem()->getURL());
						else
							copy(l_list, getDropedItem()->getURL());
						break;
					}
				default:break;
			}
		}
		contentsDropEvent_end();
	}
}


void
DirectoryView::copyingDirDone( /*KIO::CopyJob*/ KIO::Job *job)
{
	if(job->error()==0)
	{
		ListItem* dest=getDir(m_dirDest);
		if(dest)
		{
			if(!dest->isOpen())
			{
				//rien a faire
			}
			else
			{
				QString name=QDir(m_dirOrg).dirName();
				if(!getDir(m_dirDest+name))
				{
					//ajout du rep
					if(dynamic_cast<Directory*>(dest))
					{
						(void)new Directory( dynamic_cast<Directory*>(dest), name);
					}
					dest->setExpandable(true);
				}
			}
		}
		else
		{
		}
	}
	else
	{
		job->showErrorDialog(getMainWindow());
	}

}


void
DirectoryView::movingDirDone( KIO::Job *job)
{
	if(job->error()==0)
	{
		getMainWindow()->slotRefresh();
		emit moveFilesDone((dynamic_cast<KIO::CopyJob*>(job))->srcURLs(), (dynamic_cast<KIO::CopyJob*>(job))->destURL());
	}
	else
		job->showErrorDialog(getMainWindow());
}

void
DirectoryView::renameDone( KIO::Job *job)
{
	if(job->error()==0)
		emit renameListItemDone((dynamic_cast<KIO::FileCopyJob*>(job))->srcURL(), (dynamic_cast<KIO::FileCopyJob*>(job))->destURL());
	else
		job->showErrorDialog();
}

void
DirectoryView::copyingDone( KIO::Job *job)
{
	if(job->error()==0)
		getMainWindow()->setLastDestURL( (dynamic_cast<KIO::CopyJob*>(job))->destURL() );
	else
		job->showErrorDialog();
}


void
DirectoryView::movingDone( KIO::Job *job )
{
	if(job->error()==0)
	{
		getMainWindow()->setLastDestURL( (dynamic_cast<KIO::CopyJob*>(job))->destURL() );
		emit moveFilesDone((dynamic_cast<KIO::CopyJob*>(job))->srcURLs(), (dynamic_cast<KIO::CopyJob*>(job))->destURL());
	}
	else
		job->showErrorDialog();
}


bool
DirectoryView::copy (const KURL& a_dep, const KURL& a_dest)
{
	copy(KURL::List(a_dep), a_dest);
	return true;
}


bool
DirectoryView::move (const KURL& a_dep, const KURL& a_dest)
{
	move(KURL::List(a_dep), a_dest);
	return true;
}


void
DirectoryView::copy(const KURL::List& a_list, const KURL& a_dest)
{
#ifndef Q_WS_WIN
//FIXME
/*
	if(!QFileInfo(dest).isDir())
	{
		 KMessageBox::error(getImageViewer(), "<qt>"
		 	+i18n("Unable to copy files into '%1' because it is not a directory.").arg(dest)
			+"</qt>",
			i18n("File(s) Copy"));
		 return;
	}
*/
	KIO::CopyJob *job = KIO::copy(a_list, a_dest);
	connect(job, SIGNAL(result( KIO::Job * )),
		this, SLOT(copyingDone( KIO::Job * )));
#endif
}


void
DirectoryView::move(const KURL::List& a_list, const KURL& a_dest)
{
#ifndef Q_WS_WIN
//FIXME
/*
	if(!QFileInfo(dest).isDir())
	{
		 KMessageBox::error(getImageViewer(), "<qt>"
		 	+i18n("Unable to move files into '%1' because it is not a directory.").arg(dest)
			+"</qt>",
			i18n("File(s) Move"));
		 return;
	}
*/
	KIO::CopyJob *job = KIO::move(a_list, a_dest);
	connect(job, SIGNAL(result( KIO::Job * )),
		this, SLOT(movingDone( KIO::Job * )));
#endif
}


void
DirectoryView::slotDirPasteFiles()
{
	KURL::List l_list;
	if(KURLDrag::decode(KApplication::clipboard()->data(), l_list))
	{
		if(!l_list.isEmpty())
			copy(l_list, getClickedItem()->getURL());
	}

}


void
DirectoryView::slotNewAlbum()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	slotNewAlbum(getClickedItem());
}


void
DirectoryView::slotNewAlbum (ListItem *a_p_item)
{
	if(!a_p_item) return;

	bool ok=true;
	QString l_newName;
	KURL url;

	l_newName =
#if KDE_IS_VERSION(3,3,0)
	QString(KInputDialog::getText(i18n("Create New Album in %1").arg(shrinkdn((dynamic_cast<Directory*>(a_p_item))->fullName())),
					i18n("Enter album name:"),
					 i18n("Album"),
					 &ok, getImageViewer()).stripWhiteSpace());
#else
	"Album";
#endif
	url = KURL(a_p_item->getProtocol()+':'+a_p_item->fullName()+'/'+l_newName+".sia");

	while(ok && !l_newName.isEmpty() && QFileInfo(url.path()).exists())
	{
		 KMessageBox::error(getImageViewer(), "<qt>"+i18n("The album <b>%1</b> already exists").arg(url.fileName())+"</qt>");

#if KDE_IS_VERSION(3,3,0)
		 l_newName =
		 QString(KInputDialog::getText(i18n("Create New Album in %1").arg(shrinkdn((dynamic_cast<Directory*>(a_p_item))->fullName())),
						   i18n("Enter album name:"),
						   l_newName,
						   &ok, getImageViewer()).stripWhiteSpace());
#else
		ok = false;
#endif

		 url = KURL(a_p_item->getProtocol()+':'+a_p_item->fullName()+'/'+l_newName+".sia");
	}
	if(ok && !l_newName.isEmpty() && !QFileInfo(url.path()).exists())
		(dynamic_cast<Directory*>(a_p_item))->createAlbum(url.fileName());
}


void
DirectoryView::slotNewDir()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	slotNewDir(getClickedItem());
}


void
DirectoryView::slotNewDir (ListItem *a_p_item)
{
	if(!a_p_item) 
		return;
	
	KonqOperations::newDir(getMainWindow(), a_p_item->getURL());
}


void
DirectoryView:: slotTrash()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	slotTrash(getClickedItem());
}


void
DirectoryView::slotTrash (ListItem *a_p_item)
{
#ifndef Q_WS_WIN
	if(!a_p_item) 
		return;
	
	KonqOperations::del(getMainWindow(), 
		KonqOperations::TRASH, a_p_item->getURL());
#endif
}


void
DirectoryView::slotSuppr (ListItem *a_p_item)
{
#ifndef Q_WS_WIN
	if(!a_p_item) 
		return;
	
	KonqOperations::del(getMainWindow(), 
		KonqOperations::DEL, a_p_item->getURL());
#endif
}

void
DirectoryView::slotDirCopyToLast()
{
#if DIRECTORYVIEW_DEBUG_COPYMOVE > 0
	MYDEBUG<<endl;
#endif
	Directory* l_p_item = dynamic_cast<Directory*>(getClickedItem());
	
	if(!l_p_item) 
		return;
	
	if(getMainWindow()->getLastDestURL().isEmpty())
	{
		slotDirCopy();
		return;
	}
	
	//
	KURL 
		urlorg( l_p_item->getURL()), 
		urldest( getMainWindow()->getLastDestURL());

#if DIRECTORYVIEW_DEBUG_COPYMOVE > 0
	MYDEBUG
		<< "urlorg="<<urlorg.url()<< " " 
		<< "urldest="<<urldest.url()<< " " 
		<<endl;
#endif
	KIO::CopyJob *job = KIO::copy(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job * )),
			this, SLOT(copyingDone( KIO::Job * )));
}


void
DirectoryView::slotDirCopy()
{
#if DIRECTORYVIEW_DEBUG_COPYMOVE > 0
	MYDEBUG
		<<endl;
#endif
	Directory* l_p_item=dynamic_cast<Directory*>(getClickedItem());
	if(!l_p_item)
		return;

	QString destDir =
		KFileDialog::getExistingDirectory(
			getMainWindow()->getLastDestURL().isEmpty()?l_p_item->fullName():getMainWindow()->getLastDestURL().path(),
			getMainWindow(),
			i18n("Copy Directory %1 To").arg(shrinkdn(l_p_item->fullName()))
		);

	if(!destDir.isEmpty())
	{
		QString dest=destDir+'/';

		KURL
			urlorg(l_p_item->getURL()),
			urldest(l_p_item->getProtocol()+':'+dest);

		m_dirOrg  = l_p_item->fullName();
		m_dirDest = dest;

		getMainWindow()->setLastDestURL(urldest);

#if DIRECTORYVIEW_DEBUG_COPYMOVE > 0
	MYDEBUG
		<< "urlorg="<<urlorg.url()<< " " 
		<< "urldest="<<urldest.url()<< " " 
		<<endl;
#endif

		KIO::CopyJob *job = KIO::copy(urlorg, urldest, true);
		connect(job, SIGNAL(result( KIO::Job * )), this,
				SLOT(copyingDirDone( KIO::Job * )));
	}
}


void
DirectoryView::slotDirMoveToLast()
{
	Directory* l_p_item=dynamic_cast<Directory*>(getClickedItem());
	if(!l_p_item) return;
	if(getMainWindow()->getLastDestURL().isEmpty())
	{
		slotDirMove();
		return;
	}
	//
	KURL urlorg, urldest;
	urlorg  = l_p_item->getURL();
	urldest = getMainWindow()->getLastDestURL();

	KIO::CopyJob *job = KIO::move(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job * )),
			this, SLOT(movingDone( KIO::Job * )));
}

void
DirectoryView::slotDirMove()
{
	Directory* l_p_item=dynamic_cast<Directory*>(getClickedItem());
	if(!l_p_item)
		return;

	QString destDir =
		KFileDialog::getExistingDirectory(
				getMainWindow()->getLastDestURL().isEmpty()? l_p_item->fullName():getMainWindow()->getLastDestURL().path(),
				getMainWindow(),
				i18n("Move Directory %1 To").arg(shrinkdn(l_p_item->fullName())));
	QString dest;
	bool ok = true;

	while(!ok && !destDir.isEmpty())
	{
		ok = true;
		dest=destDir+'/'+l_p_item->text(0);
		if(QFileInfo(dest).exists())
		{
		 	 KMessageBox::error(getImageViewer(), "<qt>"
			 	+i18n("The directory '<b>%1</b>' already exists").arg(shrinkdn(dest))+"</qt>");
		 	 ok = false;
		}
		else
		if(!QFileInfo(QFileInfo(dest).dirPath()).isWritable())
		{
		 	 KMessageBox::error(getImageViewer(), "<qt>"
			 	+i18n("The directory '<b>%1</b>' is not writable").arg(shrinkdn(dest))+"</qt>");
			 ok = false;
		}
		if(!ok)
		{
			destDir =
			 KFileDialog::getExistingDirectory(
			 	getMainWindow()->getLastDestURL().isEmpty()?l_p_item->fullName():getMainWindow()->getLastDestURL().path(),
				getMainWindow(),
				i18n("Move Directory %1 To").arg(shrinkdn(l_p_item->fullName())));
		}
	}

	if(!ok || destDir.isEmpty()) return;

	KURL urlorg, urldest;
	urlorg = l_p_item->getURL();
	urldest.setPath(destDir);
	getMainWindow()->setLastDestURL(urldest);

	KIO::CopyJob *job = KIO::move(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job * )),
			this, SLOT(movingDone( KIO::Job * )));

	m_dirOrg  = l_p_item->fullName();
	m_dirDest = destDir+'/';
}


//------------------------------------------------------------------------------


void
DirectoryView::setShowHiddenDir(bool a_show)
{
	m_showHiddenDir=a_show;
}


bool
DirectoryView::showHiddenDir() const
{
	return m_showHiddenDir;
}

bool
DirectoryView::getShowCompressedFiles() const
{
	return m_showCompressedFiles;
}


void
DirectoryView::setShowCompressedFiles(bool a_show)
{
	m_showCompressedFiles = a_show;
}


void
DirectoryView::setShowHiddenFile(bool a_show)
{
	m_showHiddenFile=a_show;
}


bool
DirectoryView::showHiddenFile() const
{
	return m_showHiddenFile;
}


void
DirectoryView::setShowAllFile(bool a_show)
{
	m_showAllFile=a_show;
}


void
DirectoryView::setShowDir(bool a_show)
{
	m_showDir=a_show;
}


bool
DirectoryView::showDir() const
{
	return m_showDir;
}

//------------------------------------------------------------------------------

int
DirectoryView::filter ()
{
	int fil;
	if(showHiddenFile())
		fil = QDir::Hidden|QDir::Files;
	else
		fil = QDir::Files;
// 	if(showDir())
		fil = fil | QDir::Dirs;
	return fil;
}

void
DirectoryView::updateDestURLTitle(const KURL& a_url)
{
	m_p_copy_menuaction->popupMenu()->changeTitle(1, a_url.url());
	m_p_move_menuaction->popupMenu()->changeTitle(1, a_url.url());
}

void
DirectoryView::setUnrarPath(const QString& a_path)
{
	KRar::setUnrarPath(a_path);
}
QString
DirectoryView::getUnrarPath() const
{
	return KRar::getUnrarPath();
}

bool
DirectoryView::getShowVideo() const
{
	return m_showVideo;
}
void
DirectoryView::setShowVideo(bool a_show)
{
	m_showVideo = a_show;
}

Directory * DirectoryView::getRoot()
{
	return m_root_dir;
}

void DirectoryView::createRootForURL(const KURL& a_url)
{
	KURL l_url(a_url);
	l_url.setPath( "/" ); 
	new Directory(l_url);
}

void DirectoryView::createRoot()
{
	KURL l_url;

 	l_url.setProtocol( "file" );
	l_url.setPath( "/" );
	createRootForURL(l_url);	
}


#include "directoryview.moc"
