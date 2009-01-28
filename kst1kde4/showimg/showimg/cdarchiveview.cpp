/*****************************************************************************
                          cdarchiveview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
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

#include "cdarchiveview.h"

#ifndef SHOWIMG_NO_CDARCHIVE

// Local
#include "cdarchivecreatordialog.h"
#include "cdarchive.h"
#include "imageviewer.h"
#include "listitem.h"
#include "mainwindow.h"

// KDE
#include <kaction.h>
#include <kapplication.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <konq_operations.h>
#include <kpopupmenu.h>
#include <kpropertiesdialog.h>

CDArchiveView::CDArchiveView (
			QWidget    *a_p_parent,
			const char* a_p_name)
	: ListItemView (a_p_parent, a_p_name)
{
	m_p_dirWatch = new KDirWatch();
	m_p_dirWatch->addDir(CDArchive_ROOTPATH);
	connect(m_p_dirWatch, SIGNAL(dirty (const QString&)),
			getMainWindow(), SLOT(slotDirChange(const QString&)));
	connect(m_p_dirWatch, SIGNAL(created (const QString&)),
			getMainWindow(), SLOT(slotDirChange_created(const QString&)));
	connect(m_p_dirWatch, SIGNAL(deleted (const QString&)),
			getMainWindow(), SLOT(slotDirChange_deleted(const QString&)));

	startWatchDir();
}

CDArchiveView::~CDArchiveView()
{
}

void
CDArchiveView::initActions(KActionCollection *a_p_actionCollection)
{
	setActionCollection(a_p_actionCollection);

	m_p_CDArchiveNew_action=new KAction(i18n("New CD Archive..."),"cdimage", 0, this, SLOT(slotNewCDArchive()), getActionCollection() ,"editnewcdarchive");

	m_p_CDArchiveRename_action=new KAction(i18n("&Rename CD Archive..."), "item_rename", 0, this, SLOT(slotRename()), getActionCollection() , "cdarchive editdirrename");

	m_p_CDArchiveTrash_action=new KAction(i18n("&Move CD Archive to Trash"), "edittrash", 0, this, SLOT(slotTrash()), getActionCollection() , "cdarchive editdirtrash");
	m_p_CDArchiveDelete_action=new KAction(i18n("&Delete CD Archive"), "editdelete", 0, this, SLOT(slotSuppr()), getActionCollection(), "cdarchive editdirdelete");

	m_p_CDArchiveProperties_action =new KAction(i18n("Properties"), "info", 0, this, SLOT(slotCDArchiveProperty()), getActionCollection(), "cdarchive Properties");
}

void
CDArchiveView::initMenu(KActionCollection * )
{
	setPopupMenu(new KPopupMenu());
	getPopupMenu()->insertTitle("", 1);

	m_p_CDArchiveNew_action->plug(getPopupMenu());
	m_p_CDArchiveRename_action->plug(getPopupMenu());
	m_p_CDArchiveTrash_action->plug(getPopupMenu());
	m_p_CDArchiveDelete_action->plug(getPopupMenu());
	m_p_CDArchiveProperties_action->plug(getPopupMenu());
}

void
CDArchiveView::updateActions(ListItem *a_p_item)
{
	if(isDropping() || !getActionCollection()) return ;
	bool enableAction=true;
	if(!a_p_item)
	{
		getMainWindow()->getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
		if(!dynamic_cast<CDArchiveView*>(a_p_item)) enableAction=false;
	}
	m_p_CDArchiveRename_action->setEnabled(enableAction);
	m_p_CDArchiveTrash_action->setEnabled(enableAction);
	m_p_CDArchiveDelete_action->setEnabled(enableAction);
	m_p_CDArchiveProperties_action->setEnabled(enableAction);
}


ListItem*
CDArchiveView::getCDArchiveItem(const QString& cdArchivePath)
{
	ListItem *ssrep;
	ListItem *rootItems = firstChild ();
	if(cdArchivePath == CDArchive_ROOTPATH) return rootItems;
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
	if(!getClickedItem()) setClickedItem(currentItem());
	slotTrash(getClickedItem());
}


void
CDArchiveView::slotTrash(ListItem *a_p_item)
{
#ifndef Q_WS_WIN
	if(!a_p_item) return;
	ListItem *dir=dynamic_cast<ListItem*>(a_p_item);
	dir->setOpen(false);
	KonqOperations::del(getMainWindow(), KonqOperations::TRASH, dir->getURL());
#endif
}


void
CDArchiveView::slotSuppr (ListItem *a_p_item)
{
#ifndef Q_WS_WIN
	if(!a_p_item) return;
	a_p_item->setOpen(false);
	KonqOperations::del(getMainWindow(), KonqOperations::DEL, a_p_item->getURL());
#endif
}


void
CDArchiveView::slotNewCDArchive()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	slotNewCDArchive(getClickedItem());
}

void
CDArchiveView::slotNewCDArchive(ListItem * )
{
	CDArchiveCreatorDialog dial(getMainWindow()->getcdromPath(), getMainWindow());
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
CDArchiveView::startWatchDir(const QString & dir)
{
	if(m_p_dirWatch->contains(dir)) return;

	m_p_dirWatch->stopScan();
	QFileInfo info(dir);
	if(info.isDir())
	{
		m_p_dirWatch->addDir(dir);
	}
	else
		if(info.isFile())
	{
		m_p_dirWatch->addFile(dir);
	}
	m_p_dirWatch->startScan();
}


void
CDArchiveView::stopWatchDir(const QString & dir)
{
	if(QFileInfo(dir).isDir())
		m_p_dirWatch->removeDir(dir);
	else
		if(QFileInfo(dir).isFile())
			m_p_dirWatch->removeFile(dir);
}


void
CDArchiveView::startWatchDir()
{
	m_p_dirWatch->startScan();
}


void
CDArchiveView::stopWatchDir()
{
	m_p_dirWatch->stopScan();
}

void
CDArchiveView::slotCDArchiveProperty()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	if(getClickedItem())
	{
		KApplication::setOverrideCursor (waitCursor);
		KFileItem *a_p_item = new KFileItem(KFileItem::Unknown,
										KFileItem::Unknown,
										getClickedItem()->getURL(),
										true);
		KPropertiesDialog prop( a_p_item,
								getMainWindow()->getImageViewer(), "KPropertiesDialog",
								true, false);

		KApplication::restoreOverrideCursor ();
		prop.exec();
		delete(a_p_item);
	}
}

void CDArchiveView::createRoot()
{
	ListItem * l_root_cdarchive = new CDArchive ();
	l_root_cdarchive->setOpen (true);
}

ListItem * CDArchiveView::openURL(const KURL& a_url) 
{
	ListItem * l_p_list_item = getCDArchiveItem(CDArchive_ROOTPATH)->find(a_url);
	if(!l_p_list_item)
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("CDArchiveView: TODO: The URL '<b>%1</b>' ").arg(a_url.url())+"</qt>");
	else
	{
		clearSelection();
		slotShowItem(l_p_list_item);
		setCurrentItem(l_p_list_item);
		setSelected(l_p_list_item, true);
	}
	return l_p_list_item;
}

bool CDArchiveView::isManagedURL(const KURL& a_url)
{
	return 
		a_url.protocol() == "cdarchive";
}


#include "cdarchiveview.moc"

#endif /* SHOWIMG_NO_CDARCHIVE */
