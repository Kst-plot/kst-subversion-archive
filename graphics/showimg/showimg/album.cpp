/***************************************************************************
                          album.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#include "album.h"

// Local 
#include "directory.h"
#include "directoryview.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "albumimagefileiconitem.h"

// Qt 
#include <qtextstream.h>
#include <qfile.h>

// KDE 
#include <klocale.h>
#include <kapplication.h>
#ifndef Q_WS_WIN
# include <konq_operations.h>
#endif
#include <kurl.h>

Album::Album(
		ListItem * parent, 
		const QString& filename,
		MainWindow * mw)
	:ListItem(parent, filename, mw)
{
	full=this->parent()->fullName()+f.name();
	size=QString("");
	init();
}

Album::~Album()
{
}

void
Album::init()
{
	setPixmap(0, BarIcon("imagegallery", getListItemView()->getIconSize() ));
	extension = i18n("Album");
	setDropEnabled(true);
	setType("Album");
	setReadOnly(FALSE);
}

void
Album::addURL(const QStringList& lst)
{
	QFile f(fullName());
	if (!f.open(IO_Raw | IO_ReadWrite | IO_Append) )
		return;
	KURL::List list(lst);
	QTextStream stream(&f);
	for (uint i = 0; i < list.count (); i++)
	{
		stream << pathTo(list[i].path()) << '\n';
	}
	
	if(!size.isEmpty()) size=QString("%1").arg(size.toInt()+lst.count());
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
	bool hasIm=mw->getImageListView()->hasImages();
	mw->setMessage(i18n("Loading album %1...").arg(text(0)));
	if(!mw->getImageListView()->hasImages()) { mw->getImageViewer()->loadImage(); mw->getImageListView()->setContentsPos(0,0); }
	KApplication::setOverrideCursor( waitCursor ); // this might take time
	kapp->processEvents();
	mw->getImageListView()->setUpdatesEnabled( FALSE );
	mw->getImageListView()->stopLoading();

	QString currentPath=QFileInfo(fullName()).dirPath(true);
	QFile f(fullName());
	if (!f.open(IO_ReadOnly) )
	{
		KApplication::restoreOverrideCursor();
		kdWarning()<<i18n("Unable to open album %1.").arg(fullName())<<endl;
		return;
	}
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
			item=new AlbumImageFileIconItem(this, QDir::cleanDirPath(lut), mw);
			list.append(item);
			i++;
		}
	}
	f.close();
	
	mw->slotAddImage(i);
	size = QString("%1").arg(i);
	mw->getImageListView()->sort();
	mw->getImageViewer()->updateStatus();
	
	mw->getImageListView()->setUpdatesEnabled( TRUE );
	KApplication::restoreOverrideCursor();	// restore original

	repaint(); kapp->processEvents();
	if(!hasIm && getListItemView()->loadFirstImage()) mw->getImageListView()->first();
  
	mw->getImageListView()->slotLoadFirst();
	mw->getImageListView()->slotUpdate() ;
	mw->setMessage(i18n("Ready"));
}


void
Album::unLoad ()
{
	if(mw->preview())
		mw->getImageListView()->stopLoading();
	
	mw->slotRemoveImage(list.count());
	for (ImageFileIconItem *item = list.first(); item; item = list.next() )
		mw->getImageListView()->takeItem(item);
	list.clear();

	mw->getImageViewer()->updateStatus();
	mw->getImageListView()->slotUpdate();
	if(mw->preview())
		mw->getImageListView()->slotLoadFirst();
}


void
Album::removeImage (AlbumImageFileIconItem *imf)
{
	list.remove(list.find(imf));
	mw->getImageListView()->takeItem(imf);
	mw->slotRemoveImage(1);

	size = QString("%1").arg(size.toInt()-1);
	repaint();
}

void
Album::updateChildren()
{
	full = parent()->fullName()+f.name();
}

ListItem*
Album::find (const char *)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::find (const char *)" << endl;
	return NULL;
}

void
Album::goTo (const char *)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::goTo (const char *)" << endl;
}


void
Album::removeImage ( ListItem* )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::removeImage ( ListItem* )" << endl;
}


void
Album::create(const QString& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::create(QString )" << endl;
}

bool
Album::rename(const QString& newDirName, QString& )
{
#ifndef Q_WS_WIN //TODO
	KonqOperations::rename(mw, getURL(), newDirName);
	f.setName(newDirName);
	updateChildren();
	
	repaint();
#endif
	return true;
}

void
Album::rename()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::rename()" << endl;
}

void
Album::properties()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::properties()" << endl;
}

bool
Album::add(const QStringList& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO Album::add(QStringList )" << endl;
	return false;
}

