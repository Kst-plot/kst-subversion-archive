/***************************************************************************
                           categories.h  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2006 by Richard Groult
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

#ifndef SHOWIMG_CATEGORIES
#define SHOWIMG_CATEGORIES

// Local kexi
#include "categoriesdb.h"

#include <kexidb/driver.h>
#include <kexidb/connection.h>
#include <kexidb/drivermanager.h>
#include <kexidb/fieldlist.h>
#include <kexidb/cursor.h>
#include <kexidb/expression.h>
#include <kexidb/parser/sqlparser.h>

// KDE
#include <kurl.h>

// Qt
#include <qvariant.h>
#include <qptrvector.h>

class Categories : public QObject
{
Q_OBJECT

	public:
		enum SelectionMode
		{
			mode_AND=0,
			mode_OR
		};

		Categories(const QString& type, const QString& sqlitePath, const QString& mysqlUsername, const QString& mysqlPassword, const QString& mysqlHostname);
		~Categories(void);

		bool isConnected() const ;

		int addTopCategory(const QString& cat_name, const QString& desc=QString::null, const QString& icon=QString::null);
		int addSubCategory(int up_cat, const QString& cat_name, const QString& desc=QString::null, const QString& icon=QString::null);

		QStringList* topCategories(void);
		QStringList* allCategories(void);
		QStringList* subCategories(const QString&);

		int getCategoryId(const QString&);
		QStringList* getCategoryId(const QStringList&);
		QString getCategoryName(int cat_id);
		QString getCategoryIcon(int cat_id);
		QString getCategoryDescription(int cat_id);

		QStringList* getImageListId(const QStringList& img_list);

		int getImageId(const QString& ima_path);
		int getImageId(const QString& ima_name, const QString&  dir_path);
		int getImageId(const QString&  ima_name, int dir_id);
		KexiDB::Cursor* getImageEntry(const QString&  ima_name, int dir_id);
		QString  getImageName(int);

		KexiDB::Cursor* allImages();

		KexiDB::Cursor* imagesNoteList(int note, int lem);
		KexiDB::Cursor* imagesNoteList(const QStringList& noteList, const QPtrList<QVariant>& iiList, Categories::SelectionMode mode);


		KexiDB::Cursor* imagesCategoriesList(const QStringList& tab);
		KexiDB::Cursor* imagesCategoriesList_OR(QPtrList<QStringList>& l);
		KexiDB::Cursor* imagesCategoriesList_AND(QPtrList<QStringList>& l);

		KexiDB::Cursor* imagesPatternList(const QStringList& patterns,
						const QPtrList<QVariant>& iiList, Categories::SelectionMode mode);
		KexiDB::Cursor* imagesCommentList(const QString& comment);
		KexiDB::Cursor* imagesDateList(const QDate& date, int bia,
									   const QPtrList<QVariant>& iiList, Categories::SelectionMode mode);
		KexiDB::Cursor* imagesDateList(const QDate& date_begin, const QDate& date_end,
									   const QPtrList<QVariant>& iiList, Categories::SelectionMode mode);

		KexiDB::Cursor* imageIdList2ImageList(KexiDB::Cursor *);
		KexiDB::Cursor* imageIdList2ImageList(const QStringList& image_path_list);

		QDateTime getOldestImage();
		QDateTime getNewestImage();
		int getNumberOfImageForDate(int year, int month, int day);

		int getNumberOfImages();
		int getMaxImageId();
		QPtrVector<QString> getAllImageFullPath();

		///
		int addImage(const QString& name, int dir_id, const QDateTime& date, const QString& comment=QString::null, bool check=true);
		bool addImages(QPtrList <QPtrList <QVariant> > *imageList, bool check=true);
		int deleteImage(int);
		int deleteImage(const QStringList& ima_id_list);

		int renameImage(int id, const QString& new_name);
		int renameImage(const QString& oldfullname, const QString& newfullname);

		int moveImage(const QString& old_fullpath, const QString& new_path);
		int moveImage(const QString& old_fullpath, int dir_id);
		int moveImage(int ima_id, const QString& new_path);
		int moveImage(int ima_id, int dir_id);
		int moveImages(const KURL::List& fileurls, const KURL& desturl);

		bool updateImageInformations(int image_id, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories);
		bool updateImageInformations(const QStringList& image_id_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories);
		int setImageComment(int id, const QString& comment);
		int setImageNote(int id, int note);
		int setImageNote(const QStringList& image_id_list, int note);
		int setImageDate(int id, const QDateTime& begin, const QDateTime& end);

		void freeCursor(KexiDB::Cursor*);

		///
		int addDirectory(const QString& path);
		int deleteDirectory(int);
		int getDirectoryId(const QString& path);
		QString getDirectoryPath(int);

		int renameDirectory(const QString& old_path, const QString& new_path);
		int moveDirectory(const QString& old_dirPath, const QString& old_dirName, const QString& dest_path);

		///
		int addLink(const QString& image_name, const QString& image_path, const QString& cat_name);
		int addLink(int image_id, int cat_id);
		int addLink(const QStringList& imgid_list, int cat_id);
		int addLink(const QStringList& imgid_list, const QStringList& cat_id_list);

		QStringList* imageLinks(int image_id, bool getCatName=false);
		QStringList* imageLinks(const QStringList& image_id_list, bool getCatName=false, bool distinct=true);

		///
		bool renameCategory(int id, const QString& new_name);
		bool setCategoryDescription(int id, const QString& descr);
		bool setCategoryIcon(int id, const QString& icon);

		int deleteNodeCategory(int id);
		int deleteCategoryImage(int ima_id, int cat_id);
		int deleteCategoryImage(int ima_id,  const QStringList& catid_list);
		int deleteCategoryImage(const QStringList& imaid_list, const QStringList& catid_list);

		int setNewParent(int id, int new_parent);

		void printCursor(KexiDB::Cursor *cursor);

		void resetRecentAddedFileQueue();
		QStringList getRecentAddedFileQueue();

signals:
	void sigLinkAdded();

protected:
	void setUpdating(bool updating){m_updating=updating;};
	inline bool isUpdating(){return m_updating;};

	bool createDatabase(const QString& db_name);
	bool fillDatabase();

	KexiDB::Cursor* query2ImageListCursor(const QString& query);
	KexiDB::Cursor* query2ImageListCursor(KexiDB::QuerySchema& query);
	QStringList* cursor2stringlist(KexiDB::Cursor * cursor, int col=0);

	QStringList* executeQuerry(const QString& query, int col=0, bool useParser=true);
	QStringList* executeQuerry(KexiDB::QuerySchema& query, int col=0);

	QString querySingleString(const QString& query, bool useParser=true);
	int querySingleNumber(const QString& query, bool useParser=true);

	QString formatDateTime(const QString& field, const QString& format);

protected:
	KexiDB::ConnectionData *m_p_conn_data;
	KexiDB::DriverManager *m_p_manager;
	QGuardedPtr<KexiDB::Connection> m_conn_ptr;
	QGuardedPtr<KexiDB::Driver> m_driver;

private:
	KexiDB::TableSchema *m_p_t_categories;
	KexiDB::TableSchema *m_p_t_images;
	KexiDB::TableSchema *m_p_t_image_category;
	KexiDB::TableSchema *m_p_t_directories;
	KexiDB::TableSchema *m_p_t_media;

	QString
		m_datetime_format,
		m_date_format;

	bool m_updating;

	QStringList m_RecentAddedFileQueue;

};

#endif
