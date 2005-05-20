/***************************************************************************
                         categoriesdb.h  -  description
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

#ifndef CATEGORIESDB_H
#define CATEGORIESDB_H

// Local
#include "categorynode.h"
#include "imageentry.h"

#include <kexidb/cursor.h>


// Qt
#include <qptrlist.h>
#include <qptrvector.h>
#include <qstringlist.h>
#include <qvariant.h>

class Categories;

class CategoriesDB
{
public:
	enum SelectionMode
	{
		mode_AND=0,
		mode_OR
	};

	CategoriesDB();
	~CategoriesDB();

	CategoryNode* addTopCategory(const QString& title);
	CategoryNode* addSubCategory(int parent, const QString& title, const QString& desc, QString& msg);

	void addImage(const QString& name, int dir_id);
	void addImage(const QString& name, const QString& path, const QDateTime& date, const QString& comment=QString::null);

	void addLink(const QString& image, const QString& path, const QString& catname);
	void addLink(int image_id, int cat_id);
	void addLink(const QStringList& uris, int cat_id);

	int addDirectory(const QString& path);
	int renameDirectory(const QString& old_path, const QString& new_path);
	int moveDirectory(const QString& old_path, const QString& new_path);

	//
	void printCategories();

	void printSubCategories(const QString& cat_name);
	void printSubCategories(int categoryId);

	void printImageEntry(QPtrList<ImageEntry>& imageEntryList);

	//
	QPtrList<CategoryNode> getSubCategories(int categoryId);
	QPtrList<CategoryNode> getSubCategories(const QString& cat_name);

	QPtrList<ImageEntry> imagesSubCategoriesList(int categoryId);
	QPtrList<ImageEntry> imagesSubCategoriesList(const QStringList& catid_list, CategoriesDB::SelectionMode mode);
	QPtrList<ImageEntry> imagesSubCategoriesList(const QString& cat_name);

	CategoryNode* getCategoryNode(int categoryId) const;
	CategoryNode* getCategoryNode(const QString& cat_name) const;
	QPtrList<CategoryNode> getCategoryListImage(int image_id) const;
	
	QStringList* getCategoryNameListImage(int image_id) const;
	QStringList* getCategoryNameListImage(const QString& ima_path) const;
	
	QPtrList<ImageEntry> allImages();
	QPtrList<ImageEntry> imagesNoteList(int note, int lem);
	QPtrList<ImageEntry> imagesDateList(const QDate& date_begin, const QDate& date_end,
										const QPtrList<QVariant>& iiList = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesDateList(const QDate& date, int bia,
										const QPtrList<QVariant>& iiLists = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesPatternList(const QString& pattern,
										   const QPtrList<QVariant>& iiList = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesPatternList(QPtrList<QString>& patterns,
										   const QPtrList<QVariant>& iiList = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesCommentList(const QString& comment);

	//
	void moveImage(const QString& old_fullpath, const QString& newpath);
	void renameImage(int id, const QString& new_name);
	void renameImage(const QString& old_name, const QString& new_name);

	void setImageComment(int id, const QString& comment);
	void setImageNote(int id, int note);
	void setImageDate(int id, const QDateTime& begin, const QDateTime& end=QDateTime());

	bool renameCategory(int id, const QString& new_name, QString& msg);
	//
	void deleteNodeCategory(int id);
	void deleteImage(int id);
	void deleteCategoryImage(int cat_id, int ima_id);

	void moveCategory(int id, int new_parent);

	//
	QPtrList<CategoryNode> getRootCategories();
	QStringList* subCategories(const QString& cat_name);

	int getImageId(const QString& name, int dir_id);
	int getImageId(const QString& image_path);

	int getCategoryId(const QString& cat_name) const;
	int getDirectoryId(const QString& dir_path);

	//
	QDateTime getOldestImage();
	QDateTime getNewestImage();
	int getNumberOfImageForDate(int year, int month, int day);

	//
	void setUseCache(bool use);
	bool useCache();
	void flushImages();

protected:
	void constructCategories();
	void constructCategories(CategoryNode *root, const QString& cat_name);

	QString printCategories(CategoryNode *root, int s);

	QPtrList<ImageEntry> imageCursor2PtrList(KexiDB::Cursor *cursor);

protected:
	QPtrList<CategoryNode> rootCat;
	QPtrVector<CategoryNode> tabCategoryNode;

	Categories *c;

	QDict<QVariant> *pathsDict;
	bool m_useCache;


	QPtrList <QPtrList <QVariant> > *cachedImageToAdd;
};


#endif

