/***************************************************************************
                          categoryview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004 by Richard Groult
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

#include "categoryview.h"

#ifdef WANT_LIBKEXIDB

#include "mainwindow.h"
#include "listitem.h"
#include "categorylistitem.h"
#include "imageviewer.h"
#include "categorydbmanager.h"
#include "directoryview.h"

// KDE
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "


CategoryView::CategoryView (
			QWidget *parent,
			MainWindow *mw,
			const char* name)
	: ListItemView (parent, mw, name),

	cdbManager(NULL),
	categoryRoot(NULL),

	m_currentActionLabel(NULL)
{
	this->mw=mw;

	cdbManager = new CategoryDBManager(mw);
	connect(cdbManager, SIGNAL(isAddingFiles(bool) ), this, SLOT( setDisabled(bool) ));
	connect(cdbManager, SIGNAL(numberOfLeftItems(int)), this, SLOT( setNumberOfLeftItems(int) )) ;
	if(mw->getDirectoryView())
		connect(mw->getDirectoryView(), SIGNAL(loadingFinished(ListItem*) ), cdbManager, SLOT( newFilesAdded(ListItem*) ));
}

CategoryView::~CategoryView()
{
}

void
CategoryView::createRootCategory()
{
	categoryRoot = new CategoryListItemRootTag(mw);
	categoryRoot->setOpen (true);

	dateRoot = new CategoryListItemRootDate(mw);
	(void)new CategoryListItemRootSearch(mw);
}

void
CategoryView::initActions(KActionCollection *actionCollection)
{
	this->actionCollection = actionCollection;

	aCatNewCategory=new KAction(i18n("New Category..."), "kontact_mail", 0, this, SLOT(slotNewCategory()), actionCollection ,"editnewcategory");
	aCatRename=new KAction(i18n("&Rename Category..."), "item_rename", 0, this,SLOT(slotRename()), actionCollection ,"editcatrename");
	aCatDelete=new KAction(i18n("&Delete Category"), "editdelete", 0, this, SLOT(slotSuppr()), actionCollection, "editcatdelete");
	aCatProperties=new KAction(i18n("Category Properties"), "info", 0, this, SLOT(slotCatProperty()), actionCollection,"Cat Properties");

	aCatSelectAND = new KRadioAction(i18n("AND"), "edit_add", 0, this, SLOT(slotANDSelection()), actionCollection, "category selection and");
	aCatSelectOR = new KRadioAction(i18n("OR"), "edit_remove", 0, this, SLOT(slotORSelection()), actionCollection, "category selection or");
	aCatSelectAND->setExclusiveGroup("CategoryView Selection Group");aCatSelectOR->setExclusiveGroup("CategoryView Selection Group");
	aCatSelectAND->setChecked(true);


	m_currentActionLabel=new QLabel("Ready", this, "m_currentActionLabel categoryview toolbar ");
	(void)new KWidgetAction( m_currentActionLabel, i18n("Ready"), 0, 0, 0, actionCollection, "categoryview currentAction");
}

void
CategoryView::initMenu(KActionCollection *)
{
	popup = new KPopupMenu();
	popup->insertTitle("", 1);

	aCatNewCategory->plug(popup);
	popup->insertSeparator();
	aCatDelete->plug(popup);
	aCatRename->plug(popup);
	popup->insertSeparator();
	aCatProperties->plug(popup);
}

void
CategoryView::updateActions(ListItem *item)
{
	if(isDropping() || !actionCollection) return ;
	bool enableAction=true;
	bool isLeaf=false;
	if(!item)
	{
		mw->getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
		if(item->getType () != "Category")
			enableAction=false;
		else
			isLeaf=((CategoryListItemTag*)item)->isLeaf();
	}
	aCatDelete->setEnabled(enableAction && isLeaf);
	aCatNewCategory->setEnabled(enableAction);
	aCatRename->setEnabled(enableAction && ((CategoryListItemTag*)item)->getId()>0);
	aCatProperties->setEnabled(enableAction && ((CategoryListItemTag*)item)->getId()>0);

// 	aCatSelectAND->setEnabled(enableAction);
// 	aCatSelectOR->setEnabled(enableAction);
}

void
CategoryView::slotSuppr (ListItem *item)
{
	MYDEBUG<<"TODO"<<endl;
	getCategoryDBManager()->deleteNodeCategory( ((CategoryListItemTag*)item)->getId() ) ;
	delete(item);
}


void
CategoryView::slotNewCategory()
{
	if(!clickedItem) clickedItem=currentItem();
	slotNewCategory(clickedItem);
}

void
CategoryView::slotNewCategory(ListItem *item)
{
	if(!item) return;
	bool ok;
	const QString newName(
			KInputDialog::getText(i18n("Create New Category in %1").arg(item->fullName()),
								  i18n("Enter category name:"),
								  i18n("Category"),
								  &ok,
								  mw->getImageViewer())
			.stripWhiteSpace());
	if(!ok || newName.isEmpty())
		return;
	QString msg;
	if(!getCategoryDBManager()->addSubCategory(((CategoryListItemTag*)item), newName, msg))
	{
		KMessageBox::error(mw->getImageViewer(), "<qt>"+msg+"</qt>");
		return;
	}
}

void
CategoryView::slotCatProperty()
{
	if(clickedItem)
	{
		KApplication::setOverrideCursor (waitCursor);
		MYDEBUG<<"TODO"<<endl;
		KApplication::restoreOverrideCursor ();
	}
}

void
CategoryView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();
	if ( !QUriDrag::canDecode(event) || !dropItem)
	{
		event->ignore();
	}
	else
	{
		event->acceptAction(true);
		mw->getImageListView()->stopLoading();

		QStrList lst;
		if (QUriDrag::decode (event, lst))
		{
			((CategoryListItem*)dropItem)->addURL(QStringList::fromStrList(lst));
		}
	}
	contentsDropEvent_end();
}

void
CategoryView::startWatchDir(QString)
{
// 	MYDEBUG<<" nothing TODO"<<endl;
}
void
CategoryView::stopWatchDir(QString)
{
// 	MYDEBUG<<" nothing TODO"<<endl;
}
void
CategoryView::startWatchDir()
{
// 	MYDEBUG<<" nothing TODO"<<endl;
}
void
CategoryView::stopWatchDir()
{
// 	MYDEBUG<<" nothing TODO"<<endl;
}

CategoryDBManager*
CategoryView::getCategoryDBManager()
{
	return cdbManager;
}


CategoryListItem*
CategoryView::getCategoryRoot()
{
	return categoryRoot;
}

void
CategoryView::slotANDSelection()
{
	getCategoryDBManager()->setSelectionMode(CategoryDBManager::mode_AND);
	m_currentActionLabel->setText(i18n("Selection mode: AND"));
}

void
CategoryView::slotORSelection()
{
	getCategoryDBManager()->setSelectionMode(CategoryDBManager::mode_OR);
	m_currentActionLabel->setText(i18n("Selection mode: OR"));
}


void
CategoryView::setDisabled(bool disable)
{
	ListItemView::setDisabled(disable);
	if(m_currentActionLabel)
		if(disable)
			m_currentActionLabel->setText(i18n("Adding files..."));
		else
			m_currentActionLabel->setText(" ");
}

void
CategoryView::setNumberOfLeftItems(int nbr)
{
	if(m_currentActionLabel)
		m_currentActionLabel->setText(i18n("Adding files (%1 left)...").arg(nbr));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CategoryView::fileIconRenamed(const QString& org, const QString& dest)
{
// 	MYDEBUG<<org<<" "<<dest<<endl;
	cdbManager->renameImage(org, dest);
}

void
CategoryView::filesMoved(const KURL::List& fileurls, const KURL& desturl)
{
// 	MYDEBUG<<fileurls<<"->"<<desturl.url()<<endl;
	cdbManager->moveImages(fileurls, desturl);
}


void
CategoryView::fileIconsDeleted(const KURL::List& list)
{
	MYDEBUG<<"TODO "<<list<<endl;
}

void
CategoryView::directoryRenamed(const KURL& srcurl, const KURL& desturl)
{
// 	MYDEBUG<<srcurl.path(-1)<<"->"<<desturl.path(-1)<<endl;
	cdbManager->renameDirectory(srcurl, desturl);
}


#include "categoryview.moc"

#endif /* WANT_LIBKEXIDB */
