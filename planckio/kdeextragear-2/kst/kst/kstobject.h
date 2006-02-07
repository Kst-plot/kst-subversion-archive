/***************************************************************************
              kstobject.h: abstract base class for all Kst objects
                             -------------------
    begin                : May 22, 2003
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

#ifndef KSTOBJECT_H
#define KSTOBJECT_H

#include <ksharedptr.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

class KstObject : public KShared, public QObject {
public:
  KstObject();
  virtual ~KstObject();

  typedef enum {NO_CHANGE=0, READ, UPDATE} UpdateType;

  virtual UpdateType update(int updateCounter = -1) = 0;
  virtual const QString& tagName() const { return _tag; }
  virtual void setTagName(const QString& newTag) { _tag = newTag; }

  virtual QString tagLabel() const { return QString("[%1]").arg(_tag); }
  // Returns count - 2 to account for "this" and the list pointer
  virtual int getUsage() const { return _KShared_count() - 1; }

  // Returns true if update has already been done
  virtual bool checkUpdateCounter(int update_counter);

  int operator==(const QString&) const;

protected:
  QString _tag;
  int _lastUpdateCounter;
};


#include <qvaluelist.h>

template<class T>
class KstObjectList : public QValueList<T> {
  public:
    KstObjectList() : QValueList<T>() {}
    KstObjectList(const QValueList<T>& x) : QValueList<T>(x) {}
    virtual ~KstObjectList() {}

    virtual QStringList tagNames() {
      QStringList rc;
      for (typename QValueList<T>::Iterator it = QValueList<T>::begin(); it != QValueList<T>::end(); ++it) {
        rc << (*it)->tagName();
      }
      return rc;
    }

    virtual typename QValueList<T>::Iterator findTag(const QString& x) {
      for (typename QValueList<T>::Iterator it = QValueList<T>::begin(); it != QValueList<T>::end(); ++it) {
        if (*(*it) == x) {
          return it;
        }
      }
      return QValueList<T>::end();
    }

    virtual typename QValueList<T>::ConstIterator findTag(const QString& x) const {
      for (typename QValueList<T>::ConstIterator it = QValueList<T>::begin(); it != QValueList<T>::end(); ++it) {
        if (*(*it) == x) {
          return it;
        }
      }
      return QValueList<T>::end();
    }

    virtual int findIndexTag(const QString& x) const {
      int i = 0;
      for (typename QValueList<T>::ConstIterator it = QValueList<T>::begin(); it != QValueList<T>::end(); ++it) {
        if (*(*it) == x) {
          return i;
        }
        i++;
      }
      return -1;
    }

    virtual typename QValueList<T>::iterator removeTag(const QString& x) {
      typename QValueList<T>::iterator it = findTag(x);
      if (it != QValueList<T>::end()) {
        return QValueList<T>::remove(it);
      }
      return it;
    }
};

template<class T, class S>
KstObjectList<KSharedPtr<S> > kstObjectSubList(KstObjectList<KSharedPtr<T> >& list) {
  KstObjectList<KSharedPtr<S> > rc;
  typename KstObjectList<KSharedPtr<T> >::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());
    if (x != 0L) {
      rc.append(x);
    }
  }

return rc;
}


#include <qmap.h>

template<class T>
class KstObjectMap : public QMap<QString,T> {
  public:
    KstObjectMap() : QMap<QString,T>() {}
    virtual ~KstObjectMap() {}
    
    virtual QStringList tagNames() {
      QStringList rc;
      for (typename QMap<QString,T>::Iterator it = QMap<QString,T>::begin(); it != QMap<QString,T>::end(); ++it) {
        rc << it.data()->tagName();
      }
      return rc;
    }

    // Careful - sets key(value) == value->tagName();
    typename QMap<QString,T>::iterator insertObject(const T& value) {
      return QMap<QString,T>::insert(value->tagName(), value);
    }

    typename QMap<QString,T>::iterator findTag(const QString& tag) {
      typename QMap<QString,T>::iterator i;
      for (i = QMap<QString,T>::begin(); i != QMap<QString,T>::end(); ++i) {
        if (i.data()->tagName() == tag) {
          break;
        }
      }
      return i;
    }

/*
These are wrong.  We should not assume that key(x) == x->tagName().
    bool contains(const T& value) {
      return QMap<QString,T>::contains(value->tagName());
    }

    typename QMap<QString,T>::iterator find(const T& value) {
      return QMap<QString,T>::find(value->tagName());
    }

    void remove(const T& value) {
      QMap<QString,T>::remove(value->tagName());
    }

*/
};

#endif
// vim: ts=2 sw=2 et
