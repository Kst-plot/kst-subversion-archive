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
  setLineWidth(0);
  setLineStyle(0);
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

int KstBaseCurve::getIndexNearX(double x) {
  // monotonically rising: we can do a binary search
  // should be reasonably fast
  if (xIsRising()) {
    double rX, rY;
    int i0;
    int i_top = sampleCount()-1;
    int i_bot = 0;

    // don't pre-check for x outside of the curve since this is not
    // the common case.  It will be correct - just slightly slower...
    while (i_bot + 1 < i_top) {
      i0 = (i_top + i_bot)/2;
      getPoint(i0, rX, rY);
      if (x < rX) {
        i_top = i0;
      } else {
        i_bot = i0;
      }
    }
    double xt,xb;
    getPoint(i_top, xt, rY);
    getPoint(i_bot, xb, rY);
    if (xt - x < x - xb) {
      return(i_top);
    } else {
      return(i_bot);
    }
  } else {
    // Oh Oh... not monotonically rising - we have to search the entire curve!
    // May be unbearably slow for large vectors
    int i, i0, n;
    double dx, dx0, rX, rY;
    n = sampleCount();

    getPoint(0, rX, rY);
    dx0 = fabs(x-rX);
    i0 = 0;

    for (i=1; i<n; i++) {
      getPoint(i, rX, rY);
      dx = fabs(x-rX);
      if (dx < dx0) {
        dx0 = dx;
        i0 = i;
      }
    }
    return(i0);
  }
}

/** getIndexNearXY: return index of point within (or closest too)
    x +- dx which is closest to y **/
int KstBaseCurve::getIndexNearXY(double x, double dx, double y) {
  int i,i0, iN, sc, index;
  double xi, yi, dy, dyi;

  sc = sampleCount();

  if (xIsRising()) {

    iN = i0 = getIndexNearX(x);
    getPoint(i0, xi, yi);

    while ((i0>0) && (x-dx<xi)) {
      i0--;
      getPoint(i0, xi, yi);
    }
    getPoint(iN, xi, yi);

    while ((iN<sc-1) && (x+dx>xi)) {
      iN++;
      getPoint(iN, xi, yi);
    }
  } else {
    i0 = 0;
    iN = sampleCount()-1;
  }

  index = i0;
  getPoint(index, xi, yi);
  dy = fabs(y-yi);

  for (i=i0+1; i<=iN; i++) {
    getPoint(i, xi, yi);
    if (fabs(x-xi)<dx) {
      dyi = fabs(y-yi);
      if (dyi<dy) {
        dy = dyi;
        index = i;
      }
    }
  }
  return(index);
}


// vim: ts=2 sw=2 et
