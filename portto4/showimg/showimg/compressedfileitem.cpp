/*****************************************************************************
                          compressedfileitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
                               2003 OGINO Tomonori
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
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

#include "compressedfileitem.h"

//-----------------------------------------------------------------------------
#define COMPRESSEDFILEITEM_DEBUG 0
//-----------------------------------------------------------------------------

// Local
#include "compressedimagefileiconitem.h"
#include "directoryview.h"
#include "extract.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"
#include "tools.h"

// Qt
#include <qfileinfo.h>
#include <qlistview.h>
#include <qtextcodec.h>

// KDE
#include <kapplication.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

CompressedFileItem::CompressedFileItem (
			ListItem *a_p_parent,
			const QString& a_filename,
			const QString& a_path)
	:ListItem(a_p_parent, a_filename),
	m_numberOfItems(-1)
{
#if COMPRESSEDFILEITEM_DEBUG > 0
	MYDEBUG << "a_filename="<<a_filename<<" a_path="<<a_path<<endl;
	MYDEBUG << "fullName()="<<fullName()<<endl;
#endif

	QFileInfo info(fullName());

 	setSize(info.size());
	setExtension(info.extension(false));

	setPixmap(0, BarIcon(KMimeType::iconForURL(getURL()), getListItemView()->getIconSize() ));

	setDropEnabled (false);
	setReadOnly(false);
}

CompressedFileItem::~CompressedFileItem ()
{
}

uint
CompressedFileItem::getNumberOfItems() const
{
	return m_numberOfItems;
}

void
CompressedFileItem::removeIconItem (FileIconItem* a_p_item)
{
	CompressedImageFileIconItem * l_p_comp_item = dynamic_cast<CompressedImageFileIconItem*>(a_p_item);
	if(l_p_comp_item)
	{
		if(m_list.find(l_p_comp_item)!=-1)
		{
			delete(l_p_comp_item);
			getMainWindow()->slotRemoveImage(1);
		}
	}
}

void
CompressedFileItem::load (bool /*refresh*/)
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
	//--------------------------------------------------------------------------
	ListItem::load(true);
	getMainWindow()->getDirectoryView()->loadingIsStarted(this, 51);

	//--------------------------------------------------------------------------
	Extract *extract = new Extract (fullName());
	CompressedImageFileIconItem *item = NULL;
 	m_numberOfItems=0;
	for (QStringList::iterator s = extract->files.begin(); s != extract->files.end(); ++s)
	{
		if(Tools::isImage(*s))
		{
			item = new CompressedImageFileIconItem (
						text(0),
						*s);
			m_list.append (item);
 			m_numberOfItems++;
		}
	}

	//--------------------------------------------------------------------------
	getMainWindow()->getDirectoryView()->loadingIsFinished(this, 51);
}

void
CompressedFileItem::unLoad()
{
	if(getMainWindow()->preview()) getMainWindow()->getImageListView()->stopLoading();
	getMainWindow()->slotRemoveImage(getNumberOfItems());
	getMainWindow()->getImageListView()->setUpdatesEnabled( false );

	for (FileIconItem *item = m_list.first(); item; item = m_list.next() )
		delete(item);
	m_list.clear();

	KURL url;
	url.setPath(locateLocal("tmp", "showimg-cpr/"+text(0)));
	KIO::del( url );

	getMainWindow()->getImageViewer()->updateStatus();
	getMainWindow()->getImageListView()->setUpdatesEnabled( true );
 	getMainWindow()->getImageListView()->repaintContents  ( false );
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
	setFullName(parent()->fullName()+name());
}

bool
CompressedFileItem::rename(const QString& newDirName, QString& msg)
{
#ifndef Q_WS_WIN //TODO
	KonqOperations::rename(getMainWindow(), getURL(), newDirName);
	//FIXME getFile().setName(newDirName);
	updateChildren();

	repaint();
#endif
	msg="";
	return true;
}


