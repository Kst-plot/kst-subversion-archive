/***************************************************************************
    begin                : 2004-11-30
    copyright            : (C) 2004 by Jaroslaw Staniek
    email                : js@iidea.pl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   as published by the Free Software Foundation; either version 2        *
 *   of the License, or (at your option) any later version.                *
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
 ***************************************************************************/

#ifndef WIN_UTILS_H
#define WIN_UTILS_H

#include <qstring.h>

QString desktopDirPath();
QString myDocumentsDirPath();
QString myPicturesDirPath();

QString volumeName(const QString& path);

typedef enum DriveType { FixedDrive, CDROMDrive, RemovableDrive, RemoteDrive, OtherDrive };
//todo: more on http://64.233.183.104/search?q=cache:DBNiJKXrarkJ:msdn.microsoft.com/library/en-us/fileio/base/getdrivetype.asp+getdrivetype&start=1&hl=en&lr=&strip=1

DriveType driveType(const QString& path);

#endif
