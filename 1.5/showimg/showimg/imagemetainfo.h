/***************************************************************************
                          imagemetainfo.h  -  description
                             -------------------
    begin                : Fri Apr 9 2004
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

#ifndef IMAGEMETAINFO_H
#define IMAGEMETAINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <ktextedit.h>
#include <kurl.h>

// Qt
#include <qdatetime.h>
#include <qscrollview.h>

class KFileMetaInfoItem;
class KFileMetaInfo;
class KListView;
class KSqueezedTextLabel;

class QVBoxLayout;
class QLabel;


class ImageMetaInfo : public QWidget
{
  Q_OBJECT
public:
	ImageMetaInfo(QWidget *parent=0, const char* name=NULL);
	virtual ~ImageMetaInfo(){};

	void setURL(const KURL& url, const QString& mimeType);
	void reload();

	QString toString();
	QString getComments();
	QString getDimensions();
	QDateTime getDatetime();

	KURL getURL();

protected slots:
	void slotClicked(int, int);
	void slotTextChanged ();

protected:
	KSqueezedTextLabel* imagePathLabel;
	KListView* info;
	QLabel* EXIFThumbnailTxtLabel;
	QLabel* EXIFThumbLabel;
	QLabel* commentLabel;
	KTextEdit* comments;

	QVBoxLayout* Form1Layout;

private :
	QString m_lastComment;
	bool 
		m_hasComment, 
		textChanged, 
		hasClicked;
	KFileMetaInfoItem *m_p_fileMetaInfoItemComment;
	KFileMetaInfo *m_p_metaInfo;

	KURL m_current_url;
	QString m_current_mimeType;
	QString m_dimensions;
	QDateTime m_datetime;
};


///////////////////////////////////////////////
///////////////////////////////////////////////


class ImageMetaInfoView  : public QScrollView
{
public:
	ImageMetaInfoView(QWidget *parent);
	ImageMetaInfo* getImageMetaInfo(){return m_imageMetaInfo;};
private:
	ImageMetaInfo *m_imageMetaInfo;
};

#endif
