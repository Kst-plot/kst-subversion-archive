/***************************************************************************
                         categorylistitem.cpp  -  description
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

#ifndef CATEGORYLISTITEM_H
#define CATEGORYLISTITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

// Local
#include "listitem.h"

class MainWindow;
class CategoryDBManager;
class CategoryNode;
class CategoryImageFileIconItem;
class CategoryView;

class CategoryListItem : public ListItem
{
public:
	CategoryListItem( CategoryListItem* parent, const QString& title,
					  MainWindow *mw );
	~CategoryListItem();

	virtual void load(bool);
	virtual void unLoad();
	virtual void reload();

	uint getNumberOfItems();

	virtual void addURL(const QStringList& list) = 0;

	virtual bool rename(const QString& newDirName, QString& msg) = 0;


protected:
	CategoryListItem( MainWindow *mw );
	virtual void init() = 0;

	CategoryDBManager* getCategoryDBManager();
	void setCategoryDBManager(CategoryDBManager *cdbManager);

protected:
	CategoryDBManager *cdbManager;
	uint m_numberOfItems;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CategoryListItemRootTag;
class CategoryListItemTag : public CategoryListItem
{
public:
	CategoryListItemTag(CategoryListItemTag* parent, CategoryNode *node,
						MainWindow *mw );

	CategoryNode* getCategoryNode();
	virtual int getId();
	virtual bool isLeaf();

	//-----------------------
	virtual void load(bool);
	virtual void unLoad();

	virtual void addURL(const QStringList& list);

	virtual bool rename(const QString& newDirName, QString& msg);

	virtual bool setDescription(const QString& descr);
	virtual QString getDescription();

	virtual bool setIcon(const QString& icon);
	virtual QString getIcon();


protected:
	CategoryListItemTag( MainWindow *mw );
	void init();

protected:
	CategoryNode *node;
	int cat_id;
};

//------------------------------------------------------------------------------

class CategoryListItemRootTag : public CategoryListItemTag
{
	public:
		CategoryListItemRootTag( MainWindow *mw );

		void load(bool);
		void unLoad();
		bool isLeaf();
		virtual int getId();

	protected:
		void init();

	protected:
		CategoryNode *node;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CategoryListItemRootDate;
class CategoryListItemDate : public CategoryListItem
{

	public:
		enum CategoryListItemDate_type
		{
			YEAR=0,
			MONTH,
			DAY
		};

		CategoryListItemDate(CategoryListItemDate* parent,
							 const QDateTime& datetime,
							 CategoryListItemDate_type datetype,
							 MainWindow *mw );

		virtual bool isLeaf();
		QDateTime getDateTime();

		virtual void setOpen (bool);

		//-----------------------
		virtual void load(bool);
		virtual void unLoad();

		virtual void addURL(const QStringList& list);

		virtual bool rename(const QString& newDirName, QString& msg);

	protected:
		CategoryListItemDate( MainWindow *mw );
		void init();

	protected:
		CategoryListItemDate_type m_datetype;
		QDateTime m_datetime;
};

//------------------------------------------------------------------------------

class CategoryListItemRootDate : public CategoryListItemDate
{
	public:
		CategoryListItemRootDate( MainWindow *mw );

		void setOpen (bool);
		void load(bool);

	protected:
		void init();
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CategoryListItemRootSearch;
class CategoryListItemSearch : public CategoryListItem
{
	public:
		CategoryListItemSearch(CategoryListItemSearch* parent, const QString& pattern, MainWindow *mw );

		virtual bool isLeaf();

		//-----------------------
		virtual void load(bool);
		void unLoad();

		void addURL(const QStringList& list);

		bool rename(const QString& newDirName, QString& msg);

	protected:
		CategoryListItemSearch( MainWindow *mw );
		void init();

	protected:
		QString pattern;
};

//------------------------------------------------------------------------------

class CategoryListItemRootSearch : public CategoryListItemSearch
{
	public:
		CategoryListItemRootSearch( MainWindow *mw );

		bool isLeaf();

		void load(bool);

		virtual void setOpen (bool);

	protected:
		void init();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CategoryListItemRootNote;
class CategoryListItemNote : public CategoryListItem
{
	public:
		CategoryListItemNote(CategoryListItemNote* parent, const int note, MainWindow *mw );

		virtual bool isLeaf();

		//-----------------------
		virtual void load(bool);
		void unLoad();

		void addURL(const QStringList& list);

		bool rename(const QString& newDirName, QString& msg);

	protected:
		CategoryListItemNote( MainWindow *mw );
		void init();

	protected:
		QString m_note;
};

//------------------------------------------------------------------------------

class CategoryListItemRootNote : public CategoryListItemNote
{
	public:
		CategoryListItemRootNote( MainWindow *mw );

		bool isLeaf();

		void load(bool);

		virtual void setOpen (bool);

	protected:
		void init();
};
#endif /* WANT_LIBKEXIDB */

#endif /* CATEGORYLISTITEM_H */

