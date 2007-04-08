/*****************************************************************************
                          fileiconitem.h  -  m_description
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

class FileIconItemPrivate;
class ImageViewer;
class ListItem;
class MainWindow;

class QFileInfo;
class QIconView;
class QPixmap;
class QToolTip;

class KFileItem;
class KURL;

class FileIconItem : public KFileIconViewItem
{
public:
	FileIconItem(
			const KURL& a_url,
			const QString& filename);
	virtual ~FileIconItem();

	virtual void setPixmap ( const QPixmap & icon, bool m_haspreview=false );
	bool hasPreview ();
	void setHasPreview (bool preview);

	FileIconItem* nextItem ();
	FileIconItem* prevItem ();

	virtual QString name() const;
	virtual QString fullName() const;
	void setFullName (const QString& a_fullpath);

	virtual KURL getURL() const;

	virtual void setPath(const QString& newPath);
	virtual QString path() const;

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

	QString getProtocol();
	void setProtocol(const QString& protocol);

	QPixmap iconPixmap(KIcon::Group gr = KIcon::Small) const {
		return fileInfo()->pixmap(IconSize(gr)); }

protected:
	virtual void paintItem(QPainter* p, const QColorGroup& cg);
	void repaint();

	void updateExtraText();
	void wrapText();

	virtual QStringList toolTipArgs() const;
	QString shrink(const QString& str, int len=50) const;

	bool getHasTooltip() const {return m_hasToolTip;} ;
	void setHasTooltip(bool enabled=true){m_hasToolTip=enabled;};

	QString fullname()const {return m_full;};
	void setFullname(const QString& a_full) {m_full=a_full;};

	inline void setSize(int a_size){m_size=a_size;};
	inline int  getSize()const{return m_size;};

	inline void setDescription(const QString& a_description){m_description=a_description;};
	inline QString getDescription()const{return m_description; };

	inline void setExtension(const QString& a_ext){m_extension=a_ext;};
	inline QString getExtension()const{return m_extension; };

	inline void setHaspreview(const bool& a_haspreview){m_haspreview=a_haspreview;};
	inline bool getHaspreview()const{return m_haspreview; };

	inline void setDate(const QDateTime& a_date){m_date=a_date;};
	inline QDateTime getDate()const{return m_date; };

	inline void setDimension(const QSize& a_dimension){m_dimension=a_dimension;};
	inline QSize getDimension()const{return m_dimension; };

	inline QStringList getCategoryList()const{return m_categories;};
	inline void setCategoryList(const QStringList& a_categories){m_categories=a_categories;};

	MainWindow* getMainWindow();
	const MainWindow* getMainWindow() const;

private:
	int m_size;
	QDateTime m_date;

	QString m_extension;
	QString m_description;
	QString m_protocol;
	QString m_full;
	QString m_name;

	bool m_haspreview;

	QSize m_dimension;

	QStringList m_categories;

	QString
		m_extraText,
		m_extraText_short,
		m_wrapedText;
	QRect
		m_itemExtraRect;

	bool
		m_isimage,
		m_ismovable,
		m_hasToolTip;
};

#endif
