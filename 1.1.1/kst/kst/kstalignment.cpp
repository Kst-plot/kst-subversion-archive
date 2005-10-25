/***************************************************************************
                              kstalignment.cpp
                             -------------
    begin                : June 24, 2004
    copyright            : (C) 2004 The University of Toronto
                           (C) 2004 The University of British Columbia
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
#include "kstalignment.h"

KstAlignment KST::alignment;

bool operator<( const QPoint &p1, const QPoint &p2 ) {
  bool bRetVal = false;

  if (p1.x() < p2.x()) {
    bRetVal = true;
  } else if (p1.x() == p2.x()) {
    if (p1.y() < p2.y()) {
      bRetVal = true;
    }
  }

  return bRetVal;
}


KstAlignment::KstAlignment() {
}


void KstAlignment::reset() {
  _xAlignments.clear();
  _yAlignments.clear();
}


void KstAlignment::setPosition(const QRect& geometry, const QRect& plotRegion) {
  QPoint xAllRegion;
  QPoint yAllRegion;
  QPoint xPlotRegion;
  QPoint yPlotRegion;
  QPoint xPlotRegionCurrent;
  QPoint yPlotRegionCurrent;

  xAllRegion.setX(geometry.left());
  xAllRegion.setY(geometry.right());
  yAllRegion.setX(geometry.top());
  yAllRegion.setY(geometry.bottom());

  xPlotRegion.setX(plotRegion.left());
  xPlotRegion.setY(plotRegion.right());
  yPlotRegion.setX(plotRegion.top());
  yPlotRegion.setY(plotRegion.bottom());  

  if (_xAlignments.contains(xAllRegion)) {
    xPlotRegionCurrent = _xAlignments[xAllRegion];

    if (xPlotRegion.x() > xPlotRegionCurrent.x()) {
      xPlotRegionCurrent.setX(xPlotRegion.x());
    }
    if (xPlotRegion.y() > xPlotRegionCurrent.y()) {
      xPlotRegionCurrent.setY(xPlotRegion.y());
    }
    _xAlignments.insert(xAllRegion, xPlotRegionCurrent);
  } else {
    _xAlignments.insert(xAllRegion, xPlotRegion);
  }

  if (_yAlignments.contains(yAllRegion)) {
    yPlotRegionCurrent = _yAlignments[yAllRegion];

    if (yPlotRegion.x() > yPlotRegionCurrent.x()) {
      yPlotRegionCurrent.setX(yPlotRegion.x());
    }
    if (yPlotRegion.y() > yPlotRegionCurrent.y()) {
      yPlotRegionCurrent.setY(yPlotRegion.y());
    }
    _yAlignments.insert(yAllRegion, yPlotRegionCurrent);
  } else {
    _yAlignments.insert(yAllRegion, yPlotRegion);
  }
}


void KstAlignment::limits(const QRect& geometry, double& xleft, double& xright, double& ytop, double& ybottom, double dFactor) {
  QPoint xAllRegion;
  QPoint yAllRegion;

  xAllRegion.setX(geometry.left());
  xAllRegion.setY(geometry.right());
  yAllRegion.setX(geometry.top());
  yAllRegion.setY(geometry.bottom());

  if (_xAlignments.contains(xAllRegion)) {
    xleft = (double)_xAlignments[xAllRegion].x();
    xright = (double)_xAlignments[xAllRegion].y();
  } else {
    xleft = 0.0;
    xright = 0.0;
  }

  if (_yAlignments.contains(yAllRegion)) {
    ytop = (double)_yAlignments[yAllRegion].x();
    ybottom = (double)_yAlignments[yAllRegion].y();
  } else {
    ytop = 0.0;
    ybottom = 0.0;
  }

  xleft   *= dFactor;
  xright  *= dFactor;
  ytop    *= dFactor;
  ybottom *= dFactor;
}


QRect KstAlignment::limits(const QRect& geometry) {
  QPoint xAllRegion;
  QPoint yAllRegion;
  QRect plotRegion;

  xAllRegion.setX(geometry.left());
  xAllRegion.setY(geometry.right());
  yAllRegion.setX(geometry.top());
  yAllRegion.setY(geometry.bottom());

  if (_xAlignments.contains(xAllRegion)) {
    plotRegion.setLeft(_xAlignments[xAllRegion].x());
    plotRegion.setRight(_xAlignments[xAllRegion].y());
  } else {
    plotRegion.setLeft(0);
    plotRegion.setRight(0);
  }

  if (_yAlignments.contains(yAllRegion)) {
    plotRegion.setTop(_yAlignments[yAllRegion].x());
    plotRegion.setBottom(_yAlignments[yAllRegion].y());
  } else {
    plotRegion.setTop(0);
    plotRegion.setBottom(0);
  }

  return plotRegion;
}
