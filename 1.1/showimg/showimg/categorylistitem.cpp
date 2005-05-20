/***************************************************************************
                         categorylistitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2005 by Richard Groult
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

#include "categorylistitem.h"

#ifdef WANT_LIBKEXIDB

// Local
#include "mainwindow.h"
#include "directoryview.h"
#include "categorydbmanager.h"
#include "imagefileiconitem.h"
#include "imageviewer.h"
#include "categoryview.h"

#include <showimgdb/categorynode.h>
#include <showimgdb/imageentry.h>

// KDE
#include <klocale.h>
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <klocale.h>
#include <kinputdialog.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "




CategoryListItem::CategoryListItem( MainWindow *mw )
	: ListItem(mw, mw->getCategoryView())
{
	setCategoryDBManager(mw->getCategoryView()->getCategoryDBManager());
}

CategoryListItem::CategoryListItem(
			CategoryListItem* parent,
			const QString& title,
			MainWindow *mw )
	: ListItem(parent, title, mw)
{
	setReadOnly(false);
	setCategoryDBManager(mw->getCategoryView()->getCategoryDBManager());
}


CategoryListItem::~CategoryListItem()
{
}


void
CategoryListItem::setCategoryDBManager(CategoryDBManager *cdbManager)
{
	this->cdbManager=cdbManager;
}

CategoryDBManager*
CategoryListItem::getCategoryDBManager()
{
	return cdbManager;
}


//-----------------------------------------------

CategoryListItemRootTag::CategoryListItemRootTag( MainWindow *mw )
	: CategoryListItemTag(mw)
{
	node = new CategoryNode(0, full);
	full = i18n("Categories");
	f.setName (full);
	setReadOnly(true);

	init();
}

void
CategoryListItemRootTag::init()
{
	if(node->getIcon().isEmpty())
		setPixmap(0, BarIcon("kontact_mail", getListItemView()->getIconSize() ));
	else
		setPixmap(0, BarIcon(node->getIcon(), getListItemView()->getIconSize() ));
	setDropEnabled(false);
	size="";
	extension = i18n("Category");
	setType("Category");
	if(!getCategoryDBManager())
	{
		kdWarning()<<"cdbManager is NULL!!"<<endl;
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
										 CategoryNode *node,
										 MainWindow *mw )
	: CategoryListItem(parent, node->getTitle(), mw)
{
	this->node = node;
	full=parent->fullName()+"-"+node->getTitle();
	f.setName(node->getTitle());

	init();
}

CategoryListItemTag::CategoryListItemTag( MainWindow *mw )
	: CategoryListItem(mw)
{
}

void
CategoryListItemTag::init()
{
	if(node->getIcon().isEmpty())
		setPixmap(0, BarIcon("kontact_mail", getListItemView()->getIconSize() ));
	else
		setPixmap(0, BarIcon(node->getIcon(), getListItemView()->getIconSize() ));
	setDropEnabled(false);
	size="";
	extension = i18n("Category");
	setType("Category");
	if(!getCategoryDBManager())
	{
		kdWarning()<<"cdbManager is NULL!!"<<endl;
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
	getCategoryDBManager()->addCurrentCategories(getId());
}

void
CategoryListItemTag::unLoad()
{
	getCategoryDBManager()->delCurrentCategories(getId());
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
	full=getCategoryNode()->getTitle();
	f.setName (full);
	repaint();
	return true;
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
										   CategoryListItemDate_type datetype,
										   MainWindow *mw )
	: CategoryListItem(parent, QString::null, mw)
{
	m_datetype = datetype;
	m_datetime = datetime;

	switch(m_datetype)
	{
		case YEAR : f.setName(QString("%1").arg(m_datetime.date().year())); break;
		case MONTH :f.setName(QString("%1 - %2").arg(m_datetime.date().month()).arg(KGlobal::locale()->calendar()->monthName(m_datetime.date()))); break;
		case DAY : f.setName(QString("%1 - %2").arg(m_datetime.date().day()).arg(KGlobal::locale()->calendar()->weekDayName(m_datetime.date()))); break;
	}
	full = parent->fullName()+"-"+f.name();

	init();
}
CategoryListItemDate::CategoryListItemDate( MainWindow *mw )
	:CategoryListItem(mw)
{
}

void
CategoryListItemDate::init()
{
	setPixmap(0, BarIcon("clock", getListItemView()->getIconSize() ));
	setExpandable(true);

	if(m_datetype == CategoryListItemDate::YEAR)
	{
		size = QString("%1").arg(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year()));
	}
	else
	if(m_datetype == CategoryListItemDate::MONTH)
	{
		size = QString("%1").arg(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year(), m_datetime.date().month()));
	}
	else
	if(m_datetype == CategoryListItemDate::DAY)
	{
		size = QString("%1").arg(getCategoryDBManager()->getNumberOfImageForDate(m_datetime.date().year(), m_datetime.date().month(), m_datetime.date().day()));
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
				item = new CategoryListItemDate(this, QDateTime(QDate(m_datetime.date().year(), i, 1)) , CategoryListItemDate::MONTH, mw );
				if(item->getSize()<1)
					delete(item);
			}
		}
		else
		if(m_datetype == CategoryListItemDate::MONTH)
		{
			for(int i=1; i<=KGlobal::locale()->calendar()->daysInMonth(m_datetime.date()); i++)
			{
				item = new CategoryListItemDate(this, QDateTime(QDate(m_datetime.date().year(), m_datetime.date().month(), i)) , CategoryListItemDate::DAY, mw );

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
	MYDEBUG<<dateb<<" "<<datee<<endl;
	getCategoryDBManager()->addCurrentDate(dateb, datee);

}

void CategoryListItemDate::unLoad()
{
	if(!getCategoryDBManager()) return;
	getCategoryDBManager()->delCurrentDate(getDateTime(), getDateTime());
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


CategoryListItemRootDate::CategoryListItemRootDate( MainWindow *mw )
	: CategoryListItemDate(mw)
{
	full = i18n("Date");
	f.setName (full);
	setReadOnly(true);

	init();
}

void
CategoryListItemRootDate::init()
{
	setPixmap(0, BarIcon("date", getListItemView()->getIconSize() ));
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
			(void)new CategoryListItemDate(this, QDateTime(QDate(i,1,1)) , CategoryListItemDate::YEAR, mw );

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

CategoryListItemSearch::CategoryListItemSearch(CategoryListItemSearch* parent, const QString& pattern, MainWindow *mw)
	: CategoryListItem(parent, pattern, mw)
{
	this->pattern = pattern;
	init();
}


CategoryListItemSearch::CategoryListItemSearch( MainWindow *mw)
	: CategoryListItem(mw)
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
	getCategoryDBManager()->addCurrentPattern(&pattern);

}


void
CategoryListItemSearch::unLoad()
{
	getCategoryDBManager()->delCurrentPattern(&pattern);
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
	setPixmap(0, BarIcon("filefind", getListItemView()->getIconSize() ));
	setExpandable(false);
}



//------------------------------------------------------------------------------

CategoryListItemRootSearch::CategoryListItemRootSearch( MainWindow *mw )
	: CategoryListItemSearch(mw)
{
	full = i18n("Search...");
	f.setName (full);
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
	setPixmap(0, BarIcon("filefind", getListItemView()->getIconSize() ));
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
								  mw).stripWhiteSpace());
	if(ok && !newSeachPattern.isEmpty())
	{
		CategoryListItemSearch *item=new CategoryListItemSearch(this, newSeachPattern, mw);
		if(!item) return;
		if(!isOpen()) {setOpen(true);kapp->processEvents();}
		mw->getCategoryView()->clearSelection();
		mw->getCategoryView()->setSelected(item, true);
		mw->getCategoryView()->setCurrentItem(item);
	}
}

void
CategoryListItemRootSearch::setOpen (bool o)
{
	if(!isOpen() && o && !childCount())
		setSelected(true);
	CategoryListItemSearch::setOpen(o);
}
#endif /* WANT_LIBKEXIDB */
