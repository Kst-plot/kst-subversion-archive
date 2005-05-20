/***************************************************************************
                          qtfileicondrag.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
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

#include "qtfileicondrag.h"

QtFileIconDrag::QtFileIconDrag( QWidget * dragSource, const char* name )
    : QIconDrag( dragSource, name )
{}

const char*
QtFileIconDrag::format( int i ) const
{
    if ( i == 0 )
		return "application/x-qiconlist";
    else if ( i == 1 )
		return "text/uri-list";
    else
	return 0;
}

QByteArray
QtFileIconDrag::encodedData( const char* mime ) const
{
	QByteArray a;
	if ( QString( mime ) == "application/x-qiconlist" )
	{
		a = QIconDrag::encodedData( mime );
	}
	else if ( QString( mime ) == "text/uri-list" )
	{
		QString s = urls.join( "\r\n" );
		a.resize( s.length() );
		memcpy( a.data(), s.latin1(), s.length() );
	}
	return a;
}

bool
QtFileIconDrag::canDecode( QMimeSource* e )
{
	return e->provides( 
		"application/x-qiconlist" ) ||
		e->provides( "text/uri-list" );
}

void
QtFileIconDrag::append( const QIconDragItem &item, const QRect &pr,
			     const QRect &tr, const QString &url )
{
    QIconDrag::append( item, pr, tr );
    urls << url;
}

#include "qtfileicondrag.moc"
