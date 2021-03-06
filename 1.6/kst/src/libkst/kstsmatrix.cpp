/***************************************************************************
        kstsmatrix.h  - a gradient from zMin, zMax that goes across
                        in x or y direction 
                             -------------------
    begin                : July, 2005
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
#include "kstsmatrix.h"
#include <qstylesheet.h>

KstSMatrix::KstSMatrix(const QDomElement &e) : KstMatrix() {
  double in_xMin = 0.0, in_yMin = 0.0, in_xStep = 1.0, in_yStep = 1.0;
  double in_gradZMin = 0.0, in_gradZMax = 1.0;
  bool in_xDirection = true;
  int in_nX = 2, in_nY = 2;
  QString in_tag = QString::null;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "nx") {
        in_nX = e.text().toInt();
      } else if (e.tagName() == "ny") {
        in_nY = e.text().toInt();
      } else if (e.tagName() == "xmin") {
        in_xMin = e.text().toDouble();
      } else if (e.tagName() == "ymin") {
        in_yMin = e.text().toDouble();
      } else if (e.tagName() == "xstep") {
        in_xStep = e.text().toDouble();
      } else if (e.tagName() == "ystep") {
        in_yStep = e.text().toDouble();
      } else if (e.tagName() == "gradzmin") {
        in_gradZMin = e.text().toDouble();
      } else if (e.tagName() == "gradzmax") {
        in_gradZMax = e.text().toDouble(); 
      } else if (e.tagName() == "xdirection") {
        in_xDirection = (e.text() != "0");  
      }
    }
    n = n.nextSibling();
  }

  _saveable = true;
  _editable = true;
  _zSize = 0;
  change(KstObjectTag::fromString(in_tag), in_nX, in_nY, in_xMin, in_yMin, in_xStep, in_yStep, in_gradZMin, in_gradZMax, in_xDirection);
}

KstSMatrix::KstSMatrix(KstObjectTag tag,
                       uint nX, uint nY, double minX, double minY,
                       double stepX, double stepY,
                       double gradZMin, double gradZMax,
                       bool xDirection) : KstMatrix() {
  _saveable = true;
  _editable = true;
  _zSize = 0;
  change(tag, nX, nY, minX, minY, stepX, stepY, gradZMin, gradZMax, xDirection);
}

void KstSMatrix::save(QTextStream &ts, const QString& indent) {
  QString indent2 = "  ";

  ts << indent << "<smatrix>" << endl;
  ts << indent << indent2 << "<tag>" << QStyleSheet::escape(tag().tagString()) << "</tag>" << endl;
  ts << indent << indent2 << "<xmin>" << minX() << "</xmin>" << endl;
  ts << indent << indent2 << "<ymin>" << minY() << "</ymin>" << endl;
  ts << indent << indent2 << "<nx>" << xNumSteps() << "</nx>" << endl;
  ts << indent << indent2 << "<ny>" << yNumSteps() << "</ny>" << endl;
  ts << indent << indent2 << "<xstep>" << xStepSize() << "</xstep>" << endl;
  ts << indent << indent2 << "<ystep>" << xStepSize() << "</ystep>" << endl;
  ts << indent << indent2 << "<gradzmin>" << _gradZMin << "</gradzmin>" << endl;
  ts << indent << indent2 << "<gradzmax>" << _gradZMax << "</gradzmax>" << endl;
  ts << indent << indent2 << "<xdirection>" << _xDirection << "</xdirection>" << endl;
  ts << indent << "</smatrix>" << endl;
}

void KstSMatrix::change(KstObjectTag tag, uint nX,
                        uint nY, double minX, double minY, double stepX,
                        double stepY, double gradZMin, double gradZMax,
                        bool xDirection) {
  setTagName(tag);

  // some checks on parameters
  if (nX < 1) {
    nX = 1;
  }
  if (nY < 1) {
    nY = 1;
  }
  if (stepX <= 0.0) {
    stepX = 0.1;
  }
  if (stepY <= 0.0) {
    stepY = 0.1;
  }

  _nX = nX;
  _nY = nY;
  _minX = minX;
  _minY = minY;
  _stepX = stepX;
  _stepY = stepY;
  _gradZMin = gradZMin;
  _gradZMax = gradZMax;
  _xDirection = xDirection;

  if (_nX*_nY != _zSize) {
    resizeZ(_nX*_nY, false);
  }

  // zIncrement can be negative, to reverse gradient direction
  double zIncrement;
  if (_xDirection) {
    if (_nX > 1) {
      zIncrement = (_gradZMax - _gradZMin) / (_nX - 1);
    } else {
      zIncrement = 0.0;
    }
  } else {
    if (_nY > 1) {
      zIncrement = (_gradZMax - _gradZMin) / (_nY - 1);  
    } else {
      zIncrement = 0.0;
    }
  }

  double sum = 0.0;
  double sumsquared = 0.0;

  // fill in the matrix with the gradient
  for (int i = 0; i < _nX; i++) {
    for (int j = 0; j < _nY; j++) {
      if (_xDirection) {
        _z[i*nY + j] = _gradZMin + i*zIncrement;
      } else {
        _z[i*nY + j] = _gradZMin + j*zIncrement;
      }
      sum += _z[i];
      sumsquared += _z[i] * _z[i];
    }
  }

  _statScalars["sum"]->setValue(sum);
  _statScalars["sumsquared"]->setValue(sumsquared);
  _statScalars["max"]->setValue(gradZMin);
  _statScalars["min"]->setValue(gradZMax);
  _statScalars["minpos"]->setValue(0.0);
  _statScalars["mean"]->setValue((gradZMax-gradZMin)/2.0);
  _statScalars["ns"]->setValue(_nX*_nY);
  _statScalars["rms"]->setValue(0.0);
  _statScalars["sigma"]->setValue(0.0);

  setDirty(true);
}


double KstSMatrix::minValueNoSpike() const {
  return _minNoSpike;
}


double KstSMatrix::maxValueNoSpike() const {
  return _maxNoSpike;
}


void KstSMatrix::calcNoSpikeRange(double per) {
  Q_UNUSED(per)

  _minNoSpike = _gradZMin;
  _maxNoSpike = _gradZMax;
}
