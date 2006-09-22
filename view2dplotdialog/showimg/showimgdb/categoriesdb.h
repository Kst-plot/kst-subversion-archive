/***************************************************************************
                         categoriesdb.h  -  description
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

#ifndef CATEGORIESDB_H
#define CATEGORIESDB_H

// Local
#include "categorynode.h"
#include "imageentry.h"


// Qt
#include <qobject.h>
#include <qptrvector.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qdict.h>

namespace KexiDB
{
	class Cursor;
}
class Categories;

class CategoriesDB : public QObject
{

Q_OBJECT

public:
	enum SelectionMode
	{
		mode_AND=0,
		mode_OR
	};

	CategoriesDB(const QString& type, const QString& sqlitePath, const QString& mysqlUsername, const QString& mysqlPassword, const QString& mysqlHostname);
	~CategoriesDB();

	bool isConnected() const;

	CategoryNode* addTopCategory(const QString& title);
	CategoryNode* addSubCategory(int parent, const QString& title, const QString& desc, QString& msg);

	int addImage(const QString& name, int dir_id);
	void addImage(const QString& name, const QString& path, const QDateTime& date, const QString& comment=QString::null, bool check=true);

	void addLink(const QString& image, const QString& path, const QString& catname);
	void addLink(int image_id, int cat_id);
	void addLink(const QStringList& uris, int cat_id);

	void addNote(const QStringList& uris, int note);

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
	QStringList* getCategoryIdListImage(int image_id) const;
	QStringList* getCategoryIdListImage(const QStringList& image_id_list, bool distinct) const;

	QPtrList<ImageEntry> allImages();
	QPtrList<ImageEntry> imagesNoteList(int note, int lem);
	QPtrList<ImageEntry> imagesNoteList(const QStringList& noteList, const QPtrList<QVariant>& iiList, CategoriesDB::SelectionMode mode);
	QPtrList<ImageEntry> imagesDateList(const QDate& date_begin, const QDate& date_end,
						const QPtrList<QVariant>& iiList = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesDateList(const QDate& date, int bia,
						const QPtrList<QVariant>& iiLists = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);

	QPtrList<ImageEntry> imagesPatternList(const QStringList& patterns,
						const QPtrList<QVariant>& iiList = QPtrList<QVariant>(), CategoriesDB::SelectionMode mode = CategoriesDB::mode_AND);
	QPtrList<ImageEntry> imagesCommentList(const QString& comment);

	//
	bool updateImageInformations(int image_id, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories);
	bool updateImageInformations(QPtrList<ImageEntry>& image_id_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories);

	void moveImage(const QString& old_fullpath, const QString& newpath);
	void renameImage(int id, const QString& new_name);
	void renameImage(const QString& old_name, const QString& new_name);

	void setImageComment(int id, const QString& comment);
	void setImageNote(int id, int note);
	void setImageDate(int id, const QDateTime& begin, const QDateTime& end=QDateTime());

	bool renameCategory(int id, const QString& new_name, QString& msg);
	bool setCategoryDescription(int id, const QString& descr, QString& msg);
	bool setCategoryIcon(int id, const QString& icon, QString& msg);

	void moveCategory(int id, int new_parent);
	//
	void deleteNodeCategory(int id);
	void deleteImage(int id);
	int deleteImage(const QStringList& ima_id_list);
	void deleteCategoryImage(int ima_id, int cat_id);


	//
	QPtrList<CategoryNode> getRootCategories();
	QStringList* subCategories(const QString& cat_name);

	QStringList* getImageListId(const QStringList& img_list);

	int getImageId(const QString& name, int dir_id);
	int getImageId(const QString& image_path);
	ImageEntry* getImageEntry(const QString& image_path);
	QPtrList<ImageEntry> getImageEntries(const QStringList& image_path_list);

	int getCategoryId(const QString& cat_name) const;
	int getDirectoryId(const QString& dir_path);

	//
	QDateTime getOldestImage();
	QDateTime getNewestImage();
	int getNumberOfImageForDate(int year, int month, int day);

	int getNumberOfImages();
	QPtrVector<QString> getAllImageFullPath();

	//
	void setUseCache(bool use);
	bool useCache();
	void flushImages(bool check=true);

protected:
	void constructCategories();
	void constructCategories(CategoryNode *root, const QString& cat_name);

	QString printCategories(CategoryNode *root, int s);

	QPtrList<ImageEntry> imageCursor2PtrList(KexiDB::Cursor *cursor);

protected slots:
	void slotLinkAdded();

signals:
	void sigLinkAdded();
	void sigFileRenamed();
	void sigFileMoved();

protected:
	QPtrList<CategoryNode> rootCat;
	QPtrVector<CategoryNode> tabCategoryNode;

	Categories *c;

	QDict<QVariant> *pathsDict;
	bool m_useCache;


	QPtrList <QPtrList <QVariant> > *cachedImageToAdd;
};


#endif

