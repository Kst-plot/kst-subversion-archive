/***************************************************************************
                     kststring.h  -  the base string type
                             -------------------
    begin                : Sept 29, 2004
    copyright            : (C) 2004 by The University of Toronto
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

#ifndef KSTSTRING_H
#define KSTSTRING_H

#include <qdom.h>
#include "kstprimitive.h"

class KST_EXPORT KstString : public KstPrimitive {
  Q_OBJECT
  Q_PROPERTY(bool orphan READ orphan WRITE setOrphan)
  public:
    KstString(const QString& in_tag = QString::null, KstObject *provider = 0L, const QString& val = QString::null, bool orphan = false, bool doLock = true);
    KstString(QDomElement& e);
    ~KstString();

    /** Save information */
    void save(QTextStream &ts, const QString& indent = QString::null);

    /** Update the vector.  Return true if there was new data. */
    UpdateType update(int updateCounter = -1);

    KstString& operator=(const QString& v);
    KstString& operator=(const char *v);

  public slots:
    /* return the value of the string */
    const QString& value() const { return _value; }

    /** Set the value of the string - ignored for some types */
    void setValue(const QString& inV);

    bool orphan() const { return _orphan; }
    void setOrphan(bool orphan) { _orphan = orphan; }

  signals:
    void trigger();

  private:
    QString _value;
    bool _orphan : 1;
};

typedef KstSharedPtr<KstString> KstStringPtr;
typedef KstObjectList<KstStringPtr> KstStringList;
typedef KstObjectMap<KstStringPtr> KstStringMap;

#endif
// vim: ts=2 sw=2 et
