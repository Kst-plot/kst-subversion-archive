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

#ifndef CDARCHIVEVIEW_H
#define CDARCHIVEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef SHOWIMG_NO_CDARCHIVE

#include "listitemview.h"

class KAction;
class KDirWatch;

class CDArchiveView : public ListItemView
{
 Q_OBJECT

public:
	CDArchiveView (QWidget *parent, MainWindow *mw, const char* name);
	virtual ~CDArchiveView();

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);

	ListItem* getCDArchiveItem(const QString& cdArchivePath);

public slots:
	void slotSuppr (ListItem *item);

	void slotTrash();
	void slotTrash(ListItem *item);

	void startWatchDir(QString);
	void stopWatchDir(QString);
	void startWatchDir();
	void stopWatchDir();

	void slotNewCDArchive();
	void slotNewCDArchive(ListItem *item);

	void updateActions(ListItem *item);

	void slotCDArchiveProperty();

protected:
	void contentsDropEvent (QDropEvent * event);

protected:
	KDirWatch *dirWatch;

	KAction
			*aCDArchiveNew,
			*aCDArchiveProperties,
			*aCDArchiveRename, *aCDArchiveTrash, *aCDArchiveDelete;

};
#endif /* SHOWIMG_NO_CDARCHIVE */

#endif /* __CDARCHIVEVIEW_H__ */
