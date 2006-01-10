/***************************************************************************
                          kstpoint: a plot point object for kst
                             -------------------
    begin                : Sunday June 17, 2001
    copyright            : (C) 2001 by cbn
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   Permission is granted to link with any opensource library             *
 *                                                                         *
 ***************************************************************************/


#include "kstpoint.h"
#include <kglobal.h>

KstPoint::KstPoint() {
  Type = 0;
}

KstPoint::~KstPoint() {
}

void KstPoint::setType(int new_type) {
  if (new_type >= 0 && new_type < KSTPOINT_MAXTYPE) {
    Type = new_type;
  }
}

int KstPoint::type() const {
  return Type;
}

void KstPoint::draw(QPainter *p, int x, int y, int lineSize, int size) {
  int s;

  if (size == -1) {
    QRect r = p->window();
    s = (r.width() + r.height())/400;
  } else {
    s = size/200;
  }

  if (s < 1) {
    s = 1;
  }

  s += lineSize;

  switch (Type) {
    case 0:
      p->drawLine(x-s, y-s, x+s, y+s);
      p->drawLine(x-s, y+s, x+s, y-s);
      break;
    case 1:
      p->setBrush(QBrush::NoBrush);
      p->drawRect(x-s, y-s, 2*s+1, 2*s+1);
      break;
    case 2:
      p->setBrush(QBrush::NoBrush);
      p->drawEllipse(x-s, y-s, 2*s+1, 2*s+1);
      break;
    case 3:
      p->setBrush(QBrush::SolidPattern);
      p->drawEllipse(x-s, y-s, 2*s+1, 2*s+1);
      break;
    case 4:
      {
        QPointArray pts(3);

        pts.putPoints( 0, 3, x-s, y-s, x, y+s, x+s, y-s );
        p->setBrush(QBrush::NoBrush);
        p->drawPolygon(pts);
      }
      break;
    case 5:
      {
        QPointArray pts(3);

        pts.putPoints( 0, 3, x-s, y+s, x, y-s, x+s, y+s );
        p->setBrush(QBrush::NoBrush);
        p->drawPolygon(pts);
      }
      break;
    case 6:
      p->setBrush(QBrush::SolidPattern);
      p->drawRect(x-s, y-s, 2*s+1, 2*s+1);
      break;
    case 7:
      p->drawLine(x-s, y, x+s, y);
      p->drawLine(x, y-s, x, y+s);
      break;
    case 8:
      p->drawLine(x-s, y-s, x+s, y+s);
      p->drawLine(x-s, y+s, x+s, y-s);
      p->drawLine(x-s, y, x+s, y);
      p->drawLine(x, y-s, x, y+s);
      break;    
    case 9:
      {
        QPointArray pts(3);

        pts.putPoints( 0, 3, x-s, y-s, x, y+s, x+s, y-s );
        p->setBrush(QBrush::SolidPattern);
        p->drawPolygon(pts);
      }
      break;
    case 10:
      {
        QPointArray pts(3);

        pts.putPoints( 0, 3, x-s, y+s, x, y-s, x+s, y+s );
        p->setBrush(QBrush::SolidPattern);
        p->drawPolygon(pts);
      }
      break;
    case 11:
      {
        QPointArray pts(4);

        pts.putPoints( 0, 4,   x+s, y,
                               x, y+s,
                               x-s, y,
                               x, y-s );
        p->setBrush(QBrush::NoBrush);
        p->drawPolygon(pts);
      }
      break;
    case 12:
      {    
        QPointArray pts(4);

        pts.putPoints( 0, 4,   x+s, y,
                               x, y+s,
                               x-s, y,
                               x, y-s );
        p->setBrush(QBrush::SolidPattern);
        p->drawPolygon(pts);
      }
      break;
  }

  p->setBrush(QBrush::NoBrush);
}


int KstPoint::dim(QPainter *p) const {
  QRect r = p->window();
  return kMax(1, ((r.width() + r.height()) / 400));
}

// vim: ts=2 sw=2 et
