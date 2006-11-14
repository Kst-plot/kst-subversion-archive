/*****************************************************************************
                           categorydbmanager.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
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
#include <qdict.h>

class CategoriesDB;
class MainWindow;
class ListItem;
class CategoryNode;
class ImageEntry;
class CategoryImageFileIconItem;
class CategoryListItemTag;

class CategoryDBManager_private;

class KConfig;

class CategoryDBManager : public QObject
{
	Q_OBJECT

public:

	enum SelectionMode
	{
		mode_AND=0,
		mode_OR
	};

	CategoryDBManager();
	virtual ~CategoryDBManager();

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

	bool isEnabled() const;
	void setEnabled(bool enable);

	bool isConnected() const;

	//use it to reload current request: request the database  (this method uses refreshRequest())
	void reload();

	//create all the current CategoryImageFileIconItem: don't request the database
	int refreshRequest();

	int setSelectionMode(SelectionMode mode);
 	SelectionMode getSelectionMode();

	int addCurrentCategories(int cat_id);
	int delCurrentCategories(int cat_id);

	int addCurrentDate(QDateTime datetimeb, QDateTime datetimee);
	int delCurrentDate(QDateTime datetimeb, QDateTime datetimee);

	int addCurrentPattern(const QString& pattern);
	int delCurrentPattern(const QString& pattern);

	int addCurrentNote(const QString& note);
	int delCurrentNote(const QString& note);


	bool renameCategory(int cat_id, const QString& newName, QString& msg);
	bool setCategoryDescription(int cat_id, const QString& descr, QString& msg);
	bool setCategoryIcon(int cat_id, const QString& icon, QString& msg);

	bool addSubCategory(CategoryListItemTag* catparent,
				const QString& newName, QString& msg );

	bool renameImage(QDict<QString>& renamedFiles);
	bool renameImage(const QString& oldname, const QString& newname);
	bool moveImages(const KURL::List& fileurls, const KURL& desturl);

	bool renameDirectory(const KURL& srcurl, const KURL& desturl);
	bool moveDirectory(const KURL& srcurl, const KURL& desturl);


	int addImageToDB(QFileInfo *info, bool forceFlush=false, bool check=true);

	QPtrList<CategoryNode> getRootCategories();
	int getCategoryId(const QString& cat_name);
	void addCategoryListItemTag(CategoryListItemTag* parent, QPtrList<CategoryNode>&);
	void deleteNodeCategory(int cat_id);

	void addCategoryToImages(const KURL::List& a_list, int cat_id);
	void addNoteToImages(const KURL::List& a_list, int note);

	void addImagesToCategory(int image_id, QStringList* cat_list);
	void addImagesToCategory(const QString& fullname, QStringList* cat_list);

	QDateTime getOldestImage();
	QDateTime getNewestImage();
	int getNumberOfImageForDate(int year, int month=-1, int day=-1);

	int getNumberOfImages();

	QStringList* getCategoryNameListImage(int image_id) const;
	QStringList* getCategoryNameListImage(const QString& ima_path) const;
	QStringList* getCategoryIdListImage(const QString& ima_path) const;
	QStringList* getCategoryIdListImage(int image_id) const;
	QStringList* getCategoryIdListImage(const QStringList& image_id_list, bool distinct) const;

	int getDirectoryId(const QString path) const;
	ImageEntry* getImageEntry(const QString& fullname);
	QPtrList<ImageEntry> getImageEntries(const QStringList& image_path_list);

	bool updateImageInformations(int image_id, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end,const  QStringList& removedCategories, const QStringList& addedCategories);
	bool updateImageInformations(QPtrList<ImageEntry>& image_id_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end,const  QStringList& removedCategories, const QStringList& addedCategories);
	bool addImageInformations(const QStringList& image_path_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& addedCategories);

	QString getType();
	void setType(const QString& type);
	QString getSqlitePath();
	void setSqlitePath(const QString& path);
	QString getMysqlUsername();
	void setMysqlUsername(const QString& name);
	QString getMysqlPassword();
	void setMysqlPassword(const QString& pass);
	QString getMysqlHostname();
	void setMysqlHostname(const QString& host);

	int removeObsololeteFilesOfTheDatabase();

	//--------------------------------------------------------------------------
	//internal
	void flush(bool check=true);
	void __startAddingImages__();

public slots:
	void newFilesAdded(ListItem *item);

protected slots:
	void slotLinkAdded();
	void slotAddLinksStarted(int);

signals:
	void isAddingFiles(bool);
	void numberOfLeftItems(int);
	void sigHasSeenFile(int);

	void sigLinkAdded();
	void sigAddLinksStarted(int);

	void sigSetMessage(const QString&);

protected:
	int refreshRequest_private();
	CategoryNode* getCategoryNode(const QString& name);
	QPtrList<CategoryNode> getSubCategories(const QString& cat_name);
	QPtrList<CategoryNode> getSubCategories(int cat_id);

	QPtrList<ImageEntry> getImagesSubCategoriesList(bool *ok);
	QPtrList<ImageEntry> getImagesDateList(const QPtrList<ImageEntry>& imageEntryList, bool *ok);
	QPtrList<ImageEntry> getImagesPatternList(const QPtrList<ImageEntry>& imageEntryList, bool *ok);
	QPtrList<ImageEntry> getImagesNoteList(const QPtrList<ImageEntry>& imageEntryList, bool *ok);

	QPtrList<QVariant> imageEntryList2IDImageList(const QPtrList<ImageEntry>& ieList);
	
	MainWindow *getMainWindow();	

private:
	CategoriesDB *m_p_cdb;
	QStringList m_catid_list;

	SelectionMode m_selectionmode;

	bool m_isAddingFiles, m_isEnabled;

	QPtrList<ImageEntry> m_imageEntryList;

	QPtrList < CategoryImageFileIconItem > m_list;
	QDateTime m_datetime_begin, m_datetime_end;
	QStringList m_patternList;
	QStringList m_noteList;

	QString m_type, m_sqlitePath, m_mysqlUsername, m_mysqlPassword, m_mysqlHostname;

	CategoryDBManager_private *m_cp_atdbM_priv;

};

#endif /* WANT_LIBKEXIDB */

#endif /* __CATEGORYDBMANAGER_H__ */
