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

#include <qrect.h>
#include <qpoint.h>

#include <kglobal.h>

#include "kstgfxmousehandlerutils.h"

QRect KstGfxMouseHandlerUtils::newRect(const QPoint& pos, const QPoint& mouseOrigin, bool shift) {
  // pretend mouse is in the perfect position to preserve 1:1 ratio if shift
  QPoint fakePos = pos;
  if (shift) {
    int negOne = negativeOne(pos, mouseOrigin);

    if (abs(pos.x() - mouseOrigin.x()) < abs(pos.y() - mouseOrigin.y())) {
      fakePos.setX(mouseOrigin.x() + negOne*(pos.y() - mouseOrigin.y()));
    } else {
      fakePos.setY(mouseOrigin.y() + negOne*(pos.x() - mouseOrigin.x()));
    }
  }

  QPoint newTopLeft;
  if (mouseOrigin.x() < fakePos.x()) {
    newTopLeft.setX(mouseOrigin.x());
  } else {
    newTopLeft.setX(fakePos.x());
  }
  if (mouseOrigin.y() < fakePos.y()) {
    newTopLeft.setY(mouseOrigin.y());
  } else {
    newTopLeft.setY(fakePos.y());
  }

  QSize newSize(abs(fakePos.x() - mouseOrigin.x()),
                abs(fakePos.y() - mouseOrigin.y()));
  return QRect(newTopLeft, newSize);
}


QRect KstGfxMouseHandlerUtils::newLine(const QPoint& pos, const QPoint& mouseOrigin, bool shift, const QRect& boundingBox) {
  QPoint fakePos = pos;
  if (shift) {
    int negOne = negativeOne(pos, mouseOrigin);
    if (abs(pos.x() - mouseOrigin.x()) < abs(pos.y() - mouseOrigin.y())) {
      if (abs(pos.x() - mouseOrigin.x()) < abs(pos.y() - mouseOrigin.y())/2) {
        fakePos.setX(mouseOrigin.x());
      } else {
        fakePos.setX(mouseOrigin.x() + negOne*(pos.y() - mouseOrigin.y()));
      }
    } else {
      if (abs(pos.y() - mouseOrigin.y()) < abs(pos.x() - mouseOrigin.x())/2) {
        fakePos.setY(mouseOrigin.y());
      } else {
        fakePos.setY(mouseOrigin.y() + negOne*(pos.x() - mouseOrigin.x()));
      }
    }
  }
    
  QRect lineRect(mouseOrigin, fakePos);
  QRect reduced = lineRect.normalize().intersect(boundingBox);  
  
  lineRect = reduced;
  if (mouseOrigin.x() > fakePos.x()) {
    lineRect.setLeft(reduced.right());
    lineRect.setRight(reduced.left());
  }
  if (mouseOrigin.y() > fakePos.y()) {
    lineRect.setTop(reduced.bottom());
    lineRect.setBottom(reduced.top());    
  }
  
  return lineRect;
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
