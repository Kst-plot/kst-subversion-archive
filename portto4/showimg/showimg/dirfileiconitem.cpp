/*****************************************************************************
                          dirfileiconitem.cpp  -  description
                             -------------------
    begin                : dim jun 23 2002
    copyright            : (C) 2002-2005 by Richard Groult
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

#include "dirfileiconitem.h"

// Local
#include "directory.h"
#include "imagefileinfo.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"

// System
#include <typeinfo>

// Qt
#include <qdir.h>
#include <qdragobject.h>
#include <qdropsite.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpixmapcache.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qurloperator.h>

// KDE
#include <kapplication.h>
#include <kfilemetainfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpixmap.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

DirFileIconItem::DirFileIconItem (
			const QString& a_filename,
			const QString& a_path,
			const QString& a_description):
   FileIconItem (a_path, a_filename)
{
	setDescription(a_description);

	setFullname(fullname().append (a_path));
	setFullname(fullname().append (a_filename));
	if(a_filename.compare("..")==0)
	{
		setSelectable(false);
		setHasTooltip(false);
	}
	else
	{
		setIsMovable(true);
	}
	setDate(QFileInfo(fullname()).lastModified());
	setRenameEnabled(false);

	setExtension("000"+fullname());

	setKey(getMainWindow()->getImageListView()->getCurrentKey());

	setPixmap(fileInfo()->pixmap(getMainWindow()->getImageListView()->getCurrentIconSize().width()/2));
	setHaspreview(true);
	setHasTooltip(false);

	updateExtraText();
	calcRect();
}


DirFileIconItem::~DirFileIconItem ()
{
}

void
DirFileIconItem::setSelected (bool s)
{
	KIconViewItem::setSelected (s);
	if ( s )
		getMainWindow()->getImageListView()->load(NULL);
}


void
DirFileIconItem::setText( const QString& text )
{
	if(text==DirFileIconItem::text())
		return;

	QFileInfo *l_p_itemFileInfo = new QFileInfo( fullname() );
	QDir dir( l_p_itemFileInfo->dir() );

	if(QFileInfo(l_p_itemFileInfo->dirPath(true)+'/'+text).exists())
	{
		KMessageBox::error(getMainWindow()->getImageListView(), "<qt>"+i18n("The directory '<b>%1</b>' already exists").arg(text)+"</qt>");
		delete l_p_itemFileInfo;
		return;
	}

	if ( dir.rename( l_p_itemFileInfo->fileName(), text ) )
	{
		QString itemFileName = l_p_itemFileInfo->dirPath( true ) + '/' + text;
		setFullname(QString("%1/%2")
			.arg(l_p_itemFileInfo->dirPath( true ))
			.arg(text));

		delete l_p_itemFileInfo;
		l_p_itemFileInfo = new QFileInfo( itemFileName );

		//getFile().setName(text);
		FileIconItem::setText( DirFileIconItem::text() );
	}
	else
	{
		KMessageBox::error(getMainWindow()->getImageListView(), "<qt>"+i18n("The directory <b>%1</b> cannot be renamed").arg(text)+"</qt>");
	}
	delete l_p_itemFileInfo;
}


bool
DirFileIconItem::suppression (bool suprFile)
{
	if (suprFile)
		return suppression ();
	else
	{
		return true;
	}
}


bool
DirFileIconItem::suppression ()
{
	if (QDir().remove (fullName ()))
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool
DirFileIconItem::moveToTrash()
{
#ifndef Q_WS_WIN
	KURL list(getURL());
	KonqOperations::del(getMainWindow()->getImageListView(), KonqOperations::TRASH, list);
	return suppression(false);
#else
	return false;
#endif
}


bool
DirFileIconItem::shred()
{
	KIO::del(getURL(), true, false);
	suppression(false);
	return true;
}

int
DirFileIconItem::compare ( QIconViewItem * a_p_item ) const
{
	int r = -1;
	if( typeid(*a_p_item) == typeid(*this))
		r = FileIconItem::compare(a_p_item);
	return r;
}
