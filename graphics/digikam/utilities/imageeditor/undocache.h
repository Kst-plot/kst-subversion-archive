/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         J�rn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-02-05
 * Copyright 2005 by Renchi Raju, J�rn Ahrens
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ============================================================ */

#ifndef UNDOCACHE_H
#define UNDOCACHE_H

#include <qglobal.h>

class UndoCachePriv;

class UndoCache
{
public:

    UndoCache();
    ~UndoCache();

    void clear();
    bool putData(int level, int w, int h, uint* data);
    bool getData(int level, int& w, int& h, uint*& data, bool del=true);

    void erase(int level);

private:

    UndoCachePriv* d;
};

#endif /* UNDOCACHE_H */
