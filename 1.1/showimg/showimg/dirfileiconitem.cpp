/***************************************************************************
                          dirfileiconitem.cpp  -  description
                             -------------------
    begin                : dim jun 23 2002
    copyright            : (C) 2002 by Richard Groult
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

#include "dirfileiconitem.h"

// Local
#include "imageloader.h"
#include "directory.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "imagefileinfo.h"
#include "exif.h"
#include "mainwindow.h"

// Qt
#include <qdragobject.h>
#include <qdropsite.h>
#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qiconview.h>
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
	if(filename.compare("..")==0) {
		setSelectable(false);
		__hasToolTip__ = false;
	}
	else
		__ismovable__=true;

	setRenameEnabled(false);

// 	size = QString("%1").arg(full, 50).lower();

//	QDateTime epoch( QDate( 1980, 1, 1 ) );
	m_date = QFileInfo(full).lastModified();

	extension = "000"+full;

	setType("dir");
	setName("DirFileIconItem");

	setPixmap(fileInfo()->pixmap(mw->getImageListView()->getCurrentIconSize().width()/2));
	haspreview=true;

	updateExtraText();
	calcRect();
}


DirFileIconItem::~DirFileIconItem ()
{
}

void
DirFileIconItem::setText( const QString& text )
{
	if(text==DirFileIconItem::text())
		return;

	QFileInfo *itemFileInfo = new QFileInfo( full );
	QDir dir( itemFileInfo->dir() );

	if(QFileInfo(itemFileInfo->dirPath(TRUE)+"/"+text).exists())
	{
		KMessageBox::error(mw->getImageListView(), "<qt>"+i18n("The directory '<b>%1</b>' already exists").arg(text)+"</qt>");
		delete itemFileInfo;
		return;
	}

	if ( dir.rename( itemFileInfo->fileName(), text ) )
	{
		QString itemFileName = itemFileInfo->dirPath( TRUE ) + "/" + text;
		full = QString("%1/%2")
			.arg(itemFileInfo->dirPath( TRUE ))
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

