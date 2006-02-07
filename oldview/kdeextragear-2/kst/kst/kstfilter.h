/***************************************************************************
                                 kstfilter.h
                             -------------------
    begin                : Dec 02 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef KSTFILTER_H
#define KSTFILTER_H

#include <qstring.h>

#include "kstplugin.h"

class KstFilter : public KstPlugin {
  public:
    KstFilter();
    KstFilter(QDomElement &e);
    virtual ~KstFilter();

    virtual void apply(const KstVectorPtr in, KstVectorPtr out);
    virtual bool setPlugin(KstSharedPtr<Plugin> plugin);

  private:
    void *_localData;
};

typedef KstSharedPtr<KstFilter> KstFilterPtr;


class KstFilterSet : public KstObjectList<KstFilterPtr>, public KstShared {
  public:
    KstFilterSet();
    KstFilterSet(QDomElement& e);
    virtual ~KstFilterSet();

    virtual void apply(const KstVectorPtr in, KstVectorPtr out);
    const QString& name() const { return _name; }
    void setName(const QString& n) { _name = n; }

    virtual void save(QTextStream& ts);

  private:
    void parseFilter(const QString& n, QDomElement& e);

    QString _name;
};


typedef KstSharedPtr<KstFilterSet> KstFilterSetPtr;

class KstFilterSetList : public QValueList<KstFilterSetPtr> {
  public:
    KstFilterSetList() : QValueList<KstFilterSetPtr>() {}
    virtual ~KstFilterSetList() {}

    KstFilterSetPtr find(const QString& name) {
      for (Iterator i = begin(); i != end(); ++i) {
        if ((*i)->name() == name) {
          return *i;
        }
      }
      return KstFilterSetPtr();
    }

    KstRWLock& lock() const { return _lock; }

  private:
    mutable KstRWLock _lock;
};


#endif

// vim: ts=2 sw=2 et
