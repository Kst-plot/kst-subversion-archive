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

#include "kst2dplot.h"
#include "kstbasecurve.h"
#include "kstdatacollection.h"

KstBaseCurve::KstBaseCurve(const QDomElement& e) : KstDataObject(e) {
  commonConstructor();
}


KstBaseCurve::KstBaseCurve() : KstDataObject() {
  commonConstructor();
}


void KstBaseCurve::commonConstructor() {
  QColor in_color("red");
  setHasPoints(false);
  setHasLines(true);
  setHasBars(false);
  setLineWidth(0);
  setLineStyle(0);
  setBarStyle(0);
  MaxX = MinX = MeanX = MaxY = MinY = MeanY = NS = 0;
  MinPosX = MinPosY = 0;
  Color = in_color;
  NS = 0;
  _ignoreAutoScale = false;
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
      point(i0, rX, rY);
      if (x < rX) {
        i_top = i0;
      } else {
        i_bot = i0;
      }
    }
    double xt,xb;
    point(i_top, xt, rY);
    point(i_bot, xb, rY);
    if (xt - x < x - xb) {
      return i_top;
    } else {
      return i_bot;
    }
  } else {
    // Oh Oh... not monotonically rising - we have to search the entire curve!
    // May be unbearably slow for large vectors
    int i, i0, n;
    double dx, dx0, rX, rY;
    n = sampleCount();

    point(0, rX, rY);
    dx0 = fabs(x-rX);
    i0 = 0;

    for (i=1; i<n; i++) {
      point(i, rX, rY);
      dx = fabs(x-rX);
      if (dx < dx0) {
        dx0 = dx;
        i0 = i;
      }
    }
    return i0;
  }
}

/** getIndexNearXY: return index of point within (or closest too)
    x +- dx which is closest to y **/
int KstBaseCurve::getIndexNearXY(double x, double dx_per_pix, double y) {
  double xi, yi, dx, dxi, dy, dyi;
  bool first = true;
  int i,i0, iN, index;
  int sc = sampleCount();

  if (xIsRising()) {
    iN = i0 = getIndexNearX(x);

    point(i0, xi, yi);
    while (i0 > 0 && x-dx_per_pix < xi) {
      i0--;
      point(i0, xi, yi);
    }

    point(iN, xi, yi);
    while (iN < sc-1 && x+dx_per_pix > xi) {
      iN++;
      point(iN, xi, yi);
    }
  } else {
    i0 = 0;
    iN = sampleCount()-1;
  }

  index = i0;
  point(index, xi, yi);
  dx = fabs(x-xi);
  dy = fabs(y-yi);

  for (i=i0+1; i<=iN; i++) {
    point(i, xi, yi);
    dxi = fabs(x-xi);
    if (dxi < dx_per_pix) {
      dx = dxi;
      dyi = fabs(y-yi);
      if (first || dyi < dy) {
        first = false;
        index = i;
        dy = dyi;
      }
    } else if (dxi < dx) {
      dx = dxi;
      index = i;
    }
  }
  return index;
}


double KstBaseCurve::maxX() const {
  if (hasBars() && sampleCount() > 0) {
    return MaxX + (MaxX - MinX)/(2*(sampleCount()-1));
  }
  return MaxX;
}


double KstBaseCurve::minX() const {
  if (hasBars() && sampleCount() > 0) {
    return MinX - (MaxX - MinX)/(2*(sampleCount()-1));
  }
  return MinX;
}


bool KstBaseCurve::deleteDependents() {
  bool rc = KstDataObject::deleteDependents();
  KST::removeCurveFromPlots(this);
  return rc;
}


void KstBaseCurve::setHasPoints(bool in_HasPoints) {
  HasPoints = in_HasPoints;
  setDirty();
}


void KstBaseCurve::setHasLines(bool in_HasLines) {
  HasLines = in_HasLines;
  setDirty();
}


void KstBaseCurve::setHasBars(bool in_HasBars) {
  HasBars = in_HasBars;
  setDirty();
}


void KstBaseCurve::setLineWidth(int in_LineWidth) {
  LineWidth = in_LineWidth;
  setDirty();
}


void KstBaseCurve::setLineStyle(int in_LineStyle) {
  LineStyle = in_LineStyle;
  setDirty();
}


void KstBaseCurve::setBarStyle(int in_BarStyle) {
  BarStyle = in_BarStyle;
  setDirty();
}


void KstBaseCurve::setPointDensity(int in_PointDensity) {
  PointDensity = in_PointDensity;
  setDirty();
}


void KstBaseCurve::setColor(const QColor& new_c) {
  setDirty();
  Color = new_c;
}


void KstBaseCurve::setIgnoreAutoScale(bool ignoreAutoScale) {
  setDirty();
  _ignoreAutoScale = ignoreAutoScale;
}

// vim: ts=2 sw=2 et
