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

#include <QDomElement>

#include "kstprimitive.h"
#include "kstobjectcollection.h"

class KST_EXPORT KstString : public KstPrimitive {
  Q_OBJECT
  Q_PROPERTY(bool orphan READ orphan WRITE setOrphan)
  public:
    KstString(KstObjectTag in_tag = KstObjectTag::invalidTag, KstObject *provider = 0L, const QString& val = QString::null, bool orphan = false);
    KstString(QDomElement& e);
    KstString(const KstString&);   
    ~KstString();

  public:
    void setTag(const KstObjectTag& tag);
    void save(QTextStream &ts, const QString& indent = QString::null);
    UpdateType update(int updateCounter = -1);
    KstString& operator=(const QString& v);
    KstString& operator=(const char *v);

  public slots:
    const QString& value() const { return _value; }
    void setValue(const QString& inV);

    bool orphan() const { return _orphan; }
    void setOrphan(bool orphan) { _orphan = orphan; }

    bool editable() const { return _editable; }
    void setEditable(bool editable) { _editable = editable; }

  signals:
    void trigger();

  private:
    QString _value;
    bool _orphan : 1;
    bool _editable;
};

typedef QExplicitlySharedDataPointer<KstString> KstStringPtr;
typedef KstObjectList<KstStringPtr> KstStringList;
typedef KstObjectMap<KstStringPtr> KstStringMap;
typedef KstObjectCollection<KstString> KstStringCollection;

#endif
