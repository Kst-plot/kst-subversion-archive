/***************************************************************************
                          compressedfileitem.cpp  -  description
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

#include "compressedfileitem.h"

// Local
#include "extract.h"
#include "directoryview.h"
#include "imageviewer.h"
#include "compressedimagefileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "directoryview.h"
#include "directory.h"

// Qt
#include <qlistview.h>
#include <qfileinfo.h>
#include <qtextcodec.h>

// KDE
#include <kprocess.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

CompressedFileItem::CompressedFileItem (
			Directory *parent,
			const QString& filename,
			const QString& path,
			MainWindow *mw)
	:ListItem(parent, filename, mw)
{
	full.append (path);
	full.append (filename);

	QFileInfo info(fullName());
 	size = QString("%1").arg(info.size());
	extension = info.extension(false);

	setPixmap(0, BarIcon(KMimeType::iconForURL(getURL()), getListItemView()->getIconSize() ));

	setDropEnabled (FALSE);
	setReadOnly(FALSE);
}

CompressedFileItem::~CompressedFileItem ()
{
}

void
CompressedFileItem::removeImage (CompressedImageFileIconItem * imf)
{
	if(!imf)
		return;

	if(list.find(imf)!=-1)
	{
		delete(imf);
		mw->slotRemoveImage(1);
	}
}

void
CompressedFileItem::load (bool refresh)
{

	// Warn when the size of archive more than 'big(64MB).'
	QFile::Offset big = 0x4000000;// 64MB
	QFile qfile( fullName() );
	if( qfile.size() > big )
	{
		QString msg = i18n("The size of selected archive seems to be too big;\ncontinue? (size: %1MB)").arg( (qfile.size())>>20 );
		if ( KMessageBox::warningContinueCancel( 0, msg, i18n("Confirm")) == KMessageBox::Cancel)
			return;
	}

	///
	if(!mw->getImageListView()->hasImages())
	{
		mw->getImageViewer()->loadImage();
		mw->getImageListView()->setContentsPos(0,0);
	}

	KApplication::setOverrideCursor (waitCursor); // this might take time
	mw->setMessage(i18n("Loading '")+text(0)+"'...");
	kapp->processEvents();
	mw->getImageListView()->setUpdatesEnabled( FALSE );
	mw->getImageListView()->stopLoading();

	Extract *extract = new Extract (fullName());
	QStringList::iterator s;
	CompressedImageFileIconItem *item;

	int size=0;


	for (s = extract->files.begin(); s != extract->files.end(); ++s)
	{
		if(getListItemView()->isImage(new QFileInfo(*s)))
		{
			item = new CompressedImageFileIconItem (
						this,
						text(0),
						*s,
						mw);
			list.append (item);
			size++;
		}
	}
	mw->slotAddImage(size);
	mw->getImageListView()->sort();

	mw->getImageListView()->setUpdatesEnabled( TRUE );
	KApplication::restoreOverrideCursor();	// restore original
	mw->getImageListView()->slotUpdate() ;
	mw->getImageViewer()->updateStatus();

	if(refresh) mw->getImageListView()->slotLoadFirst();
	mw->setMessage(i18n("Ready"));
}

void
CompressedFileItem::unLoad()
{
	if(mw->preview()) mw->getImageListView()->stopLoading();
	mw->slotRemoveImage(list.count());
	mw->getImageListView()->setUpdatesEnabled( FALSE );

	for (FileIconItem *item = list.first(); item; item = list.next() )
		delete(item);
	list.clear ();

	KURL url;
	url.setPath(locateLocal("tmp", "showimg-cpr/"+text(0)));
	KIO::del( url );

	mw->getImageViewer()->updateStatus ();
	mw->getImageListView()->setUpdatesEnabled( TRUE );
	mw->getImageListView()->slotUpdate();
}

QString
CompressedFileItem::key (int column, bool ascending) const
{
	if(column!=1)
		return KListViewItem::key(column, ascending).lower();
	else
		return  QString("ZZ")+text(1);
}

void
CompressedFileItem::updateChildren()
{
	full = parent()->fullName()+f.name();
}

bool
CompressedFileItem::rename(const QString& newDirName, QString& msg)
{
#ifndef Q_WS_WIN //TODO
	KonqOperations::rename(mw, getURL(), newDirName);
	f.setName(newDirName);
	updateChildren();

	repaint();
#endif
	msg="";
	return true;
}


