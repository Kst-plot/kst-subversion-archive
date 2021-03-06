/***************************************************************************
                        kstcurvehint.h  -  Part of KST
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

#ifndef _KST_CURVEHINT_H
#define _KST_CURVEHINT_H

#include <qstring.h>
#include <qvaluelist.h>

#include "kstvector.h"

class KstBaseCurve;

class KstCurveHint : public KstShared {
  friend class KstDataObject;
  public:
    KstCurveHint(const QString& name = QString::null, const QString& x = QString::null, const QString& y = QString::null);

    virtual ~KstCurveHint();

    virtual const QString& curveName() const { return _curveName; }
    virtual const QString& xVectorName() const;
    virtual const QString& yVectorName() const;

    virtual KstVectorPtr xVector() const;
    virtual KstVectorPtr yVector() const;

    virtual KstSharedPtr<KstBaseCurve> makeCurve(const QString& tag, const QColor& color) const;

  protected:
    QString _curveName, _xVectorName, _yVectorName;
};

typedef KstSharedPtr<KstCurveHint> KstCurveHintPtr;
typedef QValueList<KstCurveHintPtr> KstCurveHintList;

#endif
