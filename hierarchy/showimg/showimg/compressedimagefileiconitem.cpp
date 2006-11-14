/***************************************************************************
                          compressedimagefileiconitem.cpp -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult, 2003 OGINO Tomonori
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "compressedimagefileiconitem.h"

// Local
#include "compressedfileitem.h"
#include "imageviewer.h"
#include "mainwindow.h"

// Qt
#include <qdragobject.h>
#include <qdropsite.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qpopupmenu.h>
#include <qstring.h>

// KDE
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

CompressedImageFileIconItem::CompressedImageFileIconItem (
			const QString& a_archive_fullpath,
			const QString& a_filename)
	:ImageFileIconItem(
//		a_p_parentDir,
		a_filename,
		locateLocal("tmp", "showimg-cpr/")+QFileInfo(a_archive_fullpath).fileName())
{
	m_archive_fullpath     = a_archive_fullpath;

	setSize(-1);

	setExtension (a_filename.right(3).lower());
// 	setType("zip");
	setKey(getMainWindow()->getImageListView()->getCurrentKey());

	setPixmap(BarIcon(fileInfo()->iconName(), getMainWindow()->getImageListView()->getCurrentIconSize().width()/2));

	setIsMovable(false);
}

CompressedImageFileIconItem::~CompressedImageFileIconItem ()
{
}

QString
CompressedImageFileIconItem::toolTipStr() const
{
	QString tip="<table><tr><td>"
		+i18n("<b>name</b>: %1<br><b>m_archive</b>: %2<br>")
			.arg(text())
			.arg(m_archive_fullpath)
		+"</td></tr></table>";
	tip+=fileInfo()->getToolTipText();

	return tip;
}


bool
CompressedImageFileIconItem::suppression (bool )
{
/*
	KApplication::setOverrideCursor (waitCursor); // this might take time

	QString archiveFull = m_p_parentDir->fullName();
	ZipFile (archiveFull, m_filename).deleteFile ();
	m_p_parentDir->removeImage (this);

	KApplication::restoreOverrideCursor ();       // restore original cursor

	return true;
*/
	return false;
}

bool
CompressedImageFileIconItem::moveToTrash()
{
#ifndef Q_WS_WIN
/*	KURL list;
	list.setPath(fullName());
	KonqOperations::del(iconView(), KonqOperations::TRASH, list);
	return suppression(true);
*/
	return false;
#else
	return false;
#endif
}

/*
QString
CompressedImageFileIconItem::text (int column) const
{

	if (column == 0)
	{
		QString s(getFile().name ());
		int pos = s.findRev ("/");
		return s.right(s.length () - pos - 1);
	}
	else if (column == 1)
	{
		return (getExtension());
	}
	else if (column == 2)
	{
		return QString::number(getSize());
	}
	else
		return (getType());
}
*/
