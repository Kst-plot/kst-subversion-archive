/***************************************************************************
                          listitem.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

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

#ifndef LISTITEM_H
#define LISTITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fileiconitem.h"

// KDE
#include <klistview.h>
#include <kurl.h>

// Qt
#include <qfile.h>
#include <qstring.h>

class ImageListView;
class ImageViewer;
class ListItemView;
class MainWindow;

class QListViewItem;
class QString;
class QPixmap;
class QString;
class QStringList;

class ListItem : public KListViewItem
{

public:
	ListItem(ListItemView *dirView,  const KURL& a_url = KURL());
	ListItem(ListItem *parent, const QString& filename);
	virtual ~ListItem();

	virtual QString text (int column) const;
	virtual QString displayedName () const;

	virtual QString fullName (int a_trailing = 0) const ;
	virtual void setFullName (const QString& a_fullname, const QString& a_protocol="file");

	virtual QString name () const;
	virtual QString path () const;
	virtual int getSize() const;
	virtual void setSize(int size);
	virtual uint getNumberOfItems()const;

	virtual KURL & getURL() ;
	virtual KURL  getURL() const;
	virtual void setURL(const KURL& a_url ) ;
	QString getProtocol() const;
	void setProtocol(const QString& protocol);

	virtual  void setSelected (bool select);

	virtual  void load (bool refresh=true);
	virtual  void unLoad ();
	virtual  bool refresh(bool preview=true);

	virtual  ListItem* find (const KURL& a_url);
	virtual  ListItem* find (const QString& a_end_dir);

	virtual  void goTo (const QString&);

	virtual  void removeIconItem ( FileIconItem* );
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

	ListItem* firstChild();
	ListItem* nextSibling();
	ListItem* itemBelow();
	ListItem* itemAbove();
	ListItem* parent();

	inline ListItemView* getListItemView(){return m_p_dirView;};

	virtual bool insertListItem(const KURL& /*a_fullname*/){return false;};
	virtual bool insertIconItem(const KURL& /*a_fullname*/){return false;};

	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

	static const int TREE_CHECKBOX_MAXSIZE = 16;

	inline QPtrList < FileIconItem >& getFileIconItemList(){return m_filelist;};

protected:
	void init();
	MainWindow* getMainWindow();

	void setExtension(const QString & a_extension){m_extension=a_extension;};

	bool hasSpecialIcon : 1; //!< true means that this item's icon wont be changed
	                          //!< to folder_open becasue special (e.g. "desktop") icon is used.
#ifdef Q_WS_WIN
	bool thisIsADriveNode : 1;
#endif

private:
	ListItemView *m_p_dirView;

	QPtrList < FileIconItem > m_filelist;
	KURL m_url;
	QString m_extension;

	bool m_isReadOnly : 1;

	int m_size;
};

#endif
