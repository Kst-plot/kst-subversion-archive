/***************************************************************************
                          kstpoint.h: defines a point for kst
                             -------------------
    begin                : June 3 2001
    copyright            : (C) 2001 by C. Barth Netterfield
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
#include <qpainter.h>

#ifndef KSTPOINT_H
#define KSTPOINT_H

/**A class for handling points for kst
 *@author C. Barth Netterfield
 */

#include "kst_export.h"

#define KSTPOINT_MAXTYPE 13

class KST_EXPORT KstPoint {
  public:
    KstPoint();
    ~KstPoint();

    /** draw the point on a painter the scale of the point is based on size */
    void draw(QPainter *p, int x, int y, int lineSize=0, int size=-1);
    /** set the type of the point: arbitrary meaning only */
    void setType(int type);
    /** get the type of the point for storage */
    int type() const;
    /** Get the dimension of the current point type */
    int dim(QPainter *p) const;
  private:
    int Type;
};

#endif
// vim: ts=2 sw=2 et
