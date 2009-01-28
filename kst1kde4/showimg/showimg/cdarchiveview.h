/*****************************************************************************
                         categoryview.h  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2001-2006 by Richard Groult
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
	CDArchiveView (QWidget *a_p_parent, const char* a_p_name);
	virtual ~CDArchiveView();

	void initMenu(KActionCollection *a_p_actionCollection);
	void initActions(KActionCollection *a_p_actionCollection);

	ListItem* getCDArchiveItem(const QString& cdArchivePath);

	virtual void createRoot();

	virtual bool isManagedURL(const KURL& a_url) ;
	virtual ListItem * openURL(const KURL& a_url) ;

public slots:
	void slotSuppr (ListItem *a_p_item);

	void slotTrash();
	void slotTrash(ListItem *a_p_item);

	virtual void startWatchDir(const QString & a_dir);
	virtual void stopWatchDir(const QString & a_dir);
	virtual void startWatchDir();
	virtual void stopWatchDir();

	void slotNewCDArchive();
	void slotNewCDArchive(ListItem *a_p_item);

	void updateActions(ListItem *a_p_item);

	void slotCDArchiveProperty();

protected:
	void contentsDropEvent (QDropEvent * a_p_event);

private:
	KDirWatch *m_p_dirWatch;

	KAction
			*m_p_CDArchiveNew_action,
			*m_p_CDArchiveProperties_action,
			*m_p_CDArchiveRename_action, *m_p_CDArchiveTrash_action, *m_p_CDArchiveDelete_action;

};
#endif /* SHOWIMG_NO_CDARCHIVE */

#endif /* __CDARCHIVEVIEW_H__ */
