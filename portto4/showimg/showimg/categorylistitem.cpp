/*****************************************************************************
                         categorylistitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "categorylistitem.h"

//-----------------------------------------------------------------------------
#define CATEGORYLISTITEM_DEBUG           0
//-----------------------------------------------------------------------------

#ifdef WANT_LIBKEXIDB

// Local
#include "categorydbmanager.h"
#include "categoryview.h"
#include "directoryview.h"
#include "imagefileiconitem.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"
#include <showimgdb/categorynode.h>
#include <showimgdb/imageentry.h>

// KDE
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <klocale.h>


CategoryListItem::CategoryListItem( )
	: ListItem(getMainWindow()->getCategoryView())
{
	setCategoryDBManager(getMainWindow()->getCategoryView()->getCategoryDBManager());
}

CategoryListItem::CategoryListItem(
			CategoryListItem* parent,
			const QString& title )
	: ListItem(parent, title)
{
	setReadOnly(false);
	setProtocol("category");
	setCategoryDBManager(getMainWindow()->getCategoryView()->getCategoryDBManager());
}


CategoryListItem::~CategoryListItem()
{
}


void
CategoryListItem::setCategoryDBManager(CategoryDBManager *cdbManager)
{
	m_p_cdbManager = cdbManager;
}

CategoryDBManager*
CategoryListItem::getCategoryDBManager()
{
	return m_p_cdbManager;
}

void
CategoryListItem::load(bool)
{
	ListItem::load(true);
	setSize(-1);
}
void
CategoryListItem::unLoad()
{
// 	ListItem::unLload(true);
}

void
CategoryListItem::reload()
{
	getCategoryDBManager()->reload();

}

uint
CategoryListItem::getNumberOfItems()
{
	return m_numberOfItems;
}

ListItem * CategoryListItem::find(const KURL& a_url)
{
#if CATEGORYLISTITEM_DEBUG > 0
	MYDEBUG 
		<<"getURL()"<<getURL().url()<< " "
		<<"a_url="<<a_url.url()<< " "
		<<endl;
#endif
	return find(a_url.path());
}

ListItem * CategoryListItem::find(const QString& a_end_dir)
{
#if CATEGORYLISTITEM_DEBUG > 0
	MYDEBUG 
		<<"getURL()"<<getURL().url()<< " "
		<<"a_end_dir="<<a_end_dir<< " "
		<<endl;
#endif
	QStringList l_list = QStringList::split( "/",a_end_dir);
	QString l_dirName = l_list[0];
	l_list.pop_front();
	ListItem *l_p_rootItems = firstChild ();
#if CATEGORYLISTITEM_DEBUG > 0
	MYDEBUG 
		<<"l_list="<<l_list<< " "
		<<"l_dirName="<<l_dirName<< " "
		<<endl;
#endif
	while(l_p_rootItems)
	{
#if CATEGORYLISTITEM_DEBUG > 0
	MYDEBUG 
		<<"l_p_rootItems->name()="<<l_p_rootItems->name()<< " "
		<<endl;
#endif
		if(l_p_rootItems->name() == l_dirName)
			if(l_list.isEmpty())
				return l_p_rootItems;
			else
				return l_p_rootItems->find(l_list.join("/"));
		l_p_rootItems = l_p_rootItems->nextSibling();
	}
	return NULL;
}

//-----------------------------------------------

CategoryListItemRootTag::CategoryListItemRootTag()
	: CategoryListItemTag()
{
	node = new CategoryNode(0, fullName());
	setFullName("/Category");
	setReadOnly(true);

	init();
}

void
CategoryListItemRootTag::init()
{
	setPixmap(0, BarIcon(node->getIcon(), getMainWindow()->getCategoryView()->getIconSize() ));
	setDropEnabled(false);
	setExtension(i18n("Category"));
	if(!getCategoryDBManager())
	{
		MYWARNING<<"cdbManager is NULL!!"<<endl;
		return;
	}
	QPtrList<CategoryNode> cat_list;
	cat_list = getCategoryDBManager()->getRootCategories();
	getCategoryDBManager()->addCategoryListItemTag(this, cat_list);
}

void
CategoryListItemRootTag::load(bool)
{
	MYDEBUG<<"Nothing todo"<<endl;
}

void
CategoryListItemRootTag::unLoad()
{
	MYDEBUG<<"Nothing todo"<<endl;
}

bool
CategoryListItemRootTag::isLeaf()
{
	return false;
}

int
CategoryListItemRootTag::getId()
{
	return 0;
}

//-----------------------------------------------

CategoryListItemTag::CategoryListItemTag(CategoryListItemTag* parent,
										 CategoryNode *node)
	: CategoryListItem(parent, node->getTitle())
{
	this->node = node;
	setFullName(parent->fullName()+"/"+node->getTitle());
	//FIXME getFile().setName(node->getTitle());

	init();
}

CategoryListItemTag::CategoryListItemTag(  )
	: CategoryListItem()
{
}

void
CategoryListItemTag::init()
{
	setPixmap(0, BarIcon(node->getIcon(), getMainWindow()->getCategoryView()->getIconSize() ));
	setDropEnabled(false);
	setExtension(i18n("ext Category"));
	//setType("Category");
	if(!getCategoryDBManager())
	{
		MYWARNING<<"cdbManager is NULL!!"<<endl;
		return;
	}
	QPtrList<CategoryNode> cat_list;
	cat_list = getCategoryNode()->getChildCategoryList();
	getCategoryDBManager()->addCategoryListItemTag(this, cat_list);
}

void
CategoryListItemTag::load(bool)
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::load(true);

	//FIXME getMainWindow()->setMessage(i18n("Loading query..."));
	int num = getCategoryDBManager()->addCurrentCategories(getId());
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, num);
}

void
CategoryListItemTag::unLoad()
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::unLoad();

	int num = getCategoryDBManager()->delCurrentCategories(getId());
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, num);
}

void
CategoryListItemTag::addURL(const QStringList& list)
{
	getCategoryDBManager()->addCategoryToImages(list, getId());
}

bool
CategoryListItemTag::rename(const QString& newName, QString& msg)
{
	if(!getCategoryDBManager()->renameCategory(getId(), newName, msg))
		 return false;
	setFullName(getCategoryNode()->getTitle());
	//FIXME getFile().setName (fullName());
	repaint();
	return true;
}
bool
CategoryListItemTag::setDescription(const QString& descr)
{
	QString msg;
	getCategoryDBManager()->setCategoryDescription(getId(), descr, msg);
	//getCategoryNode()->setDescription(descr);
	repaint();
	return true;
}

QString
CategoryListItemTag::getDescription()
{
	return getCategoryNode()->getDescription();
}

bool
CategoryListItemTag::setIcon(const QString& icon)
{
	MYDEBUG<<icon<<endl;
	QString msg;
	if(getCategoryDBManager()->setCategoryIcon(getId(), icon, msg))
		setPixmap(0, BarIcon(node->getIcon(), getMainWindow()->getCategoryView()->getIconSize() ));
	repaint();
	return true;
}
QString
CategoryListItemTag::getIcon()
{
	return getCategoryNode()->getIcon();
}

int
CategoryListItemTag::getId()
{
	return getCategoryNode()->getId();
}

bool
CategoryListItemTag::isLeaf()
{
	return getCategoryNode()->isLeafCategory();
}

CategoryNode*
CategoryListItemTag::getCategoryNode()
{
	return node;
}



//------------------------------------------------------------------
//------------------------------------------------------------------

CategoryListItemDate::CategoryListItemDate(CategoryListItemDate* parent,
										   const QDateTime& datetime,
										   CategoryListItemDate_type datetype
)
	: CategoryListItem(parent, QString::null)
{
	m_datetype = datetype;
	m_datetime = datetime;
	QString l_name;

	switch(m_datetype)
	{
		case YEAR : l_name=(QString("%1").arg(m_datetime.date().year()));
			break;
		case MONTH : l_name=(QString("%1 - %2").arg(m_datetime.date().month()).arg(KGlobal::locale()->calendar()->monthName(m_datetime.date())));
			break;
		case DAY : l_name=(QString("%1 - %2").arg(m_datetime.date().day()).arg(KGlobal::locale()->calendar()->weekDayName(m_datetime.date())));
			break;
	}
	setFullName(parent->fullName()+"/"+l_name, getProtocol());

	init();
}
CategoryListItemDate::CategoryListItemDate(  )
	:CategoryListItem()
{
}

void
CategoryListItemDate::init()
{
	setPixmap(0, BarIcon("clock", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(true);

	if(m_datetype == CategoryListItemDate::YEAR)
	{
		setSize(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year()));
	}
	else
	if(m_datetype == CategoryListItemDate::MONTH)
	{
		setSize(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year(), m_datetime.date().month()));
	}
	else
	if(m_datetype == CategoryListItemDate::DAY)
	{
		setSize(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year(), m_datetime.date().month(), m_datetime.date().day()));
		setExpandable(false);
	}
}

bool
CategoryListItemDate::isLeaf()
{
	return false;
}

QDateTime
CategoryListItemDate::getDateTime()
{
	return m_datetime;
}

void
CategoryListItemDate::setOpen (bool o)
{
	if(!isOpen() && o && !childCount())
	{
		KApplication::setOverrideCursor (waitCursor);
		CategoryListItem *item=NULL;
		if(m_datetype == CategoryListItemDate::YEAR)
		{
			for(int i=1; i<=12; i++)
			{
				item = new CategoryListItemDate(this, QDateTime(QDate(m_datetime.date().year(), i, 1)) , CategoryListItemDate::MONTH);
				if(item->getSize()<1)
					delete(item);
			}
		}
		else
		if(m_datetype == CategoryListItemDate::MONTH)
		{
			for(int i=1; i<=KGlobal::locale()->calendar()->daysInMonth(m_datetime.date()); i++)
			{
				item = new CategoryListItemDate(this, QDateTime(QDate(m_datetime.date().year(), m_datetime.date().month(), i)) , CategoryListItemDate::DAY);

				if(item->getSize()<1)
					delete(item);
			}
		}
		KApplication::restoreOverrideCursor ();
	}
	CategoryListItem::setOpen(o);
}

void
CategoryListItemDate::load(bool)
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::load(true);

	////////////////////////////////////////////////////////////////////////////
	QDateTime dateb, datee;

	dateb = m_datetime;
	switch(m_datetype)
	{
		case YEAR : datee = QDateTime(QDate(
											m_datetime.date().year(),
											12,
											31
										   )
									 ); break;

		case MONTH :  datee = QDateTime(QDate(
												m_datetime.date().year(),
												m_datetime.date().month(),
												KGlobal::locale()->calendar()->daysInMonth(m_datetime.date())
											 )
									   ); break;
		case DAY :  datee = m_datetime; break;
	}

	int num=getCategoryDBManager()->addCurrentDate(dateb, datee);
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getCategoryView()->loadingIsFinished(this, num);
}

void CategoryListItemDate::unLoad()
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::unLoad();
	int num=getCategoryDBManager()->delCurrentDate(getDateTime(), getDateTime());
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, num);
}

void
CategoryListItemDate::addURL(const QStringList& /*list*/)
{
	MYDEBUG<<"Nothing to add !!"<<endl;
}
bool
CategoryListItemDate::rename(const QString& newDirName, QString& msg)
{
	msg = "TODO " + newDirName;
	return false;
}


//------------------------------------------------------------------------------


CategoryListItemRootDate::CategoryListItemRootDate(  )
	: CategoryListItemDate()
{
	setFullName("/"+i18n("Date"));
	setReadOnly(true);

	init();
}

void
CategoryListItemRootDate::init()
{
	setPixmap(0, BarIcon("date", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(true);
}

void
CategoryListItemRootDate::setOpen (bool o)
{
	if(!isOpen() && o && !childCount())
	{
		KApplication::setOverrideCursor (waitCursor);

		QDateTime oldest = getCategoryDBManager()->getOldestImage();
		QDateTime newest = getCategoryDBManager()->getNewestImage();
		int yearb=oldest.date().year(), yeare=newest.date().year();
		for(int i=yearb; i<=yeare; i++)
			(void)new CategoryListItemDate(this, QDateTime(QDate(i,1,1)) , CategoryListItemDate::YEAR );

		KApplication::restoreOverrideCursor ();
	}
	CategoryListItem::setOpen(o);
}

void
CategoryListItemRootDate::load(bool)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CategoryListItemSearch::CategoryListItemSearch(CategoryListItemSearch* parent, const QString& pattern)
	: CategoryListItem(parent, pattern)
{
	this->pattern = pattern;
	init();
}


CategoryListItemSearch::CategoryListItemSearch( )
	: CategoryListItem()
{
}

bool
CategoryListItemSearch::isLeaf()
{
	return true;
}


void
CategoryListItemSearch::load(bool)
{
	CategoryListItem::load(true);

	int num=getCategoryDBManager()->addCurrentPattern(pattern);
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getCategoryView()->loadingIsFinished(this, num);
}


void
CategoryListItemSearch::unLoad()
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::unLoad();
	int num=getCategoryDBManager()->delCurrentPattern(pattern);
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, num);
}



void
CategoryListItemSearch::addURL(const QStringList& /*list*/)
{
}



bool
CategoryListItemSearch::rename(const QString& /*newDirName*/, QString& /*msg*/)
{
	return false;
}



void
CategoryListItemSearch::init()
{
	setPixmap(0, BarIcon("filefind", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(false);
}



//------------------------------------------------------------------------------

CategoryListItemRootSearch::CategoryListItemRootSearch(  )
	: CategoryListItemSearch()
{
	setFullName("/"+i18n("Search"));
	//FIXME getFile().setName ("Search...");
	setReadOnly(true);

	init();
}

bool
CategoryListItemRootSearch::isLeaf()
{
	return false;
}

void
CategoryListItemRootSearch::init()
{
	setPixmap(0, BarIcon("filefind", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(true);
}

void
CategoryListItemRootSearch::load(bool)
{
	bool ok;
	KApplication::restoreOverrideCursor ();
	const QString newSeachPattern(
			KInputDialog::getText(i18n("Search pattern"),
								  i18n("Enter pattern:"),
								  i18n("pattern"),
								  &ok,
								  getMainWindow()).stripWhiteSpace());
	if(ok && !newSeachPattern.isEmpty())
	{
		CategoryListItemSearch *item=new CategoryListItemSearch(this, newSeachPattern);
		if(!item) return;
		if(!isOpen())
		{
			setOpen(true);
// 			kapp->processEvents();
		}
		getMainWindow()->getCategoryView()->clearSelection();
		getMainWindow()->getCategoryView()->setSelected(item, true);
		getMainWindow()->getCategoryView()->setCurrentItem(item);
	}
}

void
CategoryListItemRootSearch::setOpen (bool o)
{
	if(!isOpen() && o && !childCount())
		setSelected(true);
	CategoryListItemSearch::setOpen(o);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CategoryListItemNote::CategoryListItemNote(CategoryListItemNote* parent, const int note)
	: CategoryListItem(parent, QString::number(note))
{
	this->m_note = QString::number(note);
	init();
}


CategoryListItemNote::CategoryListItemNote( )
	: CategoryListItem()
{
}

bool
CategoryListItemNote::isLeaf()
{
	return true;
}


void
CategoryListItemNote::load(bool)
{
	CategoryListItem::load(true);

// 	MYDEBUG<<m_note<<endl;
	int num=getCategoryDBManager()->addCurrentNote(m_note);
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getCategoryView()->loadingIsFinished(this, num);
}


void
CategoryListItemNote::unLoad()
{
	if(!getCategoryDBManager()) return;
	CategoryListItem::unLoad();
	int num=getCategoryDBManager()->delCurrentNote(m_note);
	getMainWindow()->getCategoryView()->loadingIsStarted(this, num);
	m_numberOfItems = getCategoryDBManager()->refreshRequest();
	getMainWindow()->getCategoryView()->loadingIsFinished(this, num);
}



void
CategoryListItemNote::addURL(const QStringList& list)
{
	getCategoryDBManager()->addNoteToImages(list, m_note.toInt());
}



bool
CategoryListItemNote::rename(const QString& /*newDirName*/, QString& /*msg*/)
{
	return false;
}



void
CategoryListItemNote::init()
{
	setPixmap(0, BarIcon("flag", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(false);

	setFullName(i18n("/Note/%1").arg(m_note));
}



//------------------------------------------------------------------------------

CategoryListItemRootNote::CategoryListItemRootNote(  )
	: CategoryListItemNote()
{
	setFullName("/"+i18n("Note"));
	//FIXME getFile().setName(i18n("Note"));
	setReadOnly(true);

	init();
}

bool
CategoryListItemRootNote::isLeaf()
{
	return false;
}

void
CategoryListItemRootNote::init()
{
	setPixmap(0, BarIcon("favorites", getMainWindow()->getCategoryView()->getIconSize() ));
	setExpandable(true);
}

void
CategoryListItemRootNote::load(bool)
{
}

void
CategoryListItemRootNote::setOpen (bool o)
{
	if(!isOpen() && o && !childCount())
	{
		for(int i=1; i<11; i++)
			(void)new CategoryListItemNote(this, i);
	}
	CategoryListItemNote::setOpen(o);
}

#endif /* WANT_LIBKEXIDB */
