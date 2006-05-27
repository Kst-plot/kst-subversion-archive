/***************************************************************************
                         categorydbmanager.cpp  -  description
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
#include <kprogress.h>
#include <kmessagebox.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "


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

	cdb(NULL),
	m_selectionmode(CategoryDBManager::mode_AND),
	m_isAddingFiles(false),

	m_type("sqlite"),
	m_sqlitePath(QString::null),
	m_mysqlUsername(QString::null),
	m_mysqlPassword(QString::null),
	m_mysqlHostname(QString::null)
{
	setName("CategoryDBManager");
	this->mw=mw;
	this->catdbM_priv=new CategoryDBManager_private(this);

// 	MYDEBUG<<"Creating CategoriesDB"<<endl;
	readConfig(KGlobal::config());
	cdb = new CategoriesDB(getType(), getSqlitePath(), getMysqlUsername(), getMysqlPassword(), getMysqlHostname());
// 	MYDEBUG<<"\tendCreating CategoriesDB"<<endl;

	//patternList.setAutoDelete(false);
	//noteList.setAutoDelete(false);

	catdbM_priv->start();

	setEnabled(false);
}

CategoryDBManager::~CategoryDBManager()
{
	writeConfig(KGlobal::config());
}

void
CategoryDBManager::readConfig(KConfig *config)
{
	config->setGroup("Categories");
	setType(config->readEntry("type", "sqlite"));
	setSqlitePath(config->readPathEntry("SqlitePath", QDir::homeDirPath()+"/.showimg/MyCategoriesDB3.sidb"));
	setMysqlUsername(config->readEntry("MysqlUsername", "myname"));
	setMysqlPassword(config->readEntry("MysqlPassword", "password"));
	setMysqlHostname(config->readEntry("MysqlHostname", "localhost.localdomain"));
}
void
CategoryDBManager::writeConfig(KConfig *config)
{
	config->setGroup("Categories");
	config->writeEntry("type", getType());
	config->writePathEntry("SqlitePath", getSqlitePath());
	config->writeEntry("MysqlUsername", getMysqlUsername());
	config->writeEntry("MysqlPassword", getMysqlPassword());
	config->writeEntry("MysqlHostname", getMysqlHostname());

	config->sync();
}


QString
CategoryDBManager::getType()
{
	return m_type;
}
void
CategoryDBManager::setType(const QString& type)
{
	m_type = type;
}
QString
CategoryDBManager::getSqlitePath()
{
	return m_sqlitePath;
}
void
CategoryDBManager::setSqlitePath(const QString& path)
{
	m_sqlitePath = path;
}
QString
CategoryDBManager::getMysqlUsername()
{
	return m_mysqlUsername;
}
void
CategoryDBManager::setMysqlUsername(const QString& name)
{
	m_mysqlUsername = name;
}
QString
CategoryDBManager::getMysqlPassword()
{
	return m_mysqlPassword;
}
void
CategoryDBManager::setMysqlPassword(const QString& pass)
{
	m_mysqlPassword = pass;
}
QString
CategoryDBManager::getMysqlHostname()
{
	return m_mysqlHostname;
}
void
CategoryDBManager::setMysqlHostname(const QString& host)
{
	m_mysqlHostname = host;
}


CategoryDBManager::SelectionMode
CategoryDBManager::getSelectionMode()
{
	return m_selectionmode;
}

bool
CategoryDBManager::isConnected() const
{
	return cdb->isConnected();
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
CategoryDBManager::reload()
{
	if(catid_list.isEmpty() && !m_datetime_begin.isValid() && patternList.isEmpty() && noteList.isEmpty())
		return;

	mw->setEnabled(false);
	mw->saveNumberOfImages();
	mw->slotRemoveImage(mw->getTotal());
	int num = refreshRequest_private();
	mw->getCategoryView()->setTotalNumberOfFiles(num);
	refreshRequest();
	mw->slotDone(num);
	mw->restoreNumberOfImages();
	mw->setEnabled(true);
}

int
CategoryDBManager::refreshRequest_private()
{
	KApplication::setOverrideCursor (waitCursor);
	//--------------------------------------------------------------------------
	mw->slotRemoveImage(list.count());
	for (CategoryImageFileIconItem *item = list.first(); item; item = list.next() )
		mw->getImageListView()->takeItem(item);
	list.clear();
	m_imageEntryList.clear();
	//--------------------------------------------------------------------------
	if(mw->getCategoryView()->isClearingSelection())
	{
		mw->getImageListView()->setUpdatesEnabled( true );
		mw->getImageListView()->slotUpdate();
		return 0;
	}
	mw->setMessage(i18n("Loading query"));
	//--------------------------------------------------------------------------
	bool ok = true;
// 	MYDEBUG<<catid_list<<endl;
// 	MYDEBUG<<patternList<<endl;
// 	MYDEBUG<<noteList<<endl;

	if(!catid_list.isEmpty())
		m_imageEntryList = getImagesSubCategoriesList(&ok);
	if(m_datetime_begin.isValid())
		m_imageEntryList = getImagesDateList(m_imageEntryList, &ok);
	if(!patternList.isEmpty())
		m_imageEntryList = getImagesPatternList(m_imageEntryList, &ok);
	if(!noteList.isEmpty())
		m_imageEntryList = getImagesNoteList(m_imageEntryList, &ok);
	//--------------------------------------------------------------------------
	KApplication::restoreOverrideCursor ();
	return m_imageEntryList.count();
}

int
CategoryDBManager::refreshRequest()
{
	int num=0;
	for(ImageEntry *image = m_imageEntryList.first(); image; image=m_imageEntryList.next())
	{
		if(QFileInfo(image->getName()).exists())
		{
			list.append(new CategoryImageFileIconItem(this, image->getName(), mw));

			mw->getCategoryView()->setHasSeenFile();
			num++;
		}
	}
	return num;
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
CategoryDBManager::getImagesSubCategoriesList(bool *ok)
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
// 		MYDEBUG << *ok<<" "<< getSelectionMode()<<endl;
		if(*ok || (!*ok && getSelectionMode()==CategoryDBManager::mode_OR))
		{
// 			MYDEBUG<<endl;
			QPtrList<QVariant> imageIDList = imageEntryList2IDImageList(imageEntryList);
			m_imageEntrylist = cdb->imagesPatternList(patternList,
					imageIDList, getSelectionMode()==mode_AND?CategoriesDB::mode_AND:CategoriesDB::mode_OR );
			if(m_imageEntrylist.isEmpty()) *ok=false;
		}
	}
	else
	{
// 		MYDEBUG;
		m_imageEntrylist = imageEntryList;
	}
	return m_imageEntrylist;
}

QPtrList<ImageEntry>
CategoryDBManager::getImagesNoteList(const QPtrList<ImageEntry>& imageEntryList, bool *ok)
{
	QPtrList<ImageEntry> m_imageEntrylist;
	if(!noteList.isEmpty())
	{
// 		MYDEBUG << *ok<<" "<< getSelectionMode()<<endl;
		if(*ok || (!*ok && getSelectionMode()==CategoryDBManager::mode_OR))
		{
// 			MYDEBUG<<endl;
			QPtrList<QVariant> imageIDList = imageEntryList2IDImageList(imageEntryList);
			m_imageEntrylist = cdb->imagesNoteList(noteList,
					imageIDList, getSelectionMode()==mode_AND?CategoriesDB::mode_AND:CategoriesDB::mode_OR );
			if(m_imageEntrylist.isEmpty()) *ok=false;
		}
	}
	else
	{
// 		MYDEBUG;
		m_imageEntrylist = imageEntryList;
	}
	return m_imageEntrylist;
}


QPtrList<ImageEntry>
CategoryDBManager::getImagesDateList(const QPtrList<ImageEntry>& imageEntryList, bool *ok)
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
// 	MYDEBUG<<m_imageEntrylist.count()<<endl;
	return m_imageEntrylist;
}

void
CategoryDBManager::addCategoryToImages(const QStringList& uris, int cat_id)
{
	if(!cdb->isConnected()) return ;

	MYDEBUG<<"BEGIN "<<endl;

	mw->saveNumberOfImages();
	////////////////////////////////////////////////////////////////////////////
	if(uris.count()>5) mw->setEnabled(false);
	mw->setMessage(i18n("Adding/checking files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(uris.count());

	connect(this, SIGNAL(sigHasSeenFile(int)),
		mw, SLOT(slotPreviewDone(int)));

	QFileInfo *info=NULL;
	KURL tmp_url;
	for ( QStringList::ConstIterator it = uris.begin(); it != uris.end(); ++it )
	{
		MYDEBUG<<*it<<endl;
		tmp_url = KURL(*it);
		if(tmp_url.protocol() == QString::fromLatin1("file"))
		{
			info = new QFileInfo(tmp_url.path());
			addImageToDB(info, false, true);
		}
	}
	flush();

	disconnect(SIGNAL(sigHasSeenFile(int)), mw);

	///////////////////////////////////////////////////////////////////////////

	mw->setMessage(i18n("Setting category for files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(uris.count());

	connect(cdb, SIGNAL(sigLinkAdded()),
		mw, SLOT(slotPreviewDone()));

	cdb->addLink(uris, cat_id);

	cdb->disconnect(SIGNAL(sigLinkAdded()), mw);

	///////////////////////////////////////////////////////////////////////////

	mw->setMessage(i18n("Ready"));
	mw->slotDone(uris.count());
	mw->restoreNumberOfImages();
	mw->setEnabled(true);
}


void
CategoryDBManager::addNoteToImages(const QStringList& uris, int note)
{
	if(!cdb->isConnected()) return ;

	MYDEBUG<<"BEGIN "<<endl;

	mw->saveNumberOfImages();
	////////////////////////////////////////////////////////////////////////////
	if(uris.count()>5) mw->setEnabled(false);
	mw->setMessage(i18n("Adding/checking files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(uris.count());

	connect(this, SIGNAL(sigHasSeenFile(int)),
		mw, SLOT(slotPreviewDone(int)));

	QFileInfo *info=NULL;
	KURL tmp_url;
	for ( QStringList::ConstIterator it = uris.begin(); it != uris.end(); ++it )
	{
		MYDEBUG<<*it<<endl;
		tmp_url = KURL(*it);
		if(tmp_url.protocol() == QString::fromLatin1("file"))
		{
			info = new QFileInfo(tmp_url.path());
			addImageToDB(info, false, true);
		}
	}
	flush();

	disconnect(SIGNAL(sigHasSeenFile(int)), mw);

	///////////////////////////////////////////////////////////////////////////

	mw->setMessage(i18n("Setting note for files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(uris.count());

	connect(cdb, SIGNAL(sigLinkAdded()),
		mw, SLOT(slotPreviewDone()));

	cdb->addNote(uris, note);

	cdb->disconnect(SIGNAL(sigLinkAdded()), mw);

	///////////////////////////////////////////////////////////////////////////

	mw->setMessage(i18n("Ready"));
	mw->slotDone(uris.count());
	mw->restoreNumberOfImages();
	mw->setEnabled(true);
}



void
CategoryDBManager::addImagesToCategory(int /*image_id*/, QStringList* /*cat_list*/)
{
// 	MYDEBUG<<image_id<<" "<<*cat_list<<endl;
}
void
CategoryDBManager::addImagesToCategory(const QString& /*fullname*/, QStringList* /*cat_list*/)
{
// 	MYDEBUG<<fullname<<" "<<*cat_list<<endl;
}

bool
CategoryDBManager::renameCategory(int cat_id, const QString& newName, QString& msg)
{
	return cdb->renameCategory(cat_id, newName, msg);
}
bool
CategoryDBManager::setCategoryDescription(int cat_id, const QString& descr, QString& msg)
{
	return cdb->setCategoryDescription(cat_id, descr, msg);
}
bool
CategoryDBManager::setCategoryIcon(int cat_id, const QString& icon, QString& msg)
{
	return cdb->setCategoryIcon(cat_id, icon, msg);
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
ImageEntry*
CategoryDBManager::getImageEntry(const QString& fullname)
{
	return cdb->getImageEntry(fullname);
}
QPtrList<ImageEntry>
CategoryDBManager::getImageEntries(const QStringList& image_path_list)
{
	return cdb->getImageEntries(image_path_list);
}

int
CategoryDBManager::addImageToDB(QFileInfo *info, bool forceFlush, bool check)
{
	if(catdbM_priv->getNumberOfLeftItems() % 20 == 0)
	{
// 		emit numberOfLeftItems(catdbM_priv->getNumberOfLeftItems());
// 		emit sigHasSeenFile(20);
	}
	emit sigHasSeenFile(1);

	int image_id=-1;
	if(!info->exists()) return -2;
	if(check)
	{
		int dir_id = getDirectoryId(info->dirPath());
		image_id = cdb->getImageId(info->fileName(), dir_id);
		if( image_id > 0) { /*MYDEBUG<<"Image already exists "<<info->fileName()<<endl;*/ return image_id;}
	}

	KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, KURL(info->absFilePath())/*iconitem->getURL()*/, true );
	KFileMetaInfo metaInfo(fileItem.metaInfo());

	QString comment = QString::null;
	QString sdatetime = QString::null;
	QDateTime datetime;
	if(metaInfo.isValid())
	{
		comment = metaInfo.item("Comment").string();
		if(comment==QString::fromLatin1("---")) comment = QString::null;

		sdatetime = metaInfo.item("Date/time").string(true).stripWhiteSpace();
// 		MYDEBUG<<sdatetime<<endl;
		if(sdatetime!="---")
		{
			datetime = QDateTime(
					KGlobal::locale()->readDate(metaInfo.item( "CreationDate" ).string( true ).stripWhiteSpace()),
					KGlobal::locale()->readTime(metaInfo.item( "CreationTime" ).string( true ).stripWhiteSpace()));
// 			MYDEBUG<<datetime<<endl;
		}
		if(!datetime.isValid ()) datetime = info->lastModified();
// 		MYDEBUG<<datetime<<endl;
	}
	else
	{
		datetime = info->lastModified();
	}
	cdb->addImage(info->fileName(), info->dirPath(), datetime, comment, false/*don't check because already done*/);
	if(forceFlush) flush(false/*don't check because already done*/);
	return 0;
}


void
CategoryDBManager::__startAddingImages__()
{
	m_isAddingFiles = true;
	emit isAddingFiles(m_isAddingFiles);
}


void
CategoryDBManager::flush(bool check)
{
	m_isAddingFiles = false;
	cdb->flushImages(check);
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

int
CategoryDBManager::getNumberOfImages()
{
	return cdb->getNumberOfImages();
}

int
CategoryDBManager::removeObsololeteFilesOfTheDatabase()
{
	if(!cdb->isConnected()) return -1;

	KProgressDialog *progres =  new KProgressDialog (mw, "remove Obsololet Files Of The Database",
			i18n("Checking for obsololete files in the database..."),
			QString::null,
			true);
	progres->show();
	progres->setLabel(i18n("Checking for the number of files..."));
	progres->adjustSize();
	kapp->processEvents();

	KApplication::setOverrideCursor (waitCursor);
	QPtrVector<QString> filePaths = cdb->getAllImageFullPath();
	KApplication::restoreOverrideCursor();

	if(filePaths.isEmpty()) return 0;
	progres->progressBar()->setTotalSteps(filePaths.size());

	QStringList ima_id_list;
	QStringList ima_path_list;
	QString *path;
	QFileInfo *info = new QFileInfo();;
	QDateTime date_last=QDateTime::currentDateTime ();
	for (unsigned int i=0; i<filePaths.size(); i++)
	{
		if(date_last.time().msecsTo(QDateTime::currentDateTime().time()) >= 500)
		{
			date_last=QDateTime::currentDateTime ();
			progres->progressBar()->setProgress(i);
			progres->setLabel(i18n("Checking file %1 (%2 left)").arg(i).arg(filePaths.size()));
			kapp->processEvents();
		}
		if ( progres->wasCancelled() ) break;

		//----------------------------
		if(!filePaths.at(i)) continue;
		path = filePaths.at(i);
		info->setFile(*path);
		if(!info->exists())
		{
			ima_id_list.append(QString::number(i));
			ima_path_list.append(*path);
		}
	}
	if ( progres->wasCancelled() ) {delete(progres); delete(info);return -1;};
	progres->progressBar()->setProgress(filePaths.size());

	int ret=-1;
	if(ima_id_list.count()>0)
	{
		if(KMessageBox::warningYesNoList(mw,
								i18n("Do you really want to remove %1 entries in the database?").arg(ima_id_list.count()),
								ima_path_list,
								i18n("Remove entries into database")) == KMessageBox::Yes)
		{
			KApplication::setOverrideCursor (waitCursor);
			int num = cdb->deleteImage(ima_id_list);
			KApplication::restoreOverrideCursor();
			if(num>0)
				ret = ima_id_list.count();
			else
				ret = -1;
		}
		else
			ret = 0;
	}
	else
		ret = 0;

	delete(progres);
	delete(info);
	return ret;
}

int
CategoryDBManager::addCurrentCategories(int cat_id)
{
	QString scat_id = QString::number(cat_id);
	if(catid_list.contains(scat_id)) return 0;
	catid_list.append(scat_id);
	return refreshRequest_private();
}

int
CategoryDBManager::delCurrentCategories(int cat_id)
{
	QString scat_id = QString::number(cat_id);
	catid_list.remove(scat_id);
	return refreshRequest_private();
}

int
CategoryDBManager::addCurrentDate(QDateTime datetimeb, QDateTime datetimee)
{
	m_datetime_begin = datetimeb;
	m_datetime_end = datetimee;
	return refreshRequest_private();
}

int
CategoryDBManager::delCurrentDate(QDateTime /*datetimeb*/, QDateTime /*datetimee*/)
{
	m_datetime_begin = QDateTime();
	m_datetime_end =  QDateTime();
	return refreshRequest_private();
}


int
CategoryDBManager::addCurrentPattern(const QString& pattern)
{
	patternList.append(pattern);
	return refreshRequest_private();
}


int
CategoryDBManager::delCurrentPattern(const QString& pattern)
{
	patternList.remove(pattern);
	return refreshRequest_private();
}

int
CategoryDBManager::addCurrentNote(const QString& note)
{
	noteList.append(note);
	return refreshRequest_private();
}


int
CategoryDBManager::delCurrentNote(const QString& note)
{
	noteList.remove(note);
	return refreshRequest_private();
}


int
CategoryDBManager::setSelectionMode(CategoryDBManager::SelectionMode mode)
{
	m_selectionmode = mode;
	return refreshRequest_private();
}



//------------------------------------------------------------------------------

bool
CategoryDBManager::updateImageInformations(int image_id,
	const QString& comment, int note,
	const QDateTime& date_begin, const QDateTime& date_end,
	const QStringList& removedCategories,  const QStringList& addedCategories)
{
	cdb->updateImageInformations(image_id, comment, note, date_begin, date_end, removedCategories, addedCategories) ;
	return  true;
}

bool
CategoryDBManager::updateImageInformations(QPtrList<ImageEntry>& image_id_list,
	const QString& comment, int note,
	const QDateTime& date_begin, const QDateTime& date_end,
	const QStringList& removedCategories,  const QStringList& addedCategories)
{
	mw->setMessage(i18n("Setting category for files in database..."));
	mw->saveNumberOfImages();
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(image_id_list.count());
	connect(cdb, SIGNAL(sigLinkAdded()),
		mw, SLOT(slotPreviewDone()));

	cdb->updateImageInformations(image_id_list, comment, note, date_begin, date_end, removedCategories, addedCategories) ;

	cdb->disconnect(SIGNAL(sigLinkAdded()), mw);
	mw->slotDone(image_id_list.count());
	mw->restoreNumberOfImages();
	mw->setMessage(i18n("Ready"));

	return  true;
}

bool
CategoryDBManager::addImageInformations(const QStringList& image_path_list,
	const QString& comment, int note,
	const QDateTime& date_begin, const QDateTime& date_end,
	const QStringList& addedCategories)
{
	if(addedCategories.isEmpty())
		return true;


	mw->saveNumberOfImages();
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	mw->setMessage(i18n("Adding files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(image_path_list.count());

	connect(this, SIGNAL(sigHasSeenFile(int)),
		mw, SLOT(slotPreviewDone(int)));
	QFileInfo *info=NULL;
	for ( QStringList::ConstIterator it = image_path_list.begin(); it != image_path_list.end(); ++it )
	{
		info = new QFileInfo(*it);
		addImageToDB(info);
	}
	flush(false /*image has already been checked */);
	disconnect(SIGNAL(sigHasSeenFile(int)), mw);
	///////////////////////////////////////////////////////////////////////////
	mw->setMessage(i18n("Setting category for files in database..."));
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(image_path_list.count());

	connect(cdb, SIGNAL(sigLinkAdded()),
		mw, SLOT(slotPreviewDone()));

	QStringList removedCategories;
	QPtrList<ImageEntry> image_id_list = cdb->getImageEntries(image_path_list);
	cdb->updateImageInformations(image_id_list,
		comment, note,
		date_begin, date_end,
		removedCategories, addedCategories) ;

	cdb->disconnect(SIGNAL(sigLinkAdded()), mw);

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	mw->slotDone(image_path_list.count());
	mw->restoreNumberOfImages();
	mw->setMessage(i18n("Ready"));
	return  true;
}

//------------------------------------------------------------------------------

bool
CategoryDBManager::moveImages(const KURL::List& fileurls, const KURL& desturl)
{
// 	MYDEBUG <<endl;
	if(!cdb->isConnected()) return false;

	if(fileurls.count()>5) mw->setEnabled(false);
	mw->setMessage(i18n("Moving files in database..."));
	KURL::List list = fileurls;
	mw->saveNumberOfImages();
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(fileurls.count());
	connect(cdb, SIGNAL(sigFileMoved()),
			mw, SLOT(slotPreviewDone()));

	for(KURL::List::iterator it=list.begin(); it != list.end(); ++it)
	{
// 		MYDEBUG << (*it).path()<<"-"<< desturl.path(-1)<<endl;
		if(!ListItemView::isImage((*it).path()))
			moveDirectory((*it).path(), desturl.path(-1));
		else
			cdb->moveImage((*it).path(), desturl.path(-1));
	}
	cdb->disconnect(SIGNAL(sigFileMoved()), mw);
	mw->slotDone(fileurls.count());
	mw->restoreNumberOfImages();
	mw->setMessage(i18n("Ready"));
	mw->setEnabled(true);

	return  true;
}

bool
CategoryDBManager::renameImage(QDict<QString>& renamedFiles)
{
	if(!cdb->isConnected()) return false;

	mw->setEnabled(false);
	mw->setMessage(i18n("Renaming files in database..."));
	mw->saveNumberOfImages();
	mw->slotRemoveImage(mw->getTotal());
	mw->getCategoryView()->setTotalNumberOfFiles(renamedFiles.count());
	connect(cdb, SIGNAL(sigFileRenamed()),
			mw, SLOT(slotPreviewDone()));

	QDictIterator<QString> newname_it( renamedFiles );
	for( ; newname_it.current(); ++newname_it )
	{
		QString oldname = newname_it.currentKey();
		QString newname = *(newname_it.current());
// 		MYDEBUG<<oldname<<" "<<newname<<endl;
		renameImage(oldname, newname);
	}

	cdb->disconnect(SIGNAL(sigFileRenamed()), mw);
	mw->slotDone(renamedFiles.count());
	mw->restoreNumberOfImages();
	mw->setMessage(i18n("Ready"));
	mw->setEnabled(true);

	return true;
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
// 	MYDEBUG << srcurl.path(-1)<<"-"<< desturl.path(-1)<<endl;
	cdb->renameDirectory(srcurl.path(-1), desturl.path(-1)) ;
	return  true;
}

bool
CategoryDBManager::moveDirectory(const KURL& srcurl, const KURL& desturl)
{
// 	MYDEBUG<<srcurl<<" " <<desturl<<endl;
	cdb->moveDirectory(srcurl.path(-1), desturl.path(-1)) ;
	return true;
}

QStringList*
CategoryDBManager::getCategoryNameListImage(int image_id) const
{
	if(!isEnabled()) return new QStringList();
	////
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
// 	MYDEBUG<<ima_path<<": "<<dir_id<<" "<<image_id<<endl;
	return getCategoryNameListImage(image_id) ;
}

QStringList*
CategoryDBManager::getCategoryIdListImage(const QStringList& image_id_list, bool distinct) const
{
	if(!cdb->isConnected()) return NULL;

	if(m_isAddingFiles)
	{
		MYDEBUG<<"I'm adding files..."<<endl;
		return new QStringList("(Updating database...)");
	}
	return cdb->getCategoryIdListImage(image_id_list, distinct) ;
}
QStringList*
CategoryDBManager::getCategoryIdListImage(const QString& ima_path) const
{
	if(m_isAddingFiles)
	{
		MYDEBUG<<"I'm adding files..."<<endl;
		return new QStringList("(Updating database...)");
	}

	QFileInfo info(ima_path);
	int dir_id = getDirectoryId(info.dirPath());
	int image_id = cdb->getImageId(info.fileName(), dir_id) ;
// 	MYDEBUG<<ima_path<<": "<<dir_id<<" "<<image_id<<endl;
	return getCategoryIdListImage(image_id) ;
}
QStringList*
CategoryDBManager::getCategoryIdListImage(int image_id) const
{
	if(m_isAddingFiles)
	{
		MYDEBUG<<"I'm adding files..."<<endl;
		return new QStringList("(Updating database...)");
	}
	return cdb->getCategoryIdListImage(image_id) ;
}

bool
CategoryDBManager::isEnabled() const
{
	return m_isEnabled;
}
void
CategoryDBManager::setEnabled(bool enable)
{
	m_isEnabled = enable;
}


void
CategoryDBManager::slotLinkAdded()
{
// 	MYDEBUG<<endl;
	emit sigLinkAdded(); kapp->processEvents();
}
void
CategoryDBManager::slotAddLinksStarted(int nbr)
{
	emit sigAddLinksStarted(nbr); kapp->processEvents();
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
