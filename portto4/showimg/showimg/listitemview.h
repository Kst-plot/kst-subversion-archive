/***************************************************************************
                         listitemview.h  -  description
                             -------------------
    begin                : 28 Dec 2004
    copyright            : (C) 2004-2006 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

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

#ifndef LISTITEMVIEW_H
#define LISTITEMVIEW_H

// KDE
#include <klistview.h>
#include <kio/job.h>

class Directory;
class ImageListView;
class ImageViewer;
class ListItem;
class ListItem;
class MainWindow;

class KAction;
class KActionCollection;
class KDirWatch;
class KIO::Job;
class KListView;
class KPopupMenu;

class QDir;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QFileInfo;
class QMouseEvent;
class QPoint;
class QPopupMenu;
class QProgressDialog;
class QString;
class QStringList;

class ListItemView : public KListView
{
	Q_OBJECT

public:
	ListItemView (QWidget *a_p_parent, const char* a_p_name);
	virtual ~ListItemView ();

	virtual void initMenu(KActionCollection *a_p_actionCollection) = 0;
	virtual void initActions(KActionCollection *a_p_actionCollection) = 0;

	/**
		@return true if the user is m_dropping files
	*/
	bool isDropping();

	/**
		return true if it have to load the first image of the selected
		directory
	*/
	void setLoadFirstImage(bool load);
	bool loadFirstImage();
	
	virtual ListItem * openURL(const KURL& a_url) = 0;
	virtual bool isManagedURL(const KURL& a_url) = 0;

	void loadingIsStarted (ListItem *a_p_item, int a_numberOfItems);
	void loadingIsFinished(ListItem *a_p_item);
	void loadingIsFinished(ListItem *a_p_item, int a_numberOfItems);

	void setHasSeenFile(int a_num=1);

	virtual int getIconSize();

	ListItem* itemAt ( const QPoint & a_viewPos ) const;
	ListItem* currentItem () const;
	ListItem* firstChild() const;

	void clearSelection ();
	bool isClearingSelection();

	bool loadThumbnails();
	void setLoadThumbnails(bool a_load);

	void setTotalNumberOfFiles(int a_total);

public:
	static const int COLUMN_NAME=0;
	static const int COLUMN_TYPE=1;
	static const int COLUMN_SIZE=2;
	static const int COLUMN_SELECT=3;

public slots:
	virtual void slotSuppr (ListItem *item) = 0;

// 	virtual void startWatchDir(const QString & a_dir) = 0;
// 	virtual void stopWatchDir(const QString & a_dir) = 0;
// 	virtual void startWatchDir() = 0;
// 	virtual void stopWatchDir() = 0;

// 	virtual void startWatchSelectedItem(const QString & /*a_fullname*/){};
// 	virtual void stopWatchSelectedItem(const QString & /*a_fullname*/){};

	virtual void updateActions(ListItem *a_p_item) = 0;

	////
	void slotShowItem (ListItem *a_p_item);

	void recursivelyOpen(ListItem *a_p_item);
	void recursivelyOpen();

	void slotSelectionChanged ();

	void goToNextDir();
	void goToPreviousDir();

	void slotSuppr ();
	void slotRename ();
	void slotRename (ListItem *a_p_item);

signals:
	void loadingStarted(int a_numberOfItems);
	void loadingFinished(int a_numberOfItems);

	void sigTotalNumberOfFiles(int);
	void sigHasSeenFile(int);

	void loadingFinished(ListItem *a_p_item);
	void currentSelectionChanged(ListItem *a_p_item);

	void sigSetMessage(const QString&);

protected slots:
	void openFolder ();

	void slotShowHideDetail_Type();
	void slotShowHideDetail_Size();
	void slotShowHideDetail_Select();


protected:
	virtual void contentsDropEvent (QDropEvent * a_p_event);

	////
	void initSelectedListItem();
	void initSelectedListItem(ListItem *a_p_item);
	void restoreSelectedListItem();

	static QString shrinkdn(const QString& _str);

	/**
		the drag'n'drop events
	*/
	QDragObject* dragObject ();

	void contentsDragEnterEvent (QDragEnterEvent * a_p_event);
	void contentsDragLeaveEvent (QDragLeaveEvent *a_p_event);
	void contentsDragMoveEvent (QDragMoveEvent * a_p_event);
	void contentsMouseMoveEvent (QMouseEvent *a_p_event);
	void contentsMousePressEvent (QMouseEvent* a_p_event);
	void contentsMouseReleaseEvent(QMouseEvent* a_p_event);
	void contentsMouseDoubleClickEvent ( QMouseEvent * a_p_event );

	void contentsDropEvent_begin();
	void contentsDropEvent_end();

protected:
	MainWindow    * getMainWindow() ;
	ImageListView * getImageListView() ;
	ImageViewer   * getImageViewer() ;

	inline void setClickedItem(ListItem*a_p_clickedItem) {m_p_clickedItem=a_p_clickedItem;};
	inline ListItem * getClickedItem() const {return m_p_clickedItem;};

	inline ListItem   * getDropedItem() const {return m_p_dropItem;};

	inline void setActionCollection(KActionCollection *a_p_actionCollection){m_p_actionCollection=a_p_actionCollection;};
	inline KActionCollection * getActionCollection()const{return m_p_actionCollection;};

	inline void setPopupMenu(KPopupMenu *a_p_popup){m_p_popup=a_p_popup;};
	inline KPopupMenu * getPopupMenu(){return m_p_popup;};


private:
	bool m_isClearingSelection;
	bool m_dropping;
	bool m_autoSkip;
	bool m_replaceAll;
	bool m_loadfirstimage;
	int m_autoopenTime;
	bool m_loadThumbnails;

	int
		m_totalNumberOfFiles,
		m_seenFiles,
		m_seenFilesSinceLastUpdate;
	QDateTime m_LastUpdateTime;

	QTimer *m_autoopen_timer;
	KPopupMenu *m_p_popup;
	ListItem
		*m_p_oldCurrent,
		*m_p_dropItem,
		*m_p_clickedItem;
	QPtrList < ListItem > m_oldCurrents;
	KActionCollection *m_p_actionCollection;

};

#endif



