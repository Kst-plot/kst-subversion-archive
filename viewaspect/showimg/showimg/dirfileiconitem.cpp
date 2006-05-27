/***************************************************************************
                          dirfileiconitem.cpp  -  description
                             -------------------
    begin                : dim jun 23 2002
    copyright            : (C) 2002-2005 by Richard Groult
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

#include "dirfileiconitem.h"

// Local
#include "imageloader.h"
#include "directory.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "imagefileinfo.h"
#include "mainwindow.h"

// Qt
#include <qdropsite.h>
#include <qstring.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdragobject.h>
#include <qlabel.h>
#include <qpixmapcache.h>
#include <qurloperator.h>

// KDE
#include <kiconloader.h>
#include <kfilemetainfo.h>
#include <kapplication.h>
#include <kpixmap.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#include <kio/job.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

DirFileIconItem::DirFileIconItem (
			Directory *parentDir,
			const QString& filename,
			const QString& path,
			MainWindow* mw,
			QString description):
   FileIconItem (parentDir, path, filename, mw)
{
	this->description=description;

	full.append (path);
	full.append (filename);
	if(filename.compare("..")==0)
	{
		setSelectable(false);
		setHasTooltip(false);
	}
	else
	{
		setIsMovable(true);
	}
	m_date = QFileInfo(full).lastModified();
	setRenameEnabled(false);

	extension = "000"+full;

	setType("directory");

	setKey(mw->getImageListView()->getCurrentKey());

	setPixmap(fileInfo()->pixmap(mw->getImageListView()->getCurrentIconSize().width()/2));
	haspreview=true;
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
		mw->getImageListView()->load(NULL);
}


void
DirFileIconItem::setText( const QString& text )
{
	if(text==DirFileIconItem::text())
		return;

	QFileInfo *itemFileInfo = new QFileInfo( full );
	QDir dir( itemFileInfo->dir() );

	if(QFileInfo(itemFileInfo->dirPath(true)+"/"+text).exists())
	{
		KMessageBox::error(mw->getImageListView(), "<qt>"+i18n("The directory '<b>%1</b>' already exists").arg(text)+"</qt>");
		delete itemFileInfo;
		return;
	}

	if ( dir.rename( itemFileInfo->fileName(), text ) )
	{
		QString itemFileName = itemFileInfo->dirPath( true ) + "/" + text;
		full = QString("%1/%2")
			.arg(itemFileInfo->dirPath( true ))
			.arg(text);

		delete itemFileInfo;
		itemFileInfo = new QFileInfo( itemFileName );

		f.setName(text);
		QIconViewItem::setText( DirFileIconItem::text() );
	}
	else
	{
		KMessageBox::error(mw->getImageListView(), "<qt>"+i18n("The directory <b>%1</b> cannot be renamed").arg(text)+"</qt>");
	}
	delete itemFileInfo;
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
	KonqOperations::del(mw->getImageListView(), KonqOperations::TRASH, list);
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

QString
DirFileIconItem::text (int column) const
{

	if (column == 0)
		return KIconViewItem::text();
	else if (column == 1)
		return (extension);
	else if (column == 2)
		return QString::number(m_size);
	else
		return (getType());
}

int
DirFileIconItem::compare ( QIconViewItem * i ) const
{
	int r;
	if( ((FileIconItem*)i)->getType()==getType())
		r = FileIconItem::compare(i);
	else
		r=-1;
	return r;
}

void
DirFileIconItem::setKey ( const QString& k )
{
	FileIconItem::setKey(k);
}

