/*****************************************************************************
                          albumimagefileiconitem.cpp  -  description
                             -------------------
    begin                : Sun Jan 6 2002
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

#include "albumimagefileiconitem.h"

// Local
#include "album.h"
#include "directory.h"
#include "imagefileinfo.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "misc/exif.h"
#include "showimg_common.h"

// KDE
#include <klocale.h>

AlbumImageFileIconItem::AlbumImageFileIconItem(
			Album         * a_p_album,
			const QString & a_fullname)
	:ImageFileIconItem(
		QFileInfo(a_fullname).fileName(),
		QFileInfo(a_fullname).dirPath(true)+'/'),

	m_p_album(a_p_album)

{
	setIsImage(true);
	setIsMovable(false);

	setKey(getMainWindow()->getImageListView()->getCurrentKey());
}

AlbumImageFileIconItem::~AlbumImageFileIconItem()
{
}

QString
AlbumImageFileIconItem::toolTipStr() const
{
	QString tip;
	ImageFileInfo iminfo(fullname(), IMAGE, true);
	tip = "<table><tr><td>"
		+i18n("<b>name</b>: %1<br><b>album</b>: %2<br><b>location</b>: %3<br>%4%5")
			.arg(name())
			.arg(m_p_album->name())
			.arg(shrink(path()))
			//.arg(QFileInfo(fullname()).extension().lower() == QString::fromLatin1("jpg")?i18n("<b>dimension</b>: ")+ProcessFile(QFile::encodeName(fullname()),true):QString())
			.arg(iminfo.hasInfo()?i18n("<br><b>description</b>: %1").arg(iminfo.getTitle()):QString())
		+"</td></tr></table>";
	tip+=fileInfo()->getToolTipText();

	return tip;
}

void
AlbumImageFileIconItem::removeEntry()
{
	QFile f(m_p_album->fullName());
	QString out;
	if ( f.open(IO_ReadOnly) )
	{    // file opened successfully
		QTextStream t( &f );        // use a text stream
		QString s;
		QString rPath=m_p_album->pathTo(fullName());
		while ( !t.eof() )
		{        // until end of file...
			s = t.readLine();       // line of text excluding '\n'
			if(s!=rPath)
				out+=s+'\n';
		}
		f.close();
		f.open(IO_WriteOnly);
		QTextStream tw( &f );
		tw << out;
		f.close();
	}
	m_p_album->removeIconItem(this);
}


bool
AlbumImageFileIconItem::suppression()
{
	removeEntry();
	return true;
}

bool
AlbumImageFileIconItem::suppression(bool )
{
	removeEntry();
	return false;
}

bool
AlbumImageFileIconItem::moveToTrash()
{
	removeEntry();
	return true;
}


bool
AlbumImageFileIconItem::shred()
{
	removeEntry();
	return true;
}
