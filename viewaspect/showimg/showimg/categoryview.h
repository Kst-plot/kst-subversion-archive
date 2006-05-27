/***************************************************************************
                         categoryview.h  -  description
                             -------------------
    begin                : Sat Dec 1 2004
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

#ifndef CATEGORYVIEW_H
#define CATEGORYVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

#include "listitemview.h"

#include <qdict.h>

class MainWindow;
class CategoryDBManager;
class CategoryListItem;
class CategoryListItemRootTag;
class CategoryListItemRootDate;

class KAction;
class KRadioAction;

class QLabel;

class CategoryView : public ListItemView
{
 Q_OBJECT

public:
	CategoryView (QWidget *parent, MainWindow *mw, const char* name);
	virtual ~CategoryView();

	bool isConnected();

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

	void createRootCategory();

	CategoryDBManager* getCategoryDBManager();
	CategoryListItem* getCategoryRoot();

	bool getAddAllImages();
	void setAddAllImages(bool addAll);
	QString getType();
	void setType(const QString& type);
	QString getSqlitePath();
	void setSqlitePath(const QString& path);
	QString getMysqlUsername();
	void setMysqlUsername(const QString& name);
	QString getMysqlPassword();
	void setMysqlPassword(const QString& pass);
	QString getMysqlHostname();
	void setMysqlHostname(const QString& host);

	int getIconSize();

public slots:

	void slotRefresh();

	void slotSuppr (ListItem *item);

	void startWatchDir(QString);
	void stopWatchDir(QString);
	void startWatchDir();
	void stopWatchDir();

	void slotNewCategory();
	void slotNewCategory(ListItem *item);

	void updateActions(ListItem *item);

	void slotCatProperty();

	void setDisabled(bool);

	//--------------------------------------------------------------------
	void fileIconRenamed(const QString&, const QString&);
	void fileIconsDeleted(const KURL::List& list);
	void filesMoved(const KURL::List&, const KURL&);

	void directoryRenamed(const KURL&, const KURL&);
	void renameImage(QDict<QString>& renamedFiles);

	/* return file paths into the databse but removed in the hard drive*/
	int removeObsololeteFilesOfTheDatabase();

protected slots:
	void slotANDSelection();
	void slotORSelection();

	void setNumberOfLeftItems(int nbr);

// 	void slotAddLinksStarted(int nbr);

protected:
	void contentsDropEvent (QDropEvent * event);

protected:
	MainWindow *mw;

	bool m_addAllImages;

	CategoryDBManager *cdbManager;
	CategoryListItemRootTag *categoryRoot;
	CategoryListItemRootDate *dateRoot;

	QLabel* m_currentActionLabel;

	KAction
			*aCatNewCategory,
			*aCatRename, *aCatDelete,
			*aCatProperties;
	KRadioAction
			*aCatSelectAND, *aCatSelectOR;

};
#endif /* WANT_LIBKEXIDB */

#endif /* CATEGORYVIEW_H */
