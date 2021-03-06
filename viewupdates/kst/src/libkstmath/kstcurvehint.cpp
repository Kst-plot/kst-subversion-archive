/***************************************************************************
                        kstcurvehint.cpp  -  Part of KST
                             -------------------
    begin                : Sun Sep 26 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "kstbasecurve.h"
#include "kstcurvehint.h"
#include "kstdatacollection.h"
#include "kstvcurve.h"

#include "ksdebug.h"

KstCurveHint::KstCurveHint(const QString& name, const QString& x, const QString& y)
: KstShared(),  _curveName(name), _xVectorName(x), _yVectorName(y) {
}


KstCurveHint::~KstCurveHint() {
}


const QString& KstCurveHint::xVectorName() const {
  return _xVectorName;
}


const QString& KstCurveHint::yVectorName() const {
  return _yVectorName;
}


KstVectorPtr KstCurveHint::xVector() const {
  return *KST::vectorList.findTag(_xVectorName);
}


KstVectorPtr KstCurveHint::yVector() const {
  return *KST::vectorList.findTag(_yVectorName);
}


KstBaseCurvePtr KstCurveHint::makeCurve(const QString& tag, const QColor& color) const {
  KstVectorPtr x = xVector();
  KstVectorPtr y = yVector();
  if (!x || !y) {
    kstdDebug() << "Couldn't find either " << _xVectorName << " or " << _yVectorName << endl;
    return 0L;
  }

  return new KstVCurve(tag, x, y, 0L, 0L, 0L, 0L, color);
}


// vim: ts=2 sw=2 et
