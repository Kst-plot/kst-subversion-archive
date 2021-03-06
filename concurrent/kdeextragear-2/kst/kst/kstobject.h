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

#include "kstsharedptr.h"

#include "rwlock.h"

#include <qmutex.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class KstObjectPrivate;

// BINARY COMPATIBILITY IS NOT YET GUARANTEED
class KstObject : public KstShared, public QObject, public KstRWLock {
public:
  KstObject();
  virtual ~KstObject();

  typedef enum {NO_CHANGE=0, READ, UPDATE} UpdateType;

  virtual UpdateType update(int updateCounter = -1) = 0;
  virtual const QString& tagName() const;
  virtual void setTagName(const QString& newTag);

  virtual QString tagLabel() const;
  // Returns count - 2 to account for "this" and the list pointer
  virtual int getUsage() const;

  // Returns true if update has already been done
  virtual bool checkUpdateCounter(int update_counter);

  int operator==(const QString&) const;

protected:
  virtual void virtual_hook(int id, void *data);
  int _lastUpdateCounter;

private:
  QString _tag;
  KstObjectPrivate *d;

  // NOTE: In order to preserve binary compatibility with plugins, you must
  //       not add, remove, or change member variables or virtual functions.
  //       You must also not remove or change non-virtual functions.  You can
  //       add new variables to the KstObjectPrivate pointer if you need
  //       them.  You can simulate virtual functions with the virtual_hook().
};


#include <qvaluelist.h>

template<class T>
class KstObjectList : public QValueList<T> {
  public:
    KstObjectList() : QValueList<T>() {}
    KstObjectList(const KstObjectList<T>& x) : QValueList<T>(x) {}
    virtual ~KstObjectList() { }

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

    KstRWLock& lock() const { return _lock; }

  private:
    mutable KstRWLock _lock;
};

/* Does locking for you automatically. */
template<class T, class S>
KstObjectList<KstSharedPtr<S> > kstObjectSubList(KstObjectList<KstSharedPtr<T> >& list) {
  list.lock().readLock();
  KstObjectList<KstSharedPtr<S> > rc;
  typename KstObjectList<KstSharedPtr<T> >::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());
    if (x != 0L) {
      rc.append(x);
    }
  }

  list.lock().readUnlock();
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
