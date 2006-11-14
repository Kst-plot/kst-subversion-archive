/*****************************************************************************
                          listitemview.cpp  -  description
                             -------------------
    begin                : 28 Dec 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "listitemview.h"

//-----------------------------------------------------------------------------
#define LISTITEMVIEW_DEBUG           0
#define LISTITEMVIEW_DEBUG_DRAGNDROP 0
//-----------------------------------------------------------------------------

// Local
#include "album.h"
#include "cdarchivecreatordialog.h"
#include "compressedimagefileiconitem.h"
#include "describealbum.h"
#include "directory.h"
#include "imagefileiconitem.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "listitem.h"
#include "mainwindow.h"
#include "showimg_common.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */

#ifdef WANT_LIBKEXIDB
#include "categorydbmanager.h"
#include "categorylistitem.h"
#endif /* WANT_LIBKEXIDB */

// Qt
#include <qclipboard.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qdragobject.h>
#include <qdropsite.h>
#include <qfile.h>
#include <qheader.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtooltip.h>

// KDE
#include <kaction.h>
#include <kapplication.h>
#include <kdirwatch.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <klineeditdlg.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpopupmenu.h>
#include <kpropertiesdialog.h>
#include <kstringhandler.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

#if  KDE_VERSION >= 0x30200
#include <kinputdialog.h>
#endif

ListItemView::ListItemView (
			QWidget    *a_p_parent,
			const char *a_p_name)
	: KListView (a_p_parent, a_p_name),

	m_loadThumbnails(true)
{
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

	///////////////////////////////////////////////////////////////////////////
	setColumnWidthMode (COLUMN_SELECT, Manual );
	setColumnWidth(COLUMN_SELECT, 3*ListItem::TREE_CHECKBOX_MAXSIZE/2);
	header()->setResizeEnabled (false, COLUMN_SELECT );

	setSorting (COLUMN_TYPE);
	sort();
	setColumnWidthMode (COLUMN_TYPE, Manual );
	setColumnWidth (COLUMN_TYPE, 0 );

	setColumnWidthMode (COLUMN_SIZE, Manual );

	///////////////////////////////////////////////////////////////////////////
	m_dropping = false;
	m_isClearingSelection=false;
	m_p_clickedItem=NULL;

	m_autoopenTime = 750;
	m_autoopen_timer = new QTimer (this);
	connect (m_autoopen_timer, SIGNAL (timeout ()), this, SLOT (openFolder ()));

	///
	connect(this, SIGNAL(selectionChanged ()),
			this, SLOT(slotSelectionChanged ()));

	connect(this, SIGNAL(sigSetMessage(const QString&)),
		getMainWindow(), SLOT(slotSetStatusBarText(const QString&)));

}

ListItemView::~ListItemView ()
{
}


void
ListItemView::slotRename ()
{
	if(!m_p_clickedItem) m_p_clickedItem=currentItem();
	slotRename(m_p_clickedItem);
}

void
ListItemView::slotRename (ListItem *a_p_item)
{
#if KDE_IS_VERSION(3,3,0)
	if(!a_p_item) return;

	ListItem *dir=a_p_item;
	QString fullname=dir->fullName();
	QString name=dir->name();
	bool ok;
	const QString newName(
				KInputDialog::getText(i18n("Rename %1").arg(name),
							  i18n("Enter new name:"),
							  name,
							  &ok,
							  getImageViewer()).stripWhiteSpace());
	if(ok && !newName.isEmpty() && newName!=name)
	{
		QString msg;
		if(!dir->rename(newName, msg))
		{
			KMessageBox::error(getImageViewer(), "<qt>"+msg+"</qt>");
		}
	}
#endif
}


void
ListItemView::initSelectedListItem()
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"BEGIN"<<endl;
#endif

	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		initSelectedListItem(rootItems);
		rootItems = rootItems->nextSibling();
	}
	setSelectionMode (QListView::Single);
	m_p_oldCurrent=currentItem();

#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"END"<<endl;
#endif
}

void
ListItemView::initSelectedListItem(ListItem *a_p_item)
{
	ListItem *myChild = a_p_item->firstChild();
	if(a_p_item->isSelected())
		m_oldCurrents.append(a_p_item);
	while( myChild )
	{
		initSelectedListItem( myChild );
		myChild = myChild->nextSibling();
	}
}

void
ListItemView::restoreSelectedListItem()
{
// 	setUpdatesEnabled(false);

	clearSelection ();
	setSelectionMode (QListView::Extended);
	ListItem *a_p_item;
	for ( a_p_item = m_oldCurrents.first(); a_p_item; a_p_item = m_oldCurrents.next() )
		setSelected (a_p_item, true);
	setCurrentItem (m_p_oldCurrent);
	m_oldCurrents.clear();
	m_p_oldCurrent=NULL;

// 	setUpdatesEnabled(true);
}


void
ListItemView::slotSelectionChanged ()
{
#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	if(isDropping())
	{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 1
		MYDEBUG<<"isDropping()"<<endl;
#endif
	}
	else
	{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 1
		MYDEBUG<<"! isDropping()"<<endl;
#endif
		updateActions(currentItem());
		if(!getImageListView()->hasImageSelected())
			getImageListView()->load(NULL);
#ifdef HAVE_KIPI
		if(getMainWindow()->pluginManager())
			getMainWindow()->pluginManager()->currentAlbumChanged(currentItem()->fullName());
#endif
	}
#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool
ListItemView::isClearingSelection()
{
	return m_isClearingSelection;
}

void
ListItemView::clearSelection ()
{
// 	setUpdatesEnabled(false);
	m_isClearingSelection=true;
	KListView::clearSelection();
	m_isClearingSelection=false;
// 	setUpdatesEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////
void
ListItemView::contentsDragEnterEvent (QDragEnterEvent * event)
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"BEGIN"<<endl;
#endif
	m_dropping = true;
	initSelectedListItem();
	if (QTextDrag::canDecode (event))
	{
		event->acceptAction ();
		ListItem *i = itemAt (contentsToViewport (event->pos ()));
		if (i)
		{
			m_p_dropItem = i;
			m_autoopen_timer->start (m_autoopenTime);
		}
	}
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"END"<<endl;
#endif
}

void
ListItemView::contentsDragLeaveEvent (QDragLeaveEvent * )
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<endl;
#endif
	m_autoopen_timer->stop ();
	restoreSelectedListItem();
	m_dropping = false;
}

void
ListItemView::contentsDragMoveEvent (QDragMoveEvent * a_p_event)
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	if(!isDropping())
	{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"!isDropping()"<<endl;
#endif
		return;
	}
	if (!QTextDrag::canDecode (a_p_event))
	{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
			MYDEBUG<<"!QTextDrag::canDecode (a_p_event)"<<endl;
#endif
		a_p_event->ignore ();
		return;
	}

	ListItem *l_p_item = itemAt (contentsToViewport ( a_p_event->pos ()));
	if (l_p_item)
	{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"l_p_item"<<endl;
#endif
//		setSelected (l_p_item, true);
		a_p_event->acceptAction ();
		if (l_p_item != m_p_dropItem)
		{
			setSelected (l_p_item, true);
			m_autoopen_timer->stop ();
			m_p_dropItem = l_p_item;
			m_autoopen_timer->start (m_autoopenTime);
		}
	}
	else
	{
		a_p_event->ignore ();
		m_autoopen_timer->stop ();
		m_p_dropItem = NULL;
	}
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<"END"<<endl;
#endif
}
void
ListItemView::contentsDropEvent_begin()
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<endl;
#endif
	m_autoopen_timer->stop ();
}
void
ListItemView::contentsDropEvent (QDropEvent * )
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<endl;
#endif
	contentsDropEvent_begin();
	contentsDropEvent_end();
}

void
ListItemView::contentsDropEvent_end()
{
#if LISTITEMVIEW_DEBUG_DRAGNDROP > 0
		MYDEBUG<<endl;
#endif
	contentsDragLeaveEvent(NULL);
 	emit sigSetMessage(i18n("Ready"));
}

QDragObject*
ListItemView::dragObject ()
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void
ListItemView::contentsMouseMoveEvent ( QMouseEvent * a_p_event )
{
	if(a_p_event->state () != LeftButton)
		KListView::contentsMouseMoveEvent(a_p_event);
}


void
ListItemView::contentsMousePressEvent(QMouseEvent * a_p_event)
{
#ifndef Q_WS_WIN
	if (a_p_event->button () == MidButton) {
		contentsMouseDoubleClickEvent(a_p_event);
	}
	else
#endif
	if (a_p_event->button () == LeftButton)
	{
		if(header()->sectionAt(a_p_event->x())!=3)
		{
			KListView::contentsMousePressEvent (a_p_event);
			m_p_clickedItem = itemAt(contentsToViewport(a_p_event->pos()));
		}
		else
		{
			m_p_clickedItem = itemAt(contentsToViewport(a_p_event->pos()));
			if(m_p_clickedItem)
			{
				setSelected(m_p_clickedItem, !m_p_clickedItem->isSelected());
			}
		}
	}
	else
	if (a_p_event->button () == RightButton)
	{
		m_p_clickedItem = itemAt(contentsToViewport(a_p_event->pos()));
		updateActions(m_p_clickedItem);
		if (m_p_clickedItem)
		{
			//KListView::contentsMousePressEvent (a_p_event);
			m_p_popup->changeTitle(1, *m_p_clickedItem->pixmap(0), m_p_clickedItem->text(0));
			m_p_popup->exec(a_p_event->globalPos());
		}
	}
}

void
ListItemView::contentsMouseDoubleClickEvent ( QMouseEvent * a_p_event )
{
	ListItem *a_p_item=itemAt(contentsToViewport(a_p_event->pos()));
	if(!a_p_item)
		return;
	if(a_p_item->isOpen())
		a_p_item->setOpen(false);
	else
		a_p_item->setOpen(true);
}


void
ListItemView::contentsMouseReleaseEvent(QMouseEvent* a_p_event)
{
	if (a_p_event->button () == LeftButton)
	{
		ListItem *a_p_item=itemAt(contentsToViewport(a_p_event->pos()));
		if(a_p_item)
			if(a_p_item->isSelected())
				return;
	}
	KListView::contentsMouseReleaseEvent(a_p_event);
}

////////////////////////////////////////////////////////////////////////////////

void
ListItemView::recursivelyOpen()
{
	if(!m_p_clickedItem) m_p_clickedItem=currentItem();
	recursivelyOpen(m_p_clickedItem);
}

void
ListItemView::recursivelyOpen(ListItem *a_p_item)
{
	if(!m_p_clickedItem) m_p_clickedItem=currentItem();
	if(!m_p_clickedItem) return;
	(dynamic_cast<Directory*>(a_p_item))->recursivelyOpen();
}


void
ListItemView::openFolder ()
{
	m_autoopen_timer->stop ();
	if (m_p_dropItem && !m_p_dropItem->isOpen ())
	{
		setOpen(m_p_dropItem, true);
	}
}

void
ListItemView:: slotSuppr()
{
	if(!m_p_clickedItem) m_p_clickedItem=currentItem();
	slotSuppr(m_p_clickedItem);
}


void
ListItemView::goToNextDir()
{
	if(!currentItem()) return;
	ListItem * a_p_item = currentItem()->itemBelow();
	if(!a_p_item) return;
	m_p_clickedItem = a_p_item;
	ensureItemVisible(m_p_clickedItem);
	clearSelection ();
	setCurrentItem(m_p_clickedItem);
	m_p_clickedItem->setSelected(true);
}

void
ListItemView::goToPreviousDir()
{
	if(!currentItem()) return;
	ListItem * a_p_item = currentItem()->itemAbove();
	if(!a_p_item) return;
	m_p_clickedItem = a_p_item;
	ensureItemVisible(m_p_clickedItem);
	clearSelection ();
	setCurrentItem(m_p_clickedItem);
	m_p_clickedItem->setSelected(true);
}

void
ListItemView::slotShowItem (ListItem * a_p_item)
{
	ensureItemVisible(a_p_item);
}


void
ListItemView::loadingIsStarted (ListItem *a_p_item, int a_numberOfItems)
{
#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	KApplication::setOverrideCursor( waitCursor ); // this might take time
	if(a_p_item) emit sigSetMessage(i18n("Loading %1...").arg(a_p_item->text(0)));
	getImageListView()->setUpdatesEnabled( false );
	getImageListView()->stopLoading();

	setTotalNumberOfFiles(a_numberOfItems);
	if(a_p_item) a_p_item->repaint();

	emit loadingStarted(a_numberOfItems);
	
#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void
ListItemView::loadingIsFinished (ListItem *a_p_item, int a_numberOfItems)
{
#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif

	if(a_p_item) 
		getMainWindow()->slotAddImage(a_p_item->getNumberOfItems());
	//
	getImageListView()->setUpdatesEnabled( true );
	getImageListView()->sort();
	//
	getImageViewer()->updateStatus();
	//
	getMainWindow()->setEnabled(true);
	//
	KApplication::restoreOverrideCursor();
	if(a_p_item) a_p_item->repaint();
	//
	emit loadingFinished(a_numberOfItems);
 	//kapp->processEvents();
	//
	if(loadThumbnails())
		getImageListView()->slotLoadFirst();
	if(getImageListView()->hasImages() && loadFirstImage())
		getImageListView()->first();

#if LISTITEMVIEW_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void
ListItemView::setTotalNumberOfFiles(int total)
{
	m_totalNumberOfFiles = total;
	m_seenFiles = 0;
	m_seenFilesSinceLastUpdate=0;
	m_LastUpdateTime = QDateTime::currentDateTime();

	getMainWindow()->slotReset(true);
	emit sigTotalNumberOfFiles(m_totalNumberOfFiles);
	//FIXME kapp->processEvents();
}
void
ListItemView::setHasSeenFile(int num)
{
	m_seenFiles+=num;
	m_seenFilesSinceLastUpdate+=num;

	if(m_LastUpdateTime.time().msecsTo(QDateTime::currentDateTime().time()) >= 500)
	{

		//FIXME if(getMainWindow()->isEnabled()) getMainWindow()->setEnabled(false);
		m_LastUpdateTime=QDateTime::currentDateTime ();
		emit sigHasSeenFile(m_seenFilesSinceLastUpdate);
		m_seenFilesSinceLastUpdate=0;
	}
}

void
ListItemView::slotShowHideDetail_Type()
{
	if(columnWidth(COLUMN_TYPE) > 0)
		setColumnWidth (COLUMN_TYPE, 0 );
	else
		setColumnWidth (COLUMN_TYPE, 80 );
}
void
ListItemView::slotShowHideDetail_Size()
{
	if(columnWidth(COLUMN_SIZE) > 0)
		setColumnWidth (COLUMN_SIZE, 0 );
	else
		setColumnWidth (COLUMN_SIZE, 60 );
}
void
ListItemView::slotShowHideDetail_Select()
{
	if(columnWidth(COLUMN_SELECT) > 0)
		setColumnWidth (COLUMN_SELECT, 0 );
	else
		setColumnWidth (COLUMN_SELECT, 3*ListItem::TREE_CHECKBOX_MAXSIZE/2 );
}

bool
ListItemView::loadThumbnails()
{
	return m_loadThumbnails;
}

void
ListItemView::setLoadThumbnails(bool load)
{
	m_loadThumbnails = load;
	if(load) 
		getImageListView()->slotLoadFirst();
}


int
ListItemView::getIconSize()
{
	return KIcon::SizeSmall;
}

ListItem*
ListItemView::itemAt ( const QPoint & viewPos ) const
{
	return dynamic_cast<ListItem*>(KListView::itemAt(viewPos));
}
ListItem*
ListItemView::currentItem () const
{
	return dynamic_cast<ListItem*>(KListView::currentItem ());
}
ListItem*
ListItemView::firstChild() const
{
	return dynamic_cast<ListItem*>(KListView::firstChild ());
}

bool
ListItemView::isDropping()
{
	return m_dropping;
}

void
ListItemView::setLoadFirstImage(bool load)
{
	m_loadfirstimage=load;
}

bool
ListItemView::loadFirstImage()
{
	return m_loadfirstimage;
}


QString
ListItemView::shrinkdn(const QString& a_str)
{
	const unsigned int l_len=20;
	return KStringHandler::csqueeze(a_str, l_len);
/*
	if(str.length()<=len)
		return str;
	else
		return QString(str.left(len/2) + "..." + str.right(len/2));
*/
}

MainWindow * ListItemView::getMainWindow()
{
	return &MAINWINDOW;
};

ImageListView * ListItemView::getImageListView()
{
	return getMainWindow()->getImageListView();
};

ImageViewer * ListItemView::getImageViewer()
{
	return getMainWindow()->getImageViewer();
};

#include "listitemview.moc"
