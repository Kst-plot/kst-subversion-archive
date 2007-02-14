/***************************************************************************
                         categories.cpp  -  description
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

#include "categories.h"

// Local
#include <showimg/showimg_common.h>

// KDE
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// QT
#include <qdatetime.h>
#include <qdir.h>

Categories::Categories(const QString& type, const QString& sqlitePath, const QString& mysqlUsername, const QString& mysqlPassword, const QString& mysqlHostname)
	: QObject(),

	m_p_conn_data(NULL),
	m_p_manager(NULL),
	m_conn_ptr(NULL),
	m_driver(NULL),

	m_updating(false)
{
	m_datetime_format = "yyyy-MM-dd hh:mm:ss";
	m_date_format = "yyyy-MM-dd";

	QString db_name;
	QString drv_name;
	if(type == QString::fromLatin1("sqlite"))
	{
		db_name = sqlitePath;
		drv_name = "SQLite3";
		m_p_manager = new KexiDB::DriverManager();
		m_driver = m_p_manager->driver(drv_name);
		if(!m_driver)
		{
			MYWARNING<<"no m_driver found for "<<drv_name<<". DB support disabled."<<endl;
			return;
		}
		m_p_conn_data = new KexiDB::ConnectionData();
		QFileInfo dbFileinfo(db_name);
		if(!dbFileinfo.exists())
		{
			QFileInfo dbPathFileInfo(dbFileinfo.dirPath());
			if(!dbPathFileInfo.exists())
				QDir().mkdir(dbPathFileInfo.absFilePath ());
		}
		m_p_conn_data->setFileName( db_name );
	}
	else
	{
		drv_name = "mysql";
		//drv_name = "PostgreSQL";

		m_p_manager = new KexiDB::DriverManager();
		m_driver = m_p_manager->driver(drv_name);
		if(!m_driver)
		{
			MYWARNING<<"no m_driver found for "<<drv_name<<". DB support disabled."<<endl;
			MYWARNING<<m_p_manager->serverErrorMsg()<<endl;
			m_p_manager->debugError();
			KMessageBox::error(NULL, m_p_manager->errorMsg());
			return;
		}
		m_p_conn_data = new KexiDB::ConnectionData();
		m_p_conn_data->userName = mysqlUsername;
		m_p_conn_data->password=mysqlPassword;
		m_p_conn_data->hostName=mysqlHostname;
		db_name = m_p_conn_data->userName + "_ShowimgCategories";
	}

	m_conn_ptr = m_driver->createConnection(*m_p_conn_data);

	if (!m_conn_ptr || m_driver->error())
	{
		MYWARNING<<"ERROR !m_conn_ptr || m_driver->error()"<<endl;
		KMessageBox::detailedError(NULL, m_driver->errorMsg(), m_driver->serverErrorMsg(), i18n("Connection Error"));
		m_driver->debugError();
		delete(m_conn_ptr); m_conn_ptr=NULL;
		return;
	}
	if (!m_conn_ptr->connect())
	{
		MYWARNING<<"ERROR!m_conn_ptr->connect()"<<endl;
		m_conn_ptr->debugError();
		KMessageBox::detailedError(NULL, m_conn_ptr->errorMsg(), m_conn_ptr->serverErrorMsg(), i18n("Connection Error"));
		delete(m_conn_ptr);m_conn_ptr=NULL;
		return;
	}
	if(!m_conn_ptr->databaseExists( db_name ))
	{
		if (!createDatabase( db_name ))
		{
			MYWARNING<<"!m_conn_ptr->databaseExists( db_name )"<<endl;
			KMessageBox::detailedError(NULL, m_conn_ptr->errorMsg(), m_conn_ptr->serverErrorMsg(), i18n("Connection Error"));
			//m_conn_ptr->debugError();
			delete(m_conn_ptr);m_conn_ptr=NULL;
			return;
		}
	}
	else
	{
		if (!m_conn_ptr->useDatabase( db_name ))
		{
			MYWARNING<<"!m_conn_ptr->useDatabase( db_name )"<<endl;
			KMessageBox::detailedError(NULL, m_conn_ptr->errorMsg(), m_conn_ptr->serverErrorMsg(), i18n("Connection Error"));
			//m_conn_ptr->debugError();
			delete(m_conn_ptr);m_conn_ptr=NULL;
			return;
		}
		else
		{
			m_p_t_categories = m_conn_ptr->tableSchema("categories");
			m_p_t_images = m_conn_ptr->tableSchema("images");
			m_p_t_image_category = m_conn_ptr->tableSchema("image_category");
			m_p_t_directories = m_conn_ptr->tableSchema("directories");
		}
	}
}

Categories::~Categories(void)
{
	if (!m_conn_ptr)
		return;
	if (!m_conn_ptr->closeDatabase())
	        m_conn_ptr->debugError();
	if (!m_conn_ptr->disconnect())
	        m_conn_ptr->debugError();

	delete(m_p_manager);
	delete(m_p_conn_data);
}

bool
Categories::isConnected() const
{
	return m_conn_ptr?true:false;
}

bool
Categories::createDatabase(const QString& db_name)
{
	if (!m_conn_ptr->createDatabase( db_name ))
	{
		m_conn_ptr->debugError();
		return false;
	}
	//------------------------------------------------------------------------------
	if (!m_conn_ptr->useDatabase( db_name ))
	{
		m_conn_ptr->debugError();
		return false;
	}
	//------------------------------------------------------------------------------
 	m_conn_ptr->setAutoCommit(false);
	KexiDB::Transaction t;
	if(m_driver->transactionsSupported())
	{
		t = m_conn_ptr->beginTransaction();
		if (m_conn_ptr->error())
		{
			m_conn_ptr->debugError();
			return false;
		}
	}
	//------------------------------------------------------------------------------
	//sqlite_exec(db, "CREATE TABLE categories (category_name VARCHAR(256) UNIQUE, category_id INTEGER PRIMARY KEY, category_up UNSIGNED INTEGER);", NULL, NULL, NULL);
	KexiDB::Field *f;
	m_p_t_categories = new KexiDB::TableSchema("categories");
	m_p_t_categories->setCaption("All categories");

	m_p_t_categories->addField( f=new KexiDB::Field("category_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned));
	f->setCaption("category id");

	m_p_t_categories->addField( f=new KexiDB::Field("category_name", KexiDB::Field::Text, KexiDB::Field::Unique) );
	f->setCaption("Name");

	m_p_t_categories->addField( f=new KexiDB::Field("category_desc", KexiDB::Field::Text) );
	f->setCaption("Description");

	m_p_t_categories->addField( f=new KexiDB::Field("category_icon", KexiDB::Field::Text) );
	f->setCaption("Icon");

	m_p_t_categories->addField( f=new KexiDB::Field("category_up", KexiDB::Field::Integer, 0, KexiDB::Field::Unsigned, 0, 0, QVariant(0)) );
	f->setCaption("Parent category id");

	if (!m_conn_ptr->createTable( m_p_t_categories ))
	{
		m_conn_ptr->debugError();
		MYWARNING<<"ERROR creating m_p_t_categories"<<endl;
		return false;
	}
	m_p_t_categories->debug();
	//------------------------------------------------------------------------------
	m_p_t_media = new KexiDB::TableSchema("media");
	m_p_t_media->setCaption("All media");

	m_p_t_media->addField( f=new KexiDB::Field("media_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("media id");

	m_p_t_media->addField( f=new KexiDB::Field("media_name", KexiDB::Field::Text) );
	f->setCaption("media name");

	m_p_t_media->addField( f=new KexiDB::Field("media_icon", KexiDB::Field::Text) );
	f->setCaption("media name");

	//------------------------------------------------------------------------------
	//	sqlite_exec(db, "CREATE TABLE images (image_id INTEGER PRIMARY KEY, image_name VARCHAR(256) NOT NULL, image_directory_id INTEGER NOT NULL DEFAULT 1, image_comment SALLTEXT DEFAULT '', image_note INTEGER DEFAULT -1, image_date_begin DATETIME DEFAULT '1900-01-01T00:00:00', image_date_end DATETIME DEFAULT '1900-01-01T00:00:00', UNIQUE(image_directory_id, image_name) );", NULL, NULL, NULL);
	m_p_t_images = new KexiDB::TableSchema("images");
	m_p_t_images->setCaption("All images");

	m_p_t_images->addField( f=new KexiDB::Field("image_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("image id");

	m_p_t_images->addField( f=new KexiDB::Field("image_name", KexiDB::Field::Text) );
	f->setCaption("image name");

	m_p_t_images->addField( f=new KexiDB::Field("image_dir_id", KexiDB::Field::Integer) );
	f->setCaption("image directory id");

	m_p_t_images->addField( f=new KexiDB::Field("image_media_id", KexiDB::Field::Integer, 0, 0, 0, 0,
						QVariant(0)) );
	f->setCaption("image media id");

	m_p_t_images->addField( f=new KexiDB::Field("image_comment", KexiDB::Field::Text) );
	f->setCaption("image name");

	m_p_t_images->addField( f=new KexiDB::Field("image_note", KexiDB::Field::Integer) );
	f->setCaption("image note");

	m_p_t_images->addField( f=new KexiDB::Field("image_date_begin", KexiDB::Field::DateTime, 0, 0, 0, 0,
						 QVariant(QDateTime(QDate(1977, 1, 1), QTime(0, 0, 0)))) );
	f->setCaption("image begin date");

	m_p_t_images->addField( f=new KexiDB::Field("image_date_end", KexiDB::Field::DateTime, 0, 0, 0, 0,
						 QVariant(QDateTime(QDate(1977, 1, 1), QTime(0, 0, 0)))) );
	f->setCaption("image end date");

	if (!m_conn_ptr->createTable( m_p_t_images ))
	{
		m_conn_ptr->debugError();
		MYWARNING<<"ERROR creating m_p_t_images"<<endl;
		return false;
	}
	m_p_t_images->debug();

	//------------------------------------------------------------------------------
	m_p_t_image_category = new KexiDB::TableSchema("image_category");
	m_p_t_image_category->setCaption("Links between images and categories");

	m_p_t_image_category->addField( f=new KexiDB::Field("imacat_ima_id", KexiDB::Field::Integer) );
	f->setCaption("image id");

	m_p_t_image_category->addField( f=new KexiDB::Field("imacat_cat_id", KexiDB::Field::Integer) );
	f->setCaption("category id");


	if (!m_conn_ptr->createTable( m_p_t_image_category ))
	{
	        m_conn_ptr->debugError();
		MYWARNING<<"ERROR creating m_p_t_image_category"<<endl;
		return false;
	}
	m_p_t_image_category->debug();


	//------------------------------------------------------------------------------
	m_p_t_directories = new KexiDB::TableSchema("directories");
	m_p_t_directories->setCaption("All directories");

	m_p_t_directories->addField( f=new KexiDB::Field("directory_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("directory id");

	m_p_t_directories->addField( f=new KexiDB::Field("directory_path", KexiDB::Field::Text, KexiDB::Field::Unique) );
	f->setCaption("directory path");

	m_p_t_directories->addField( f=new KexiDB::Field("directory_icon", KexiDB::Field::Text) );
	f->setCaption("directory icon");

	if (!m_conn_ptr->createTable( m_p_t_directories ))
	{
		m_conn_ptr->debugError();
		MYWARNING<<"ERROR creating m_p_t_directories"<<endl;
		return false;
	}
	m_p_t_directories->debug();

	//------------------------------------------------------------------------------
	if (m_driver->transactionsSupported() && !m_conn_ptr->commitTransaction(t))
	{
		MYWARNING<<"ERROR commitTransaction"<<endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
		return false;
	}

	QStringList tnames = m_conn_ptr->tableNames();
	for (QStringList::iterator it = tnames.begin(); it!=tnames.end(); ++it)
	{
	}

 	m_conn_ptr->setAutoCommit(false);
	return fillDatabase();
}

bool
Categories::fillDatabase()
{
	addTopCategory(i18n("Location"), i18n("Some places and locations"), "wp");
	addTopCategory(i18n("People"), i18n("All people you know"), "kdmconfig");
	addTopCategory(i18n("Events"), i18n("Some events"), "knotes");
	addTopCategory(i18n("Keywords"), i18n("All keywords you want"), "personal");

	KexiDB::FieldList list;
	list.addField(m_p_t_media->field("media_id"));
	list.addField(m_p_t_media->field("media_name"));
	list.addField(m_p_t_media->field("media_icon"));
	m_conn_ptr->insertRecord(list, QVariant(0), QVariant("HDD"),  QVariant("hdd_umount"));
	m_conn_ptr->insertRecord(list, QVariant(1), QVariant("CDROM"),  QVariant("cdrom_umount"));

	return true;
}


int
Categories::addTopCategory(const QString& cat_name, const QString& desc, const QString& icon)
{
	KexiDB::FieldList list;
	list.addField(m_p_t_categories->field("category_name"));
	list.addField(m_p_t_categories->field("category_desc"));
	list.addField(m_p_t_categories->field("category_icon"));
	m_conn_ptr->insertRecord(list, QVariant(cat_name), QVariant(desc), QVariant(icon));
	return m_conn_ptr->lastInsertedAutoIncValue("category_id", *m_p_t_categories);
}

int
Categories::addSubCategory(int up_cat, const QString& cat_name, const QString& desc, const QString& icon)
{
	KexiDB::FieldList list;
	list.addField(m_p_t_categories->field("category_name"));
	list.addField(m_p_t_categories->field("category_desc"));
	list.addField(m_p_t_categories->field("category_icon"));
	list.addField(m_p_t_categories->field("category_up"));
	if(m_conn_ptr->insertRecord(list, QVariant(cat_name), QVariant(desc), QVariant(icon), QVariant(up_cat)))
		return m_conn_ptr->lastInsertedAutoIncValue("category_id", *m_p_t_categories);
	else
		return -1;
}




int
Categories::querySingleNumber(const QString& query, bool useParser)
{
	if(!m_conn_ptr)
	{
		MYWARNING<<"!m_conn_ptr"<<endl;
		return -1;
	}

	////
	int id=-1;
	if(useParser)
	{
		KexiDB::Parser parser(m_conn_ptr);
		const bool ok = parser.parse(query );
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q)
		{
			m_conn_ptr->querySingleNumber(m_conn_ptr->selectStatement( *q ), id);
		}
	}
	else
	{
		if(!m_conn_ptr->querySingleNumber(query, id))
		{
		}
	}
	return id;
}

QString
Categories::querySingleString(const QString& query, bool useParser)
{
	if(!m_conn_ptr)
	{
		MYWARNING<<"!m_conn_ptr"<<endl;
		return NULL;
	}
	////
	QString s;
	if(useParser)
	{
		KexiDB::Parser parser(m_conn_ptr);
		const bool ok = parser.parse(query );
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q)
		{
			m_conn_ptr->querySingleString(m_conn_ptr->selectStatement( *q ), s);
		}
	}
	else
	{
		if(!m_conn_ptr->querySingleString(query, s))
		{
		}
	}
	return s;
}

QStringList*
Categories::getImageListId(const QStringList& img_path_list)
{
	if(!isUpdating())
	{

		QStringList m_imglist = img_path_list;
		QStringList m_quoteimglist;
		for ( QStringList::ConstIterator it = m_imglist.begin(); it != m_imglist.end(); ++it )
		{
			m_quoteimglist.append(QString("'%1'").arg(*it));
		}
 		QString query;
		if(m_p_conn_data->driverName.lower() == QString::fromLatin1("mysql"))
		{
			query = QString("SELECT image_id FROM images, directories WHERE image_dir_id =directory_id AND CONCAT(directory_path, '/', image_name) IN (%1) LIMIT %2 ;")
				.arg(m_quoteimglist.join(", "))
				.arg(img_path_list.count());
		 }
		 else
		 {
			query = QString("SELECT image_id FROM images, directories WHERE image_dir_id=directory_id AND directory_path || '/'||image_name IN (%1) LIMIT %2 ;")
				.arg(m_quoteimglist.join(", "))
				.arg(img_path_list.count());
		 }
		return executeQuerry(query, 0, false);
	}
	else
	{
		MYWARNING<<"Unable to get ID because DB is updating"<<endl;
		return new QStringList();
	}
}
int
Categories::getCategoryId(const QString& cat_name)
{
	QString query = QString("SELECT category_id FROM categories WHERE category_name = '%1'  ")
			.arg(cat_name);
	return querySingleNumber(query, false);
}

QStringList*
Categories::getCategoryId(const QStringList& cat_name_list)
{
	QStringList list;
	for ( QStringList::ConstIterator it = cat_name_list.begin(); it != cat_name_list.end(); ++it )
	{
		list.append(QString("'%1'").arg(*it));
	}
	QString query = QString("SELECT category_id FROM categories WHERE category_name IN (%1) LIMIT %2;")
			.arg(list.join(", "))
			.arg(cat_name_list.count());
	return executeQuerry(query, 0, false);
}


int
Categories::getImageId(const QString& ima_path)
{
	QFileInfo info(ima_path);
	return getImageId(info.fileName(), info.dirPath());
}


int
Categories::getImageId(const QString& ima_name, const QString& dir_path)
{
	int id=-1;
	if(!isUpdating())
	{
		QString query = QString("SELECT image_id FROM images, directories WHERE image_dir_id=directory_id AND image_name = '%1' AND directory_path='%2' ")
			.arg(ima_name)
			.arg(dir_path);
		id = querySingleNumber(query, false);
	}
	else
	{
		MYWARNING<<"Unable to get ID because DB is updating"<<endl;
	}
	if(id<0)
	{
// 		MYWARNING<<"Image "<<dir_path<<"/"<<ima_name<<" doe not exist"<<endl;
	}
	return id;
}

int
Categories::getImageId(const QString&  ima_name, int dir_id)
{
	if(dir_id<0)
	{
// 		MYWARNING<<"ERROR in the dir_id "<< dir_id<<endl;
		return -1;
	}
	QString query = QString("SELECT image_id FROM images WHERE image_name='%1' AND image_dir_id=%2  ")
			.arg(ima_name)
			.arg(dir_id);
	int id=-1;
	if(!isUpdating())
		id =  querySingleNumber(query, false);
	if(id<0)
	{
// 		MYWARNING<<"ERROR: can't find file " << ima_name << " in " << dir_id<<endl;
// 		MYWARNING<<query<<endl;
	}
	return id;
}


KexiDB::Cursor*
Categories::getImageEntry(const QString& ima_name, int dir_id)
{
	QString query = QString("SELECT image_id FROM images WHERE image_name='%1' AND image_dir_id=%2  LIMIT 1 ;")
			.arg(ima_name)
			.arg(dir_id);
	return query2ImageListCursor(query);
}


QString
Categories::getCategoryName(int cat_id)
{
	QString query = QString("SELECT category_name FROM categories WHERE category_id = %1  ")
			.arg(cat_id);
	return querySingleString(query, false);
}
QString
Categories::getCategoryIcon(int cat_id)
{
	QString query = QString("SELECT category_icon FROM categories WHERE category_id = %1 ")
			.arg(cat_id);
	return querySingleString(query, false);
}
QString
Categories::getCategoryDescription(int cat_id)
{
	QString query = QString("SELECT category_desc FROM categories WHERE category_id = %1 ")
			.arg(cat_id);
	return querySingleString(query, false);
}




QString
Categories::getImageName(int ima_id)
{
	QString query = QString("SELECT image_name FROM images WHERE image_id = %1  ")
			.arg(ima_id);
	return querySingleString(query, false);
}


QStringList*
Categories::topCategories(void)
{
	QString query=("SELECT category_name FROM categories WHERE category_up = 0 ; ");
	return executeQuerry(query, 0, false);
}

QStringList*
Categories::allCategories(void)
{
	QString query=("SELECT category_id FROM categories ; ");
	return executeQuerry(query, 0, false);
}

void
Categories::printCursor(KexiDB::Cursor *cursor)
{
	QString s="\n";
	cursor->moveFirst();
	while(!cursor->eof())
	{
		for(unsigned int i=0; i<cursor-> fieldCount(); i++)
		{
			QVariant variant = cursor->value(i);
			s+=variant.toString()+" ";
		}
		s+="\n";
		cursor->moveNext();
	}
}


QStringList*
Categories::cursor2stringlist(KexiDB::Cursor * cursor, int col)
{
	QStringList *l = new QStringList();
	if(!cursor) return l;
	cursor->moveFirst();
	while(!cursor->eof())
	{
		l->append (cursor->value(col).toString());
		cursor->moveNext();
	}
	return l;
}


QStringList*
Categories::executeQuerry(const QString& query, int col, bool useParser)
{
//	MYWARNING<<query<<endl;
	if(!m_conn_ptr)
	{
		MYWARNING<<"!m_conn_ptr"<<endl;
		return NULL;
	}

	////
	KexiDB::Cursor *cursor=NULL;
	if(useParser)
	{
		KexiDB::Parser parser(m_conn_ptr);
		const bool ok = parser.parse(query);
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q) cursor = m_conn_ptr->executeQuery(*q);
	}
	else
	{
		cursor = m_conn_ptr->executeQuery(query);
	}
	if(!cursor)
	{
		MYWARNING << "ERROR " << endl;
		m_driver->debugError();
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
	}

	QStringList *l=cursor2stringlist(cursor, col);
	freeCursor(cursor);
	return l;
}


QStringList*
Categories::executeQuerry(KexiDB::QuerySchema& query, int col)
{
	KexiDB::Cursor *cursor = m_conn_ptr->executeQuery( query, KexiDB::Cursor::Buffered );
	if(!cursor)
	{
		MYWARNING << "ERROR " << endl;
		m_driver->debugError();
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
	}

	QStringList *l=cursor2stringlist(cursor, col);
	freeCursor(cursor);
	return l;
}

QStringList*
Categories::subCategories(const QString& cat_name)
{
	QString query = QString("SELECT category_name FROM categories WHERE category_up = (SELECT category_id FROM categories WHERE category_name = '%1');")
			.arg(cat_name);
	return executeQuerry(query, 0, false);
}

bool
Categories::addImages(QPtrList <QPtrList <QVariant> > *imageList,  bool check)
{
	if(imageList->isEmpty()) return true;

	setUpdating(true);

	KexiDB::Transaction t;
	if(m_driver->transactionsSupported())
	{	t = m_conn_ptr->beginTransaction();
		if (m_conn_ptr->error())
		{
			MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
			MYWARNING << m_conn_ptr->errorMsg()<<endl;
			MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
			m_conn_ptr->setAutoCommit(true);
			setUpdating(false);
			return false;
		}
	}

	QPtrList <QVariant> *image;
	int image_id=-1;
	for(image = imageList->first(); image; image=imageList->next())
	{
		image_id = addImage(
				image->at(0)->toString(),
				image->at(1)->toInt(),
				image->at(2)->toDateTime(),
				image->at(3)->toString(),

				check);
		if(image_id>0) m_RecentAddedFileQueue.append(QString::number(image_id));
	}


	if (m_driver->transactionsSupported() && !m_conn_ptr->commitTransaction(t))
	{
		MYWARNING<<"ERROR adding files" <<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
	}

	setUpdating(false);
	return true;
}

int
Categories::addImage(const QString& name, int dir_id, const QDateTime& date, const QString& comment,  bool check)
{
	if(dir_id<=0)
	{
		MYWARNING<<"directory numbered " << dir_id<< " is not valid!"<<endl;
		return -1;
	}

	//------------------------------------------------------------------------------
	if(check)
	{
		int ima_id = getImageId(name, dir_id);
		if(ima_id > 0) return ima_id;
	}

	//------------------------------------------------------------------------------
	KexiDB::FieldList list;
	list.addField(m_p_t_images->field("image_name"));
	list.addField(m_p_t_images->field("image_dir_id"));
	list.addField(m_p_t_images->field("image_date_begin"));
	list.addField(m_p_t_images->field("image_date_end"));
	list.addField(m_p_t_images->field("image_comment"));
	bool success = m_conn_ptr->insertRecord(list,
									  QVariant(name),
									  QVariant(dir_id),
									  QVariant(date),
									  QVariant(date),
									  QVariant(comment));

	if(!success)
	{
		MYWARNING << " ERRROR inserting " << name<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		m_conn_ptr->debugError();
		return -1;
	}
	else
	{
		return m_conn_ptr->lastInsertedAutoIncValue("image_id", *m_p_t_images);
	}
}

int
Categories::addDirectory(const QString& path)
{
	QString query = QString("SELECT directory_id FROM directories WHERE directory_path='%1'  ")
			.arg(path);
	KexiDB::RowData data;
	if(m_conn_ptr->querySingleRecord(query, data))
	{
		MYWARNING<<"Directory " << path << " already exists!"<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		int id = data[0].toInt();
		return id;
	}
	KexiDB::FieldList list;
	list.addField(m_p_t_directories->field("directory_path"));
	bool success = m_conn_ptr->insertRecord(list, QVariant(path));
// 	query = QString("INSERT INTO directories(directory_path) VALUES ('%1');")
// 		.arg(path);
// 	bool success = m_conn_ptr->executeQuery( query );

	if(!success)
	{
		MYWARNING << " ERROR inserting "<<path<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		MYWARNING << query << endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;
		return -1;
	}
	else
		return m_conn_ptr->lastInsertedAutoIncValue("directory_id", *m_p_t_directories);
}

int
Categories::deleteDirectory(int dir_id)
{
	QString query = QString("SELECT 1 FROM images WHERE image_dir_id = %1  ")
			.arg(dir_id);
	KexiDB::RowData data;
	if(m_conn_ptr->querySingleRecord(query, data))
	{
		MYWARNING<<"Directory " << getDirectoryPath(dir_id)<<"-"<<dir_id <<   " is not empty!"<<endl;
		return -1;
	}
	//------------------------------------------------------------------------------
	query = QString("DELETE FROM directories WHERE directory_id = %1 ;")
			.arg(dir_id);
	return m_conn_ptr->executeSQL( query );
}

int
Categories::getDirectoryId(const QString& path)
{
	QString query = QString("SELECT directory_id FROM directories WHERE directory_path='%1' ")
			.arg(path);
	return querySingleNumber(query, false);
}
QString
Categories::getDirectoryPath(int dir_id)
{
	QString query = QString("SELECT directory_path FROM directories WHERE directory_id=%1 ")
			.arg(dir_id);
	return querySingleString(query, false);
}



KexiDB::Cursor*
Categories::query2ImageListCursor(const QString& query)
{
// 	MYWARNING<<query<<endl;
	if(!m_conn_ptr)
	{
		MYWARNING<<"!m_conn_ptr"<<endl;
		return NULL;
	}
	return imageIdList2ImageList(m_conn_ptr->executeQuery( query ));
}

KexiDB::Cursor*
Categories::query2ImageListCursor(KexiDB::QuerySchema& query)
{
	return imageIdList2ImageList(m_conn_ptr->executeQuery( query ));
}


KexiDB::Cursor *
Categories::imageIdList2ImageList(KexiDB::Cursor *cursor)
{
// 	MYWARNING<<endl;
	if (!cursor) return NULL;
	if (cursor->eof()) return NULL;

	QString query= "SELECT DISTINCT image_id, image_name, image_dir_id, image_comment, image_note, image_date_begin, image_date_end FROM images WHERE image_id IN (";
	cursor->moveFirst();
	while(!cursor->eof())
	{
		query += cursor->value(0).toString();
		cursor->moveNext();
		if(!cursor->eof()) query += ", ";
	}
	query += " );";
	KexiDB::Cursor *tmp_cursor = m_conn_ptr->executeQuery( query );
	if(!tmp_cursor)
	{
		MYWARNING << "ERROR " << endl;
		m_driver->debugError();
		MYWARNING << " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		MYWARNING << m_conn_ptr->errorMsg()<<endl;
		MYWARNING << m_conn_ptr->serverErrorMsg()<<endl;

	}
	else
	{
	}
	return tmp_cursor;
}

KexiDB::Cursor *
Categories::imageIdList2ImageList(const QStringList& image_id_list)
{
	if (image_id_list.isEmpty()) return NULL;

	QString query = QString("SELECT DISTINCT image_id, image_name, image_dir_id, image_comment, image_note, image_date_begin, image_date_end FROM images WHERE image_id IN (%1)")
		.arg(image_id_list.join(", "));
	return  m_conn_ptr->executeQuery( query );
}


KexiDB::Cursor *
Categories::imagesCategoriesList(const QStringList& tab)
{
	QString query = QString("SELECT DISTINCT imacat_ima_id FROM image_category WHERE imacat_cat_id IN (%1);")
			.arg(tab.join(", "));
	return query2ImageListCursor(query);
}


KexiDB::Cursor*
Categories::imagesCategoriesList_OR(QPtrList<QStringList>& l)
{
	QStringList list;
	 for(QStringList *id_list = l.first();id_list;id_list=l.next())
		list+=*id_list;
	return imagesCategoriesList(list);
}

KexiDB::Cursor*
Categories::imagesCategoriesList_AND(QPtrList<QStringList>& l)
{
	QString query = QString("SELECT imacat_ima_id "
			"FROM  image_category "
			"WHERE imacat_cat_id IN (%1) ").
			arg(l.at(0)->join(", "));
	QStringList *image_id_list = executeQuerry(query, false, 0);
	for(unsigned int i=1; i<l.count(); i++)
	{
		query = QString(
			"SELECT imacat_ima_id "
			"FROM image_category "
			"WHERE imacat_cat_id IN (%1) AND imacat_ima_id IN (%2) ")
				.arg(l.at(i)->join(", "))
				.arg(image_id_list->join(", "));
		if(i<l.count()-1)
			image_id_list = executeQuerry(query, false, 0);
	}
	return query2ImageListCursor(query);

}







KexiDB::Cursor*
Categories::imagesCommentList(const QString& comment)
{
	QString query = "SELECT image_id FROM images WHERE image_comment LIKE '%";
	query += comment;
	query += "%' ;";
	return query2ImageListCursor(query);
}


KexiDB::Cursor*
Categories::imagesPatternList(const QStringList& patterns,
				const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	QString query = QString("SELECT DISTINCT image_id FROM images WHERE ");
	for( unsigned int i=0; i<patterns.count()-1; i++)
	{
		query += QString(" (image_name LIKE '%%1%') ").arg(*patterns.at(i));
		if(mode == mode_OR)
			query += " OR ";
		else
			query += " AND ";
	}
	query += QString("image_name LIKE '%%1%' ").arg(*patterns.at(patterns.count()-1));

	///
	if(!iiList.isEmpty())
	{
		if(mode == mode_OR)
			query += " OR ( ";
		else
			query += " AND ( ";

		query += " image_id IN (";
		QPtrList<QVariant> m_iiList(iiList);
		for( unsigned int i=0; i<m_iiList.count()-1; i++)
		{
			query += QString("%1, ").arg(m_iiList.at(i)->toInt());
		}
		query += QString("%1").arg(m_iiList.at(m_iiList.count()-1)->toInt());
		query += ") )";
	}
	///
	query += ";";
	return query2ImageListCursor(query);
}

KexiDB::Cursor*
Categories::imagesDateList(const QDate& date, int bia,
						   const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	QString query = "SELECT DISTINCT image_id FROM images WHERE DATE(image_date_begin)%1'%2' ";
	QString sign;
	if(bia<0)
		sign = "<=";
	else if(bia==0)
		sign = "=";
	else
		sign= ">=";
	query = query.arg(sign).arg(date.toString(m_date_format));
	///
	if(!iiList.isEmpty())
	{
		if(mode == mode_OR)
			query += " OR ";
		else
			query += " AND ";

		query += " image_id IN (";
		QPtrList<QVariant> m_iiList(iiList);
		for( unsigned int i=0; i<m_iiList.count()-1; i++)
		{
			query += QString("%1, ").arg(m_iiList.at(i)->toInt());
		}
		query += QString("%1").arg(m_iiList.at(m_iiList.count()-1)->toInt());
		query += ")";
	}
	///
	query += ";";

	return query2ImageListCursor(query);
}

KexiDB::Cursor*
Categories::imagesDateList(const QDate& date_begin, const QDate& date_end,
						   const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	QString
			sdate_begin=date_begin.toString(m_date_format),
			sdate_end  =date_end.toString(m_date_format);

	QString query = "SELECT DISTINCT  image_id FROM images WHERE ";
	query += "(";
	query += QString("(date(image_date_begin) <= '%1' AND date(image_date_begin) >= '%2' )").arg(sdate_begin).arg(sdate_end);
	query += QString(" OR (date(image_date_begin) >= '%1' AND date(image_date_begin) <= '%2' )").arg(sdate_begin).arg(sdate_end);

	query += QString(" OR (date(image_date_begin)<= '%1' AND date(image_date_begin) BETWEEN '%1' AND '%3' )").arg(sdate_begin).arg(sdate_end).arg(sdate_end);
	query += QString(" OR (date(image_date_begin) BETWEEN '%1' AND '%2' AND date(image_date_end) >= '%3' )").arg(sdate_begin).arg(sdate_end).arg(sdate_end);
	query += ")";

	///
	if(!iiList.isEmpty())
	{
		if(mode == mode_OR)
			query += " OR ";
		else
			query += " AND ";

		query += " image_id IN (";
		QPtrList<QVariant> m_iiList(iiList);
		for( unsigned int i=0; i<m_iiList.count()-1; i++)
		{
			query += QString("%1, ").arg(m_iiList.at(i)->toInt());
		}
		query += QString("%1").arg(m_iiList.at(m_iiList.count()-1)->toInt());
		query += ")";
	}
	///
	query +=";";

	return query2ImageListCursor(query);
}



void
Categories::freeCursor(KexiDB::Cursor* cursor)
{
	m_conn_ptr->deleteCursor(cursor);
}

int
Categories::addLink(const QStringList& imgid_list, int cat_id)
{
	KexiDB::Transaction t;
	if(m_driver->transactionsSupported())
	{
		t = m_conn_ptr->beginTransaction();
		if (m_conn_ptr->error())
		{
			m_conn_ptr->debugError();
			return false;
		}
	}
	//--------------------------------------------------------------------------
	for ( QStringList::ConstIterator it = imgid_list.begin(); it != imgid_list.end(); ++it )
	{
		addLink((*it).toInt(), cat_id);
	}
	//--------------------------------------------------------------------------
	if (m_driver->transactionsSupported() && !m_conn_ptr->commitTransaction(t))
	{
		m_conn_ptr->debugError();
		return false;
	}

	return 0;
}

int
Categories::addLink(const QStringList& imgid_list, const QStringList& cat_id_list)
{
	if(cat_id_list.isEmpty() || imgid_list.isEmpty()) return 0;

	KexiDB::Transaction t;
	if(m_driver->transactionsSupported())
	{
		t = m_conn_ptr->beginTransaction();
		if (m_conn_ptr->error())
		{
			m_conn_ptr->debugError();
			return false;
		}
	}
	//--------------------------------------------------------------------------
	KexiDB::FieldList list;
	list.addField(m_p_t_image_category->field("imacat_ima_id"));
	list.addField(m_p_t_image_category->field("imacat_cat_id"));
	for ( QStringList::ConstIterator img_it = imgid_list.begin(); img_it != imgid_list.end(); ++img_it )
	{
		QStringList *img_cat_id = imageLinks((*img_it).toInt());
		QStringList cat_id_to_add = cat_id_list;
		for ( QStringList::Iterator cat_it = img_cat_id->begin(); cat_it != img_cat_id->end(); ++cat_it )
			cat_id_to_add.remove(*cat_it);

		QStringList values;
		for ( QStringList::Iterator cat_it = cat_id_to_add.begin(); cat_it != cat_id_to_add.end(); ++cat_it )
			if(!m_conn_ptr->insertRecord(list, QVariant(*img_it), QVariant(*cat_it)))
			{
				MYWARNING<<"ERROR inserting link"<<endl;
				MYWARNING<< " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
			}
		emit sigLinkAdded();
	}
	//--------------------------------------------------------------------------
	if(m_driver->transactionsSupported() && !m_conn_ptr->commitTransaction(t))
	{
		m_conn_ptr->debugError();
		return false;
	}

	return 0;
}

int
Categories::addLink(int image_id, int cat_id)
{
	QString query = QString("SELECT 1 FROM image_category WHERE imacat_ima_id=%1 AND imacat_cat_id=%2  ")
			.arg(image_id)
			.arg(cat_id);
	KexiDB::RowData data;
	//FIXME  must be checked before
	if(m_conn_ptr->querySingleRecord(query, data))
	{
		MYWARNING<<"Link already exists"<<endl;
		emit sigLinkAdded();
		return -1;
	}

	int ret;
	KexiDB::FieldList list;
	list.addField(m_p_t_image_category->field("imacat_ima_id"));
	list.addField(m_p_t_image_category->field("imacat_cat_id"));
	if(!m_conn_ptr->insertRecord(list, QVariant(image_id), QVariant(cat_id)))
	{
		MYWARNING<<"ERROR inserting link"<<endl;
		MYWARNING<< " RECENT SQL STATEMENT: " << m_conn_ptr->recentSQLString() << endl;
		ret = -1;
	}
	else
	{
		ret = 0;
	}
	emit sigLinkAdded();
	return ret;
}

int
Categories::addLink(const QString& image_name, const QString& image_path, const QString& cat_name)
{
	return addLink(getImageId(image_name, image_path), getCategoryId(cat_name));
}

QStringList*
Categories::imageLinks(int image_id, bool getCatName)
{
	QStringList* imageCats;
	if(image_id<0)
	{
		//MYWARNING<<"image has wrong id " << image_id << endl;
		imageCats = new QStringList();
	}
	else
	{

		QString query = QString("SELECT imacat_cat_id FROM image_category WHERE imacat_ima_id = %1;")
			.arg(image_id);
		imageCats = executeQuerry(query, 0, false);
		if(imageCats->isEmpty())
		{
		}
		if(getCatName && !imageCats->isEmpty())
		{
			query = QString("SELECT category_name FROM categories WHERE category_id IN (%1) ;")
				.arg(imageCats->join(", "));
			imageCats = executeQuerry(query, 0, false);
		}

	}
	return imageCats;
}

QStringList*
Categories::imageLinks(const QStringList& image_id_list, bool getCatName, bool distinct)
{
	QStringList* imageCats;
	if(image_id_list.isEmpty())
	{
 		MYWARNING<<"image_id_list is wrong " << endl;
		imageCats = new QStringList();
	}
	else
	{

		QString query = QString("SELECT %1 imacat_cat_id FROM image_category WHERE imacat_ima_id IN (%1) ;")
			.arg(distinct?" DISTINCT ": " ")
			.arg(image_id_list.join(", "));
		imageCats = executeQuerry(query, 0, false);
		if(imageCats->isEmpty())
		{
		}
		if(getCatName && !imageCats->isEmpty())
		{
			query = QString("SELECT category_name FROM categories WHERE category_id IN (%1) ;")
				.arg(imageCats->join(", "));
			imageCats = executeQuerry(query, 0, false);
		}

	}
	return imageCats;
}


KexiDB::Cursor*
Categories::imagesNoteList(int note, int lem)
{
	QString query = QString ("SELECT image_id FROM images WHERE image_note %1 %2 AND image_note > 0 ;");
	QString sign;
	if(lem<0)
		sign = " <= ";
	else if (lem==0)
		sign = " = ";
	else
		sign = " >= ";
	query = query.arg(sign).arg(note);

	return query2ImageListCursor(query);
}

KexiDB::Cursor*
Categories::imagesNoteList(const QStringList& noteList,
			const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	QString query = QString("SELECT DISTINCT image_id FROM images WHERE ");
	if(mode == mode_OR)
	{
		query += QString(" (image_note IN (%1) ) ")
					.arg(noteList.join(", "));
	}
	else
	{
		int min = noteList.first().toInt();
		int max = noteList.first().toInt();
		for ( QStringList::ConstIterator it = noteList.begin(); it != noteList.end(); ++it )
		{
			if((*it).toInt()<min)
				min=(*it).toInt();
			else
			if((*it).toInt()>max)
				max=(*it).toInt();
		}
		query += QString(" (image_note BETWEEN %1 AND %2 ) ").arg(min).arg(max);
	}
	///
	if(!iiList.isEmpty())
	{
		if(mode == mode_OR)
			query += " OR ";
		else
			query += " AND ";

		query += " image_id IN (";
		QPtrList<QVariant> m_iiList(iiList);
		for( unsigned int i=0; i<m_iiList.count()-1; i++)
		{
			query += QString("%1, ").arg(m_iiList.at(i)->toInt());
		}
		query += QString("%1").arg(m_iiList.at(m_iiList.count()-1)->toInt());
		query += ")";
	}
	///
	query +=";";
	return query2ImageListCursor(query);
}

int
Categories::moveImages(const KURL::List& /*fileurls*/, const KURL& /*desturl*/)
{
	return 1;
}

int
Categories::moveImage(const QString& old_fullpath, const QString& new_path)
{
	return moveImage(getImageId(old_fullpath), new_path);
}

int
Categories::moveImage(const QString& old_fullpath, int dir_id)
{
	return moveImage(getImageId(old_fullpath), dir_id);
}

int
Categories::moveImage(int ima_id, const QString& new_path)
{
	int dir_id = getDirectoryId(new_path);
	if(dir_id < 0)
		dir_id = addDirectory(new_path);
	return moveImage(ima_id, dir_id);
}


bool
Categories::updateImageInformations(int image_id, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories)
{

	QString query = QString("UPDATE images SET image_comment='%1', image_note=%2, image_date_begin = '%3 ', image_date_end = '%4' WHERE image_id = %5 ;")
			.arg(comment)
			.arg(note)
			.arg(date_begin.toString(m_datetime_format))
			.arg(date_end.toString(m_datetime_format))
			.arg(image_id)
			;
	m_conn_ptr->executeSQL( query );

	/////////////////
	/////////////////
	deleteCategoryImage(image_id, removedCategories );

	/////////////////
	for ( QStringList::ConstIterator cat_id = addedCategories.begin(); cat_id != addedCategories.end(); ++cat_id )
	{
		addLink(image_id, (*cat_id).toInt());
	}
	return true;
}

bool
Categories::updateImageInformations(const QStringList& image_id_list, const QString& comment, int note, const QDateTime& date_begin, const QDateTime& date_end, const QStringList& removedCategories, const QStringList& addedCategories)
{
	QStringList updates;
	if(!comment.isNull())    updates.append(QString("image_comment='%1'").arg(comment));
	if(note>-1)              updates.append(QString("image_note=%1").arg(note));
	if(date_begin.isValid()) updates.append(QString("image_date_begin='%1'").arg(date_begin.toString(m_datetime_format)));
	if(date_end.isValid())   updates.append(QString("image_date_end='%1'").arg(date_end.toString(m_datetime_format)));
	if(!updates.isEmpty()&&!image_id_list.isEmpty())
	{
		QString query = QString("UPDATE images SET %1 WHERE image_id in (%2) ;" )
					.arg(updates.join(", "))
					.arg(image_id_list.join(", "));
		m_conn_ptr->executeSQL( query );
	}

	deleteCategoryImage(image_id_list, removedCategories );

	/////////////////
	addLink(image_id_list, addedCategories);
	return true;
}

int
Categories::moveImage(int ima_id, int dir_id)
{
	if(dir_id<0)
	{
		MYWARNING<<"directories has wrong id="<<dir_id<<endl;
		return -1;
	}
	QString query = QString("UPDATE images SET image_dir_id = '%1' WHERE image_id = %2 ;")
			.arg(dir_id)
			.arg(ima_id);
	return m_conn_ptr->executeSQL( query );
}

int
Categories::renameImage(const QString& oldfullname, const QString& newfullname)
{
	QFileInfo oldinfo(oldfullname);
	QFileInfo newinfo(newfullname);
	int id = getImageId(oldfullname);
	(void)renameImage(id, newinfo.fileName());
	if(oldinfo.dirPath() != newinfo.dirPath())
		moveImage(id, newinfo.dirPath());
	return 1;
}


int
Categories::renameImage(int id, const QString& new_name)
{
	QString query = QString("UPDATE images SET image_name = '%1' WHERE image_id = %2 ;")
			.arg(new_name)
			.arg(id);
	return m_conn_ptr->executeSQL( query );
}

bool
Categories::renameCategory(int id, const QString& new_name)
{
	QString query = QString("UPDATE categories SET category_name = '%1' WHERE category_id = %2;")
			.arg(new_name)
			.arg(id);
	return m_conn_ptr->executeSQL( query ) == 1;
}


bool
Categories::setCategoryDescription(int id, const QString& descr)
{
	QString query = QString("UPDATE categories SET category_desc = '%1' WHERE category_id = %2;")
			.arg(descr)
			.arg(id);
	return m_conn_ptr->executeSQL( query ) == 1;
}

bool
Categories::setCategoryIcon(int id, const QString& icon)
{
	QString query = QString("UPDATE categories SET category_icon = '%1' WHERE category_id = %2;")
			.arg(icon)
			.arg(id);
	return m_conn_ptr->executeSQL( query ) == 1;
}

int
Categories::moveDirectory(const QString& old_dirPath, const QString& old_dirName, const QString& dest_path)
{
	QString old_fullpath = QString("%1/%2").arg(old_dirPath).arg(old_dirName);
	QString new_fullpath = QString("%1/%2").arg(dest_path).arg(old_dirName);

	QString query = QString("UPDATE directories SET directory_path='%1' WHERE directory_path = '%2' ;")
			.arg(new_fullpath)
			.arg(old_fullpath);
	m_conn_ptr->executeSQL( query );

	int old_dirPath_len = old_dirPath.length()+1;
	old_fullpath+="/%";
	if(m_p_conn_data->driverName.lower() == QString::fromLatin1("mysql"))
	{
		query = QString("UPDATE directories SET directory_path = CONCAT('%1', SUBSTR(directory_path, %2, LENGTH(directory_path))) WHERE directory_path LIKE '%3' ; ")
				.arg(dest_path)
				.arg(old_dirPath_len)
				.arg(old_fullpath);
	}
	else
	{
		query = QString("UPDATE directories SET directory_path = '%1' || SUBSTR(directory_path, %2, LENGTH(directory_path)) WHERE directory_path LIKE '%3' ; ")
				.arg(dest_path)
				.arg(old_dirPath_len)
				.arg(old_fullpath);
	}
	m_conn_ptr->executeSQL( query );

	return 1;
}


int
Categories::renameDirectory(const QString& old_path, const QString& new_path)
{
	QString query = QString("UPDATE directories SET directory_path='%2' WHERE directory_path='%2' ; ")
			.arg(new_path)
			.arg(old_path);
	m_conn_ptr->executeSQL( query );

	if(m_p_conn_data->driverName.lower() == QString::fromLatin1("mysql"))
	{
		query = QString("UPDATE directories SET directory_path = CONCAT('%1', '/', SUBSTR(directory_path, LENGTH('%2')+2, LENGTH(directory_path)-LENGTH('%3')+1)) WHERE directory_path LIKE '%4/%' ; ")
				.arg(new_path)
				.arg(old_path)
				.arg(old_path)
				.arg(old_path);
	}
	else
	{
		query = QString("UPDATE directories SET directory_path='%1' || '/' || SUBSTR(directory_path, LENGTH('%2')+2, LENGTH(directory_path)-LENGTH('%3')+1) WHERE directory_path LIKE '%4/%' ; ")
				.arg(new_path)
				.arg(old_path)
				.arg(old_path)
				.arg(old_path);

	}
	m_conn_ptr->executeSQL( query );

	return 1;
}

int
Categories::deleteNodeCategory(int id)
{
	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id = %1 ;")
			.arg(id);
	int res = m_conn_ptr->executeSQL( query );
	if (res != 0)
		return res;

	query = QString("DELETE FROM categories WHERE category_id = %1 ;")
			.arg(id);

	return m_conn_ptr->executeSQL( query );
}

int
Categories::setNewParent(int id, int new_parent)
{
	QString query = QString("UPDATE categories SET category_up = %1  WHERE category_id = %2;")
			.arg(new_parent)
			.arg(id);

	return  m_conn_ptr->executeSQL( query );
}

int
Categories::deleteImage(int ima_id)
{
	QString query = QString("DELETE FROM image_category WHERE imacat_ima_id = %1 ;")
			.arg(ima_id);
	int res = m_conn_ptr->executeSQL( query );
	if (res != 0)
		return res;

	query = QString("DELETE FROM images WHERE image_id = %1 ;")
			.arg(ima_id);

	return m_conn_ptr->executeSQL( query );
}
int
Categories::deleteImage(const QStringList& ima_id_list)
{
	QString query = QString("DELETE FROM image_category WHERE imacat_ima_id IN (%1) ")
			.arg(ima_id_list.join(", "));
	m_conn_ptr->executeSQL( query );


	query = QString("DELETE FROM images WHERE image_id IN (%1) ")
			.arg(ima_id_list.join(", "));

	return m_conn_ptr->executeSQL( query );
}

int
Categories::deleteCategoryImage(int ima_id, int cat_id)
{
	if(ima_id<0 ||cat_id<0 ) return 0;
	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id = %1 AND imacat_ima_id = %2 ;")
			.arg(cat_id)
			.arg(ima_id);

	return m_conn_ptr->executeSQL( query );
}

int
Categories::deleteCategoryImage(int ima_id, const QStringList& catid_list)
{
	if(ima_id<0 ||catid_list.isEmpty() ) return 0;

	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id IN (%1) AND imacat_ima_id = %2 ;")
			.arg(catid_list.join(", "))
			.arg(ima_id);

	return m_conn_ptr->executeSQL( query );
}

int
Categories::deleteCategoryImage(const QStringList& imaid_list, const QStringList& catid_list)
{
	if(imaid_list.isEmpty() || catid_list.isEmpty()) return 0;

	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id IN (%1) AND imacat_ima_id IN (%2) ;")
			.arg(catid_list.join(", "))
			.arg(imaid_list.join(", "));

	return m_conn_ptr->executeSQL( query );
}

int
Categories::setImageComment(int id, const QString& comment)
{
	QString query = QString("UPDATE images SET image_comment = '%1' WHERE image_id = %2 ;")
			.arg(comment)
			.arg(id);

	return m_conn_ptr->executeSQL( query );
}

int
Categories::setImageNote(int id, int note)
{
	QString query = QString("UPDATE images SET image_note =  %1 WHERE image_id = %2 ;")
			.arg(note)
			.arg(id);

	return m_conn_ptr->executeSQL( query );
}

int
Categories::setImageNote(const QStringList& image_id_list, int note)
{
	QString query = QString("UPDATE images SET image_note =  %1 WHERE image_id IN (%2) ")
			.arg(note)
			.arg(image_id_list.join(", "));

	return m_conn_ptr->executeSQL( query );
}


int
Categories::setImageDate(int id, const QDateTime&  begin, const QDateTime& end)
{
	QString query = QString("UPDATE images SET image_date_begin = '%1 ', image_date_end = '%2' WHERE image_id = %3 ;")
			.arg(begin.toString(m_datetime_format))
			.arg(end.toString(m_datetime_format))
			.arg(id);

	return  m_conn_ptr->executeSQL( query );
}

KexiDB::Cursor*
Categories::allImages()
{
	KexiDB::QuerySchema query(m_p_t_images);
	query.clear();
	query.addField(m_p_t_images->field("image_id"));
	return query2ImageListCursor(query);
}

QDateTime
Categories::getOldestImage()
{
	QString query = "SELECT MIN(image_date_begin) FROM images";
	return QDateTime::fromString(querySingleString(query, false), Qt::ISODate);
}

QDateTime
Categories::getNewestImage()
{
	QString query = "SELECT MAX(image_date_end) FROM images";
	return QDateTime::fromString(querySingleString(query, false), Qt::ISODate);
}


QString
Categories::formatDateTime(const QString& format, const QString& field )
{
	if(m_p_conn_data->driverName.lower()==QString::fromLatin1("mysql"))
	{
		return QString("DATE_FORMAT(")+field+QString(", '")+format+QString("')");
	}
	else
		return QString("STRFTIME('")+format+QString("', ")+field+QString(")");
}

int
Categories::getNumberOfImageForDate(int year, int month, int day)
{
	QString query = "SELECT COUNT(*) FROM images WHERE ";
	if(day==-1)
	{
		if(month==-1)
		{
			query += QString("%1 = '%2' ")
				.arg(formatDateTime("%Y","image_date_begin"))
				.arg(year);
		}
		else
		{
			query += QString("%1 = '%2%3%4' ")
				.arg(formatDateTime("%Y%m", "image_date_begin"))
				.arg(year)
				.arg(month<10?"0":"").arg(month);
		}
	}
	else
	{
		query += QString("%1 = '%2%3%4%5%6' ")
			.arg(formatDateTime("%Y%m%d", "image_date_begin"))
			.arg(year)
			.arg(month<10?"0":"").arg(month)
			.arg(day<10?"0":"").arg(day);
	}
	int num = querySingleNumber(query, false);
	return num;
}

//-----------------------------------------------------------------------------
int
Categories::getNumberOfImages()
{
	QString query = "SELECT COUNT(*) FROM images  ";
	int num = querySingleNumber(query, false);
	return num;
}

int
Categories::getMaxImageId()
{
	QString query = "SELECT MAX(image_id) FROM images  ";
	int num = querySingleNumber(query, false);
	return num;
}

QPtrVector<QString>
Categories::getAllImageFullPath()
{
	int total = getMaxImageId()+1;
	QPtrVector<QString> imagePaths(total);
	QString query("SELECT image_id, ");
	if(m_p_conn_data->driverName.lower() == QString::fromLatin1("mysql"))
	{
		query += QString("CONCAT(directory_path, '/', image_name)");
	}
	else
	{
		query += QString("directory_path || '/' || image_name");
	}
	query += " FROM directories, images WHERE directory_id=image_dir_id";
	KexiDB::Cursor *cursor = m_conn_ptr->executeQuery( query );
	if(!cursor) return imagePaths;

	cursor->moveFirst();
	while(!cursor->eof())
	{
		int id = cursor->value(0).toInt();
		QString *path = new QString(cursor->value(1).toString());
		imagePaths.insert(id, path);
		cursor->moveNext();
	}
	return imagePaths;

}

//-----------------------------------------------------------------------------
void
Categories::resetRecentAddedFileQueue()
{
	m_RecentAddedFileQueue.clear();
}
QStringList
Categories::getRecentAddedFileQueue()
{
	return m_RecentAddedFileQueue;
}


#include "categories.moc"

