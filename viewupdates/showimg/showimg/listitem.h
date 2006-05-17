/***************************************************************************
                          listitem.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#ifndef LISTITEM_H
#define LISTITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fileiconitem.h"

// KDE
#include <klistview.h>

// Qt
#include <qfile.h>
#include <qstring.h>

class ImageViewer;
class MainWindow;
class ListItemView;
class ImageListView;

class QListViewItem;
class QString;
class QPixmap;
class QString;
class QStringList;

class KURL;



class ListItem : public KListViewItem
{

public:
	ListItem(MainWindow *mw, ListItemView *dirView, const QString& path = QString::null);
	ListItem(ListItem *parent, const QString& filename, MainWindow *mw);
	virtual ~ListItem();

	virtual QString text (int column) const;

	virtual QString fullName ();
	virtual QString name ();
	virtual QString path ();
	virtual int getSize() const;
	virtual void setSize(int size);
	virtual uint getNumberOfItems();
	virtual KURL getURL();
	QString getProtocol() const;
	void setProtocol(const QString& protocol);

	virtual  void setSelected (bool select);

	virtual  void load (bool refresh=true);
	virtual  void unLoad ();
	virtual  bool refresh(bool preview=true);
	virtual  ListItem* find (const QString&);
	virtual  void goTo (const QString&);

	virtual  void removeImage ( FileIconItem* );
	virtual  QString key (int column, bool ascending)  const;
	virtual int compare ( QListViewItem * i, int col, bool ascending ) const;

	virtual  void create(const QString& dirName);

	virtual  bool add(const QStringList& uris);

	virtual  bool acceptDrop();

	virtual  void setReadOnly(bool readOnly=true);
	virtual  bool isReadOnly();


	virtual  bool rename(const QString& newDirName, QString& msg);
	virtual  void rename();
	virtual  void updateChildren();
	virtual  void properties();

	QString getType(){return m_type;};
	void setType(const QString& type){m_type = type;};

	ListItem* firstChild();
	ListItem* nextSibling();
	ListItem* itemBelow();
	ListItem* itemAbove();
	ListItem* parent();
	ListItemView* getListItemView();

	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

	static const int TREE_CHECKBOX_MAXSIZE = 16;

	QPtrList < FileIconItem > getFileIconItemList();

protected:
	void init();

protected:
	MainWindow *mw;
	ListItemView *dirView;
	QPtrList < FileIconItem > list;
	QFile f;
	QString full;
	QString extension;
	QString m_protocol;
	bool isReadOnly_ : 1;
	bool hasSpecialIcon : 1; //!< true means that this item's icon wont be changed
	                          //!< to folder_open becasue special (e.g. "desktop") icon is used.
#ifdef Q_WS_WIN
	bool thisIsADriveNode : 1;
#endif

private:
	int m_size;
	QString m_type;
};

#endif
