/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "XMLImageDateCollection.h"
#include "DB/ImageDB.h"

XMLImageDateCollection::XMLImageDateCollection()
    : _dirtyLower( false ), _dirtyUpper( false )
{
}

void XMLImageDateCollection::append( const DB::ImageDate& date )
{
    _dates.append( date );
    _dirtyLower = true;
    _dirtyUpper = true;
}

DB::ImageCount XMLImageDateCollection::count( const DB::ImageDate& range )
{
    if ( _cache.contains( range ) )
        return _cache[range];

    int exact = 0, rangeMatch = 0;
    for( QValueList<DB::ImageDate>::Iterator it = _dates.begin(); it != _dates.end(); ++it ) {
        DB::ImageDate::MatchType tp = (*it).isIncludedIn( range );
        switch (tp) {
        case DB::ImageDate::ExactMatch: exact++;break;
        case DB::ImageDate::RangeMatch: rangeMatch++; break;
        case DB::ImageDate::DontMatch: break;
        }
    }

    DB::ImageCount res( exact, rangeMatch );
    _cache.insert( range, res );
    return res;
}

QDateTime XMLImageDateCollection::lowerLimit() const
{
    static QDateTime _lower = QDateTime( QDate( 1900, 1, 1 ) );
    if ( _dirtyLower && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<DB::ImageDate>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            if ( first ) {
                _lower = (*it).start();
                first = false;
            }
            else if ( (*it).start() < _lower )
                _lower = (*it).start();
        }
    }
    _dirtyLower = false;
    return _lower;
}

QDateTime XMLImageDateCollection::upperLimit() const
{
    static QDateTime _upper = QDateTime( QDate( 2100, 1, 1 ) );
    if ( _dirtyUpper && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<DB::ImageDate>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            if ( first ) {
                _upper = (*it).end();
                first = false;
            }
            else if ( (*it).end() > _upper ) {
                _upper = (*it).end();
            }
        }
    }
    _dirtyUpper = false;
    return _upper;
}

XMLImageDateCollection::XMLImageDateCollection( const QStringList& list )
{
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( *it );
        append( info->date() );
    }
}

