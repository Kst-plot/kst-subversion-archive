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
#include <qguardedptr.h>
#include "kstobject.h"

#include "kst_export.h"

/** The base class for all scalars. */

class KST_EXPORT KstScalar : public KstObject {
  Q_OBJECT
  Q_PROPERTY(bool orphan READ orphan WRITE setOrphan)
  Q_PROPERTY(double value READ value WRITE setValue)
  Q_PROPERTY(bool displayable READ displayable WRITE setDisplayable)
  public:
    KstScalar(const QString& in_tag = QString::null, double val = 0.0, 
              bool orphan = false, bool displayable = true, 
              bool bLock = true, bool editable = false);
    KstScalar(const QDomElement& e);
    virtual ~KstScalar();

    /* return true if any scalars are dirty at the moment */
    static bool scalarsDirty();
    /* For use by the update thread */
    static void clearScalarsDirty();

    /* return a string representation of the scalar */
    QString label() const;

    /** Save scalar information */
    virtual void save(QTextStream &ts, const QString& indent = QString::null);

    /** Update the scalar.  Return true if there was new data. */
    virtual UpdateType update(int updateCounter = -1);

    KstScalar& operator=(double v);

    bool isGlobal() const;

    // Must not be a KstObjectPtr!
    virtual void setProvider(KstObject *obj);

  public slots:
    double value() const;

    /** Set the value of the scalar - ignored for some types */
    void setValue(double inV);

    bool orphan() const;
    void setOrphan(bool orphan);

    bool displayable() const;
    void setDisplayable(bool displayable);

    bool editable() const;
    void setEditable(bool editable);
  
  signals:
    void trigger();

  private:
    double _value;
    bool _orphan;
    bool _displayable;
    bool _editable;
    /** Possibly null.  Be careful, this is non-standard usage of a KstShared.
     * The purpose of this is to trigger hierarchical updates properly.
     */
    QGuardedPtr<KstObject> _provider;
};

typedef KstSharedPtr<KstScalar> KstScalarPtr;
typedef KstObjectList<KstScalarPtr> KstScalarList;
typedef KstObjectMap<KstScalarPtr> KstScalarMap;

#endif
// vim: ts=2 sw=2 et
