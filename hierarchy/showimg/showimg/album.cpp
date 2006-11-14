/*****************************************************************************
                          album.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#include "album.h"

// Local
#include "albumimagefileiconitem.h"
#include "directoryview.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"

// Qt
#include <qfile.h>
#include <qtextstream.h>

// KDE
#include <klocale.h>
#include <kapplication.h>
#include <kurl.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

Album::Album(
		ListItem        * a_p_parent,
		const QString&    a_filename)
	:ListItem(a_p_parent, a_filename)
{
	setFullName(this->parent()->fullName()+name());
	init();
}

Album::~Album()
{
}

void
Album::init()
{
	setPixmap(0, BarIcon("imagegallery", getListItemView()->getIconSize() ));
	setExtension(i18n("Album"));
	setDropEnabled(true);
	setReadOnly(false);
}

void
Album::addURL(const KURL::List& a_list)
{
	QFile f(fullName());
	if (!f.open(IO_Raw | IO_ReadWrite | IO_Append) )
		return;
	QTextStream stream(&f);
	for (uint i = 0; i < a_list.count (); i++)
	{
		stream << pathTo(a_list[i].path()) << '\n';
	}

	if(getSize()>0) 
		setSize(getSize() + a_list.count());
	repaint();
	f.close();

}

QString
Album::pathTo(const QString& fileName)
{
	int i=0;
	while(fileName[i] == fullName()[i] && (unsigned int)i<fileName.length() && (unsigned int)i< fullName().length())
		i++;
	if(fileName[i]=='/') i--;
	int pos=fileName.findRev('/', i, false);
	QString rFileName=fileName.right(fileName.length()-pos-1);
	QString rFullName=fullName().right(fullName().length()-pos-1);
	for(i=0; i<rFullName.contains('/', false); i++)
		rFileName="../"+rFileName;
	return rFileName;

}


void
Album::load (bool )
{
	ListItem::load(true);

	///////////////////////////////////////////////////////////////////////////
	QString currentPath=QFileInfo(fullName()).dirPath(true);
	QFile f(fullName());
	if (!f.open(IO_ReadOnly) )
	{
		KApplication::restoreOverrideCursor();
		MYWARNING<<i18n("Unable to open album %1.").arg(fullName())<<endl;
		return;
	}
	///////////////////////////////////////////////////////////////////////////
	getMainWindow()->getDirectoryView()->loadingIsStarted(this, 51);

	///////////////////////////////////////////////////////////////////////////
	QTextStream ts(&f);
	QString lut;
	int i=0;
	AlbumImageFileIconItem *item;
	while(!ts.eof())
	{
		lut=currentPath+'/'+ts.readLine ();
		QFileInfo fi(lut);
		if(fi.exists())
		{
			item=new AlbumImageFileIconItem(this, QDir::cleanDirPath(lut));
			m_list.append(item);
			i++;
		}
	}
	f.close();
	///////////////////////////////////////////////////////////////////////////
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, 51);
}


void
Album::unLoad ()
{
	if(getMainWindow()->preview())
		getMainWindow()->getImageListView()->stopLoading();

	getMainWindow()->slotRemoveImage(m_list.count());
	for (ImageFileIconItem *item = m_list.first(); item; item = m_list.next() )
		getMainWindow()->getImageListView()->takeItem(item);
	m_list.clear();

	getMainWindow()->getImageViewer()->updateStatus();
	getMainWindow()->getImageListView()->slotUpdate();
	if(getMainWindow()->preview())
		getMainWindow()->getImageListView()->slotLoadFirst();
}


void
Album::removeIconItem ( FileIconItem* a_p_item)
{
	AlbumImageFileIconItem *l_p_album_item = dynamic_cast<AlbumImageFileIconItem*>(a_p_item);
	if(l_p_album_item)
	{
		m_list.remove(m_list.find(l_p_album_item));
		getMainWindow()->getImageListView()->takeItem(l_p_album_item);
		getMainWindow()->slotRemoveImage(1);

		setSize(getSize()-1);
		repaint();
	}
}

void
Album::updateChildren()
{
	setFullName(parent()->fullName()+name());
}


bool
Album::rename(const QString& newDirName, QString& )
{
#ifndef Q_WS_WIN //TODO
	KonqOperations::rename(getMainWindow(), getURL(), newDirName);
	//getFile().setName(newDirName);
	getURL().setPath(newDirName);
	updateChildren();

	repaint();
#endif
	return true;
}

