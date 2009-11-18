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

#include <qmap.h>
#include <qmutex.h>
#include <qlinkedlist.h>
#include <qobject.h>
#include <qpointer.h>
#include <QExplicitlySharedDataPointer>
#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kglobal.h>

#include "kst_export.h"
#include "rwlock.h"

//
// we define two different keys for datasource VS dataobject plugins so that 
// if the API for one changes, the other doesn't have to be updated also...
//

#define KST_CURRENT_DATASOURCE_KEY 0x00000007

#define KST_KEY_DATASOURCE_PLUGIN(x) extern "C" Q_UINT32 key_##x() { return KST_CURRENT_DATASOURCE_KEY; }

#define KST_CURRENT_DATAOBJECT_KEY 0x00000006

#define KST_KEY_DATAOBJECT_PLUGIN(x) extern "C" Q_UINT32 key_##x() { return KST_CURRENT_DATAOBJECT_KEY; }

class KstObjectPrivate;

// NOTE: In order to preserve binary compatibility with plugins, you must
//       update the plugin keys whenever you add, remove, or change member
//       variables or virtual functions, or when you remove or change
//       non-virtual functions.


class KstObjectTag {
  public:
    static const KstObjectTag invalidTag;

    static const QChar tagSeparator;
    static const QChar tagSeparatorReplacement;

    static const QStringList globalTagContext;
    static const QStringList constantTagContext;
    static const QStringList orphanTagContext;


    // construct a tag in a given context
    KstObjectTag(const QString& tag, const QStringList& context,
        unsigned int minDisplayComponents = 1) : _tag(cleanTag(tag)),
                                                 _context(context),
                                                 _minDisplayComponents(minDisplayComponents),
                                                 _uniqueDisplayComponents(UINT_MAX)
    {
    }

    // construct a tag in the context of another tag
    KstObjectTag(const QString& tag, const KstObjectTag& contextTag, bool alwaysShowContext = true) :
      _uniqueDisplayComponents(UINT_MAX)
    {
      _tag = cleanTag(tag);
      _context = contextTag.fullTag();
      _minDisplayComponents = 1 + (alwaysShowContext ? qMax(contextTag._minDisplayComponents, (unsigned int)1) : 0);
    }

    // construct a tag from a fullTag representation
    KstObjectTag(QStringList fullTag) : _minDisplayComponents(1), _uniqueDisplayComponents(UINT_MAX) {
      _tag = cleanTag(fullTag.last());
      fullTag.pop_back();
      _context = fullTag;
    }

    QString tag() const { return _tag; }
    QStringList fullTag() const { 
      QStringList ft(_context);
      ft << _tag;
      return ft;
    }
    QStringList context() const { return _context; }

    unsigned int components() const { 
      if (!isValid()) {
        return 0;
      } else {
        return 1 + _context.count();
      }
    }

    // change the tag, maintaining context
    void setTag(const QString& tag) {
      _tag = cleanTag(tag);
      _uniqueDisplayComponents = UINT_MAX;
    }

    // change the context
    void setContext(const QStringList& context) {
      _context = context;
      _uniqueDisplayComponents = UINT_MAX;
    }

    // change the tag and context
    void setTag(const QString& tag, const QStringList& context) {
      setTag(tag);
      setContext(context);
    }

    bool isValid() const { return !_tag.isEmpty(); }

    QString tagString() const { return fullTag().join(tagSeparator); }

    // display methods
    void setUniqueDisplayComponents(unsigned int n) {
      _uniqueDisplayComponents = n;
    }
    unsigned int uniqueDisplayComponents() const { return _uniqueDisplayComponents; }

    void setMinDisplayComponents(unsigned int n) {
      _minDisplayComponents = n;
    }

    QStringList displayFullTag() const { 
      QStringList outTag = _context + QStringList(_tag);
      int componentsToDisplay = qMin(qMax(_uniqueDisplayComponents, _minDisplayComponents), components());

      while (outTag.count() > componentsToDisplay) {
        outTag.pop_front();
      }
      return outTag;
    }

    QString displayString() const { 
      return displayFullTag().join(tagSeparator);
    }

    // factory for String representation
    static KstObjectTag fromString(const QString& str) {
      QStringList l = str.split(tagSeparator);
      if (l.isEmpty()) {
        return invalidTag;
      }

      QString t = l.last();
      l.pop_back();
      return KstObjectTag(t, l);
    }

    bool operator==(const KstObjectTag& tag) const {
      return (_tag == tag._tag && _context == tag._context);
    }

    bool operator!=(const KstObjectTag& tag) const {
      return (_tag != tag._tag || _context != tag._context);
    }

    static QString cleanTag(const QString& in_tag) {
      if (in_tag.contains(tagSeparator)) {
        QString tag = in_tag;
        tag.replace(tagSeparator, tagSeparatorReplacement);
        return tag;
      } else {
        return in_tag;
      }
    }

  private:
    QString _tag;
    QStringList _context;
    unsigned int _minDisplayComponents; // minimum number of components to use in display tag
    unsigned int _uniqueDisplayComponents;  // number of components necessary for unique display tag
};


class KST_EXPORT KstObject : public QObject, public QSharedData, public KstRWLock {
  Q_OBJECT

  public:
    KstObject();
    virtual ~KstObject();

    enum UpdateType { NO_CHANGE = 0, UPDATE };

    virtual UpdateType update(int updateCounter = -1) = 0;

    virtual QString tagName() const;
    virtual KstObjectTag& tag();
    virtual const KstObjectTag& tag() const;
    virtual void setTagName(const KstObjectTag& tag);

    virtual QString tagLabel() const;
    // Returns count - 2 to account for "this" and the list pointer, therefore
    // you MUST have a reference-counted pointer to call this function
    virtual int getUsage() const;

    // Returns true if update has already been done
    virtual bool checkUpdateCounter(int update_counter);

    int operator==(const QString&) const;

    virtual bool deleteDependents();

    virtual void setDirty(bool dirty = true);
    bool dirty() const;

    friend class UpdateThread;

    int _lastUpdateCounter;

    UpdateType setLastUpdateResult(UpdateType result);
    UpdateType lastUpdateResult() const;

  signals:
    void tagChanged();

  private:
    KstObjectTag _tag;
    bool _dirty;
    KstObject::UpdateType _lastUpdate;
};

typedef QExplicitlySharedDataPointer<KstObject> KstObjectPtr;

#include <qlinkedlist.h>

template<class T>
class KstObjectList : public QLinkedList<T> {
  public:
    KstObjectList() : QLinkedList<T>() {}
    KstObjectList(const KstObjectList<T>& x) : QLinkedList<T>(x) {}
    virtual ~KstObjectList() { }

    KstObjectList& operator=(const KstObjectList& l) {
      this->QLinkedList<T>::operator=(l);
      return *this;
    }

    virtual QStringList tagNames() {
      QStringList rc;
      for (typename QLinkedList<T>::ConstIterator it = QLinkedList<T>::begin(); it != QLinkedList<T>::end(); ++it) {
        rc << (*it)->tagName();
      }
      return rc;
    }

    QStringList tagNames() const {
      QStringList rc;
      for (typename QLinkedList<T>::ConstIterator it = QLinkedList<T>::begin(); it != QLinkedList<T>::end(); ++it) {
        rc << (*it)->tagName();
      }
      return rc;
    }

    virtual typename QLinkedList<T>::Iterator findTag(const QString& x) {
      for (typename QLinkedList<T>::Iterator it = QLinkedList<T>::begin(); it != QLinkedList<T>::end(); ++it) {
        if (*(*it) == x) {
          return it;
        }
      }
      return QLinkedList<T>::end();
    }

    virtual typename QLinkedList<T>::ConstIterator findTag(const QString& x) const {
      for (typename QLinkedList<T>::ConstIterator it = QLinkedList<T>::begin(); it != QLinkedList<T>::end(); ++it) {
        if (*(*it) == x) {
          return it;
        }
      }
      return QLinkedList<T>::end();
    }

    virtual int findIndexTag(const QString& x) const {
      int i = 0;
      for (typename QLinkedList<T>::ConstIterator it = QLinkedList<T>::begin(); it != QLinkedList<T>::end(); ++it) {
        if (*(*it) == x) {
          return i;
        }
        i++;
      }
      return -1;
    }

    virtual typename QLinkedList<T>::Iterator removeTag(const QString& x) {
      typename QLinkedList<T>::Iterator it = findTag(x);
      if (it != QLinkedList<T>::end()) {
        return QLinkedList<T>::erase(it);
      }
      return it;
    }

    KstRWLock& lock() const { return _lock; }

  private:
    mutable KstRWLock _lock;
};

/* Does locking for you automatically. */
template<class T, class S>
KstObjectList<QExplicitlySharedDataPointer<S> > kstObjectSubList(KstObjectList<QExplicitlySharedDataPointer<T> >& list) {
  list.lock().readLock();
  KstObjectList<QExplicitlySharedDataPointer<S> > rc;
  typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());
    if (x != 0L) {
      rc.append(x);
    }
  }

  list.lock().unlock();
  return rc;
}


/* Does locking for you automatically. */
template<class T, class S>
void kstObjectSplitList(KstObjectList<QExplicitlySharedDataPointer<T> >& list, KstObjectList<QExplicitlySharedDataPointer<S> >& inclusive, KstObjectList<QExplicitlySharedDataPointer<T> >& exclusive) {
  list.lock().readLock();
  typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());
    if (x != 0L) {
      inclusive.append(x);
    } else {
      exclusive.append(*it);
    }
  }

  list.lock().unlock();
}

template<class T>
class KstObjectMap : public QMap<QString,T> {
  public:
    KstObjectMap() : QMap<QString,T>() {}
    virtual ~KstObjectMap() {}

    virtual QStringList tagNames() {
      QStringList rc;
      for (typename QMap<QString,T>::ConstIterator it = QMap<QString,T>::begin(); it != QMap<QString,T>::end(); ++it) {
        rc << it.data()->tagName();
      }
      return rc;
    }

    QStringList tagNames() const {
      QStringList rc;
      for (typename QMap<QString,T>::ConstIterator it = QMap<QString,T>::begin(); it != QMap<QString,T>::end(); ++it) {
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
};

template <typename T, typename U>
inline T* kst_cast(QExplicitlySharedDataPointer<U> object) {
  return dynamic_cast<T*>(object.data());
}

#endif
