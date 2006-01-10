/***************************************************************************
                         categoriesdb.cpp  -  description
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

// Local
#include "categoriesdb.h"

#include "categories.h"

// QT
#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qvariant.h>
#include <qdict.h>

// KDE
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

CategoriesDB::CategoriesDB(const QString& type, const QString& sqlitePath, const QString& mysqlUsername, const QString& mysqlPassword, const QString& mysqlHostname)
{
//  	MYDEBUG<<"Categories Creation"<<endl;
	this->c = new Categories(type, sqlitePath, mysqlUsername, mysqlPassword, mysqlHostname);
//  	MYDEBUG<<"\tendCategories Creation"<<endl;

	tabCategoryNode.resize(1024*2);
	tabCategoryNode.setAutoDelete(true);

	pathsDict = new QDict<QVariant>;

	cachedImageToAdd = new QPtrList <QPtrList <QVariant> >;
	cachedImageToAdd->setAutoDelete(true);

	setUseCache(true);

	constructCategories();

	connect(c, SIGNAL(sigLinkAdded()),
		this, SLOT(slotLinkAdded()));
}

CategoriesDB::~CategoriesDB()
{
//  	MYDEBUG<<endl;
	delete(c);
}

bool
CategoriesDB::isConnected() const
{
	return c->isConnected();
}

CategoryNode*
CategoriesDB::addTopCategory(const QString& title)
{
	if(!isConnected()) return NULL;

	int id = c->addTopCategory(title);
	if( id > 0)
	{
		CategoryNode *node = new CategoryNode(id, QString(title));

		tabCategoryNode.insert(node->getId(), node);
		rootCat.append(node);

		return node;
	}
	else
	{
		kdWarning() << "Erreur" << endl;
		return NULL;
	}
}

CategoryNode*
CategoriesDB::addSubCategory(int parent, const QString& title, const QString& desc, QString &msg)
{
	if(!isConnected()) return NULL;

	if(parent == 0)
	{
		return addTopCategory(title);
	}

	//
	int id = c->addSubCategory(parent, title, desc);
	if( id > 0)
	{
		CategoryNode *node_par = tabCategoryNode[parent];
		CategoryNode *node_chi = new CategoryNode(id, title, QString());
		node_par->addChildCategory(node_chi);

		tabCategoryNode.insert(node_chi->getId(), node_chi);
		return node_chi;
	}
	else
	{
		kdWarning() << "Erreur" << endl;
		msg=i18n("Unable to insert sub-category '%1'.").arg(title);
		return NULL;
	}
}


void
CategoriesDB::constructCategories()
{
// 	MYDEBUG<<endl;
	if(!isConnected()) return ;

	QStringList *l = c->topCategories();
	if(!l)
	{
		kdWarning()<<"No top category found!"<<endl;
		return;
	}
	for(QStringList::Iterator it = l->begin(); it != l->end(); ++it)
	{
		int cat_id = c->getCategoryId(*it);
		CategoryNode *node = new CategoryNode(cat_id, *it,  c->getCategoryDescription(cat_id), c->getCategoryIcon(cat_id));
		rootCat.append(node);
		tabCategoryNode.insert(cat_id, node);

		constructCategories(node, *it);
	}
// 	MYDEBUG<<"\t"<<endl;
}


void
CategoriesDB::constructCategories(CategoryNode *root, const QString& cat_name)
{
//	MYDEBUG<<endl;
	if(!isConnected()) return ;

	QStringList *l = c->subCategories(cat_name);
	if(!l) return;
	for(QStringList::Iterator it = l->begin(); it != l->end(); ++it)
	{
		int cat_id = c->getCategoryId(*it);
		CategoryNode *node = new CategoryNode(cat_id, *it,  c->getCategoryDescription(cat_id), c->getCategoryIcon(cat_id));
		root->addChildCategory(node);
		tabCategoryNode.insert(cat_id, node);

		constructCategories(node, *it);
	}
// 	MYDEBUG<<"\t"<<endl;
}



void
CategoriesDB::printImageEntry(QPtrList<ImageEntry>& imageEntryList)
{
	QString s("\n");
	ImageEntry *image;
	for(image = imageEntryList.first();image;image=imageEntryList.next())
	{
		s+= image->toString() + "\n";
		CategoryNode *node;
		QPtrList<CategoryNode> list = getCategoryListImage(image->getId());
		for(node = list.first();node; node = list.next())
			s+= "\t" + node->toString()+"\n";
		//s+="\n";
	}
	MYDEBUG<<s<<endl;
}

void
CategoriesDB::printCategories()
{
	QString s("\n");
	CategoryNode *node;
	for(node = rootCat.first(); node; node=rootCat.next())
	{
		s+=printCategories(node, 0);
	}
	MYDEBUG<<s<<endl;
}

QString
CategoriesDB::printCategories(CategoryNode *root, int s)
{
	QString sp;
	for(int j = 0;j < s;j++) sp+= ' ';
	sp+= "(" + QString::number(root->getId()) + ") -" + root->getTitle() + "-"+root->getDescription()+"\n";

	QPtrList<CategoryNode> list=root->getChildCategoryList();
	CategoryNode *cat;
	for(cat = list.first();cat;cat=list.next())
	{
		sp += printCategories(cat, s + 2);
	}
	return sp;
}

void
CategoriesDB::printSubCategories(const QString& cat_name)
{
	if(!isConnected()) return ;

	printSubCategories(c->getCategoryId(cat_name));
}

void
CategoriesDB::printSubCategories(int categoryId)
{
	CategoryNode *root=getCategoryNode(categoryId);
	if(!root) return;
	kdDebug(0) << root->getId() << "--" << root->getTitle() << endl;
	QPtrList<CategoryNode> list=root->getSubCategoryList();
	CategoryNode *cat;
	for(cat = list.first();cat;cat=list.next())
	{
		kdDebug(0) << cat->getId() << "--" << cat->getTitle() << endl;
	}
}

QPtrList<CategoryNode>
CategoriesDB::getSubCategories(int categoryId)
{
	//MYDEBUG<<endl;
	CategoryNode *root=getCategoryNode(categoryId);
	if (root)
	{
		//MYDEBUG<<endl;
		return root->getSubCategoryList();
	}
	else
	{
		QPtrList<CategoryNode> emptyList;
		return emptyList;
	}
}

QPtrList<CategoryNode>
CategoriesDB::getSubCategories(const QString& cat_name)
{
	if(!isConnected()) return QPtrList<CategoryNode>();

	return getSubCategories(c->getCategoryId(cat_name));
}


CategoryNode*
CategoriesDB::getCategoryNode(int categoryId) const
{
	if(categoryId>=0 && categoryId<=(int)tabCategoryNode.size())
	{
		CategoryNode* cat = tabCategoryNode[categoryId];
		return cat;
	}
	else
		return NULL;
}

CategoryNode*
CategoriesDB::getCategoryNode(const QString& cat_name) const
{
	return getCategoryNode(getCategoryId(cat_name));
}

QPtrList<CategoryNode>
CategoriesDB::getCategoryListImage(int image_id) const
{
	if(!isConnected()) return QPtrList<CategoryNode>();

	QPtrList<CategoryNode> list;
	QStringList *l = c->imageLinks(image_id);
	if(!l)return list;

	CategoryNode *node;
	for(QStringList::Iterator it = l->begin(); it != l->end(); ++it)
	{
		node = getCategoryNode((*it).toInt());
		if (node) list.append(node);
	}
	return list;
}

QStringList*
CategoriesDB::getCategoryNameListImage(int image_id) const
{
	if(!isConnected()) return NULL;

	return c->imageLinks(image_id, true);
}

QStringList*
CategoriesDB::getCategoryIdListImage(int image_id) const
{
	if(!isConnected()) return NULL ;

	return c->imageLinks(image_id, false);
}

QStringList*
CategoriesDB::getCategoryIdListImage(const QStringList& image_id_list, bool distinct) const
{
	if(!isConnected()) return NULL;

	return c->imageLinks(image_id_list, false, distinct);
}

QStringList*
CategoriesDB::getCategoryNameListImage(const QString& ima_path) const
{
	return c->imageLinks(c->getImageId(ima_path), true);
}


/**
 *
 * @param name
 * @param path
 * @param date
 * @param comment
 */
void
CategoriesDB::addImage(const QString& name, const QString& path, const QDateTime& date, const QString& comment, bool check)
{
// 	MYDEBUG<<name<<" " <<path<<" "<<check<<endl;
	QPtrList <QVariant> *list = new QPtrList <QVariant>; list->setAutoDelete(true);
	list->append(new QVariant(name));
	list->append(new QVariant(addDirectory(path)));
	list->append(new QVariant(date));
	list->append(new QVariant(comment));
// 	MYDEBUG<<"=========Adding in cache "<<*list->at(0)<< " " << *list->at(1)<<endl;
	cachedImageToAdd->append(list);

// 	MYDEBUG<<cachedImageToAdd->count()<<endl;
	if(cachedImageToAdd->count()>=20)
	{
// 		MYDEBUG<<"I'll flush..."<<check<<endl;
		flushImages(check);
	}
}
void
CategoriesDB::flushImages(bool check)
{
//  	MYDEBUG<<check<<endl;
	if(!isConnected()) return ;

	c->addImages(cachedImageToAdd, check);
	cachedImageToAdd->clear();
}

/**
 *
 * @param name
 * @param dir_id
 */
int
CategoriesDB::addImage(const QString& name, int dir_id)
{
	if(!isConnected()) return -1 ;

	return c->addImage(name, dir_id, QDateTime::currentDateTime ());
}

int
CategoriesDB::getDirectoryId(const QString& dir_path)
{
	if(!useCache())
		return c->getDirectoryId(dir_path);

	QVariant *val = (*pathsDict)[dir_path];
	if(val)
		return val->toInt();
	else
	{
		int dir_id = c->getDirectoryId(dir_path);
// 		if(dir_id<0) {MYDEBUG<<"Dir entry does not exist: "<<dir_path<<endl;}
// 		dir_id = addDirectory(dir_path);
		if(dir_id>0) pathsDict->insert(dir_path, new QVariant(dir_id));
// 		MYDEBUG<<dir_path<<": "<<dir_id<<endl;
		return dir_id;
	}
}


int
CategoriesDB::addDirectory(const QString& path)
{
// 	MYDEBUG<<path<<endl;
	if(!useCache())
		return c->addDirectory(path);

// 	MYDEBUG<<endl;
	QVariant *val = pathsDict->find(path);
	if(val>0)
	{
// 		MYDEBUG<<endl;
		return val->toInt();
	}
	else
	{
// 		MYDEBUG<<endl;
		int dir_id = c->addDirectory(path);
		if(dir_id>0) pathsDict->insert(path, new QVariant(dir_id));
		return dir_id;
	}
}


void
CategoriesDB::addLink(const QString& image, const QString& path, const QString& catname)
{
	c->addLink(image, path, catname);
}

void
CategoriesDB::addLink(int image_id, int cat_id)
{
	c->addLink(image_id, cat_id);
}

void
CategoriesDB::addLink(const QStringList& uris, int cat_id)
{
	if(!isConnected()) return ;

	MYDEBUG << "BEGIN for catid="<< cat_id << endl;
	QStringList m_list=uris;
	QStringList *imgid_list;
	QStringList img_list;
	KURL tmp_url;
	for ( QStringList::ConstIterator it = m_list.begin(); it != m_list.end(); ++it )
	{
		tmp_url = KURL(*it);
		img_list.append(tmp_url.path());
	}
	imgid_list = getImageListId(img_list);

	c->addLink(*imgid_list, cat_id);
	delete(imgid_list);
	MYDEBUG << "END! " << endl;
}


void
CategoriesDB::addNote(const QStringList& uris, int note)
{
	if(!isConnected()) return ;

	MYDEBUG << "BEGIN for note="<< note << endl;
	QStringList m_list=uris;
	QStringList *imgid_list;
	QStringList img_list;
	KURL tmp_url;
	for ( QStringList::ConstIterator it = m_list.begin(); it != m_list.end(); ++it )
	{
		tmp_url = KURL(*it);
		img_list.append(tmp_url.path());
	}
	imgid_list = getImageListId(img_list);

	c->setImageNote(*imgid_list, note);
	delete(imgid_list);
	MYDEBUG << "END! " << endl;
}


QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(const QStringList& catid_list, SelectionMode mode)
{
// 	MYDEBUG<<endl;
	QPtrList<ImageEntry> imageEntryList;
	if(catid_list.count()<=0) return imageEntryList;

	QPtrList<QStringList> l;
	QStringList m_catid_list = catid_list;
	QStringList *tab;
	QPtrList<CategoryNode> list;
	CategoryNode *cat;
	for(QStringList::iterator it = m_catid_list.begin(); it!=m_catid_list.end(); ++it)
	{
		tab = new QStringList();
		tab->append(*it);

		list = getSubCategories((*it).toInt());

		for(cat = list.first();cat;cat=list.next())
			tab->append(QString::number(cat->getId()));
		l.append(tab);
// 		MYDEBUG<<*tab<<endl;
	}
	//
	KexiDB::Cursor *result;
	if(mode==mode_OR)
	{
//  		MYDEBUG<<"mode==mode_OR"<<endl;
		result = c->imagesCategoriesList_OR(l);
	}
	else
	{
//  		MYDEBUG<<"BEGIN mode==mode_AND"<<endl;
		result = c->imagesCategoriesList_AND(l);
//  		MYDEBUG<<"END mode==mode_AND"<<endl;
	}
	imageEntryList = imageCursor2PtrList(result);
	c->freeCursor(result);
	return imageEntryList;
}


QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(const QString& cat_name)
{
// 	MYDEBUG<<endl;
	int id = c->getCategoryId(cat_name);
	return imagesSubCategoriesList(id);
}


QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(int categoryId)
{
// 	MYDEBUG<<endl;
	QPtrList<ImageEntry> imageEntryList;

	if(categoryId<0) return imageEntryList;

	QStringList l;
	QPtrList<CategoryNode> list = getSubCategories(categoryId);
	QStringList tab;
	tab.append(QString::number(categoryId));
	CategoryNode *cat;
	for(cat = list.first();cat;cat=list.next())
		tab.append(QString::number(cat->getId()));
	//
	KexiDB::Cursor *result = c->imagesCategoriesList(tab);
	imageEntryList = imageCursor2PtrList(result);

	c->freeCursor(result);
// 	MYDEBUG<<endl;
	return imageEntryList;
}

QPtrList<ImageEntry>
CategoriesDB::imagesNoteList(int note, int lem)
{
	if(!isConnected()) return QPtrList<ImageEntry>();

	KexiDB::Cursor *result = c->imagesNoteList(note, lem);
	QPtrList<ImageEntry> imageEntryList = imageCursor2PtrList(result);

	c->freeCursor(result);

	return imageEntryList;
}

QPtrList<ImageEntry>
CategoriesDB::imagesNoteList(const QStringList& noteList, const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	KexiDB::Cursor *result = c->imagesNoteList(noteList,
						iiList, mode==mode_OR?Categories::mode_OR:Categories::mode_AND);
	QPtrList<ImageEntry> imageEntryList = imageCursor2PtrList(result);
	c->freeCursor(result);
	return imageEntryList;
}



QPtrList<ImageEntry>
CategoriesDB::imageCursor2PtrList(KexiDB::Cursor *cursor)
{
// 	MYDEBUG<<endl;
	QPtrList<ImageEntry> list;
	if(!cursor) return list;
	cursor->moveFirst();
	while(!cursor->eof())
	{
		//image_id, image_name, image_directory_id, image_comment, image_note, image_date_begin, image_date_end
		int id = cursor->value(0).toInt();
		QString name(cursor->value(1).toString());
		int directory_id = cursor->value(2).toInt();
		QString comment(cursor->value(3).toString());
		int note = cursor->value(4).toInt();
		QDateTime date_begin = QDateTime::fromString(cursor->value(5).toString(), Qt::ISODate);
		QDateTime date_end = QDateTime::fromString(cursor->value(6).toString(), Qt::ISODate);
		//FIXME add method getDirectoryPath
		list.append(new ImageEntry(id, c->getDirectoryPath(directory_id)+"/"+name, directory_id, comment, note, date_begin, date_end));

		cursor->moveNext();
	}
	return list;
}

bool
CategoriesDB::updateImageInformations(int image_id, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories)
{
	if(!isConnected()) return false;

	c->updateImageInformations(image_id, comment, note, date_begin, date_end, removedCategories, addedCategories) ;
	return true;
}

bool
CategoriesDB::updateImageInformations(QPtrList<ImageEntry>& image_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories)
{
	if(!isConnected()) return false;

	QStringList image_id_list;
	for(ImageEntry *image = image_list.first(); image; image=image_list.next())
		image_id_list.append(QString::number(image->getId()));
	c->updateImageInformations(image_id_list, comment, note, date_begin, date_end, removedCategories, addedCategories) ;
	return true;
}


void
CategoriesDB::moveImage(const QString& old_fullpath, const QString& newpath)
{
// 	MYDEBUG<<endl;
	c->moveImage(old_fullpath, addDirectory(newpath));
	emit sigFileMoved();
}


void
CategoriesDB::renameImage(int id, const QString& new_name)
{
	c->renameImage(id, new_name);
	emit sigFileRenamed();
}

void
CategoriesDB::renameImage(const QString& old_name, const QString& new_name)
{
	c->renameImage(old_name, new_name);
	emit sigFileRenamed();
}

int
CategoriesDB::renameDirectory(const QString& old_path, const QString& new_path)
{
	return c->renameDirectory(old_path, new_path);
}

int
CategoriesDB::moveDirectory(const QString& old_path, const QString& new_path)
{
	QFileInfo old_info(old_path);
	return c->moveDirectory(old_info.dirPath(), old_info.fileName(), new_path);
}


bool
CategoriesDB::renameCategory(int id, const QString& new_name, QString& msg)
{
	CategoryNode *node = getCategoryNode(id);
	if(!node) return false;

	if(c->renameCategory(id, new_name))
	{
		node->setTitle(new_name);
		return true;
	}
	else
	{
		msg=i18n("Unable to rename category '%1'.").arg(c->getCategoryName(id));
		return false;
	}
}

bool
CategoriesDB::setCategoryDescription(int id, const QString& descr, QString& msg)
{
	CategoryNode *node = getCategoryNode(id);
	if(!node) return false;

	if(c->setCategoryDescription(id, descr))
	{
		node->setDescription(descr);
		return true;
	}
	else
	{
		msg=i18n("Unable to set category description for '%1'.").arg(c->getCategoryName(id));
		return false;
	}
}

bool
CategoriesDB::setCategoryIcon(int id, const QString& icon, QString& msg)
{
	CategoryNode *node = getCategoryNode(id);
	if(!node) return false;

	if(c->setCategoryIcon(id, icon))
	{
		node->setIcon(icon);
		return true;
	}
	else
	{
		msg=i18n("Unable to set category icon for '%1'.").arg(c->getCategoryName(id));
		return false;
	}
}



void
CategoriesDB::deleteNodeCategory(int id)
{
	CategoryNode *node = getCategoryNode(id);
	if(node)
	{
		if(!node->hasChildClasses())
		{
			if( c->deleteNodeCategory(id) == 0 )
			{
				tabCategoryNode.remove( id );
				if ( rootCat.findRef( node ) != -1 )
    				        rootCat.removeRef(node);

			}
		}
	}
}

void
CategoriesDB::moveCategory(int id, int new_parent)
{
// 	MYDEBUG<<endl;
	CategoryNode *curnode = getCategoryNode(id);
	CategoryNode *node = NULL;
	if(curnode) node = new CategoryNode(*curnode);
	CategoryNode *parent_node = getCategoryNode(new_parent);
	if(!(node&&parent_node)) return;
	int res= c->setNewParent(id, new_parent);
	if( res == 0)
	{
		tabCategoryNode.remove( id );
		parent_node->addChildCategory(node);
	}
}


void
CategoriesDB::deleteImage(int ima_id)
{
	c->deleteImage(ima_id);
}

int
CategoriesDB::deleteImage(const QStringList& ima_id_list)
{
	return c->deleteImage(ima_id_list);
}

void
CategoriesDB::deleteCategoryImage(int ima_id, int cat_id)
{
	c->deleteCategoryImage(ima_id, cat_id);
}


void
CategoriesDB::setImageComment(int id, const QString& comment)
{
	c->setImageComment(id, comment);
}

void
CategoriesDB::setImageNote(int id, int note)
{
	c->setImageNote(id, note);
}

void
CategoriesDB::setImageDate(int id, const QDateTime& begin, const QDateTime& end)
{
	c->setImageDate(id, begin, end.isValid() ? end : begin);
}

QPtrList<ImageEntry>
CategoriesDB::imagesDateList(const QDate& date, int bia,
			const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	KexiDB::Cursor *result = c->imagesDateList(date, bia,
						iiList, mode==mode_OR?Categories::mode_OR:Categories::mode_AND);
	QPtrList<ImageEntry> imageEntryList = imageCursor2PtrList(result);
	c->freeCursor(result);
	return imageEntryList;
}

QPtrList<ImageEntry>
CategoriesDB::imagesDateList(const QDate& date_begin, const QDate& date_end,
							 const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	KexiDB::Cursor *result = c->imagesDateList(date_begin, date_end,
											   iiList, mode==mode_OR?Categories::mode_OR:Categories::mode_AND);
	QPtrList<ImageEntry> list = imageCursor2PtrList(result);
	c->freeCursor(result);
	return list;
}

/*
QPtrList<ImageEntry>
CategoriesDB::imagesPatternList(const QString& pattern,
					const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	QStringList list;
	list.append(pattern);
	return imagesPatternList(list, iiList, mode);
}
*/

QPtrList<ImageEntry>
CategoriesDB::imagesPatternList(const QStringList& patterns,
				const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
// 	MYDEBUG<<mode<<endl;
	KexiDB::Cursor *result = c->imagesPatternList(patterns,
							iiList, mode==mode_OR?Categories::mode_OR:Categories::mode_AND);
	QPtrList<ImageEntry> list = imageCursor2PtrList(result);
	c->freeCursor(result);
	return list;
}

QPtrList<ImageEntry>
CategoriesDB::imagesCommentList(const QString& comment)
{
	KexiDB::Cursor *result  = c->imagesCommentList(comment);
	QPtrList<ImageEntry> list = imageCursor2PtrList(result);
	c->freeCursor(result);
	return list;
}

QPtrList<ImageEntry>
CategoriesDB::allImages()
{
	KexiDB::Cursor *result  = c->allImages();
	QPtrList<ImageEntry> list = imageCursor2PtrList(result);
	c->freeCursor(result);
	return list;
}


QPtrList<CategoryNode>
CategoriesDB::getRootCategories()
{
	return rootCat;
}


int
CategoriesDB::getCategoryId(const QString& cat_name) const
{
	return c->getCategoryId(cat_name);
}

int
CategoriesDB::getImageId(const QString& name, int dir_id)
{
	return c->getImageId(name, dir_id);
}

int
CategoriesDB::getImageId(const QString& image_path)
{
	QFileInfo info(image_path);
	return getImageId(info.fileName(), getDirectoryId(info.dirPath()));
}

QStringList*
CategoriesDB::getImageListId(const QStringList& img_list)
{
	if(img_list.count()>1)
		return c->getImageListId(img_list);
	else
		return new QStringList(QString::number(getImageId(img_list.first())));
}

ImageEntry*
CategoriesDB::getImageEntry(const QString& image_path)
{
	QPtrList<ImageEntry> imageEntryList;
	QFileInfo info(image_path);
	KexiDB::Cursor *result = c->getImageEntry(info.fileName(), getDirectoryId(info.dirPath()));
	imageEntryList = imageCursor2PtrList(result);
	c->freeCursor(result);
	return imageEntryList.first();
}

QPtrList<ImageEntry>
CategoriesDB::getImageEntries(const QStringList& image_path_list)
{
	QStringList *image_id_list = getImageListId(image_path_list);
// 	MYDEBUG<<*image_id_list<<endl;
	KexiDB::Cursor *result = c->imageIdList2ImageList(*image_id_list);
	QPtrList<ImageEntry> imageEntryList = imageCursor2PtrList(result);
 	c->freeCursor(result);
	return imageEntryList;
}



QDateTime
CategoriesDB::getOldestImage()
{
	return c->getOldestImage();
}


QDateTime
CategoriesDB::getNewestImage()
{
	return c->getNewestImage();
}

int
CategoriesDB::getNumberOfImageForDate(int year, int month, int day)
{
	return c->getNumberOfImageForDate(year, month, day);
}

int
CategoriesDB::getNumberOfImages()
{
	return c->getNumberOfImages();
}

QPtrVector<QString>
CategoriesDB::getAllImageFullPath()
{
	return c->getAllImageFullPath();
}

void
CategoriesDB::setUseCache(bool use)
{
	m_useCache = use;
}

bool
CategoriesDB::useCache()
{
	return m_useCache;
}

void
CategoriesDB::slotLinkAdded()
{
// 	MYDEBUG<<endl;
	emit sigLinkAdded();
}


#include "categoriesdb.moc"
