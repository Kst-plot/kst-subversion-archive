/***************************************************************************
                          imagefileiconitem.cpp  -  description
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

#include "imagefileiconitem.h"

// Local
#include "imageloader.h"
#include "directory.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "imagefileinfo.h"
#include "misc/exif.h"
#include "mainwindow.h"
#include "categorydbmanager.h"

// Qt
#include <qdropsite.h>
#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qlabel.h>
#include <qiconview.h>
#include <qpixmapcache.h>
#include <qurloperator.h>
#include <qregexp.h>
#include <qstringlist.h>

// KDE
#include <kiconloader.h>
#include <kpixmap.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#include <kio/job.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kapplication.h>
#include <kstandarddirs.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

ImageFileIconItem::ImageFileIconItem (
				ListItem* parentDir,
				const QString& filename,
				const QString& path,
				MainWindow *mw,
				const QString& description,
				bool initExtraText):
   FileIconItem (parentDir, path, filename, mw)
{
	this->parentDir = parentDir;
	this->description=description;

	readable = true;
	selected = false;

	full.append (path);
	full.append (filename);

	setRenameEnabled(false);

	QFileInfo info(fullName());

	m_size = info.size();

	setIsImage(((fileInfo()->mimetype().left(5) == QString::fromLatin1("image")) || (info.extension().lower()==QString::fromLatin1("mng"))));
	setIsMovable(true);

	QDateTime datetime = info.lastModified();
	if(info.extension().lower()==QString::fromLatin1("jpg") && mw->getImageListView()->getShowDate())
	{
		KFileMetaInfo fileMetaInfo(fullName(), mimetype());
		if(fileMetaInfo.contains("Date/time"))
		{
			KFileMetaInfoItem metainfoitem = fileMetaInfo.item("Date/time");
			QString s_date("---");
			if( metainfoitem.isValid())
			{
				s_date = metainfoitem.string(true).stripWhiteSpace();
			}
			if(s_date!="---")
			{
				datetime = QDateTime(
						 KGlobal::locale()->readDate(fileMetaInfo.item( "CreationDate" ).string( true ).stripWhiteSpace()),
						 KGlobal::locale()->readTime(fileMetaInfo.item( "CreationTime" ).string( true ).stripWhiteSpace()));
			}
		}
	}
	m_date = datetime;

	extension = info.extension().lower();

	setType("file");

	doPreload = true;

	setKey(mw->getImageListView()->getCurrentKey());

#ifdef WANT_LIBKEXIDB
	if(mw->getCategoryDBManager()&&mw->getImageListView()->getShowCategoryInfo())
		m_categories =  QStringList(*(mw->getCategoryDBManager()->getCategoryNameListImage(fullName())));
#else
		m_categories = QStringList();
#endif
	if(initExtraText) if(mw->getImageListView()->getShowDimension())initDimension();
	setPixmap(fileInfo()->pixmap(mw->getImageListView()->getCurrentIconSize().width()/2));
	if(initExtraText)updateExtraText();
	calcRect();
}


ImageFileIconItem::~ImageFileIconItem ()
{
}

void
ImageFileIconItem::setText( const QString & text )
{
	if(text==ImageFileIconItem::text())
		return;

	QFileInfo itemFileInfo( fullName() );
	QDir dir( itemFileInfo.dir() );

	if(QFileInfo(itemFileInfo.dirPath(true)+"/"+text).exists())
	{
		KMessageBox::error(mw->getImageListView(), "<qt>"+i18n("The file '<b>%1</b>' already exists").arg(text)+"</qt>");
		return;
	}

	if ( dir.rename( itemFileInfo.fileName(), text ) )
	{
		QString itemFileName = itemFileInfo.dirPath( true ) + "/" + text;
		full = QString("%1/%2")
				.arg(itemFileInfo.dirPath( true ))
				.arg(text);

		itemFileInfo.setFile( itemFileName );

		f.setName(text);
		QIconViewItem::setText( ImageFileIconItem::text() );
	}
	else
	{
		KMessageBox::error(mw->getImageListView(), "<qt>"+i18n("The file <b>%1</b> cannot be renamed").arg(text)+"</qt>");
	}
}

bool
ImageFileIconItem::suppression (bool suprFile)
{
	if (suprFile)
		return suppression ();
	else
	{
		parentDir->removeImage (this);
		return true;
	}
}

bool
ImageFileIconItem::suppression ()
{
	if (QDir().remove (fullName ()))
	{
		parentDir->removeImage (this);
		return true;
	}
	else
	{
		return false;
	}
}

bool
ImageFileIconItem::moveToTrash()
{
	return suppression(false);
}

bool
ImageFileIconItem::shred()
{
	suppression(false);
	return true;
}


QString
ImageFileIconItem::shrink(const QString& str, int len) const
{
	if(str.length()<=(unsigned int)len)
		return str;
	else
	{
		return str.left(len/2) + "..." + str.right(len/2);
	}
}

QStringList
ImageFileIconItem::toolTipArgs() const
{
	QStringList args( FileIconItem::toolTipArgs() );

	if (QFileInfo(fullName()).extension().lower()==QString::fromLatin1("jpg"))
		args << i18n("Dimension:") << ProcessFile(QFile::encodeName(fullName()),true);

	return args;
}

QString
ImageFileIconItem::text (int column) const
{
	if (column == 0)
		return KIconViewItem::text();
	else if (column == 1)
		return extension;
	else if (column == 2)
		return QString::number(m_size);
	else
		return getType();
}

void
ImageFileIconItem::initDimension()
{
	if(!isImage()) return;

	KFileMetaInfo metaInfo = fileInfo()->metaInfo();
	QString value;
	if(metaInfo.isValid())
	{
		value = metaInfo.item("Dimensions").string();
	}
	else
		return;
	bool ok;
	QRegExp rx( "^(\\d+)( x )(\\d+)" );
	rx.search( value );
	QStringList list = rx.capturedTexts();
	m_dimension=QSize(list[1].toUInt(&ok), list[3].toUInt(&ok));
}

int
ImageFileIconItem::compare ( QIconViewItem * i ) const
{
	int r;
	if( ((FileIconItem*)i)->getType()==getType())
		r = FileIconItem::compare(i);
	else
		r=1;
	return r;
}

