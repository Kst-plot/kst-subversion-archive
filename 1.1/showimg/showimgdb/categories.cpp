/***************************************************************************
                         categories.cpp  -  description
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
#include "categories.h"

// KDE
#include <kdebug.h>
#include <klocale.h>

// QT
#include <qdatetime.h>
#include <qdir.h>

#define MYDEBUG kdDebug()<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "
#define MYWARNING kdWarning()<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

Categories::Categories(void)
	: m_updating(false)
{
	datetime_format = "yyyy-MM-dd hh:mm:ss";
	date_format = "yyyy-MM-dd";

	QString db_name(QDir::homeDirPath()+"/.showimg/MyCategoiesDB.sidb");
	QString drv_name("SQLite2");

// 	MYDEBUG<<endl;
	driver = manager.driver(drv_name);
// 	MYDEBUG<<endl;
	conn_data.setFileName( db_name );
// 	MYDEBUG<<endl;

// 	QValueList<QCString> names = driver->propertyNames();
// 	MYDEBUG << QString("%1 properties found:").arg(names.count()) << endl;
// 	for (QValueList<QCString>::ConstIterator it = names.constBegin(); it!=names.constEnd(); ++it) {
// 		MYDEBUG << " - " << (*it) << ":"
// 			<< " caption=\"" << driver->propertyCaption(*it) << "\""
// 			<< " type=" << driver->propertyValue(*it).typeName()
// 			<< " value=\""<<driver->propertyValue(*it).toString()<<"\"" << endl;
// 	}



// 	MYDEBUG<<endl;
	conn = driver->createConnection(conn_data);
// 	MYDEBUG<<endl;

	if (!conn || driver->error())
	{
		driver->debugError();
	}
	if (!conn->connect())
	{
		conn->debugError();
	}
	if(!conn->databaseExists( db_name ))
	{
		if (!createDatabase( db_name ))
		{
			conn->debugError();
		}
	}
	else
	{
		if (!conn->useDatabase( db_name ))
		{
			conn->debugError();
		}
		else
		{
			t_categories = conn->tableSchema("categories");
// 			MYDEBUG<<endl;
			t_images = conn->tableSchema("images");
// 			MYDEBUG<<endl;
			t_image_category = conn->tableSchema("image_category");
			t_directories = conn->tableSchema("directories");
		}
	}
// 	MYDEBUG<<"All created/used"<<endl;
/*
	sqlite_exec(db, "CREATE TABLE directories (dir_id INTEGER PRIMARY KEY, dir_path VARCHAR(256), dir_icon VARCHAR(256));", NULL, NULL, NULL);
*/
}

Categories::~Categories(void)
{
	if (!conn->closeDatabase())
	        conn->debugError();
	if (conn )conn->disconnect();
}


bool
Categories:: createDatabase(const QString& db_name)
{
	if (!conn->createDatabase( db_name ))
	{
		conn->debugError();
		return false;
	}
	//------------------------------------------------------------------------------
	MYDEBUG << "DB '" << db_name << "' created"<< endl;
	if (!conn->useDatabase( db_name ))
	{
		conn->debugError();
		return false;
	}
	//------------------------------------------------------------------------------
// 	conn->setAutoCommit(false);
	KexiDB::Transaction t = conn->beginTransaction();
	if (conn->error())
	{
		conn->debugError();
		return false;
	}
	//------------------------------------------------------------------------------
	//sqlite_exec(db, "CREATE TABLE categories (category_name VARCHAR(256) UNIQUE, category_id INTEGER PRIMARY KEY, category_up UNSIGNED INTEGER);", NULL, NULL, NULL);
	KexiDB::Field *f;
	t_categories = new KexiDB::TableSchema("categories");
	t_categories->setCaption("All categories");

	t_categories->addField( f=new KexiDB::Field("category_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned));
	f->setCaption("category id");

	t_categories->addField( f=new KexiDB::Field("category_name", KexiDB::Field::Text, KexiDB::Field::Unique) );
	f->setCaption("Name");

	t_categories->addField( f=new KexiDB::Field("category_desc", KexiDB::Field::Text) );
	f->setCaption("Description");

	t_categories->addField( f=new KexiDB::Field("category_icon", KexiDB::Field::Text) );
	f->setCaption("Icon");

	t_categories->addField( f=new KexiDB::Field("category_up", KexiDB::Field::Integer, 0, KexiDB::Field::Unsigned, 0, 0, QVariant(0)) );
	f->setCaption("Parent category id");

	if (!conn->createTable( t_categories ))
	{
		conn->debugError();
		return false;
	}
	MYDEBUG << "-- categories created --" << endl;
	t_categories->debug();
	//------------------------------------------------------------------------------
	t_media = new KexiDB::TableSchema("media");
	t_media->setCaption("All media");

	t_media->addField( f=new KexiDB::Field("media_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("media id");

	t_media->addField( f=new KexiDB::Field("media_name", KexiDB::Field::Text) );
	f->setCaption("media name");

	t_media->addField( f=new KexiDB::Field("media_icon", KexiDB::Field::Text) );
	f->setCaption("media name");

	//------------------------------------------------------------------------------
	//	sqlite_exec(db, "CREATE TABLE images (image_id INTEGER PRIMARY KEY, image_name VARCHAR(256) NOT NULL, image_directory_id INTEGER NOT NULL DEFAULT 1, image_comment SALLTEXT DEFAULT '', image_note INTEGER DEFAULT -1, image_date_begin DATETIME DEFAULT '1900-01-01T00:00:00', image_date_end DATETIME DEFAULT '1900-01-01T00:00:00', UNIQUE(image_directory_id, image_name) );", NULL, NULL, NULL);
	t_images = new KexiDB::TableSchema("images");
	t_images->setCaption("All images");

	t_images->addField( f=new KexiDB::Field("image_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("image id");

	t_images->addField( f=new KexiDB::Field("image_name", KexiDB::Field::Text) );
	f->setCaption("image name");

	t_images->addField( f=new KexiDB::Field("image_dir_id", KexiDB::Field::Integer) );
	f->setCaption("image directory id");

	t_images->addField( f=new KexiDB::Field("image_media_id", KexiDB::Field::Integer, 0, 0, 0, 0,
						QVariant(0)) );
	f->setCaption("image media id");

	t_images->addField( f=new KexiDB::Field("image_comment", KexiDB::Field::Text) );
	f->setCaption("image name");

	t_images->addField( f=new KexiDB::Field("image_note", KexiDB::Field::Integer) );
	f->setCaption("image note");

	t_images->addField( f=new KexiDB::Field("image_date_begin", KexiDB::Field::DateTime, 0, 0, 0, 0,
						 QVariant(QDateTime(QDate(1977, 1, 1), QTime(0, 0, 0)))) );
	f->setCaption("image begin date");

	t_images->addField( f=new KexiDB::Field("image_date_end", KexiDB::Field::DateTime, 0, 0, 0, 0,
						 QVariant(QDateTime(QDate(1977, 1, 1), QTime(0, 0, 0)))) );
	f->setCaption("image end date");

	if (!conn->createTable( t_images ))
	{
		conn->debugError();
		return false;
	}
	MYDEBUG << "-- t_images created --" << endl;
	t_images->debug();

	//------------------------------------------------------------------------------
	//sqlite_exec(db, "CREATE TABLE links (link_image UNSIGNED INTEGER, link_category  UNSIGNED INTEGER, PRIMARY KEY (link_image, link_category));", NULL, NULL, NULL);
	//MYDEBUG << __FILE__ << " " << __LINE__<<endl;
	t_image_category = new KexiDB::TableSchema("image_category");
	t_image_category->setCaption("Links between images and categories");

	t_image_category->addField( f=new KexiDB::Field("imacat_ima_id", KexiDB::Field::Integer) );
	f->setCaption("image id");

	t_image_category->addField( f=new KexiDB::Field("imacat_cat_id", KexiDB::Field::Integer) );
	f->setCaption("category id");


	if (!conn->createTable( t_image_category ))
	{
	        conn->debugError();
		return false;
	}
	MYDEBUG << "-- t_image_category created --" << endl;
	t_image_category->debug();


	//------------------------------------------------------------------------------
	t_directories = new KexiDB::TableSchema("directories");
	t_directories->setCaption("All directories");

	t_directories->addField( f=new KexiDB::Field("directory_id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned) );
	f->setCaption("directory id");

	t_directories->addField( f=new KexiDB::Field("directory_path", KexiDB::Field::Text, KexiDB::Field::Unique) );
	f->setCaption("directory path");

	t_directories->addField( f=new KexiDB::Field("directory_icon", KexiDB::Field::Text) );
	f->setCaption("directory icon");

	if (!conn->createTable( t_directories ))
	{
		conn->debugError();
		return false;
	}
	MYDEBUG << "-- t_directories created --" << endl;
	t_directories->debug();

	//------------------------------------------------------------------------------
	if (!conn->commitTransaction(t))
	{
		conn->debugError();
		return false;
	}

	MYDEBUG << "NOW, TABLE LIST: " << endl;
	QStringList tnames = conn->tableNames();
	for (QStringList::iterator it = tnames.begin(); it!=tnames.end(); ++it)
	{
		MYDEBUG << " - " << (*it) << endl;
	}

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
	list.addField(t_media->field("media_id"));
	list.addField(t_media->field("media_name"));
	list.addField(t_media->field("media_icon"));
	conn->insertRecord(list, QVariant(0), QVariant("HDD"),  QVariant("hdd_umount"));
	conn->insertRecord(list, QVariant(1), QVariant("CDROM"),  QVariant("cdrom_umount"));

	return true;
}


int
Categories::addTopCategory(const QString& cat_name, const QString& desc, const QString& icon)
{
	KexiDB::FieldList list;
	list.addField(t_categories->field("category_name"));
	list.addField(t_categories->field("category_desc"));
	list.addField(t_categories->field("category_icon"));
	conn->insertRecord(list, QVariant(cat_name), QVariant(desc), QVariant(icon));
	MYDEBUG<< " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
	return conn->lastInsertedAutoIncValue("category_id", *t_categories);
}

int
Categories::addSubCategory(int up_cat, const QString& cat_name, const QString& desc, const QString& icon)
{
	KexiDB::FieldList list;
	list.addField(t_categories->field("category_name"));
	list.addField(t_categories->field("category_desc"));
	list.addField(t_categories->field("category_icon"));
	list.addField(t_categories->field("category_up"));
	if(conn->insertRecord(list, QVariant(cat_name), QVariant(desc), QVariant(icon), QVariant(up_cat)))
		return conn->lastInsertedAutoIncValue("category_id", *t_categories);
	else
		return -1;
}




int
Categories::querySingleNumber(const QString& query, bool useParser)
{
	int id=-1;
	if(useParser)
	{
		KexiDB::Parser parser(conn);
		const bool ok = parser.parse(query );
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q)
		{
// 			MYDEBUG<<conn->selectStatement( *q )<<endl;
			conn->querySingleNumber(conn->selectStatement( *q ), id);
// 			MYDEBUG<<id<<endl;
		}
	}
	else
	{
		conn->querySingleNumber(query, id);
	}
	return id;
}

QString
Categories::querySingleString(const QString& query, bool useParser)
{
	QString s;
	if(useParser)
	{
		KexiDB::Parser parser(conn);
		const bool ok = parser.parse(query );
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q)
		{
// 			MYDEBUG<<conn->selectStatement( *q )<<endl;
			conn->querySingleString(conn->selectStatement( *q ), s);
		}
	}
	else
	{
		conn->querySingleString(query, s);
	}
	return s;
}

int
Categories::getCategoryId(const QString& cat_name)
{
	QString query = QString("SELECT category_id FROM categories WHERE category_name = '%1'  LIMIT 1 ;")
			.arg(cat_name);
	return querySingleNumber(query, false);
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
	MYDEBUG<<ima_name<<" " <<dir_path<<endl;
	int id=-1;
	if(!isUpdating())
	{
		QString query = QString("SELECT image_id FROM images, directories WHERE image_dir_id=directory_id AND image_name = '%1' AND directory_path='%2' LIMIT 1;")
			.arg(ima_name)
			.arg(dir_path);
		id =  querySingleNumber(query, false);
	}
	else
	{
		MYWARNING<<"Unable to get ID because DB is updating"<<endl;
	}
	if(id<0)
	{
		MYWARNING << conn->errorMsg()<<endl;
		MYWARNING << conn->serverErrorMsg()<<endl;
	}
	return id;
}

int
Categories::getImageId(const QString&  ima_name, int dir_id)
{
// 	MYDEBUG<<endl;
	if(dir_id<0)
	{
		MYWARNING<<"ERROR in the dir_id "<< dir_id<<endl;
		return -1;
	}
	QString query = QString("SELECT image_id FROM images WHERE image_name='%1' AND image_dir_id=%2  LIMIT 1 ;")
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

QString
Categories::getCategoryName(int cat_id)
{
	QString query = QString("SELECT category_name FROM categories WHERE category_id = %1  LIMIT 1 ;")
			.arg(cat_id);
	return querySingleString(query, false);
}
QString
Categories::getCategoryIcon(int cat_id)
{
	QString query = QString("SELECT category_icon FROM categories WHERE category_id = %1  LIMIT 1 ;")
			.arg(cat_id);
	return querySingleString(query, false);
}




QString
Categories::getImageName(int ima_id)
{
	QString query = QString("SELECT image_name FROM images WHERE image_id = %1  LIMIT 1 ;")
			.arg(ima_id);
	return querySingleString(query, false);
}


QStringList*
Categories::topCategories(void)
{
	QString query=("SELECT category_name FROM categories WHERE category_up = 0 ; ");
	return executeQuerry(query);
}

QStringList*
Categories::allCategories(void)
{
	//KexiDB::QuerySchema query;
	//query.setParentTable(t_categories);
	//query.addField(t_categories->field("category_id"));
	QString query=("SELECT category_id FROM categories ; ");
	return executeQuerry(query);
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
	MYDEBUG<<s<<endl;

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
// 	MYDEBUG<<query<< " " << useParser<<endl;
	KexiDB::Cursor *cursor=NULL;

	if(useParser)
	{
		KexiDB::Parser parser(conn);
		const bool ok = parser.parse(query);
		KexiDB::QuerySchema *q = parser.query();
		if(ok&&q) cursor = conn->executeQuery(*q);
	}
	else
	{
		cursor = conn->executeQuery(query);
	}
	QStringList *l=cursor2stringlist(cursor, col);
	freeCursor(cursor);
	return l;
}


QStringList*
Categories::executeQuerry(KexiDB::QuerySchema& query, int col)
{
	KexiDB::Cursor *cursor = conn->executeQuery( query, KexiDB::Cursor::Buffered );
	QStringList *l=cursor2stringlist(cursor, col);
	freeCursor(cursor);
	return l;
}

QStringList*
Categories::subCategories(const QString& cat_name)
{
	QString query = QString("SELECT category_name FROM categories WHERE category_up = (SELECT category_id FROM categories WHERE category_name = '%1')")
			.arg(cat_name);
	return executeQuerry(query, 0, false);
}

bool
Categories::addImages(QPtrList <QPtrList <QVariant> > *imageList)
{
	//MYDEBUG<<endl;
	if(imageList->isEmpty()) return true;

	MYDEBUG<<"Adding files:" << imageList->count()<<endl;
	setUpdating(true);

	KexiDB::Transaction t = conn->beginTransaction();
	if (conn->error())
	{
		MYDEBUG<<"Error in begin transaction"<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		MYWARNING << conn->errorMsg()<<endl;
		MYWARNING << conn->serverErrorMsg()<<endl;
		conn->setAutoCommit(true);
		setUpdating(false);
		return false;
	}

	MYDEBUG<<"---------------------------"<<endl;
	MYDEBUG<<" BEGIN TRASACTION"<<endl;

	QPtrList <QVariant> *image;
	for(image = imageList->first(); image; image=imageList->next())
	{
// 		MYDEBUG<<"================ADDING "<<image->at(0)->toString()<<endl;
		addImage(
				image->at(0)->toString(),
				image->at(1)->toInt(),
				image->at(2)->toDateTime(),
				image->at(3)->toString());
	}


	if (!conn->commitTransaction(t))
	{
		MYWARNING<<"ERROR adding files" <<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		MYWARNING << conn->errorMsg()<<endl;
		MYWARNING << conn->serverErrorMsg()<<endl;
	}

	MYDEBUG<<" END TRASACTION"<<endl;
	MYDEBUG<<"---------------------------"<<endl;
// 	conn->setAutoCommit(false);
	setUpdating(false);
	return true;
}

int
Categories::addImage(const QString& name, int dir_id, const QDateTime& date, const QString& comment)
{
	MYDEBUG<<"== add "<<name<<endl;
	if(dir_id<=0)
	{
		MYWARNING<<"directory numbered " << dir_id<< " is not valid!"<<endl;
		return -1;
	}

	//------------------------------------------------------------------------------
	int ima_id = getImageId(name, dir_id);
	if(ima_id > 0) return ima_id;

	//------------------------------------------------------------------------------
	KexiDB::RowData data;
	QString query = QString("SELECT 1 FROM directories WHERE directory_id=%1 ;")
			.arg(dir_id);
	if(! conn->querySingleRecord(query, data))
	{
		MYWARNING<<"directory numbered " << dir_id<< " does NOT exist!"<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		MYWARNING << conn->errorMsg()<<endl;
		MYWARNING << conn->serverErrorMsg()<<endl;
		return -1;
	}

	//------------------------------------------------------------------------------
	KexiDB::FieldList list;
	list.addField(t_images->field("image_name"));
	list.addField(t_images->field("image_dir_id"));
	list.addField(t_images->field("image_date_begin"));
	list.addField(t_images->field("image_date_end"));
	list.addField(t_images->field("image_comment"));
	bool success = conn->insertRecord(list,
									  QVariant(name),
									  QVariant(dir_id),
									  QVariant(date),
									  QVariant(date),
									  QVariant(comment));
	if(!success)
	{
		MYWARNING << " ERRROR inserting " << name<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		conn->debugError();
		return -1;
	}
	else
	{
		MYDEBUG << "OK!"<<endl<<endl;
		return conn->lastInsertedAutoIncValue("image_id", *t_images);
	}
}

int
Categories::addDirectory(const QString& path)
{
	MYDEBUG<<path<<endl;
	QString query = QString("SELECT 1 FROM directories WHERE directory_path='%1' LIMIT 1; ")
			.arg(path);
	KexiDB::RowData data;
	if(conn->querySingleRecord(query, data))
	{
		//MYWARNING<<"Directory " << path << " already exists!"<<endl;
		int id = data[0].toInt();
		return id;
	}
	KexiDB::FieldList list;
	list.addField(t_directories->field("directory_path"));
	bool success = conn->insertRecord(list, QVariant(path));
// 	query = QString("INSERT INTO directories(directory_path) VALUES ('%1');")
// 		.arg(path);
// 	bool success = conn->executeQuery( query );

	if(!success)
	{
		MYWARNING << " ERROR inserting "<<path<<endl;
		MYWARNING << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		MYWARNING << query << endl;
		MYWARNING << conn->errorMsg()<<endl;
		MYWARNING << conn->serverErrorMsg()<<endl;
		return -1;
	}
	else
		return conn->lastInsertedAutoIncValue("directory_id", *t_directories);
}

int
Categories::deleteDirectory(int dir_id)
{
	QString query = QString("SELECT 1 FROM images WHERE image_dir_id = %1  LIMIT 1 ;")
			.arg(dir_id);
	KexiDB::RowData data;
	if(conn->querySingleRecord(query, data))
	{
		MYWARNING<<"Directory " << getDirectoryPath(dir_id)<<"-"<<dir_id <<   " is not empty!"<<endl;
		return -1;
	}
	//------------------------------------------------------------------------------
	query = QString("DELETE FROM directories WHERE directory_id = %1 ;")
			.arg(dir_id);
	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::getDirectoryId(const QString& path)
{
	//MYDEBUG<<endl;
	QString query = QString("SELECT directory_id FROM directories WHERE directory_path='%1'  LIMIT 1 ;")
			.arg(path);
	return querySingleNumber(query, false);
}
QString
Categories::getDirectoryPath(int dir_id)
{
	//MYDEBUG<<endl;
	QString query = QString("SELECT directory_path FROM directories WHERE directory_id=%1 LIMIT 1  ;")
			.arg(dir_id);
	return querySingleString(query, false);
}



KexiDB::Cursor*
Categories::query2ImageListCursor(const QString& query)
{
	return imageIdList2ImageList(conn->executeQuery( query ));
}

KexiDB::Cursor*
Categories::query2ImageListCursor(KexiDB::QuerySchema& query)
{
	return imageIdList2ImageList(conn->executeQuery( query ));
}


KexiDB::Cursor *
Categories::imageIdList2ImageList(KexiDB::Cursor *cursor)
{
	if (!cursor) return NULL;
	if (cursor->eof()) return NULL;
// 	MYDEBUG << " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;

	QString query= "SELECT DISTINCT image_id, image_name, image_dir_id, image_comment, image_note, image_date_begin, image_date_end FROM images WHERE image_id IN (";
	cursor->moveFirst();
	while(!cursor->eof())
	{
		query += cursor->value(0).toString() + ", ";
		cursor->moveNext();
	}
	query += " );";
	return  conn->executeQuery( query );
}


KexiDB::Cursor *
Categories::imagesCategoriesList(const QStringList& tab)
{
	QString query = QString("SELECT DISTINCT imacat_ima_id FROM image_category WHERE imacat_cat_id IN (%1);")
			.arg(tab.join(", "));
	MYDEBUG<<query<<endl;
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
	for(unsigned int i=1; i<l.count(); i++)
	{
		query = QString(
			"SELECT imacat_ima_id "
			"FROM image_category "
			"WHERE imacat_cat_id IN (%1) AND imacat_ima_id IN (%2) ")
				.arg(l.at(i)->join(", "))
				.arg(query);
	}
	MYDEBUG<<query<<endl;
	return query2ImageListCursor(query);
}







KexiDB::Cursor*
Categories::imagesCommentList(const QString& comment)
{
	MYDEBUG<<endl;
	QString query = "SELECT image_id FROM images WHERE image_comment LIKE '%";
	query += comment;
	query += "%' ;";
	return query2ImageListCursor(query);
}


KexiDB::Cursor*
Categories::imagesPatternList(QPtrList<QString>& patterns,
							  const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	QString query = QString("SELECT DISTINCT image_id FROM images WHERE ");
	for( unsigned int i=0; i<patterns.count()-1; i++)
	{
		query += QString(" (image_name LIKE '\%%1\%') ").arg(*patterns.at(i));
		if(mode == mode_OR)
			query += " OR ";
		else
			query += " AND ";
	}
	query += QString("image_name LIKE '\%%1\%' ").arg(*patterns.at(patterns.count()-1));

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
	MYDEBUG<<query<<endl;
	return query2ImageListCursor(query);
}

KexiDB::Cursor*
Categories::imagesDateList(const QDate& date, int bia,
						   const QPtrList<QVariant>& iiList, Categories::SelectionMode mode)
{
	//MYDEBUG<<endl;
	QString query = "SELECT DISTINCT image_id FROM images WHERE DATE(image_date_begin)%1'%2' ";
	QString sign;
	if(bia<0)
		sign = "<=";
	else if(bia==0)
		sign = "=";
	else
		sign= ">=";
	query = query.arg(sign).arg(date.toString(date_format));
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
	//MYDEBUG<<endl;
	QString
			sdate_begin=date_begin.toString(date_format),
			sdate_end  =date_end.toString(date_format);

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

	MYDEBUG<<query<<endl;
	return query2ImageListCursor(query);
}



void
Categories::freeCursor(KexiDB::Cursor* cursor)
{
	conn->deleteCursor(cursor);
}

int
Categories::addLink(const QStringList& imgid_list, int cat_id)
{
	//MYDEBUG<<imgid_list<<endl;
/*
	QString query = QString("SELECT COUNT(*) FROM image_category WHERE imacat_ima_id IN (%1) AND imacat_cat_id=%2")
			.arg(imgid_list.join(", "))
			.arg(cat_id);
	int nbr=-1;
	conn->querySingleNumber(query, nbr);
	MYDEBUG<<query<< " " << nbr<<endl;
*/

	KexiDB::Transaction t = conn->beginTransaction();
	if (conn->error())
	{
		conn->debugError();
		return false;
	}
	//--------------------------------------------------------------------------
	QStringList m_imgid_list=imgid_list;
	for ( QStringList::Iterator it = m_imgid_list.begin(); it != m_imgid_list.end(); ++it )
	{
// 		MYDEBUG << " - " << (*it) << endl;
		addLink((*it).toInt(), cat_id);
	}
	//--------------------------------------------------------------------------
	if (!conn->commitTransaction(t))
	{
		conn->debugError();
		return false;
	}

	return 0;
}


int
Categories::addLink(int image_id, int cat_id)
{
	//MYDEBUG<<endl;
	QString query = QString("SELECT 1 FROM image_category WHERE imacat_ima_id=%1 AND imacat_cat_id=%2  LIMIT 1 ;")
			.arg(image_id)
			.arg(cat_id);
	KexiDB::RowData data;
	if(conn->querySingleRecord(query, data))
	{
		return -1;
	}


	KexiDB::FieldList list;
	list.addField(t_image_category->field("imacat_ima_id"));
	list.addField(t_image_category->field("imacat_cat_id"));
	if(!conn->insertRecord(list, QVariant(image_id), QVariant(cat_id)))
	{
		MYWARNING<<"ERROR inserting link"<<endl;
		MYWARNING<< " RECENT SQL STATEMENT: " << conn->recentSQLString() << endl;
		return -1;
	}
	else
		return 0;
}

int
Categories::addLink(const QString& image_name, const QString& image_path, const QString& cat_name)
{
	//MYDEBUG<<endl;
	return addLink(getImageId(image_name, image_path), getCategoryId(cat_name));
}

QStringList*
Categories::imageLinks(int image_id, bool getCatName)
{
	QStringList* imageCats;
	if(image_id<0)
	{
		MYWARNING<<"image has wrong id " << image_id << endl;
		imageCats = new QStringList();
	}
	else
	{
		QString query = QString("SELECT %1 FROM categories, image_category WHERE category_id = imacat_cat_id  AND imacat_ima_id = '%2';")
			.arg(!getCatName?"category_id":"category_name")
			.arg(image_id);
		imageCats = executeQuerry(query);
		if(imageCats->isEmpty()==0)
		{
			MYDEBUG<<"No category found for image " << image_id<<endl;
			MYDEBUG<<query<<endl;
		}
	}
	return imageCats;
}

KexiDB::Cursor*
Categories::imagesNoteList(int note, int lem)
{
	//MYDEBUG<<endl;
	QString query = QString ("SELECT image_id FROM images WHERE image_note %1 %2 AND image_note > 0 ;");
	QString sign;
	if(lem<0)
		sign = " <= ";
	else if (lem==0)
		sign = " = ";
	else
		sign = " >= ";
	query = query.arg(sign).arg(note);

	MYDEBUG<<query<<endl;
	return query2ImageListCursor(query);
}

int
Categories::moveImages(const KURL::List& fileurls, const KURL& desturl)
{
	MYDEBUG << fileurls <<"->"<<desturl<<endl;
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

int
Categories::moveImage(int ima_id, int dir_id)
{
	QString query = QString("UPDATE images SET image_dir_id = '%1' WHERE image_id = %2 ;")
			.arg(dir_id)
			.arg(ima_id);
	MYDEBUG<<query<<endl;
	return conn->executeSQL( query );
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
	MYDEBUG<<endl;
	QString query = QString("UPDATE images SET image_name = '%1' WHERE image_id = %2 ;")
			.arg(new_name)
			.arg(id);
	MYDEBUG<<query<<endl;
	return conn->executeSQL( query );
}

int
Categories::renameCategory(int id, const QString& new_name)
{
	MYDEBUG<<endl;
	QString query = QString("UPDATE categories SET category_name = '%1' WHERE category_id = %2;")
			.arg(new_name)
			.arg(id);
	return conn->executeSQL( query );
}

int
Categories::moveDirectory(const QString& old_dirPath, const QString& old_dirName, const QString& dest_path)
{
	MYDEBUG<<"'"<<old_dirPath<<"', '"<<old_dirName<<"', '"<<dest_path<<"'"<<endl;
	QString old_fullpath = QString("%1/%2").arg(old_dirPath).arg(old_dirName);
	QString new_fullpath = QString("%1/%2").arg(dest_path).arg(old_dirName);

	//'/mnt/win_e'||'/'||'qwerty' from directories where directory_path = "/home/rgroult/tmp/qwerty";
	QString query = QString("UPDATE directories SET directory_path='%1' WHERE directory_path = '%2' ;")
			.arg(new_fullpath)
			.arg(old_fullpath);
	MYDEBUG<<query<<endl;
	conn->executeSQL( query );

// 	'/mnt/win_e'||substr(directory_path,length('/home/rgroult/tmp')+1,length(directory_path))  from directories where directory_path like "/home/rgroult/tmp/qwerty/%";
	int old_dirPath_len = old_dirPath.length()+1;
	old_fullpath+="/%";
	query = QString("UPDATE directories SET directory_path = '%1' || SUBSTR(directory_path, %2, LENGTH(directory_path)) WHERE directory_path LIKE '%3' ; ")
			.arg(dest_path)
			.arg(old_dirPath_len)
			.arg(old_fullpath);
	MYDEBUG<<query<<endl;
	conn->executeSQL( query );

	return 1;
}


int
Categories::renameDirectory(const QString& old_path, const QString& new_path)
{
	QString query = QString("UPDATE directories SET directory_path='%2' WHERE directory_path='%2' ; ")
			.arg(new_path)
			.arg(old_path);
	MYDEBUG<<query<<endl;
	conn->executeSQL( query );

	query = QString("UPDATE directories SET directory_path='%1' || '/' || SUBSTR(directory_path, LENGTH('%2')+2, LENGTH(directory_path)-LENGTH('%3')+1) WHERE directory_path LIKE '%4/\%' ; ")
			.arg(new_path)
			.arg(old_path)
			.arg(old_path)
			.arg(old_path);
	MYDEBUG<<query<<endl;
	conn->executeSQL( query );

	return 1;
}

int
Categories::deleteNodeCategory(int id)
{
	MYDEBUG<<endl;
	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id = %1 ;")
			.arg(id);
	MYDEBUG << query<<endl;
	int res = conn->executeSQL( query );
	if (res != 0)
		return res;

	query = QString("DELETE FROM categories WHERE category_id = %1 ;")
			.arg(id);

	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::setNewParent(int id, int new_parent)
{
	MYDEBUG<<endl;
	QString query = QString("UPDATE categories SET category_up = %1  WHERE category_id = %2;")
			.arg(new_parent)
			.arg(id);

	MYDEBUG << query<<endl;
	return  conn->executeSQL( query );
}

int
Categories::deleteImage(int ima_id)
{
	MYDEBUG<<endl;
	QString query = QString("DELETE FROM image_category WHERE imacat_ima_id = %1 ;")
			.arg(ima_id);
	MYDEBUG << query<<endl;
	int res = conn->executeSQL( query );
	if (res != 0)
		return res;

	query = QString("DELETE FROM images WHERE image_id = %1 ;")
			.arg(ima_id);

	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::deleteCategoryImage(int cat_id, int ima_id)
{
	MYDEBUG<<endl;
	return -1;
	QString query = QString("DELETE FROM image_category WHERE imacat_cat_id = %1 AND imacat_ima_id = %2 ;")
			.arg(cat_id)
			.arg(ima_id);

	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::setImageComment(int id, const QString& comment)
{
	MYDEBUG<<endl;
	QString query = QString("UPDATE images SET image_comment = '%1' WHERE image_id = %2 ;")
			.arg(comment)
			.arg(id);

	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::setImageNote(int id, int note)
{
	MYDEBUG<<endl;
	QString query = QString("UPDATE images SET image_note =  %1 WHERE image_id = %2 ;")
			.arg(note)
			.arg(id);

	MYDEBUG << query<<endl;
	return conn->executeSQL( query );
}

int
Categories::setImageDate(int id, const QDateTime&  begin, const QDateTime& end)
{
	MYDEBUG<<endl;
	QString query = QString("UPDATE images SET image_date_begin = '%1 ', image_date_end = '%2' WHERE image_id = %3 ;")
			.arg(begin.toString(datetime_format))
			.arg(end.toString(datetime_format))
			.arg(id);

	MYDEBUG<<query<<endl;
	return  conn->executeSQL( query );
}

KexiDB::Cursor*
Categories::allImages()
{
	MYDEBUG<<endl;
	KexiDB::QuerySchema query(t_images);
	query.clear();
	query.addField(t_images->field("image_id"));
	return query2ImageListCursor(query);
}

QDateTime
Categories::getOldestImage()
{
	QString query = "SELECT MIN(image_date_begin) FROM images;";
	return QDateTime::fromString(querySingleString(query, false), Qt::ISODate);
}

QDateTime
Categories::getNewestImage()
{
	QString query = "SELECT MAX(image_date_end) FROM images;";
	return QDateTime::fromString(querySingleString(query, false), Qt::ISODate);
}

int
Categories::getNumberOfImageForDate(int year, int month, int day)
{
	QString query = "SELECT COUNT(*) FROM images WHERE ";
	if(day==-1)
	{
		if(month==-1)
		{
			query += QString("STRFTIME('%Y',image_date_begin) = %1;").arg(year);
		}
		else
		{
			query += QString("STRFTIME('%Y%m',image_date_begin) = %1%2%3;")
				.arg(year)
				.arg(month<10?"0":"").arg(month);
		}
	}
	else
	{
		query += QString("STRFTIME('%Y%m%d',image_date_begin) = %1%2%3%4%5;")
			.arg(year)
			.arg(month<10?"0":"").arg(month)
			.arg(day<10?"0":"").arg(day);
;
;
	}
	MYDEBUG<<query<<endl;
	return querySingleNumber(query, false);
}

