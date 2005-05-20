/***************************************************************************
                         categoryview.h  -  description
                             -------------------
    begin                : Sat Dec 1 2004
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

#ifndef CATEGORYVIEW_H
#define CATEGORYVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

#include "listitemview.h"

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

	void createRootCategory();

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);

	CategoryDBManager* getCategoryDBManager();
	CategoryListItem* getCategoryRoot();

public slots:
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

protected slots:
	void slotANDSelection();
	void slotORSelection();

	void setNumberOfLeftItems(int nbr);

protected:
	void contentsDropEvent (QDropEvent * event);

protected:
	MainWindow *mw;

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

#endif /* __CATEGORYVIEW_H__ */
