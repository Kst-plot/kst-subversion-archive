/*****************************************************************************
                          imagefileiconitem.cpp  -  description
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
#include "imagefileiconitem.h"

//-----------------------------------------------------------------------------
#define IMAGEFILEICONITEM_DEBUG        0
//-----------------------------------------------------------------------------

// Local
#include "categorydbmanager.h"
#include "directory.h"
#include "imagefileinfo.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "misc/exif.h"
#include "showimg_common.h"
#include "tools.h"

// System
#include <typeinfo>

// Qt
#include <qapplication.h>
#include <qdir.h>
#include <qdragobject.h>
#include <qdropsite.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qpixmapcache.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qurloperator.h>

// KDE
#include <kapplication.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kfilemetainfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <kurl.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

ImageFileIconItem::ImageFileIconItem (
			const QString & a_filename,
			const KURL& a_url,
			const QString & a_description/*=""*/,
			bool            a_initExtraText/*=true*/)
   : FileIconItem (a_url, a_filename)
{
#if IMAGEFILEICONITEM_DEBUG > 0
	MYDEBUG
		<< " a_filename="<<a_filename<< " "
		<< " a_url="<<a_url<< " "
		<< " a_description="<<a_description<< " "
		<< " a_initExtraText="<<a_initExtraText<< " "
		<< " fullname()="<<fullname()<< " "
	<<endl;
#endif
	setDescription(a_description);

	setRenameEnabled(false);

	bool l_has_size=true;
	setSize(fileInfo()->size(l_has_size));
	if(!l_has_size)
	{
#if IMAGEFILEICONITEM_DEBUG > 0
		MYDEBUG<<"No size found"<<endl;
#endif
		setSize(-1);
	}
	setIsImage(Tools::isImage(getURL(), mimetype()));
	setIsMovable(fileInfo()->isWritable());

	QDateTime l_datetime;
	bool l_has_date=true;
	l_datetime.setTime_t(fileInfo()->time(KIO::UDS_MODIFICATION_TIME, l_has_date));
	if(
		l_has_date &&
		Tools::isJPEG(getURL(), mimetype()) &&
		getMainWindow()->getImageListView()->getShowDate()
	)
	{
		KFileMetaInfo l_fileMetaInfo = fileInfo()->metaInfo();
		if(l_fileMetaInfo.contains("Date/time"))
		{
			KFileMetaInfoItem metainfoitem = l_fileMetaInfo.item("Date/time");
			QString s_date("---");
			if( metainfoitem.isValid())
			{
				s_date = metainfoitem.string(true).stripWhiteSpace();
			}
			if(s_date!="---")
			{
				l_datetime = QDateTime(
						 KGlobal::locale()->readDate(l_fileMetaInfo.item( "CreationDate" ).string( true ).stripWhiteSpace()),
						 KGlobal::locale()->readTime(l_fileMetaInfo.item( "CreationTime" ).string( true ).stripWhiteSpace()));
			}
		}
	}
	setDate(l_datetime);
	setExtension(mimetype());
	setKey(getMainWindow()->getImageListView()->getCurrentKey());

#ifdef WANT_LIBKEXIDB
	if(
		getMainWindow()->getCategoryDBManager() &&
		getMainWindow()->getImageListView()->getShowCategoryInfo()
	)
	{
		setCategoryList(QStringList(*(getMainWindow()->getCategoryDBManager()->getCategoryNameListImage(fullName()))));
	}
#else
	setCategoryList(QStringList());
#endif
	if(a_initExtraText)
		if(getMainWindow()->getImageListView()->getShowDimension())
			initDimension();
	setPixmap(fileInfo()->pixmap(getMainWindow()->getImageListView()->getCurrentIconSize().width()/2));
	if(a_initExtraText)
		updateExtraText();
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

	QFileInfo l_itemFileInfo( fullName() );
	QDir l_dir( l_itemFileInfo.dir() );

	if(QFileInfo(l_itemFileInfo.dirPath(true)+'/'+text).exists())
	{
		KMessageBox::error(getMainWindow()->getImageListView(), "<qt>"+i18n("The file '<b>%1</b>' already exists").arg(text)+"</qt>");
		return;
	}

	if ( l_dir.rename( l_itemFileInfo.fileName(), text ) )
	{
		QString l_itemFileName = l_itemFileInfo.dirPath( true ) + '/' + text;
		setFullname(QString("%1/%2")
				.arg(l_itemFileInfo.dirPath( true ))
				.arg(text));

		l_itemFileInfo.setFile( l_itemFileName );

		FileIconItem::setText( ImageFileIconItem::text() );
	}
	else
	{
		KMessageBox::error(getMainWindow()->getImageListView(), "<qt>"+i18n("The file <b>%1</b> cannot be renamed").arg(text)+"</qt>");
	}
}

bool
ImageFileIconItem::suppression (bool suprFile)
{
	if (suprFile)
		return suppression ();
	else
	{
		//FIXME getParentDir()->removeImage (this);
		return true;
	}
}

bool
ImageFileIconItem::suppression ()
{
	if (QDir().remove (fullName ()))
	{
		//FIXME getParentDir()->removeImage (this);
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

	//if (QFileInfo(fullName()).extension().lower()==QString::fromLatin1("jpg"))
	//	args << i18n("Dimension:") << ProcessFile(QFile::encodeName(fullName()),true);

	return args;
}

QString
ImageFileIconItem::text (int column) const
{
	if (column == 0)
		return KIconViewItem::text();
	else if (column == 1)
		return getExtension();
	else if (column == 2)
		return QString::number(getSize());
	else
		return QString();
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
	setDimension(QSize(list[1].toUInt(&ok), list[3].toUInt(&ok)));
}

int
ImageFileIconItem::compare ( QIconViewItem * a_p_item ) const
{
	int r = 1;
	if( typeid(*a_p_item) == typeid(*this))
		r = FileIconItem::compare(a_p_item);
	return r;
}

