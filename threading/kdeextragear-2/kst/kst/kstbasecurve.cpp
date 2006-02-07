/***************************************************************************
                   kstbasecurve.cpp: base class for a curve
                             -------------------
    begin                : June 2003
    copyright            : (C) 2003 University of Toronto
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
#include "kstbasecurve.h"


KstBaseCurve::KstBaseCurve(QDomElement& e) : KstDataObject(e) {
  commonConstructor();
}


KstBaseCurve::KstBaseCurve() : KstDataObject() {
  commonConstructor();
}


void KstBaseCurve::commonConstructor() {
  QColor in_color("red");
  setHasPoints(false);
  setHasLines(true);
  MaxX = MinX = MeanX = MaxY = MinY = MeanY = NS = 0;
  MinPosX = MinPosY = 0;
  NumUsed = 0;
  HasPoints = false;
  HasLines = true;
  Color = in_color;
  NS = 0;
}


KstBaseCurve::~KstBaseCurve() {
}

