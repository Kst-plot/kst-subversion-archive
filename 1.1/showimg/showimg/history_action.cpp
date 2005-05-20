/***************************************************************************
                         history_action.cpp  -  description
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

#include "history_action.h"

// Qt 
#include <qfileinfo.h>
#include <qpopupmenu.h>

// KDE 
#include <kstringhandler.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>

HistoryAction::HistoryAction( const QString& text, const QString& icon, int accel, 
				const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KAction( text, icon, accel, receiver, slot, parent, name )
{
	m_popup = 0;
}

HistoryAction::~HistoryAction()
{
	if ( m_popup )
		delete m_popup;
}

int HistoryAction::plug( QWidget *widget, int index )
{
	KToolBar *bar = (KToolBar *)widget;

	int id_ = KAction::getToolButtonID();
	bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
			   SLOT( slotActivated() ), isEnabled(), plainText(),
			   index );

	addContainer( bar, id_ );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	bar->setDelayedPopup( id_, popupMenu(), true );
	return containerCount() - 1;
}

void HistoryAction::unplug( QWidget *widget )
{
	KToolBar *bar = (KToolBar *)widget;
	int idx = findContainer( bar );
	if ( idx != -1 )
	{
		bar->removeItem( itemId( idx ) );
		removeContainer( idx );
	}
}

void HistoryAction::fillHistoryPopup( const HistoryEntryList& history,
                                          QPopupMenu * popup,
                                          bool onlyBack,
                                          bool onlyForward,
                                          uint startPos )
{
	QPtrListIterator<HistoryEntry> it( history );
	if (onlyBack || onlyForward) 
	{
		it += history.at(); // Jump to current item
		if ( !onlyForward )
			--it;
		else
			++it; // And move off it
	} 
	else
	if ( startPos )
		it += startPos; // Jump to specified start pos

	uint i = 0;
	while ( it.current() ) 
	{
		QString text = QString("%1").arg(it.current()->filePath);
		text = KStringHandler::csqueeze(text, 50);
		popup->insertItem( BarIcon("folder",16), text );
		if ( ++i > 10 )
			break;
		if ( !onlyForward )
			--it;
		else
			++it;
	}
}

void HistoryAction::setEnabled( bool b )
{
	int len = containerCount();
	for ( int i = 0; i < len; i++ )
	{
		QWidget *w = container( i );
		if ( w->inherits( "KToolBar" ) )
		  ((KToolBar *)w)->setItemEnabled( itemId( i ), b );
	}
	KAction::setEnabled( b );
}

void HistoryAction::setIconSet( const QIconSet& iconSet )
{
	int len = containerCount();
	for ( int i = 0; i < len; i++ )
	{
		QWidget *w = container( i );

		if ( w->inherits( "KToolBar" ) )
			((KToolBar *)w)->setButtonPixmap( itemId( i ), iconSet.pixmap() );

	}
	KAction::setIconSet( iconSet );
}

QPopupMenu *HistoryAction::popupMenu()
{
	if ( m_popup )
		return m_popup;

	m_popup = new QPopupMenu();
	return m_popup;
}

#include "history_action.moc"
