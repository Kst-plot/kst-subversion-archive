/***************************************************************************
                           categorydbmanager.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#ifndef CATEGORYDBMANAGER_H
#define CATEGORYDBMANAGER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

// KDE
#include <kurl.h>

// Qt
#include <qobject.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qfileinfo.h>

class CategoriesDB;
class MainWindow;
class ListItem;
class CategoryNode;
class ImageEntry;
class CategoryImageFileIconItem;
class CategoryListItemTag;

class CategoryDBManager_private;

class CategoryDBManager : public QObject
{
	Q_OBJECT

public:

	enum SelectionMode
	{
		mode_AND=0,
		mode_OR
	};

	CategoryDBManager( MainWindow* parent );
	virtual ~CategoryDBManager();

	void setSelectionMode(SelectionMode mode);
 	SelectionMode getSelectionMode();

	void addCurrentCategories(int cat_id);
	void delCurrentCategories(int cat_id);

	void addCurrentDate(QDateTime datetimeb, QDateTime datetimee);
	void delCurrentDate(QDateTime datetimeb, QDateTime datetimee);

	void addCurrentPattern(const QString* pattern);
	void delCurrentPattern(const QString* pattern);

	bool renameCategory(int cat_id, const QString& newName, QString& msg);
	bool addSubCategory(CategoryListItemTag* catparent,
				const QString& newName, QString& msg );

	bool renameImage(const QString& oldname, const QString& newname);
	bool moveImages(const KURL::List& fileurls, const KURL& desturl);

	bool renameDirectory(const KURL& srcurl, const KURL& desturl);
	bool moveDirectory(const KURL& srcurl, const KURL& desturl);


	void addImageToDB(QFileInfo *info);

	QPtrList<CategoryNode> getRootCategories();
	int getCategoryId(const QString& cat_name);
	void addCategoryListItemTag(CategoryListItemTag* parent, QPtrList<CategoryNode>&);
	void deleteNodeCategory(int cat_id);
	void addCategoryToImages(const QStringList& uris, int cat_id);

	QDateTime getOldestImage();
	QDateTime getNewestImage();
	int getNumberOfImageForDate(int year, int month=-1, int day=-1);

	QStringList* getCategoryNameListImage(int image_id) const;
	QStringList* getCategoryNameListImage(const QString& ima_path) const;

	int getDirectoryId(const QString path) const;

	//--------------------------------------------------------------------------
	void flush();
	void __startAddingImages__();

public slots:
	void newFilesAdded(ListItem *item);

signals:
	void isAddingFiles(bool);
	void numberOfLeftItems(int);

protected:
	void refreshRequest();
	CategoryNode* getCategoryNode(const QString& name);
	QPtrList<CategoryNode> getSubCategories(const QString& cat_name);
	QPtrList<CategoryNode> getSubCategories(int cat_id);

	QPtrList<ImageEntry> imagesSubCategoriesList(bool *ok);
	QPtrList<ImageEntry> imagesDateList(const QPtrList<ImageEntry>& imageEntryList, bool *ok);
	QPtrList<ImageEntry> getImagesPatternList(const QPtrList<ImageEntry>& imageEntryList, bool *ok);

	QPtrList<QVariant> imageEntryList2IDImageList(const QPtrList<ImageEntry>& ieList);

protected:
	MainWindow *mw;
	CategoriesDB *cdb;
	QStringList catid_list;

	SelectionMode m_selectionmode;

	bool m_isAddingFiles;

	QPtrList < CategoryImageFileIconItem > list;
	QDateTime m_datetime_begin, m_datetime_end;
	QPtrList < QString > patternList;

private:
	CategoryDBManager_private *catdbM_priv;

};

#endif /* WANT_LIBKEXIDB */

#endif /* __CATEGORYDBMANAGER_H__ */
