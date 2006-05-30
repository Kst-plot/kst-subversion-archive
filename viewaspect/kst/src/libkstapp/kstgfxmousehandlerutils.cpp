/***************************************************************************
                 kstgfxmousehandlerutils.cpp  -  description
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by University of British Columbia
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <qrect.h>
#include <qpoint.h>

#include <kglobal.h>

#include "kstgfxmousehandlerutils.h"

QPoint KstGfxMouseHandlerUtils::findNearestPtOnLine(const QPoint& fromPoint, const QPoint& toPoint, const QPoint& pos, const QRect &bounds) {
  // from the line described by fromPoint and toPoint, returns the point on the line (and inside bounds) with the shortest distance to pos. pos and fromPoint should be inside bounds.
  QPoint npos = pos;
  
  if (fromPoint.y() == toPoint.y() ) {
    npos.setY(fromPoint.y());
  } else if (fromPoint.x() == toPoint.x()) {
    npos.setX(fromPoint.x());
  } else {
    double newxpos, newypos;
    double slope = double(toPoint.y() - fromPoint.y()) / double(toPoint.x() - fromPoint.x());

    newxpos = (((double)pos.y()) + slope*((double)fromPoint.x()) + ((double)pos.x())/slope -((double)fromPoint.y())) / (slope + 1.0/slope); //we want the tip of our new line to be as close as possible to the original line (while still maintaining aspect). 

    newxpos = QMIN(newxpos, bounds.right()); //ensure that our x is inside the bounds.
    newxpos = QMAX(newxpos, bounds.left()); // ""
    newypos = slope*(newxpos - ((double)fromPoint.x())) + ((double)fromPoint.y()); //consistency w/ x.

    newypos = QMIN(newypos, bounds.bottom()); //ensure that our y is inside the bounds.
    newypos = QMAX(newypos, bounds.top()); // ""*/
    newxpos = ((double)fromPoint.x()) + (newypos - ((double)fromPoint.y()))/slope; // x will still be inside the bounds because we have just moved newypos closer to fromPoint.y(), which will send newxpos closer to fromPoint.x(), ie. in the direction further 'into' the bounds.

    npos.setX((int)newxpos);
    npos.setY((int)newypos);
  }

  return npos;
}


QRect KstGfxMouseHandlerUtils::resizeRectFromCorner(const QPoint& anchorPoint, const QPoint& movePoint, const QPoint& pos, const QRect &bounds, bool maintainAspect) {
  QRect newSize;
  QPoint npos = pos;

  if (maintainAspect) {
    double bestdistance;
    QPoint bestnpos, tempnpos;

    bestnpos = findNearestPtOnLine(anchorPoint,movePoint, pos, bounds);
    bestdistance = pow((bestnpos - pos).x(),2)+pow((bestnpos - pos).y(),2);

    // so the user can flip the rect around the anchorPoint.
    QPoint fakeMovePoint = (anchorPoint + QPoint(-(movePoint - anchorPoint).x(),(movePoint - anchorPoint).y()));

    tempnpos  = findNearestPtOnLine(anchorPoint, fakeMovePoint, pos, bounds);

    if ( (pow((tempnpos - pos).x(),2)+pow((tempnpos - pos).y(),2)) < bestdistance ) { bestnpos = tempnpos; }

    npos = bestnpos;
  }

  newSize.setTopLeft(anchorPoint);
  newSize.setBottomRight(npos);
  newSize = newSize.normalize();

  return newSize;
}


QRect KstGfxMouseHandlerUtils::resizeRectFromEdge(const QRect& originalSize, const QPoint& anchorPoint, const QPoint& movePoint, const QPoint& pos, const QRect &bounds, bool maintainAspect) {
  QRect newSize;

  if (movePoint.y() == anchorPoint.y() ) {
      int newWidth = pos.x() - anchorPoint.x() + 1; // +1 for the way widths are defined in QRect.
      double newHalfHeight = originalSize.height()/2.0;

      if (maintainAspect) {
        newHalfHeight = originalSize.height()*abs(newWidth)/originalSize.width()/2.0;

        newHalfHeight = QMIN(movePoint.y() - bounds.top(), newHalfHeight); // ensure we are still within the bounds.
        newHalfHeight = QMIN(bounds.bottom() - movePoint.y(), newHalfHeight);

        if (newWidth == 0) { newWidth = 1; } // anything better to be done?

        newWidth = (int)(originalSize.width()*newHalfHeight*2.0/originalSize.height()*newWidth/abs(newWidth)); // consistency of width w/ the newly calculated height.
      }

      newSize.setLeft(anchorPoint.x());
      newSize.setWidth(newWidth);
      newSize.setTop(anchorPoint.y() + (int)newHalfHeight);
      newSize.setBottom(anchorPoint.y() - (int)newHalfHeight);

    } else if (movePoint.x() == anchorPoint.x() ) {
      // mimic the case for (movePoint.y() == anchorPoint.y()). comments are there.
      int newHeight = pos.y() - anchorPoint.y() + 1;
      double newHalfWidth = originalSize.width()/2.0;

      if (maintainAspect) {
        newHalfWidth = originalSize.width()*abs(newHeight)/originalSize.height()/2.0;

        newHalfWidth = QMIN(movePoint.x() - bounds.left(), newHalfWidth);
        newHalfWidth = QMIN(bounds.right() - movePoint.x(), newHalfWidth);

        if (newHeight == 0) { newHeight = 1; }

        newHeight = (int)(originalSize.height()*newHalfWidth*2.0/originalSize.width()*newHeight/abs(newHeight));
      }

      newSize.setTop(anchorPoint.y());
      newSize.setHeight(newHeight);
      newSize.setLeft(anchorPoint.x() + (int)newHalfWidth);
      newSize.setRight(anchorPoint.x() - (int)newHalfWidth);
    }

    newSize = newSize.normalize();

    return newSize;
}


QRect KstGfxMouseHandlerUtils::newRect(const QPoint& pos, const QPoint& mouseOrigin, const QRect& bounds, bool squareAspect) {
  return resizeRectFromCorner(mouseOrigin, mouseOrigin + QPoint(1,1), pos, bounds, squareAspect);
}


QRect KstGfxMouseHandlerUtils::newLine(const QPoint& pos, const QPoint& mouseOrigin, bool specialAspect, const QRect& bounds) {

  if (KDE_ISLIKELY(!specialAspect)) {
    return QRect(mouseOrigin,pos);
  } else { //want special 45deg, or vertical, or horizontal line. 
    QPoint npos;
    QPoint mouseDisplacement(pos - mouseOrigin); // for picking type of line..

    if (mouseDisplacement.x() == 0) { // vertical line.
      npos = findNearestPtOnLine(mouseOrigin,mouseOrigin + QPoint(0,1), pos, bounds);
    } else if (mouseDisplacement.y() == 0) { // horizontal line.
      npos = findNearestPtOnLine(mouseOrigin,mouseOrigin + QPoint(1,0), pos, bounds);
    } else { // 45deg or vertical or horizontal.
      int dx = (int)round(2.0*mouseDisplacement.x()/abs(mouseDisplacement.y()));
      int dy = (int)round(2.0*mouseDisplacement.y()/abs(mouseDisplacement.x()));

      if (dx != 0) {dx /= fabs(dx);}
      if (dy != 0) {dy /= fabs(dy);} // type of line picked.
  
      npos = findNearestPtOnLine(mouseOrigin,mouseOrigin + QPoint(dx,dy), pos, bounds);
    }
    return QRect(mouseOrigin,npos);
  }
}

int KstGfxMouseHandlerUtils::negativeOne(const QPoint& pos, const QPoint& mouseOrigin) {
  if ((pos.y() < mouseOrigin.y() && pos.x() > mouseOrigin.x()) ||
       (pos.y() > mouseOrigin.y() && pos.x() < mouseOrigin.x())) {
    return -1;
  } else {
    return 1;
  }
}


// vim: ts=2 sw=2 et
