/***************************************************************************
                         directoryview.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef DIRECTORYVIEW_H
#define DIRECTORYVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "listitemview.h"

// KDE
#include <kfileitem.h>

class KAction;
class KActionMenu;
class KDirLister;

class DirectoryView : public ListItemView
{
 Q_OBJECT

public:
	DirectoryView (QWidget *parent, MainWindow *mw, const char* name);
	virtual ~DirectoryView();

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);

	/**
		return true if it have to show the hidden directories
	*/
	void setShowHiddenDir(bool show);
	bool showHiddenDir();

	/**
		copy a list of local (ie file:/) urls into  the dest directory
	*/
	void copy(const QStringList& uris, const QString& dest);
	/**
		move a list of local (ie file:/) urls into  the dest directory
	*/
	void move(const QStringList& uris, const QString& dest);

	/**
		return true if it have to show the hidden files
	*/
	void setShowHiddenFile(bool show);
	bool showHiddenFile();

	/**
		return true if it have to show the directories
	*/
	void setShowDir(bool show);
	bool showDir();

	bool getShowCompressedFiles();
	void setShowCompressedFiles(bool show = true);

	bool getShowVideo();
	void setShowVideo(bool show);

	/**
		return true if it have to show ALL files
	*/
	void setShowAllFile(bool show);
	inline bool showAllFile() {return m_showAllFile;};

	int filter();

	ListItem* getDir(const QString& dirfullname);
	void removeDir(const QString& path);

	void setUnrarPath(const QString& path);
	QString getUnrarPath();

public slots:
	void startWatchDir(QString);
	void stopWatchDir(QString);
	void startWatchDir();
	void stopWatchDir();

	void slotDirInfo();
	void slotDirProperty();

	void slotNewDir (ListItem *item);
	void slotNewDir ();
	void slotNewAlbum (ListItem *item);
	void slotNewAlbum ();

	void slotSuppr (ListItem *item);

	void slotTrash ();
	void slotTrash (ListItem *item);

	void slotDirPasteFiles();

	void slotDirCopy();
	void slotDirCopyToLast();
	void slotDirMove();
	void slotDirMoveToLast();

	void updateActions(ListItem *item);

signals:
	void moveFilesDone(const KURL::List& srcURL, const KURL& destURL);
	void renameListItemDone(const KURL& srcURL, const KURL& destURL );

protected slots:
	void copyingDone( KIO::Job *);
	void movingDone( KIO::Job *);

	void copyingDirDone( KIO::Job *);
	void movingDirDone( KIO::Job *);

	void renameDone( KIO::Job *job);

	void updateDestDirTitle(const QString& dir);

protected:
	void contentsDropEvent (QDropEvent * event);

protected:
	KAction
			*aDirCopy,  *aDirMove, *aDirCopyToLast, *aDirMoveToLast,
			*aDirCut, *aDirPaste, *aDirPasteFiles, *aDirRecOpen,
			*aDirRename, *aDirTrash, *aDirDelete,
			*aDirInfo,
			*aPreviousDir,*aNextDir,
			*aDirProperties,
			*aDirNewFolder, *aDirNewAlbum,
			*aDetailType, *aDetailSize, *aDetailSelect;
	KActionMenu
			*actionNewItems,
			*aCopyActions, *aMoveActions;

private:
		bool copy (QString *dep, QString *dest);
		bool move (QString *dep, QString *dest);

private:
		bool m_showCompressedFiles;
		bool m_showVideo;

		QString dirOrg, dirDest;
		QString lastDestDir;

		KDirWatch *dirWatch;
		bool m_showHiddenDir, m_showHiddenFile,  m_showDir, m_showAllFile;

		KDirLister *m_dirLister;

		QString m_unurarPath;
};

#endif
