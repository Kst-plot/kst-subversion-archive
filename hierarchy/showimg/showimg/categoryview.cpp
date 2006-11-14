/*****************************************************************************
                          categoryview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2006 by Richard Groult
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

#include "categoryview.h"

//-----------------------------------------------------------------------------
#define CATEGORYVIEW_DEBUG           0
//-----------------------------------------------------------------------------

#ifdef WANT_LIBKEXIDB

#include "categorydbmanager.h"
#include "categorylistitem.h"
#include "categoryproperties.h"
#include "directoryview.h"
#include "imageviewer.h"
#include "listitem.h"
#include "mainwindow.h"
#include "showimg_common.h"

// KDE
#include <kactioncollection.h>
#include <kaction.h>
#include <kapplication.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>


CategoryView::CategoryView (
			QWidget    * a_p_parent,
			const char * a_p_name)
	: ListItemView (a_p_parent, a_p_name),

	m_p_cdbManager(NULL),
	m_p_categoryRoot(NULL),

	m_p_currentActionLabel(NULL)
{
	setAddAllImages(false);

	m_p_cdbManager = new CategoryDBManager();
	if(!m_p_cdbManager->isConnected())
		setEnabled(false);

	connect(getCategoryDBManager(), SIGNAL(isAddingFiles(bool) ),
		this, SLOT( setDisabled(bool) ));
	connect(getCategoryDBManager(), SIGNAL(numberOfLeftItems(int)),
		this, SLOT( setNumberOfLeftItems(int) )) ;

}

CategoryView::~CategoryView()
{
	delete(m_p_cdbManager);
	delete(m_p_categoryRoot);
	delete(m_p_dateRoot);
}

int
CategoryView::getIconSize()
{
	return KIcon::SizeSmallMedium;
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void
CategoryView::createRoot()
{
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not create RootCategory"<<endl;
#endif
		return;
	}
	m_p_cdbManager->setEnabled(true);
	
	//
	m_p_categoryRoot = new CategoryListItemRootTag();
	m_p_categoryRoot->setOpen (true);
	
	//
	m_p_dateRoot = new CategoryListItemRootDate();
	
	//
	(void)new CategoryListItemRootSearch();
	
	//
	(void)new CategoryListItemRootNote();

}

bool
CategoryView::isConnected()
{
	if (m_p_cdbManager != NULL)
		return m_p_cdbManager->isConnected();
	else
		return false;
}

//-----------------------------------------------------------------------------

void
CategoryView::initActions(KActionCollection *actionCollection)
{
	setActionCollection(actionCollection);

	m_p_catNewCategory_action=new KAction(i18n("New Category..."), "kontact_mail", 0, this, SLOT(slotNewCategory()), actionCollection ,"editnewcategory");
	m_p_catRename_action=new KAction(i18n("&Rename Category..."), "item_rename", 0, this,SLOT(slotRename()), actionCollection ,"editcatrename");
	m_p_catDelete_action=new KAction(i18n("&Delete Category"), "editdelete", 0, this, SLOT(slotSuppr()), actionCollection, "editcatdelete");
	m_p_catProperties_action=new KAction(i18n("Category Properties"), "info", 0, this, SLOT(slotCatProperty()), actionCollection,"Cat Properties");

	m_p_catSelectAND_action = new KRadioAction(i18n("AND"), "raise", 0, this, SLOT(slotANDSelection()), actionCollection, "category selection and");
	m_p_catSelectOR_action = new KRadioAction(i18n("OR"), "lower", 0, this, SLOT(slotORSelection()), actionCollection, "category selection or");
	m_p_catSelectAND_action->setExclusiveGroup("CategoryView Selection Group");m_p_catSelectOR_action->setExclusiveGroup("CategoryView Selection Group");
	m_p_catSelectAND_action->setChecked(true);


	m_p_currentActionLabel=new QLabel("Ready", 0, "m_p_currentActionLabel categoryview toolbar ");
	(void)new KWidgetAction( m_p_currentActionLabel, i18n("Ready"), 0, 0, 0, actionCollection, "categoryview currentAction");

	///////////////////////////////////////////////////////////////////////////
	connect(this, SIGNAL(sigTotalNumberOfFiles(int)),
			getMainWindow(), SLOT(slotAddImage(int)));
	connect(this, SIGNAL(sigHasSeenFile(int)),
			getMainWindow(), SLOT(slotPreviewDone(int)));
	connect(this, SIGNAL(loadingFinished(int)),
			getMainWindow(), SLOT(slotDone(int)));

}

void
CategoryView::initMenu(KActionCollection * )
{
	setPopupMenu(new KPopupMenu());
	getPopupMenu()->insertTitle("", 1);

	m_p_catNewCategory_action->plug(getPopupMenu());
	getPopupMenu()->insertSeparator();
	m_p_catDelete_action->plug(getPopupMenu());
	m_p_catRename_action->plug(getPopupMenu());
	getPopupMenu()->insertSeparator();
	m_p_catProperties_action->plug(getPopupMenu());
}

void
CategoryView::updateActions(ListItem *item)
{
	if(isDropping() || !getActionCollection()) return ;
	bool enableAction=true;
	bool isLeaf=false;
	CategoryListItemTag  * l_p_cat_list_item_tag = dynamic_cast<CategoryListItemTag*>(item);
	if(!l_p_cat_list_item_tag)
	{
		getMainWindow()->getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
		if(!dynamic_cast<CategoryListItem*>(l_p_cat_list_item_tag))
		{
			enableAction=false;
		}
		else
		{
			if(l_p_cat_list_item_tag)
				isLeaf=l_p_cat_list_item_tag->isLeaf();
			else
				isLeaf=false;
		}
	}
	m_p_catDelete_action->setEnabled(enableAction && isLeaf);
	m_p_catNewCategory_action->setEnabled(enableAction);
	if(l_p_cat_list_item_tag)
		enableAction = enableAction && l_p_cat_list_item_tag->getId()>0;
	else
		enableAction = false;
	m_p_catRename_action->setEnabled(enableAction);
	m_p_catProperties_action->setEnabled(enableAction);
}

//-----------------------------------------------------------------------------

void
CategoryView::slotSuppr (ListItem *item)
{
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not slotSuppr"<<endl;
#endif
		return;
	}
	getCategoryDBManager()->deleteNodeCategory( (dynamic_cast<CategoryListItemTag*>(item))->getId() ) ;
	delete(item);
}


void
CategoryView::slotNewCategory()
{
	if(!getClickedItem()) setClickedItem(currentItem());
	slotNewCategory(getClickedItem());
}

void
CategoryView::slotNewCategory(ListItem *item)
{
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not slotNewCategory"<<endl;
#endif
		return;
	}
	if(!item) return;
	bool ok;
	const QString newName(
			KInputDialog::getText(i18n("Create New Category in %1").arg(item->fullName()),
								  i18n("Enter category name:"),
								  i18n("Category"),
								  &ok,
								  getMainWindow()->getImageViewer())
			.stripWhiteSpace());
	if(!ok || newName.isEmpty())
		return;
	QString msg;
	if(!getCategoryDBManager()->addSubCategory((dynamic_cast<CategoryListItemTag*>(item)), newName, msg))
	{
		KMessageBox::error(getMainWindow()->getImageViewer(), "<qt>"+msg+"</qt>");
		return;
	}
}

void
CategoryView::slotCatProperty()
{
	if(getClickedItem())
	{
		CategoryListItemTag* l_categoryListItemTag = dynamic_cast<CategoryListItemTag*>(getClickedItem());
		KApplication::setOverrideCursor (waitCursor);
		CategoryProperties catprop(this, l_categoryListItemTag);
		KApplication::restoreOverrideCursor ();
		if(!catprop.exec()) return;

		QString msg;
		l_categoryListItemTag->rename(catprop.getName(),msg );
		l_categoryListItemTag->setDescription(catprop.getDescription());
		l_categoryListItemTag->setIcon(catprop.getIcon());
	}
}

//-----------------------------------------------------------------------------

void
CategoryView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();
	////////////////////////////////////////////////////////////////////////////
	if ( !QUriDrag::canDecode(event) || !getDropedItem())
	{
		event->ignore();
	}
	else
	{
		event->acceptAction(true);
		QStrList lst;
		if (QUriDrag::decode (event, lst))
		{
			getMainWindow()->getImageListView()->stopLoading();
			getMainWindow()->update();

			(dynamic_cast<CategoryListItem*>(getDropedItem()))->addURL(QStringList::fromStrList(lst));

		}
	}
	////////////////////////////////////////////////////////////////////////////
	contentsDropEvent_end();
}


void
CategoryView::startWatchDir(const QString&)
{
}
void
CategoryView::stopWatchDir(const QString&)
{
}
void
CategoryView::startWatchDir()
{
}
void
CategoryView::stopWatchDir()
{
}

//-----------------------------------------------------------------------------

CategoryDBManager*
CategoryView::getCategoryDBManager()
{
	return m_p_cdbManager;
}
const CategoryDBManager*
CategoryView::getCategoryDBManager() const
{
        return m_p_cdbManager;
}



CategoryListItem*
CategoryView::getCategoryRoot()
{
	return m_p_categoryRoot;
}

//-----------------------------------------------------------------------------

void
CategoryView::slotANDSelection()
{
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not slotANDSelection"<<endl;
#endif
		return;
	}
	m_p_currentActionLabel->setText(i18n("Selection mode: AND"));

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
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not slotORSelection"<<endl;
#endif
		return;
	}
	m_p_currentActionLabel->setText(i18n("Selection mode: OR"));

	int num=getCategoryDBManager()->setSelectionMode(CategoryDBManager::mode_OR);
	loadingIsStarted(currentItem(), num);
	getCategoryDBManager()->refreshRequest();
	loadingIsFinished(currentItem(), num);
}


void
CategoryView::setDisabled(bool disable)
{
	ListItemView::setDisabled(disable);
	if(m_p_currentActionLabel)
		if(disable)
			m_p_currentActionLabel->setText(i18n("Adding files..."));
		else
			m_p_currentActionLabel->setText(" ");
}

void
CategoryView::setNumberOfLeftItems(int nbr)
{
	if(m_p_currentActionLabel)
		m_p_currentActionLabel->setText(i18n("Adding files (%1 left)...").arg(nbr));
}

//-----------------------------------------------------------------------------

void
CategoryView::fileIconRenamed(const QString& org, const QString& dest)
{
#if CATEGORYVIEW_DEBUG > 0
 	MYDEBUG<<org<<" "<<dest<<endl;
#endif
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not fileIconRenamed"<<endl;
#endif
		return;
	}
	getCategoryDBManager()->renameImage(org, dest);
}

void
CategoryView::filesMoved(const KURL::List& fileurls, const KURL& desturl)
{
#if CATEGORYVIEW_DEBUG > 0
 	MYDEBUG<<fileurls<<"->"<<desturl.url()<<endl;
#endif
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not filesMoved"<<endl;
#endif
		return;
	}
#if CATEGORYVIEW_DEBUG > 0
 	MYDEBUG<<"BEGIN"<<endl;
#endif
	getMainWindow()->getImageListView()->disconnect(SIGNAL(sigSetMessage(const QString&)));
	getCategoryDBManager()->moveImages(fileurls, desturl);
	connect(getMainWindow()->getImageListView(), SIGNAL(sigSetMessage(const QString&)),
			getMainWindow(), SLOT(slotSetStatusBarText(const QString&)));
#if CATEGORYVIEW_DEBUG > 0
 	MYDEBUG<<"END"<<endl;
#endif
}


void
CategoryView::fileIconsDeleted(const KURL::List& list)
{
#if CATEGORYVIEW_DEBUG > 0
	MYDEBUG<<"TODO "<<list<<endl;
#endif
}

void
CategoryView::directoryRenamed(const KURL& srcurl, const KURL& desturl)
{
#if CATEGORYVIEW_DEBUG > 0
  	MYDEBUG<<srcurl.path(-1)<<"->"<<desturl.path(-1)<<endl;
#endif
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not directoryRenamed"<<endl;
#endif
		return;
	}
	getCategoryDBManager()->renameDirectory(srcurl, desturl);
}


void
CategoryView::renameImage(QDict<QString>& renamedFiles)
{
	if(!getCategoryDBManager())
	{
#if CATEGORYVIEW_DEBUG > 0
		MYDEBUG<<"NO CategoryDBManager, I do not directoryRenamed"<<endl;
#endif
		return;
	}
	getCategoryDBManager()->renameImage(renamedFiles);
}

void
CategoryView::setAddAllImages(bool addAll)
{
	m_addAllImages = addAll;
	if(!getMainWindow()->getDirectoryView())
		return;
	if(m_addAllImages)
	{
		connect(getMainWindow()->getDirectoryView(), SIGNAL(loadingFinished(ListItem* ) ),
			getCategoryDBManager(), SLOT( newFilesAdded(ListItem* ) ));
	}
	else
	{
		getMainWindow()->getDirectoryView()->disconnect(this, SIGNAL(loadingFinished(ListItem* )));
	}

}

//-----------------------------------------------------------------------------

bool
CategoryView::getAddAllImages()
{
	return m_addAllImages;
}
QString
CategoryView::getType()
{
	return m_p_cdbManager->getType();
}
void
CategoryView::setType(const QString& type)
{
	m_p_cdbManager->setType(type);
}
QString
CategoryView::getSqlitePath()
{
	return m_p_cdbManager->getSqlitePath();
}
void
CategoryView::setSqlitePath(const QString& path)
{
	m_p_cdbManager->setSqlitePath(path);
}
QString
CategoryView::getMysqlUsername()
{
	return m_p_cdbManager->getMysqlUsername();
}
void
CategoryView::setMysqlUsername(const QString& name)
{
	m_p_cdbManager->setMysqlUsername(name);
}
QString
CategoryView::getMysqlPassword()
{
	return m_p_cdbManager->getMysqlPassword();
}
void
CategoryView::setMysqlPassword(const QString& pass)
{
	m_p_cdbManager->setMysqlPassword(pass);
}
QString
CategoryView::getMysqlHostname()
{
	return m_p_cdbManager->getMysqlHostname();
}
void
CategoryView::setMysqlHostname(const QString& host)
{
	m_p_cdbManager->setMysqlHostname(host);
}

int
CategoryView::removeObsololeteFilesOfTheDatabase()
{
	return getCategoryDBManager()->removeObsololeteFilesOfTheDatabase();
}


void
CategoryView::slotRefresh()
{
	loadingIsStarted(NULL, 0);
	getCategoryDBManager()->reload();
	loadingIsFinished(NULL, 0);
}

//-----------------------------------------------------------------------------

ListItem * CategoryView::openURL(const KURL& a_url) 
{
	ListItem *l_p_list_item = firstChild ();
	ListItem *l_p_found_item = NULL;

	QStringList l_list = QStringList::split( "/",a_url.path());
	QString l_type = l_list[0];
	l_list.pop_front();
	while(l_p_list_item && !l_p_found_item)
	{
		if(l_p_list_item->name() == l_type)
			l_p_found_item = l_p_list_item->find(l_list.join("/"));
		if(!l_p_found_item)	
			l_p_list_item = l_p_list_item->nextSibling();
	}

	if(!l_p_found_item)
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("CDArchiveView: TODO: The URL '<b>%1</b>' ").arg(a_url.url())+"</qt>");
	else
	{
		clearSelection();
		slotShowItem(l_p_found_item);
		setCurrentItem(l_p_found_item);
		setSelected(l_p_found_item, true);
	}
	return l_p_found_item;
}

bool CategoryView::isManagedURL(const KURL& a_url) 
{
	return 
		a_url.protocol()=="category";
	
}

#include "categoryview.moc"

#endif /* WANT_LIBKEXIDB */
