/***************************************************************************
                          fileiconitem.h  -  description
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

#ifndef FILEICONITEM_H
#define FILEICONITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "imagelistview.h"

// Qt
#include <qtooltip.h>
#include <qfile.h>

class ImageViewer;
class FileIconItemPrivate;
class ListItem;
class MainWindow;

class QIconView;
class QPixmap;
class QFileInfo;
class QToolTip;

class KFileItem;
class KURL;

class FileIconItem : public KFileIconViewItem
{
public:
	FileIconItem(
			ListItem *parentDir,
			const QString& path,
			const QString& filename,
			MainWindow* mw);
	virtual ~FileIconItem();

	virtual void setPixmap ( const QPixmap & icon, bool haspreview=false );
	bool hasPreview ();
	void setHasPreview (bool preview);

	FileIconItem* nextItem ();
	FileIconItem* prevItem ();

	virtual void setType(const QString& type);
	virtual QString getType() const;

	virtual QString fullName() const;
	virtual KURL getURL();

	virtual void setPath(const QString& newPath);
	virtual QString path();

	QString mimetype() const;
	bool isImage() const;
	void setIsImage(bool isImage);
	bool isMovable() const;
	void setIsMovable(bool isMovable);

	virtual bool suppression(bool);
	virtual bool suppression();

	virtual bool moveToTrash();
	virtual bool shred();

	virtual QString text(int i=0) const;

	QString toolTipStr() const;

	virtual void setWallpaper();

	virtual void setKey ( const QString& k );
	virtual int compare ( QIconViewItem * i ) const;

	virtual void setSelected (bool s);

	virtual void calcRect();

	QString getProtocol() const;
	void setProtocol(const QString& protocol);

	QPixmap iconPixmap(KIcon::Group gr = KIcon::Small) const {
		return mKFileItem->pixmap(IconSize(gr)); }

protected:
	virtual void paintItem(QPainter* p, const QColorGroup& cg);
	void repaint();

	void updateExtraText();
	void wrapText();

	static QString getFileName(QString *fullName);
	static QString getFileExt(QString *fullName);
	static QString getFullName(QString *fullName);
	static QString getFullPath(QString *fullName);

	static QString getFileName(const QString& fullName);
	static QString getFileExt(const QString& fullName);
	static QString getFullName(const QString& fullName);
	static QString getFullPath(const QString& fullName);

	virtual QStringList toolTipArgs() const;
	QString shrink(const QString& str, int len=50) const;

	bool getHasTooltip() const {return m_hasToolTip;} ;
	void setHasTooltip(bool enabled=true){m_hasToolTip=enabled;};

protected:
	int m_size;
	QString extension;
	QDateTime m_date;
	QString description;
	QString m_protocol;

	QFile f;

	QString full;
	bool haspreview;
	KFileItem *mKFileItem;

	QSize m_dimension;

	ListItem *parentDir;
	MainWindow *mw;

	QStringList m_categories;

private:
	QString m_extraText;
	QString m_extraText_short;

	QString m_wrapedText;
	QRect	m_itemExtraRect;

	bool m_isimage, m_ismovable, m_hasToolTip;
	QString m_type;
	QString m_name;
};

#endif
