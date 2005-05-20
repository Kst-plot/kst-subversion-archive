/***************************************************************************
                          cdarchiveview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
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

#include "cdarchiveview.h"

#ifndef SHOWIMG_NO_CDARCHIVE

// Local
#include "mainwindow.h"
#include "listitem.h"
#include "imageviewer.h"
#include "cdarchive.h"
#include "cdarchivecreatordialog.h"

// KDE
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdirwatch.h>
#include <konq_operations.h>
#include <kpropertiesdialog.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

CDArchiveView::CDArchiveView (
			QWidget *parent,
			MainWindow *mw,
			const char* name)
	: ListItemView (parent, mw, name)
{
	dirWatch = new KDirWatch();
	dirWatch->addDir(CDArchive_ROOTPATH);
	connect(dirWatch, SIGNAL(dirty (const QString&)),
			mw, SLOT(slotDirChange(const QString&)));
	connect(dirWatch, SIGNAL(created (const QString&)),
			mw, SLOT(slotDirChange_created(const QString&)));
	connect(dirWatch, SIGNAL(deleted (const QString&)),
			mw, SLOT(slotDirChange_deleted(const QString&)));

	startWatchDir();
}

CDArchiveView::~CDArchiveView()
{
}

void
CDArchiveView::initActions(KActionCollection *actionCollection)
{
	this->actionCollection = actionCollection;

	aCDArchiveNew=new KAction(i18n("New CD Archive..."),"cdimage", 0, this, SLOT(slotNewCDArchive()), actionCollection ,"editnewcdarchive");

	aCDArchiveRename=new KAction(i18n("&Rename CD Archive..."), "item_rename", 0, this, SLOT(slotRename()), actionCollection , "cdarchive editdirrename");

	aCDArchiveTrash=new KAction(i18n("&Move CD Archive to Trash"), "edittrash", 0, this, SLOT(slotTrash()), actionCollection , "cdarchive editdirtrash");
	aCDArchiveDelete=new KAction(i18n("&Delete CD Archive"), "editdelete", 0, this, SLOT(slotSuppr()), actionCollection, "cdarchive editdirdelete");

	aCDArchiveProperties =new KAction(i18n("Properties"), "info", 0, this, SLOT(slotCDArchiveProperty()), actionCollection, "cdarchive Properties");
}

void
CDArchiveView::initMenu(KActionCollection *)
{
	popup = new KPopupMenu();
	popup->insertTitle("", 1);

	aCDArchiveNew->plug(popup);
	aCDArchiveRename->plug(popup);
	aCDArchiveTrash->plug(popup);
	aCDArchiveDelete->plug(popup);
	aCDArchiveProperties->plug(popup);
}

void
CDArchiveView::updateActions(ListItem *item)
{
	if(isDropping() || !actionCollection) return ;
	bool enableAction=true;
	if(!item)
	{
		mw->getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
		//MYDEBUG<<item->getType ()<<endl;
		if(item->getType () != "CD Archive") enableAction=false;
	}
	//aCDArchiveNew->setEnabled(enableAction);
	aCDArchiveRename->setEnabled(enableAction);
	aCDArchiveTrash->setEnabled(enableAction);
	aCDArchiveDelete->setEnabled(enableAction);
	aCDArchiveProperties->setEnabled(enableAction);
}


ListItem*
CDArchiveView::getCDArchiveItem(const QString& cdArchivePath)
{
	MYDEBUG<<cdArchivePath<<" " << CDArchive_ROOTPATH <<endl;
	ListItem *ssrep;
	ListItem *rootItems = firstChild ();
	if(cdArchivePath == CDArchive_ROOTPATH) return rootItems;
	MYDEBUG<<cdArchivePath<<endl;
	while(rootItems)
	{
		if(cdArchivePath.startsWith(rootItems->fullName()))
		{
			ssrep = rootItems->find(cdArchivePath);
			if (ssrep)
			{
				return ssrep;
			}
		}
		rootItems = rootItems->nextSibling();
	}
	return NULL;
}

void
CDArchiveView::slotTrash()
{
	if(!clickedItem) clickedItem=currentItem();
	slotTrash(clickedItem);
}


void
CDArchiveView::slotTrash(ListItem *item)
{
#ifndef Q_WS_WIN
	if(!item) return;
	ListItem *dir=(ListItem*)item;
	dir->setOpen(false);
	KonqOperations::del(mw, KonqOperations::TRASH, dir->getURL());
#endif
}


void
CDArchiveView::slotSuppr (ListItem *item)
{
#ifndef Q_WS_WIN
	if(!item) return;
	//Directory *dir=(Directory*)item;
	item->setOpen(false);
	KonqOperations::del(mw, KonqOperations::DEL, item->getURL());
#endif
}


void
CDArchiveView::slotNewCDArchive()
{
	if(!clickedItem) clickedItem=currentItem();
	slotNewCDArchive(clickedItem);
}

void
CDArchiveView::slotNewCDArchive(ListItem *)
{
	CDArchiveCreatorDialog dial(mw->getcdromPath(), mw);
	dial.exec();
}


void
CDArchiveView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();
	event->ignore();
	event->acceptAction(true);
	contentsDropEvent_end();
}

void
CDArchiveView::startWatchDir(QString dir)
{
	if(dirWatch->contains(dir)) return;

	dirWatch->stopScan();
	QFileInfo info(dir);
	if(info.isDir())
	{
		dirWatch->addDir(dir);
	}
	else
		if(info.isFile())
	{
		dirWatch->addFile(dir);
	}
	dirWatch->startScan();
}


void
CDArchiveView::stopWatchDir(QString dir)
{
	if(QFileInfo(dir).isDir())
		dirWatch->removeDir(dir);
	else
		if(QFileInfo(dir).isFile())
			dirWatch->removeFile(dir);
}


void
CDArchiveView::startWatchDir()
{
	dirWatch->startScan();
}


void
CDArchiveView::stopWatchDir()
{
	dirWatch->stopScan();
}

void
CDArchiveView::slotCDArchiveProperty()
{
	if(!clickedItem) clickedItem=currentItem();
	if(clickedItem)
	{
		KApplication::setOverrideCursor (waitCursor);
		KFileItem *item = new KFileItem(KFileItem::Unknown,
										KFileItem::Unknown,
										clickedItem->getURL(),
										true);
		KPropertiesDialog prop( item,
								mw->getImageViewer(), "KPropertiesDialog",
								true, false);

		KApplication::restoreOverrideCursor ();
		prop.exec();
		delete(item);
	}
}


#include "cdarchiveview.moc"

#endif /* SHOWIMG_NO_CDARCHIVE */
