/***************************************************************************
                  kstgfxmousehandlerutils.h  -  Part of KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 The University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTGFXMOUSEHANDLERUTILS_H
#define KSTGFXMOUSEHANDLERUTILS_H

class QRect;
class QPoint;

namespace KstGfxMouseHandlerUtils {
    // returns a rectangle representing position and size 
    QRect newRect(const QPoint& pos, const QPoint& mouseOrigin, bool shift);
    // returns a rectangle with topLeft = from, and bottomRight = to
    QRect newLine(const QPoint& pos, const QPoint& mouseOrigin, bool shift, const QRect& boundingBox);
    // returns -1 if pos is in 1st or 3rd quadrant with origin mouseOrigin, 1 otherwise
    int negativeOne(const QPoint& pos, const QPoint& mouseOrigin);
}

#endif

// vim: ts=2 sw=2 et
