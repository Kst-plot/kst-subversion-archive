/***************************************************************************
                          listitemview.cpp  -  description
                             -------------------
    begin                : 28 Dec 2004
    copyright            : (C) 2004 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "listitemview.h"

// Local
#include "describealbum.h"
#include "directory.h"
#include "imageviewer.h"
#include "imagefileiconitem.h"
#include "compressedimagefileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "album.h"
#include "listitem.h"
#include "cdarchivecreatordialog.h"
#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */
#ifdef WANT_LIBKEXIDB
#include "categorylistitem.h"
#include "categorydbmanager.h"
#endif /* WANT_LIBKEXIDB */

// Qt
#include <qstring.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qprogressdialog.h>
#include <qdropsite.h>
#include <qdragobject.h>
#include <qtooltip.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qheader.h>

// KDE
#include <kurlrequesterdlg.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <kglobalsettings.h>
#include <kpropertiesdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kdirwatch.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kimageio.h>
#include <kmimetype.h>
#include <kapplication.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif
#if  KDE_VERSION >= 0x30200
#include <kinputdialog.h>
#endif

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

ListItemView::ListItemView (
			QWidget *parent,
			MainWindow *mw,
			const char* name)
	: KListView (parent, name)
{
	this->mw = mw;

	setRootIsDecorated(true);

	addColumn (i18n("Name"));
	addColumn (i18n("Type"));
	addColumn (i18n("Size"));
	addColumn (" ");

	setColumnAlignment( COLUMN_TYPE, Qt::AlignLeft );
	setColumnAlignment( COLUMN_SIZE, Qt::AlignRight );

	setAcceptDrops (true);
	setAllColumnsShowFocus(true);
	setShowToolTips(true);
	setShowSortIndicator(true);
	setSelectionModeExt (KListView::Extended);

	setFullWidth(true);

	header()->setClickEnabled(false, COLUMN_SELECT);
	header()->setStretchEnabled(true, COLUMN_NAME);
	header()->setMovingEnabled ( false );

	setColumnWidthMode (COLUMN_SELECT, Manual );
	setColumnWidth(COLUMN_SELECT, 3*ListItem::TREE_CHECKBOX_MAXSIZE/2);
	header()->setResizeEnabled (false, COLUMN_SELECT );

	setSorting (COLUMN_TYPE);
	sort();
	setColumnWidthMode (COLUMN_TYPE, Manual );
	setColumnWidth (COLUMN_TYPE, 0 );

	dropping = false;
	m_isClearingSelection=false;
	clickedItem=NULL;

	autoopenTime = 750;
	autoopen_timer = new QTimer (this);
	connect (autoopen_timer, SIGNAL (timeout ()), this, SLOT (openFolder ()));

	///
	connect(this, SIGNAL(selectionChanged ()), SLOT(slotSelectionChanged ()));
}

ListItemView::~ListItemView ()
{
}


bool
ListItemView::isImage(const QString& info)
{
	QFileInfo *finfo = new QFileInfo(info);
	bool isImg=isImage(finfo);
	delete(finfo);
	return isImg;
}


void
ListItemView::slotRename ()
{
	if(!clickedItem) clickedItem=currentItem();
	slotRename(clickedItem);
}

void
ListItemView::slotRename (ListItem *item)
{
	if(!item) return;

	ListItem *dir=item;
	QString fullname=dir->fullName();
	QString name=dir->name();
	bool ok;
	const QString newName(
				KInputDialog::getText(i18n("Rename %1").arg(name),
							  i18n("Enter new name:"),
							  name,
							  &ok,
							  mw->getImageViewer()).stripWhiteSpace());
	if(ok && !newName.isEmpty() && newName!=name)
	{
		QString msg;
		if(!dir->rename(newName, msg))
		{
			QString newDirName=dir->path()+"/"+newName;
			if(QFileInfo(newDirName).exists())
			{
				KMessageBox::error(mw->getImageViewer(), "<qt>"+msg+"</qt>");
				return;
			}

		}
	}
}

bool
ListItemView::isImage(QFileInfo *info)
{
	KMimeType::Ptr mime = KMimeType::findByPath( info->absFilePath(), 0, true );
	if( mime->is(KMimeType::defaultMimeType()) )
		mime = KMimeType::findByFileContent( info->absFilePath() );
        if(
            mime->is("image/jpeg")
         || mime->is("image/gif")
         || mime->is("image/png")
         || mime->is("image/bmp")
         || mime->is("image/tiff")
         || mime->is("image/xpm")
         || mime->is("image/xcf")
         || mime->is("image/x-xcf")
         || mime->is("image/x-gimp")
         || mime->is("image/x-xcf-gimp")
         || mime->is("image/xcf")
         || mime->is("image/ico")
         || mime->is("image/pbm")
         || mime->is("image/eps")
         || mime->is("image/krl")
         || mime->is("image/ppm")
         || mime->is("image/x-psd")
         || mime->is("image/xbm")
         || mime->is("image/pgm"))
            return true;
	else
		return KImageIO::canRead(KImageIO::type(info->filePath ()));
}

void
ListItemView::initSelectedListItem()
{
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		initSelectedListItem(rootItems);
		rootItems = rootItems->nextSibling();
	}
	setSelectionMode (QListView::Single);
	oldCurrent=currentItem();
}

void
ListItemView::initSelectedListItem(ListItem *item)
{
	ListItem *myChild = item->firstChild();
	if(item->isSelected())
		oldCurrents.append(item);
	while( myChild )
	{
		initSelectedListItem( myChild );
		myChild = myChild->nextSibling();
	}
}

void
ListItemView::restoreSelectedListItem()
{
	setUpdatesEnabled(false);

	clearSelection ();
	setSelectionMode (QListView::Extended);
	ListItem *item;
	for ( item = oldCurrents.first(); item; item = oldCurrents.next() )
		setSelected (item, true);
	setCurrentItem (oldCurrent);
	oldCurrents.clear();
	oldCurrent=NULL;

	setUpdatesEnabled(true);
}


void
ListItemView::slotSelectionChanged ()
{
	if(isDropping()) return;
	updateActions(currentItem());
	if(!mw->getImageListView()->hasImageSelected()) mw->getImageListView()->load(NULL);
#ifdef HAVE_KIPI
	if(mw->pluginManager()) mw->pluginManager()->currentAlbumChanged(currentItem()->fullName());
#endif
}

void
ListItemView::contentsDragEnterEvent (QDragEnterEvent * event)
{
	dropping = true;
	initSelectedListItem();
	if (QTextDrag::canDecode (event))
	{
		event->acceptAction ();
		ListItem *i = itemAt (contentsToViewport (event->pos ()));
		if (i)
		{
			dropItem = i;
			autoopen_timer->start (autoopenTime);
		}
	}
}

bool
ListItemView::isClearingSelection()
{
	return m_isClearingSelection;
}

void
ListItemView::clearSelection ()
{
	setUpdatesEnabled(false);
	m_isClearingSelection=true;
	KListView::clearSelection();
	m_isClearingSelection=false;
	setUpdatesEnabled(true);
}
void
ListItemView::contentsDragLeaveEvent (QDragLeaveEvent *)
{
	autoopen_timer->stop ();
	restoreSelectedListItem();
	dropping = false;
}

void
ListItemView::contentsDragMoveEvent (QDragMoveEvent * e)
{
	if(!dropping)
		return;

	if (!QTextDrag::canDecode (e))
	{
		e->ignore ();
		return;
	}

	ListItem *i = itemAt (contentsToViewport ( e->pos ()));
	if (i)
	{
		setSelected (i, true);
		e->acceptAction ();
		if (i != dropItem)
		{
			autoopen_timer->stop ();
			dropItem = i;
			autoopen_timer->start (autoopenTime);
		}
	}
	else
	{
		e->ignore ();
		autoopen_timer->stop ();
		dropItem = NULL;
	}
}
void
ListItemView::contentsDropEvent_begin()
{
	autoopen_timer->stop ();
}
void
ListItemView::contentsDropEvent (QDropEvent * )
{
	contentsDropEvent_begin();
	contentsDropEvent_end();
}

void
ListItemView::contentsDropEvent_end()
{
	contentsDragLeaveEvent(NULL);
}



void
ListItemView::contentsMouseMoveEvent ( QMouseEvent * e )
{
	if(e->state () != LeftButton)
		KListView::contentsMouseMoveEvent(e);
}


void
ListItemView::contentsMousePressEvent(QMouseEvent * e)
{
#ifndef Q_WS_WINdirView
	if (e->button () == MidButton) {
		contentsMouseDoubleClickEvent(e);
	}
	else
#endif
	if (e->button () == LeftButton)
	{
		if(header()->sectionAt(e->x())!=3)
			KListView::contentsMousePressEvent (e);
		else
		{
			ListItem *item = itemAt(contentsToViewport(e->pos()));
			setSelected(item, !item->isSelected());
		}
	}
	else
	if (e->button () == RightButton)
	{
		clickedItem = itemAt(contentsToViewport(e->pos()));
		updateActions(clickedItem);
		if (clickedItem)
		{
			//KListView::contentsMousePressEvent (e);
			popup->changeTitle(1, *clickedItem->pixmap(0), clickedItem->text(0));
			popup->exec(e->globalPos());
		}
	}
}

void
ListItemView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
	ListItem *item=itemAt(contentsToViewport(e->pos()));
	if(!item)
		return;
	if(item->isOpen())
		item->setOpen(false);
	else
		item->setOpen(true);
}


void
ListItemView::contentsMouseReleaseEvent(QMouseEvent* e)
{
	if (e->button () == LeftButton)
	{
		ListItem *item=itemAt(contentsToViewport(e->pos()));
		if(item)
			if(item->isSelected())
				return;
	}
	KListView::contentsMouseReleaseEvent(e);
}

QDragObject*
ListItemView::dragObject ()
{
	return NULL;
}


void
ListItemView::recursivelyOpen()
{
	if(!clickedItem) clickedItem=currentItem();
	recursivelyOpen(clickedItem);
}

void
ListItemView::recursivelyOpen(ListItem *item)
{
	if(!clickedItem) clickedItem=currentItem();
	if(!clickedItem) return;
	((Directory*)item)->recursivelyOpen();
}


void
ListItemView::openFolder ()
{
	autoopen_timer->stop ();
	if (dropItem && !dropItem->isOpen ())
	{
		setOpen(dropItem, true);
	}
}

void
ListItemView:: slotSuppr()
{
	if(!clickedItem) clickedItem=currentItem();
	slotSuppr(clickedItem);
}


void
ListItemView::goToNextDir()
{
	if(!currentItem()) return;
	ListItem * item = currentItem()->itemBelow();
	if(!item) return;
	ensureItemVisible(item);
	clearSelection ();
	setCurrentItem(item);
	item->setSelected(true);
}

void
ListItemView::goToPreviousDir()
{
	if(!currentItem()) return;
	ListItem * item = currentItem()->itemAbove();
	if(!item) return;
	ensureItemVisible(item);
	clearSelection ();
	setCurrentItem(item);
	item->setSelected(true);
}

void
ListItemView::slotShowItem (ListItem * item)
{
	ensureItemVisible(item);
}


void
ListItemView::loadingIsFinished (ListItem *item)
{
	emit loadingFinished(item);
}

int
ListItemView::getIconSize()
{
	return KIcon::SizeSmall;
	//return KIcon::SizeMedium;
}

ListItem*
ListItemView::itemAt ( const QPoint & viewPos ) const
{
	return (ListItem*)KListView::itemAt(viewPos);
}
ListItem*
ListItemView::currentItem () const
{
	return (ListItem*)KListView::currentItem ();
}
ListItem*
ListItemView::firstChild() const
{
	return (ListItem*)KListView::firstChild ();
}

bool
ListItemView::isDropping()
{
	return dropping;
}

void
ListItemView::setLoadFirstImage(bool load)
{
	__loadfirstimage__=load;
}

bool
ListItemView::loadFirstImage()
{
	return __loadfirstimage__;
}


QString
ListItemView::shrinkdn(const QString& str)
{
	const unsigned int len=20;
	if(str.length()<=len)
		return str;
	else
		return QString(str.left(len/2) + "..." + str.right(len/2));
}

#include "listitemview.moc"
