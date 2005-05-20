/***************************************************************************
                          fileiconitem.h  -  description
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

	virtual void setName(const QString& n);
	virtual QString name() const;

	virtual void setPath(const QString& newPath);
	virtual QString path();

	QString mimetype() const;
	bool isImage() const;
	bool isMovable() const;

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

	int m_size;
	QString extension;
	QString type_;
	QDateTime m_date;
	QString description;

	QFile f;

	QString myName_;
	QString full;
	bool haspreview;
	KFileItem *mKFileItem;

	bool __isimage__, __ismovable__, __hasToolTip__;

	QSize dimension_;

	ListItem *parentDir;

	MainWindow *mw;

private:
    QString extraText_;
    QString wrapedText_;
    QRect   itemExtraRect_;
};

#endif
