/***************************************************************************
                          compressedimagefileiconitem.cpp -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult, 2003 OGINO Tomonori
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
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

#include "compressedimagefileiconitem.h"

// Local
#include "zipfile.h"
#include "imageviewer.h"
#include "compressedfileitem.h"
#include "mainwindow.h"

// Qt
#include <qdragobject.h>
#include <qdropsite.h>
#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qstring.h>

// KDE
#include <klocale.h>
#include <kprocess.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

CompressedImageFileIconItem::CompressedImageFileIconItem (
			CompressedFileItem *parentDir,
			const QString& archive,
			const QString& filename,
			MainWindow *mw)
	:ImageFileIconItem(
		parentDir,
		filename,
		locateLocal("tmp", "showimg-cpr/")+QFileInfo(archive).fileName(),
		mw)
{
	this->parentDir = parentDir;
	this->archive = archive;
	this->filename = filename;

	m_size = -1;
	readable = TRUE;

	extension = filename.right(3).lower();
	setType("zip");
	setName("CompressedImageFileIconItem");
	setKey(mw->getImageListView()->getCurrentKey());

	setPixmap(BarIcon(fileInfo()->iconName(), mw->getImageListView()->getCurrentIconSize().width()/2));

	__ismovable__=false;
}

CompressedImageFileIconItem::~CompressedImageFileIconItem ()
{
}

QString
CompressedImageFileIconItem::toolTipStr() const
{
	QString tip="<table><tr><td>"
		+i18n("<b>name</b>: %1<br><b>archive</b>: %2<br>")
			.arg(text())
			.arg(archive)
		+"</td></tr></table>";
	tip+=fileInfo()->getToolTipText();

	return tip;
}


bool
CompressedImageFileIconItem::suppression (bool )
{
	KApplication::setOverrideCursor (waitCursor); // this might take time

	QString archiveFull = parentDir->fullName();
	ZipFile (archiveFull, filename).deleteFile ();
	parentDir->removeImage (this);

	KApplication::restoreOverrideCursor ();       // restore original cursor

	return true;
}

bool
CompressedImageFileIconItem::moveToTrash()
{
#ifndef Q_WS_WIN
	KURL list;
	list.setPath(fullName());
	KonqOperations::del(iconView(), KonqOperations::TRASH, list);
	return suppression(true);
#else
	return false;
#endif
}

QString
CompressedImageFileIconItem::text () const
{
	return text (0);
}


QString
CompressedImageFileIconItem::text (int column) const
{

	if (column == 0)
	{
		QString s(f.name ());
		int pos = s.findRev ("/");
		return s.right(s.length () - pos - 1);
	}
	else if (column == 1)
	{
		return (extension);
	}
	else if (column == 2)
	{
		return QString::number(m_size);
	}
	else
		return (getType());
}

