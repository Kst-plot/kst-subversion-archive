/***************************************************************************
                          categoryview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "categoryview.h"

#ifdef WANT_LIBKEXIDB

#include "mainwindow.h"
#include "listitem.h"
#include "categorylistitem.h"
#include "imageviewer.h"
#include "categorydbmanager.h"
#include "directoryview.h"
#include "categoryproperties.h"

// KDE
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "


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
	setAddAllImages(false);

// 	MYDEBUG<<"Creating CategoryDBManager"<<endl;
	cdbManager = new CategoryDBManager(mw);
	if(!cdbManager->isConnected())
		setEnabled(false);

	connect(getCategoryDBManager(), SIGNAL(isAddingFiles(bool) ),
		this, SLOT( setDisabled(bool) ));
	connect(getCategoryDBManager(), SIGNAL(numberOfLeftItems(int)),
		this, SLOT( setNumberOfLeftItems(int) )) ;

}

CategoryView::~CategoryView()
{
// 	MYDEBUG<<endl;
	delete(cdbManager);
	delete(categoryRoot);
	delete(dateRoot);
}

int
CategoryView::getIconSize()
{
	return KIcon::SizeSmallMedium;
}


void
CategoryView::readConfig(KConfig *config)
{
	config->setGroup("CategorCategories");
	setAddAllImages(config->readBoolEntry("add all images", false));
}
void
CategoryView::writeConfig(KConfig *config)
{
	config->setGroup("Categories");
	config->writeEntry("add all images", getAddAllImages());
}


void
CategoryView::createRootCategory()
{
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not create RootCategory"<<endl;
		return;
	}
	cdbManager->setEnabled(true);
	////
	categoryRoot = new CategoryListItemRootTag(mw);
	categoryRoot->setOpen (true);
	////
	dateRoot = new CategoryListItemRootDate(mw);
	////
	(void)new CategoryListItemRootSearch(mw);
	////
	(void)new CategoryListItemRootNote(mw);

}

bool
CategoryView::isConnected()
{
	if (cdbManager != NULL)
		return cdbManager->isConnected();
	else
		return false;
}

void
CategoryView::initActions(KActionCollection *actionCollection)
{
	this->actionCollection = actionCollection;

	aCatNewCategory=new KAction(i18n("New Category..."), "kontact_mail", 0, this, SLOT(slotNewCategory()), actionCollection ,"editnewcategory");
	aCatRename=new KAction(i18n("&Rename Category..."), "item_rename", 0, this,SLOT(slotRename()), actionCollection ,"editcatrename");
	aCatDelete=new KAction(i18n("&Delete Category"), "editdelete", 0, this, SLOT(slotSuppr()), actionCollection, "editcatdelete");
	aCatProperties=new KAction(i18n("Category Properties"), "info", 0, this, SLOT(slotCatProperty()), actionCollection,"Cat Properties");

	aCatSelectAND = new KRadioAction(i18n("AND"), "raise", 0, this, SLOT(slotANDSelection()), actionCollection, "category selection and");
	aCatSelectOR = new KRadioAction(i18n("OR"), "lower", 0, this, SLOT(slotORSelection()), actionCollection, "category selection or");
	aCatSelectAND->setExclusiveGroup("CategoryView Selection Group");aCatSelectOR->setExclusiveGroup("CategoryView Selection Group");
	aCatSelectAND->setChecked(true);


	m_currentActionLabel=new QLabel("Ready", 0, "m_currentActionLabel categoryview toolbar ");
	(void)new KWidgetAction( m_currentActionLabel, i18n("Ready"), 0, 0, 0, actionCollection, "categoryview currentAction");

	///////////////////////////////////////////////////////////////////////////
	connect(this, SIGNAL(sigTotalNumberOfFiles(int)),
			mw, SLOT(slotAddImage(int)));
	connect(this, SIGNAL(sigHasSeenFile(int)),
			mw, SLOT(slotPreviewDone(int)));
	connect(this, SIGNAL(loadingFinished(int)),
			mw, SLOT(slotDone(int)));

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
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not slotSuppr"<<endl;
		return;
	}
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
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not slotNewCategory"<<endl;
		return;
	}
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
		CategoryProperties catprop(this, (CategoryListItemTag*)clickedItem);
		KApplication::restoreOverrideCursor ();
		if(!catprop.exec()) return;

		QString msg;
		((CategoryListItemTag*)clickedItem)->rename(catprop.getName(),msg );
		((CategoryListItemTag*)clickedItem)->setDescription(catprop.getDescription());
		((CategoryListItemTag*)clickedItem)->setIcon(catprop.getIcon());

		//KMessageBox::sorry(mw, "<qt>"+i18n("TODO: Category Properties")+"</qt>");
	}
}

void
CategoryView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();
	////////////////////////////////////////////////////////////////////////////
	if ( !QUriDrag::canDecode(event) || !dropItem)
	{
		event->ignore();
	}
	else
	{
		event->acceptAction(true);
		QStrList lst;
		if (QUriDrag::decode (event, lst))
		{
			mw->getImageListView()->stopLoading();
			mw->update();
			kapp->processEvents();

			((CategoryListItem*)dropItem)->addURL(QStringList::fromStrList(lst));

		}
	}
	////////////////////////////////////////////////////////////////////////////
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
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not slotANDSelection"<<endl;
		return;
	}
	m_currentActionLabel->setText(i18n("Selection mode: AND"));

	int num=getCategoryDBManager()->setSelectionMode(CategoryDBManager::mode_AND);
	loadingIsStarted(currentItem(), num);
	getCategoryDBManager()->refreshRequest();
	loadingIsFinished(currentItem(), num);
}

void
CategoryView::slotORSelection()
{
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not slotORSelection"<<endl;
		return;
	}
	m_currentActionLabel->setText(i18n("Selection mode: OR"));

	int num=getCategoryDBManager()->setSelectionMode(CategoryDBManager::mode_OR);
	loadingIsStarted(currentItem(), num);
	getCategoryDBManager()->refreshRequest();
	loadingIsFinished(currentItem(), num);
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
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not fileIconRenamed"<<endl;
		return;
	}
	getCategoryDBManager()->renameImage(org, dest);
}

void
CategoryView::filesMoved(const KURL::List& fileurls, const KURL& desturl)
{
// 	MYDEBUG<<fileurls<<"->"<<desturl.url()<<endl;
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not filesMoved"<<endl;
		return;
	}
// 	MYDEBUG<<"BEGIN"<<endl;
	mw->getImageListView()->disconnect(SIGNAL(sigSetMessage(const QString&)));
	getCategoryDBManager()->moveImages(fileurls, desturl);
	connect(mw->getImageListView(), SIGNAL(sigSetMessage(const QString&)),
			mw, SLOT(setMessage(const QString&)));
// 	MYDEBUG<<"END"<<endl;
}


void
CategoryView::fileIconsDeleted(const KURL::List& list)
{
	MYDEBUG<<"TODO "<<list<<endl;
}

void
CategoryView::directoryRenamed(const KURL& srcurl, const KURL& desturl)
{
//  	MYDEBUG<<srcurl.path(-1)<<"->"<<desturl.path(-1)<<endl;
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not directoryRenamed"<<endl;
		return;
	}
	getCategoryDBManager()->renameDirectory(srcurl, desturl);
}


void
CategoryView::renameImage(QDict<QString>& renamedFiles)
{
	if(!getCategoryDBManager())
	{
		MYDEBUG<<"NO CategoryDBManager, I do not directoryRenamed"<<endl;
		return;
	}
	getCategoryDBManager()->renameImage(renamedFiles);
}

void
CategoryView::setAddAllImages(bool addAll)
{
	m_addAllImages = addAll;
	if(!mw->getDirectoryView())
		return;
	if(m_addAllImages)
	{
		connect(mw->getDirectoryView(), SIGNAL(loadingFinished(ListItem*) ),
			getCategoryDBManager(), SLOT( newFilesAdded(ListItem*) ));
	}
	else
	{
		mw->getDirectoryView()->disconnect(this, SIGNAL(loadingFinished(ListItem*)));
	}

}
bool
CategoryView::getAddAllImages()
{
	return m_addAllImages;
}
QString
CategoryView::getType()
{
	return cdbManager->getType();
}
void
CategoryView::setType(const QString& type)
{
	cdbManager->setType(type);
}
QString
CategoryView::getSqlitePath()
{
	return cdbManager->getSqlitePath();
}
void
CategoryView::setSqlitePath(const QString& path)
{
	cdbManager->setSqlitePath(path);
}
QString
CategoryView::getMysqlUsername()
{
	return cdbManager->getMysqlUsername();
}
void
CategoryView::setMysqlUsername(const QString& name)
{
	cdbManager->setMysqlUsername(name);
}
QString
CategoryView::getMysqlPassword()
{
	return cdbManager->getMysqlPassword();
}
void
CategoryView::setMysqlPassword(const QString& pass)
{
	cdbManager->setMysqlPassword(pass);
}
QString
CategoryView::getMysqlHostname()
{
	return cdbManager->getMysqlHostname();
}
void
CategoryView::setMysqlHostname(const QString& host)
{
	cdbManager->setMysqlHostname(host);
}

int
CategoryView::removeObsololeteFilesOfTheDatabase()
{
	return getCategoryDBManager()->removeObsololeteFilesOfTheDatabase();
}


void
CategoryView::slotRefresh()
{
// 	MYDEBUG<<endl;
	loadingIsStarted(NULL, 0);
	getCategoryDBManager()->reload();
	loadingIsFinished(NULL, 0);
}

#include "categoryview.moc"

#endif /* WANT_LIBKEXIDB */
