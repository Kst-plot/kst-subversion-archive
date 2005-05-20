/***************************************************************************
                         categoriesdb.cpp  -  description
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

#define MYDEBUG kdDebug()<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

CategoriesDB::CategoriesDB()
{
// 	MYDEBUG<<"Categories Creation"<<endl;
	this->c = new Categories();

	tabCategoryNode.resize(100);
	tabCategoryNode.setAutoDelete(true);

	pathsDict = new QDict<QVariant>;

	cachedImageToAdd = new QPtrList <QPtrList <QVariant> >;
	cachedImageToAdd->setAutoDelete(true);

	setUseCache(true);

	constructCategories();
}

CategoriesDB::~CategoriesDB()
{
	delete(c);
}

CategoryNode*
CategoriesDB::addTopCategory(const QString& title)
{
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
		msg=i18n("Unable to insert sub-category %1.").arg(title);
		return NULL;
	}
}


void
CategoriesDB::constructCategories()
{
	QStringList *l = c->topCategories();
	if(!l)
	{
		kdWarning()<<"No top category found!"<<endl;
		return;
	}
	for(QStringList::Iterator it = l->begin(); it != l->end(); ++it)
	{
		CategoryNode *node = new CategoryNode(c->getCategoryId(*it), *it);
		node->setIcon(c->getCategoryIcon(node->getId()));
		rootCat.append(node);
		tabCategoryNode.insert(node->getId(), node);

		constructCategories(node, *it);
	}
}


void
CategoriesDB::constructCategories(CategoryNode *root, const QString& cat_name)
{
	//MYDEBUG<<endl;
	QStringList *l = c->subCategories(cat_name);
	if(!l) return;
	for(QStringList::Iterator it = l->begin(); it != l->end(); ++it)
	{
		CategoryNode *node = new CategoryNode(c->getCategoryId(*it), *it);
		node->setIcon(c->getCategoryIcon(node->getId()));
		root->addChildCategory(node);
		tabCategoryNode.insert(node->getId(), node);

		constructCategories(node, *it);
	}
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
	printSubCategories(c->getCategoryId(cat_name));
}

void
CategoriesDB::printSubCategories(int categoryId)
{
	CategoryNode *root=getCategoryNode(categoryId);
	if(!root) return;
	kdDebug() << root->getId() << "--" << root->getTitle() << endl;
	QPtrList<CategoryNode> list=root->getSubCategoryList();
	CategoryNode *cat;
	for(cat = list.first();cat;cat=list.next())
	{
		kdDebug() << cat->getId() << "--" << cat->getTitle() << endl;
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
	return c->imageLinks(image_id, true);
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
CategoriesDB::addImage(const QString& name, const QString& path, const QDateTime& date, const QString& comment)
{
	MYDEBUG<<name<<" " <<path<<endl;
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
		flushImages();
	}
}
void
CategoriesDB::flushImages()
{
	c->addImages(cachedImageToAdd);
	cachedImageToAdd->clear();
}

/**
 *
 * @param name
 * @param dir_id
 */
void
CategoriesDB::addImage(const QString& name, int dir_id)
{
	c->addImage(name, dir_id, QDateTime::currentDateTime ());
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
		if(dir_id>0) pathsDict->insert(dir_path, new QVariant(dir_id));
		return dir_id;
	}
}


int
CategoriesDB::addDirectory(const QString& path)
{
	if(!useCache())
		return c->addDirectory(path);

	QVariant *val = pathsDict->find(path);
	if(val>0)
	{
		return val->toInt();
	}
	else
	{
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
	int img_id=-1;
	QStringList m_list=uris;
	QStringList imgid_list;
	for ( QStringList::Iterator it = m_list.begin(); it != m_list.end(); ++it )
	{
// 		MYDEBUG << " - " << (*it) << endl;
		KURL tmp_url(*it);
		img_id=getImageId(tmp_url.path());
		if(img_id >0) imgid_list.append(QString::number(img_id));
	}
	c->addLink(imgid_list, cat_id);
}

QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(const QStringList& catid_list, SelectionMode mode)
{
// 	MYDEBUG<<endl;
	QPtrList<ImageEntry> imageEntryList;
	if(catid_list.count()<=0) return imageEntryList;

	QPtrList<QStringList> l;
	QStringList m_catid_list = catid_list;
	for(QStringList::iterator it = m_catid_list.begin(); it!=m_catid_list.end(); ++it)
	{

		QStringList *tab = new QStringList();
		tab->append(*it);

		QPtrList<CategoryNode> list = getSubCategories((*it).toInt());
		CategoryNode *cat;
		for(cat = list.first();cat;cat=list.next())
			tab->append(QString::number(cat->getId()));
		l.append(tab);
	}
	//
	KexiDB::Cursor *result;
	if(mode==mode_OR)
		result = c->imagesCategoriesList_OR(l);
	else
		result = c->imagesCategoriesList_AND(l);

	imageEntryList = imageCursor2PtrList(result);
	c->freeCursor(result);
	return imageEntryList;
}


QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(const QString& cat_name)
{
	MYDEBUG<<endl;
	int id = c->getCategoryId(cat_name);
	return imagesSubCategoriesList(id);
}


/*
QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(int* categoryIdTab, int size)
{
	MYDEBUG<<endl;
	QPtrList<ImageEntry> list;
	int i=0;
	while(i<size)
	{
		QPtrList<ImageEntry> tmplist = imagesSubCategoriesList(categoryIdTab[i]);
		ImageEntry *image;
		for(image = tmplist.first();image;image=tmplist.next())
			list.append(image);
		i=i+1;
	}
	MYDEBUG<<endl;
	return list;
}
*/

QPtrList<ImageEntry>
CategoriesDB::imagesSubCategoriesList(int categoryId)
{
	MYDEBUG<<endl;
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
	return imageEntryList;
}

QPtrList<ImageEntry>
CategoriesDB::imagesNoteList(int note, int lem)
{
	KexiDB::Cursor *result = c->imagesNoteList(note, lem);
	QPtrList<ImageEntry> imageEntryList = imageCursor2PtrList(result);

	c->freeCursor(result);

	return imageEntryList;

}




QPtrList<ImageEntry>
CategoriesDB::imageCursor2PtrList(KexiDB::Cursor *cursor)
{
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

		list.append(new ImageEntry(id, c->getDirectoryPath(directory_id)+"/"+name, directory_id, comment, note, date_begin, date_end));

		cursor->moveNext();
	}
	return list;
}

void
CategoriesDB::moveImage(const QString& old_fullpath, const QString& newpath)
{
	if(c->moveImage(old_fullpath, addDirectory(newpath)) > 0)
		return;
}


void
CategoriesDB::renameImage(int id, const QString& new_name)
{
	if(c->renameImage(id, new_name)==0)
		return;
}

void
CategoriesDB::renameImage(const QString& old_name, const QString& new_name)
{
	if(c->renameImage(old_name, new_name)==0)
		return;
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

	if(c->renameCategory(id, new_name) == 0)
	{
		node->setTitle(new_name);
		return true;
	}
	else
	{
		msg=i18n("Unable to rename category %1.").arg(c->getCategoryName(id));
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
	MYDEBUG<<endl;
	CategoryNode *curnode = getCategoryNode(id);
	CategoryNode *node = NULL;
	if(curnode) node = new CategoryNode(*curnode);
	MYDEBUG<<endl;
	CategoryNode *parent_node = getCategoryNode(new_parent);
	MYDEBUG<<endl;
	if(!(node&&parent_node)) return;
	MYDEBUG<<endl;
	int res= c->setNewParent(id, new_parent);
	MYDEBUG<<res<<endl;
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

void
CategoriesDB::deleteCategoryImage(int cat_id, int ima_id)
{
	c->deleteCategoryImage(cat_id, ima_id);
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
	QPtrList<ImageEntry> list = imageCursor2PtrList(result);
	c->freeCursor(result);
	return list;
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

QPtrList<ImageEntry>
CategoriesDB::imagesPatternList(const QString& pattern,
								const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	QPtrList<QString> list;
	list.append(new QString(pattern));
	return imagesPatternList(list, iiList, mode);
}


QPtrList<ImageEntry>
CategoriesDB::imagesPatternList(QPtrList<QString>& patterns,
								const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode)
{
	MYDEBUG<<mode<<endl;
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
