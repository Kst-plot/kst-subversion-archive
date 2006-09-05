/***************************************************************************
    begin                : 2004-11-30
    copyright            : (C) 2004-2005 by Jaroslaw Staniek
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 ***************************************************************************/

#include "win_utils.h"

#include <win/win32_utils.h>

#include <qdir.h>

QString shellFolderDirPath(const QCString& folderName)
{
	QString path( getRegistryValue(HKEY_CURRENT_USER,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", folderName.data()) );
	if (!path.endsWith(QChar(QDir::separator())))
		path.append(QDir::separator());
	return path;
}

QString desktopDirPath()
{
	return shellFolderDirPath("Desktop");
}

QString myDocumentsDirPath()
{
	return shellFolderDirPath("Personal");
}

QString myPicturesDirPath()
{
	return shellFolderDirPath("My Pictures");
}

QString volumeName(const QString& path)
{
	DWORD dwFlags, dwDummy;
	TCHAR name[MAX_PATH];

	QString path_ = QDir::convertSeparators(path);

	if (0==GetVolumeInformation(path_.ucs2(), name, MAX_PATH, NULL, &dwDummy, &dwFlags, 0, 0))
		return QString::null;
	return QString::fromUcs2( name );
}

DriveType driveType(const QString& path)
{
	QString path_ = QDir::convertSeparators(path);
	UINT res = GetDriveType(path_.ucs2());
	switch (res) {
	case DRIVE_FIXED: return FixedDrive;
	case DRIVE_REMOTE: return RemoteDrive;
	case DRIVE_CDROM: return CDROMDrive;
	case DRIVE_REMOVABLE: return RemovableDrive;
	default:;
	}
	return OtherDrive;
}
