/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef TYPEFOLDER_H
#define TYPEFOLDER_H
#include "Folder.h"
#include "DB/Category.h"

namespace Browser
{
class BrowserItem;

class TypeFolder :public Folder {
public:
    TypeFolder( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* parent );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    virtual QString imagesLabel() const;
    virtual QString videosLabel() const;

private:
    const DB::CategoryPtr _category;
};


}

#endif /* TYPEFOLDER_H */

