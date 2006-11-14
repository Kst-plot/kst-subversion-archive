/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLMD5Map.h"
#include "QueryHelper.h"

using namespace SQLDB;

SQLMD5Map::SQLMD5Map(Connection& connection):
    _qh(connection)
{
}

void SQLMD5Map::insert(const QString&, const QString&)
{
}

QString SQLMD5Map::lookup(const QString& md5sum)
{
    return _qh.filenameForMD5Sum(md5sum);
}

bool SQLMD5Map::contains(const QString& md5sum)
{
    return _qh.containsMD5Sum(md5sum);
}

void SQLMD5Map::clear()
{
}
