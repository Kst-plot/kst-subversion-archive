/***************************************************************************
                          kstscalar.h  -  the base scalar type
                             -------------------
    begin                : March 24, 2003
    copyright            : (C) 2003 by cbn
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

#ifndef KSTSCALAR_H
#define KSTSCALAR_H

#include <qdom.h>
#include <qstring.h>
#include "kstobject.h"

/** The base class for all scalars. */

class KstScalar : public KstObject {
  Q_OBJECT
  Q_PROPERTY(bool orphan READ orphan WRITE setOrphan)
  public:
    KstScalar(const QString& in_tag = QString::null, double val = 0.0);
    KstScalar(QDomElement& e);
    virtual ~KstScalar();

    /* return a string representation of the scalar */
    const QString label() const { return QString::number(_value); }

    /** Save vector information */
    virtual void save(QTextStream &ts);

    /** Update the vector.  Return true if there was new data. */
    virtual UpdateType update(int updateCounter = -1);

    KstScalar& operator=(double v);

    bool isGlobal() const;

  public slots:
    /* return the value of the scalar */
    double value() const { return _value; }

    /** Set the value of the scalar - ignored for some types */
    void setValue(double inV);

    bool orphan() const { return _orphan; }

    void setOrphan(bool orphan) { _orphan = orphan; }
    
  signals:
    void trigger();

  private:
    double _value;
    bool _orphan;
};

typedef KstSharedPtr<KstScalar> KstScalarPtr;
typedef KstObjectList<KstScalarPtr> KstScalarList;
typedef KstObjectMap<KstScalarPtr> KstScalarMap;

#endif
// vim: ts=2 sw=2 et
