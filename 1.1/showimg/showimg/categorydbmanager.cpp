/***************************************************************************
                         categorydbmanager.cpp  -  description
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

#include "categorydbmanager.h"

#ifdef WANT_LIBKEXIDB

// Local
#include "mainwindow.h"
#include "listitem.h"
#include "categoryimagefileiconitem.h"
#include "categorylistitem.h"
#include "categoryview.h"
#include "imageviewer.h"
#include <showimgdb/categoriesdb.h>

// QT
#include <qfileinfo.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>

// KDE
#include <kfilemetainfo.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kapplication.h>

#define MYDEBUG kdDebug()<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "


class CategoryDBManager_private : public QThread
{
public:
	CategoryDBManager_private(CategoryDBManager *catdbM);
	virtual ~CategoryDBManager_private();

	void addFileInfo(QFileInfo *info);
	int getNumberOfLeftItems();

	virtual void run();

protected:
	CategoryDBManager *catdbM;
	QPtrList <QFileInfo> *listItemStack;
	QWaitCondition *stackNotEmpty;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


CategoryDBManager::CategoryDBManager( MainWindow *mw )
	: QObject(),
	m_selectionmode(CategoryDBManager::mode_AND),
	m_isAddingFiles(false)
{
	this->mw=mw;
	this->catdbM_priv=new CategoryDBManager_private(this);
	cdb = new CategoriesDB();

	patternList.setAutoDelete(false);

	catdbM_priv->start();
}

CategoryDBManager::~CategoryDBManager()
{
}

CategoryDBManager::SelectionMode
CategoryDBManager::getSelectionMode()
{
	MYDEBUG<<m_selectionmode<<endl;
	return m_selectionmode;
}


void
CategoryDBManager::newFilesAdded(ListItem *item)
{
	QFileInfo *info;
	QPtrList < FileIconItem > list = item->getFileIconItemList();
	for ( FileIconItem *iconitem = list.first(); iconitem; iconitem = list.next() )
	{
		if(mw->getCategoryView()->isImage(iconitem->fullName()))
		{
			info = new QFileInfo(iconitem->fullName());
			catdbM_priv->addFileInfo(info);
		}
	}
}



void
CategoryDBManager::addCurrentCategories(int cat_id)
{
	QString scat_id = QString::number(cat_id);
	if(catid_list.contains(scat_id)) return;
	catid_list.append(scat_id);
	refreshRequest();
}

void
CategoryDBManager::delCurrentCategories(int cat_id)
{
	QString scat_id = QString::number(cat_id);
	catid_list.remove(scat_id);
	refreshRequest();
}

void
CategoryDBManager::refreshRequest()
{
	mw->getImageListView()->setUpdatesEnabled( false );

	if(mw->preview()) mw->getImageListView()->stopLoading();
	mw->getImageListView()->stopLoading();
	if(!mw->getImageListView()->hasImages())
	{
		mw->getImageViewer()->loadImage();
		mw->getImageListView()->setContentsPos(0,0);
	}
	//--------------------------------------------------------------------------
	mw->slotRemoveImage(list.count());
	for (CategoryImageFileIconItem *item = list.first(); item; item = list.next() )
		mw->getImageListView()->takeItem(item);
	list.clear();
	//--------------------------------------------------------------------------
	if(mw->getCategoryView()->isClearingSelection())
	{
		mw->getImageListView()->setUpdatesEnabled( true );
		mw->getImageListView()->slotUpdate();
		return;
	}
	mw->setMessage(i18n("Loading query"));
	//--------------------------------------------------------------------------
	KApplication::setOverrideCursor( waitCursor ); // this might take time

	bool ok = true;
	QPtrList<ImageEntry> imageEntryList;
	if(!catid_list.isEmpty())
		imageEntryList = imagesSubCategoriesList(&ok);
	if(m_datetime_begin.isValid())
		imageEntryList = imagesDateList(imageEntryList, &ok);
	if(!patternList.isEmpty())
		imageEntryList = getImagesPatternList(imageEntryList, &ok);

	for(ImageEntry *image = imageEntryList.first();image;image=imageEntryList.next())
	{
		if(QFileInfo(image->getName()).exists())
			list.append(new CategoryImageFileIconItem(this, image->getName(), mw));
	}
	//--------------------------------------------------------------------------
	mw->setMessage(i18n("Ready"));
	mw->getImageListView()->sort();
	mw->slotAddImage(list.count());
	mw->getImageViewer()->updateStatus();
	mw->getImageListView()->setUpdatesEnabled( true );
	mw->getImageListView()->slotUpdate();

	KApplication::restoreOverrideCursor ();
	if(mw->preview()) mw->getImageListView()->slotLoadFirst();
}

QPtrList<CategoryNode>
CategoryDBManager::getRootCategories()
{
	return cdb->getRootCategories();
}


QPtrList<CategoryNode>
CategoryDBManager::getSubCategories(const QString& cat_name)
{
	return getSubCategories(getCategoryId(cat_name));
}

QPtrList<CategoryNode>
CategoryDBManager::getSubCategories(int cat_id)
{
	return cdb->getSubCategories(cat_id);
}

int
CategoryDBManager::getCategoryId(const QString& cat_name)
{
	return cdb->getCategoryId(cat_name);
}

QPtrList<ImageEntry>
CategoryDBManager::imagesSubCategoriesList(bool *ok)
{
	QPtrList<ImageEntry> ieList;
	if(!catid_list.isEmpty())
	{
		ieList = cdb->imagesSubCategoriesList(catid_list,
										getSelectionMode()==CategoryDBManager::mode_AND?CategoriesDB::mode_AND:CategoriesDB::mode_OR);
		if(ieList.isEmpty()) *ok = false;
	}
	return ieList;
}

QPtrList<QVariant>
CategoryDBManager::imageEntryList2IDImageList(const QPtrList<ImageEntry>& ieList)
{
	QPtrList<ImageEntry> m_ieList(ieList);
	QPtrList<QVariant> iiList;
	for(ImageEntry *ima_id = m_ieList.first(); ima_id; ima_id=m_ieList.next())
		iiList.append(new QVariant(ima_id->getId()));
	return iiList;
}


QPtrList<ImageEntry>
CategoryDBManager::getImagesPatternList(const QPtrList<ImageEntry>& imageEntryList, bool *ok)
{
	QPtrList<ImageEntry> m_imageEntrylist;
	if(!patternList.isEmpty())
	{
		MYDEBUG << *ok<<" "<< getSelectionMode()<<endl;
		if(*ok || (!*ok && getSelectionMode()==CategoryDBManager::mode_OR))
		{
			MYDEBUG<<endl;
			QPtrList<QVariant> imageIDList = imageEntryList2IDImageList(imageEntryList);
			m_imageEntrylist = cdb->imagesPatternList(patternList,
					imageIDList, getSelectionMode()==mode_AND?CategoriesDB::mode_AND:CategoriesDB::mode_OR );
			if(m_imageEntrylist.isEmpty()) *ok=false;
		}
	}
	else
	{
		MYDEBUG;
		m_imageEntrylist = imageEntryList;
	}
	return m_imageEntrylist;
}



QPtrList<ImageEntry>
CategoryDBManager::imagesDateList(const QPtrList<ImageEntry>& imageEntryList, bool *ok)
{
	QPtrList<ImageEntry> m_imageEntrylist;
	if(m_datetime_begin.isValid())
	{
		if(*ok || (!*ok && getSelectionMode()==CategoryDBManager::mode_OR))
		{
			QPtrList<QVariant> imageIDList = imageEntryList2IDImageList(imageEntryList);
			m_imageEntrylist =
				cdb->imagesDateList(m_datetime_begin.date(), m_datetime_end.date(),
									imageIDList, getSelectionMode()==mode_AND?CategoriesDB::mode_AND:CategoriesDB::mode_OR);
			if(m_imageEntrylist.isEmpty()) *ok=false;
		}
	}
	else
	{
		m_imageEntrylist = imageEntryList;
	}

	return m_imageEntrylist;
}

void
CategoryDBManager::addCategoryToImages(const QStringList& uris, int cat_id)
{
	cdb->addLink(uris, cat_id);
}


bool
CategoryDBManager::renameCategory(int cat_id, const QString& newName, QString& msg)
{
	return cdb->renameCategory(cat_id, newName, msg);
}

bool
CategoryDBManager::addSubCategory(CategoryListItemTag* catparent, const QString& newName, QString& msg )
{
	QString desc;
	CategoryNode* node = cdb->addSubCategory(catparent->getId(), newName, desc, msg);
	if(node)
	{
		(void)new CategoryListItemTag(catparent, node, mw);
		return true;
	}
	else
	{
		return false;
	}
}

CategoryNode*
CategoryDBManager::getCategoryNode(const QString& name)
{
	return cdb->getCategoryNode(name);
}

void
CategoryDBManager::addCategoryListItemTag(CategoryListItemTag* parent, QPtrList<CategoryNode>& cat_list)
{
	for(CategoryNode *node = cat_list.first(); node; node=cat_list.next())
		(void)new CategoryListItemTag(parent, node, mw);
}

void
CategoryDBManager::deleteNodeCategory(int cat_id)
{
	cdb->deleteNodeCategory(cat_id);
}

int
CategoryDBManager::getDirectoryId(const QString path) const
{
	return cdb->getDirectoryId(path);
}

void
CategoryDBManager::addImageToDB(QFileInfo *info)
{
	if(catdbM_priv->getNumberOfLeftItems() % 20 == 0) emit numberOfLeftItems(catdbM_priv->getNumberOfLeftItems());

	if(!info->exists()) return;
	int dir_id = getDirectoryId(info->dirPath());
// 	MYDEBUG<<info->dirPath()<<" is numbered " << dir_id<<endl;
	if(cdb->getImageId(info->fileName(), dir_id)>0) return;

	KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, KURL(info->absFilePath())/*iconitem->getURL()*/, true );
	KFileMetaInfo metaInfo(fileItem.metaInfo());

	QString comment = QString::null;
	QString sdatetime = QString::null;
	QDateTime datetime = info->lastModified();
	if(metaInfo.isValid())
	{
		comment = metaInfo.item("Comment").string();
		if(comment=="---") comment = QString::null;

		sdatetime = metaInfo.item("Date/time").string(true).stripWhiteSpace();
		if(sdatetime!="---") datetime = QDateTime::fromString(sdatetime, Qt::ISODate);
		if(!datetime.isValid ()) datetime = info->lastModified();
	}
	cdb->addImage(info->fileName(), info->dirPath(), datetime, comment);
}


void
CategoryDBManager::__startAddingImages__()
{
	m_isAddingFiles = true;
	emit isAddingFiles(m_isAddingFiles);
}


void
CategoryDBManager::flush()
{
	m_isAddingFiles = false;
	cdb->flushImages();
	emit isAddingFiles(m_isAddingFiles);
}

QDateTime
CategoryDBManager::getOldestImage()
{
	return cdb->getOldestImage();
}


QDateTime
CategoryDBManager::getNewestImage()
{
	return cdb->getNewestImage();
}

int
CategoryDBManager::getNumberOfImageForDate(int year, int month, int day)
{
	return cdb->getNumberOfImageForDate(year, month, day);
}

void
CategoryDBManager::addCurrentDate(QDateTime datetimeb, QDateTime datetimee)
{
	m_datetime_begin = datetimeb;
	m_datetime_end = datetimee;
	refreshRequest();
}

void
CategoryDBManager::delCurrentDate(QDateTime /*datetimeb*/, QDateTime /*datetimee*/)
{
	m_datetime_begin = QDateTime();
	m_datetime_end =  QDateTime();
	refreshRequest();
}


void
CategoryDBManager::addCurrentPattern(const QString* pattern)
{
	patternList.append(pattern);
	refreshRequest();
}


void
CategoryDBManager::delCurrentPattern(const QString* pattern)
{
	patternList.removeRef(pattern);
	refreshRequest();
}


void
CategoryDBManager::setSelectionMode(CategoryDBManager::SelectionMode mode)
{
	m_selectionmode = mode;
	refreshRequest();
}



//------------------------------------------------------------------------------

bool
CategoryDBManager::moveImages(const KURL::List& fileurls, const KURL& desturl)
{
	KURL::List list = fileurls;
	for(KURL::List::iterator it=list.begin(); it != list.end(); ++it)
	{
		MYDEBUG << (*it).path()<<"-"<< desturl.path(-1)<<endl;
		if(!ListItemView::isImage((*it).path()))
			moveDirectory((*it).path(), desturl.path(-1));
		else
			cdb->moveImage((*it).path(), desturl.path(-1));
	}

	return  true;
}


bool
CategoryDBManager::renameImage(const QString& oldname, const QString& newname)
{
	cdb->renameImage(oldname, newname) ;
	return  true;
}

bool
CategoryDBManager::renameDirectory(const KURL& srcurl, const KURL& desturl)
{
	MYDEBUG << srcurl.path(-1)<<"-"<< desturl.path(-1)<<endl;
	cdb->renameDirectory(srcurl.path(-1), desturl.path(-1)) ;
	return  true;
}

bool
CategoryDBManager::moveDirectory(const KURL& srcurl, const KURL& desturl)
{
	MYDEBUG<<srcurl<<" " <<desturl<<endl;
	cdb->moveDirectory(srcurl.path(-1), desturl.path(-1)) ;
	return true;
}

QStringList*
CategoryDBManager::getCategoryNameListImage(int image_id) const
{
	if(m_isAddingFiles)
	{
		MYDEBUG<<"I'm adding files..."<<endl;
		return new QStringList(i18n("(Updating database...)"));
	}
	else
		return cdb->getCategoryNameListImage(image_id) ;
}

QStringList*
CategoryDBManager::getCategoryNameListImage(const QString& ima_path) const
{
	if(m_isAddingFiles)
	{
		MYDEBUG<<"I'm adding files..."<<endl;
		return new QStringList("(Updating database...)");
	}

	QFileInfo info(ima_path);
	int dir_id = getDirectoryId(info.dirPath());
	int image_id = cdb->getImageId(info.fileName(), dir_id) ;
	return getCategoryNameListImage(image_id) ;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


CategoryDBManager_private::CategoryDBManager_private(CategoryDBManager *catdbM)
{
	this->catdbM=catdbM;
	listItemStack = new QPtrList <QFileInfo>;
	stackNotEmpty = new QWaitCondition();

	listItemStack->setAutoDelete(true);
}

CategoryDBManager_private::~CategoryDBManager_private()
{
}

void
CategoryDBManager_private::addFileInfo(QFileInfo *info)
{
	listItemStack->append(info);
	if(listItemStack->count()==1)
	{
		catdbM->__startAddingImages__();
		stackNotEmpty->wakeAll();
	}
}

int
CategoryDBManager_private::getNumberOfLeftItems()
{
	return listItemStack->count();
}


void
CategoryDBManager_private::run()
{
	QFileInfo *info;
	while(true)
	{
		if(!listItemStack->isEmpty())
		{
			info=listItemStack->first();
			catdbM->addImageToDB(info);
			listItemStack->removeFirst();
// 			MYDEBUG<<listItemStack->count()<<endl;
			//if(listItemStack->count()%10 == 0)
			//	emit catdbM->numberOfLeftItems(listItemStack->count());

			usleep(1);
		}
		else
		{
			catdbM->flush();
			stackNotEmpty->wait();
		}

	}
}

#include "categorydbmanager.moc"

#endif /* WANT_LIBKEXIDB */
