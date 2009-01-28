/*****************************************************************************
                         directoryview.h  -  description
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

#ifndef DIRECTORYVIEW_H
#define DIRECTORYVIEW_H

// Local
#include "listitemview.h"
#include "directory.h"

#include <kfileitem.h>

class KAction;
class KActionMenu;
class KDirLister;

class DirectoryView : public ListItemView
{
 Q_OBJECT

public:
	DirectoryView (QWidget *a_p_parent, const char* a_p_name);
	virtual ~DirectoryView();

	void readConfig(KConfig *a_p_config);
	void writeConfig(KConfig *a_p_config);

	void initMenu(KActionCollection *a_p_actionCollection);
	void initActions(KActionCollection *a_p_actionCollection);

	/**
		return true if it have to show the hidden directories
	*/
	void setShowHiddenDir(bool a_show);
	bool showHiddenDir()const;

	/**
		copy a list of urls into  the dest directory
	*/
	void copy(const KURL::List& a_list, const KURL& a_dest);
	/**
		move a list of urls into  the dest directory
	*/
	void move(const KURL::List& a_list, const KURL& a_dest);

	/**
		return true if it have to show the hidden files
	*/
	void setShowHiddenFile(bool a_show);
	bool showHiddenFile()const;

	/**
		return true if it have to show the directories
	*/
	void setShowDir(bool a_show);
	bool showDir() const;

	bool getShowCompressedFiles() const;
	void setShowCompressedFiles(bool a_show = true);

	bool getShowVideo() const;
	void setShowVideo(bool a_show);

	/**
		return true if it have to show ALL files
	*/
	void setShowAllFile(bool a_show);
	inline bool showAllFile() const {return m_showAllFile;};

	int filter();

	ListItem* getDir(const KURL& a_url);
	void removeDir(const QString& a_path);

	void setUnrarPath(const QString& a_path);
	QString getUnrarPath() const;

	Directory * getRoot();
	void createRoot();

	virtual ListItem * openURL(const KURL & a_url);
	virtual bool isManagedURL(const KURL& a_url) ;

	void refresh(const KURL& a_dir);
	

public slots:
	virtual void startWatchURL(const KURL & a_url);
	virtual void stopWatchURL(const KURL & a_url);

	virtual void startWatchSelectedItem(const KURL & a_url);
	virtual void stopWatchSelectedItem(const KURL & a_url);

	void slotDirInfo();
	void slotDirProperty();

	void slotNewDir (ListItem *a_p_item);
	void slotNewDir ();
	void slotNewAlbum (ListItem *a_p_item);
	void slotNewAlbum ();

	virtual void slotSuppr (ListItem *a_p_item);

	void slotTrash ();
	void slotTrash (ListItem *a_p_item);

	void slotDirPasteFiles();

	void slotDirCopy();
	void slotDirCopyToLast();
	void slotDirMove();
	void slotDirMoveToLast();

	virtual void updateActions(ListItem *a_p_item);

	void contentsDropEvent (QDropEvent * a_p_event);

signals:
	void moveFilesDone(const KURL::List& a_srcURL, const KURL& a_destURL);
	void renameListItemDone(const KURL& a_srcURL, const KURL& a_destURL );

protected :
	friend  void Directory::setOpen( bool a_is_open );
	void openingFinished( Directory * a_p_directory = NULL);

	bool copy (const KURL& a_dep, const KURL& a_dest);
	bool move (const KURL& a_dep, const KURL& a_dest);

	void createRootForURL(const KURL& a_url);
	
protected slots:
	void copyingDone( KIO::Job *);
	void movingDone( KIO::Job *);

	void copyingDirDone( KIO::Job *);
	void movingDirDone( KIO::Job *);

	void renameDone( KIO::Job *job);

	void updateDestURLTitle(const KURL& a_dir);

	void treeview_refreshItems( const KFileItemList & a_item_list);
	void treeview_cleared(const KURL& a_url);
	void treeview_newItem(const KFileItemList & a_item_list);
	void treeview_itemDeleted( KFileItem * a_p_item );
	void treeview_jobStarted( const KURL& a_url );
	void treeview_jobCompleted( );

	void selecteditems_refreshItems( const KFileItemList & a_item_list);
	void selecteditems_cleared(const KURL& a_url);
	void selecteditems_cleared();
	void selecteditems_newItem(const KFileItemList & a_item_list);
	void selecteditems_itemDeleted( KFileItem * a_p_item );
	void selecteditems_jobStarted( const KURL& a_url );
	void selecteditems_jobCompleted( );


private:
	KAction
			*m_p_dirCopy_action,  *m_p_dirMove_action, *m_p_dirCopyToLast_action, *m_p_dirMoveToLast_action,
			*m_p_dirPasteFiles_action, *m_p_dirRecOpen_action,
			*m_p_dirRename_action, *m_p_dirTrash_action, *m_p_dirDelete_action,
			*m_p_dirInfo_action,
			*m_p_previousDir_action,*m_p_nextDir_action,
			*m_p_dirProperties_action,
			*m_p_dirNewFolder_action, *m_p_dirNewAlbum_action,
			*m_p_detailType_action, *m_p_detailSize_action, *m_p_detailSelect_action;
	KActionMenu
			*m_p_newItems_menuaction,
			*m_p_copy_menuaction, *m_p_move_menuaction;
	bool 
		m_showCompressedFiles,
		m_showVideo;

	QString 
		m_dirOrg, 
		m_dirDest,
		m_lastDestDir,
		m_unurarPath;

	bool
		m_showHiddenDir,
		m_showHiddenFile,
		m_showDir,
		m_showAllFile;

	KDirLister
		* m_p_treeview_lister,
		* m_p_selecteditems_lister;


	KURL 
		m_openingURL;
	bool 
		m_is_openingURL;

	Directory * m_root_dir;

};

#endif
