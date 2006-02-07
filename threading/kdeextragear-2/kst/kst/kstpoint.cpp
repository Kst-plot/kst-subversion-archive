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

#include <qpainter.h>
#include <qrect.h>
#include <qbrush.h>

#include "kstpoint.h"

KstPoint::KstPoint() {
  Type = 0;
}

KstPoint::~KstPoint() {
}

void KstPoint::setType(int new_type) {
  if ((new_type >=0) && (new_type <= KSTPOINT_MAXTYPE)) {
    Type = new_type;
  }
}

int KstPoint::getType() {
  return Type;
}

void KstPoint::draw(QPainter *p, int x, int y, int size) {
  int s;

  if (size == -1) {
    s = (p->window().width()+p->window().height())/400;
  } else {
    s = size/200;
  }

  if (s < 1)
    s = 1;

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
    p->drawLine(x-s, y-s, x+s, y-s);
    p->drawLine(x-s, y-s, x, y+s);
    p->drawLine(x, y+s, x+s, y-s);
    break;
  case 5:
    p->drawLine(x-s, y+s, x+s, y+s);
    p->drawLine(x-s, y+s, x, y-s);
    p->drawLine(x, y-s, x+s, y+s);
    break;
  }
  p->setBrush(QBrush::NoBrush);
}

int KstPoint::getDim(QPainter *p) {
  int s;

  s = (p->window().width()+p->window().height())/400;

  if (s < 1)
    s = 1;

  return s;
}

