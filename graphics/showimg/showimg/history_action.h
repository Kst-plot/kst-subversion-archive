/***************************************************************************
                         history_action.h  -  description
                             -------------------
    begin                : 2000
    copyright            : (C) 2000 by Simon Hausmann
    email                : hausmann@kde.org
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

#ifndef HISTORY_ACTION_H
#define HISTORY_ACTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kaction.h>

// Qt
#include <qptrlist.h>

class QPopupMenu;

struct HistoryEntry
{
  QString filePath;
};

typedef QPtrList<HistoryEntry> HistoryEntryList;

class HistoryAction : public KAction
{
Q_OBJECT

public:
	HistoryAction( const QString& text, const QString& icon, int accel,
		      const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

	virtual ~HistoryAction();

	virtual int plug( QWidget *widget, int index = -1 );
	virtual void unplug( QWidget *widget );

	static void fillHistoryPopup( const HistoryEntryList& history,
				      QPopupMenu * popup,
				      bool onlyBack = false,
				      bool onlyForward = false,
				      uint startPos = 0 );

	virtual void setEnabled( bool b );
	virtual void setIconSet( const QIconSet& iconSet );

	QPopupMenu *popupMenu();

signals:
	void activated( int );

private:
	QPopupMenu *m_popup;
};

#endif
