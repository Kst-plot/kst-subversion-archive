/***************************************************************************
                         listitemview.h  -  description
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

#ifndef LISTITEMVIEW_H
#define LISTITEMVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <klistview.h>
#include <kio/job.h>

class ImageViewer;
class ListItem;
class ImageListView;
class MainWindow;
class Directory;
class ListItem;

class KActionCollection;
class KAction;
class KDirWatch;
class KListView;
class KPopupMenu;
class KIO::Job;

class QDragEnterEvent;
class QDropEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QMouseEvent;
class QStringList;
class QString;
class QProgressDialog;
class QPopupMenu;
class QDir;
class QFileInfo;
class QPoint;
	////

class ListItemView : public KListView
{
	Q_OBJECT

public:
	ListItemView (QWidget *parent, MainWindow *mw, const char* name);
	virtual ~ListItemView ();

	virtual void initMenu(KActionCollection *actionCollection) = 0;
	virtual void initActions(KActionCollection *actionCollection) = 0;

	static bool isImage(QFileInfo *info);
	static bool isImage(const QString& info);

	/**
		@return true if the user is dropping files
	*/
	bool isDropping();

	/**
		return true if it have to load the first image of the selected
		directory
	*/
	void setLoadFirstImage(bool load);
	bool loadFirstImage();

	void loadingIsFinished (ListItem *item);

	int getIconSize();

	ListItem* itemAt ( const QPoint & viewPos ) const;
	ListItem* currentItem () const;
	ListItem* firstChild() const;

	void clearSelection ();
	bool isClearingSelection();

public:
	static const int COLUMN_NAME=0;
	static const int COLUMN_TYPE=1;
	static const int COLUMN_SIZE=2;
	static const int COLUMN_SELECT=3;

public slots:
	virtual void slotSuppr (ListItem *item) = 0;

	virtual void startWatchDir(QString) = 0;
	virtual void stopWatchDir(QString) = 0;
	virtual void startWatchDir() = 0;
	virtual void stopWatchDir() = 0;

	virtual void updateActions(ListItem *item) = 0;

	////
	void slotShowItem (ListItem *item);

	void recursivelyOpen(ListItem *item);
	void recursivelyOpen();

	void slotSelectionChanged ();

	void goToNextDir();
	void goToPreviousDir();

	void slotSuppr ();
	void slotRename ();
	void slotRename (ListItem *item);


signals:
	void loadingFinished(ListItem *item);
	void currentSelectionChanged(ListItem *item);

protected slots:
	void openFolder ();

protected:
	virtual void contentsDropEvent (QDropEvent * event);

	////
	void initSelectedListItem();
	void initSelectedListItem(ListItem *item);
	void restoreSelectedListItem();

	static QString shrinkdn(const QString& str);

	/**
		the drag'n'drop events
	*/
	QDragObject * dragObject ();

	void contentsDragEnterEvent (QDragEnterEvent * event);
	void contentsDragLeaveEvent (QDragLeaveEvent *);
	void contentsDragMoveEvent (QDragMoveEvent * e);
	void contentsMouseMoveEvent (QMouseEvent *e);
	void contentsMousePressEvent (QMouseEvent* e);
	void contentsMouseReleaseEvent(QMouseEvent* e);
	void contentsMouseDoubleClickEvent ( QMouseEvent * e );

	void contentsDropEvent_begin();
	void contentsDropEvent_end();

protected:
	MainWindow *mw;

	bool m_isClearingSelection;
	bool dropping;
	bool autoSkip;
	bool replaceAll;
	bool __loadfirstimage__;
	int autoopenTime;

	ListItem *clickedItem;
	QTimer *autoopen_timer;
	KPopupMenu *popup;
	ListItem *oldCurrent;
	ListItem *dropItem;
	QPtrList < ListItem > oldCurrents;
	KActionCollection *actionCollection;

};

#endif



