/***************************************************************************
              kstobjectcollection.h: collection of KstObjects
                             -------------------
    begin                : November 22, 2006
    copyright            : (C) 2006 The University of Toronto
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

#ifndef KSTOBJECTCOLLECTION_H
#define KSTOBJECTCOLLECTION_H

#include "ksdebug.h"
#include "kstobject.h"

// Forward Declarations
template <class T>
class KstObjectTreeNode;

class KstMatrix;
class KstScalar;
class KstString;
class KstVector;

// Typedefs
template <class T>
class KstObjectNameIndex : public QMap<QString, QValueList<KstObjectTreeNode<T> *> > {
};


/** KstObject Naming Tree */
template <class T>
class KstObjectTreeNode {
  public:
    KstObjectTreeNode(const QString& tag = QString::null);
    ~KstObjectTreeNode();

    QString nodeTag() const { return _tag; }
    QStringList fullTag() const;

    T *object() const { return _object; }

    KstObjectTreeNode<T> *parent() const { return _parent; }
    KstObjectTreeNode<T> *child(const QString& tag) const;
    QMap<QString, KstObjectTreeNode<T> *> children() const { return _children; }

    KstObjectTreeNode<T> *descendant(QStringList tag);
    KstObjectTreeNode<T> *addDescendant(T *o, KstObjectNameIndex<T> *index = NULL);
    bool removeDescendant(T *o, KstObjectNameIndex<T> *index = NULL);

    void clear();

  private:
    QString _tag;
    QGuardedPtr<T> _object;
    KstObjectTreeNode<T> *_parent;
    QMap<QString, KstObjectTreeNode<T> *> _children;
};


template <class T>
class KstObjectCollection {
  public:
    bool addObject(T *o);
    bool removeObject(T *o);
    void doRename(T *o, KstObjectTag newTag);

    KstSharedPtr<T> retrieveObject(QStringList tag);
    KstSharedPtr<T> retrieveObject(KstObjectTag tag);

    KstObjectTreeNode<T> *nameTreeRoot() { return &_root; }

    // QValueList compatibility
    bool isEmpty() const { return _list.isEmpty(); }
    typename KstObjectList<KstSharedPtr<T> >::size_type count() const { return _list.count(); }
    void append(T *o);
    void remove(T *o);
    void clear();
    QStringList tagNames() const;
    typename KstObjectList<KstSharedPtr<T> >::Iterator removeTag(const QString& x);
    typename KstObjectList<KstSharedPtr<T> >::Iterator findTag(const QString& x);
    typename KstObjectList<KstSharedPtr<T> >::ConstIterator findTag(const QString& x) const;
    typename KstObjectList<KstSharedPtr<T> >::Iterator begin();
    typename KstObjectList<KstSharedPtr<T> >::ConstIterator begin() const;
    typename KstObjectList<KstSharedPtr<T> >::Iterator end();
    typename KstObjectList<KstSharedPtr<T> >::ConstIterator end() const;
    KstObjectList<KstSharedPtr<T> >& list() { return _list; } // FIXME: this should be const, but it will break KstObjectSubList
    KstSharedPtr<T>& operator[](int i) { return _list[i]; }
    const KstSharedPtr<T>& operator[](int i) const { return _list[i]; }

    // locking
    KstRWLock& lock() const { return _list.lock(); }

  private:
    KstObjectTreeNode<T> _root;
    KstObjectNameIndex<T> _index;
    KstObjectList<KstSharedPtr<T> > _list; // owns the objects
};


/******************************************************************************/

// FIXME: this should probably return either a const list or another KstObjectCollection
template<class T, class S>
const KstObjectList<KstSharedPtr<S> > kstObjectSubList(KstObjectCollection<T>& coll) {
  KstObjectList<KstSharedPtr<T> > list = coll.list();
  list.lock().readLock();
  KstObjectList<KstSharedPtr<S> > rc;
  typename KstObjectList<KstSharedPtr<T> >::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());
    if (x != 0L) {
      rc.append(x);
    }
  }

  list.lock().unlock();
  return rc;
}

/******************************************************************************/

template <class T>
KstObjectTreeNode<T>::KstObjectTreeNode(const QString& tag) : _tag(tag),
                                                           _object(NULL),
                                                           _parent(NULL)
{
}


template <class T>
KstObjectTreeNode<T>::~KstObjectTreeNode() {
  for (QMapIterator<QString, KstObjectTreeNode*> i = _children.begin(); i != _children.end(); ++i) {
    delete (i.data());
  }
}


template <class T>
QStringList KstObjectTreeNode<T>::fullTag() const {
  QStringList tag;
  const KstObjectTreeNode *p = this;

  while (p) {
    if (!p->_tag.isEmpty()) {
      tag.prepend(p->_tag);
    }
    p = p->_parent;
  }

  return tag;
}


template <class T>
KstObjectTreeNode<T> *KstObjectTreeNode<T>::child(const QString& tag) const {
  if (_children.contains(tag)) {
    return _children[tag];
  } else {
    return NULL;
  }
}


template <class T>
KstObjectTreeNode<T> *KstObjectTreeNode<T>::descendant(QStringList tag) {
  KstObjectTreeNode<T> *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    currNode = currNode->child(*i);
    if (!currNode) {
      return NULL;
    }
  }

  return currNode;
}


template <class T>
KstObjectTreeNode<T> *KstObjectTreeNode<T>::addDescendant(T *o, KstObjectNameIndex<T> *index) {
  if (!o) {
    return NULL;
  }

  QStringList tag = o->tag().fullTag();

  KstObjectTreeNode<T> *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    KstObjectTreeNode<T> *nextNode = currNode->child(*i);
    if (!nextNode) {
      nextNode = new KstObjectTreeNode<T>(*i);
      nextNode->_parent = currNode;
      currNode->_children[*i] = nextNode;
      if (index) {
        (*index)[*i].append(nextNode);
      }
    }
    currNode = nextNode;
  }

  if (currNode->_object) {
    kstdDebug() << "Tried to add KstObject to naming tree:" << o->tag().tagString() << ", but there's already an object with that name" << endl;
    return NULL;
  } else {
    currNode->_object = o;
    kstdDebug() << "Added KstObject to naming tree:" << o->tag().tagString() << endl;
    return currNode;
  }
}


template <class T>
bool KstObjectTreeNode<T>::removeDescendant(T *o, KstObjectNameIndex<T> *index) {
  if (!o) {
    return false;
  }

  QStringList tag = o->tag().fullTag();

  KstObjectTreeNode<T> *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    KstObjectTreeNode<T> *nextNode = currNode->child(*i);
    if (!nextNode) {
//      kstdDebug() << "Tried to remove KstObject from naming tree:" << o->tag().tagString() << ", but the node is not in the tree" << endl;
      return false;
    }
    currNode = nextNode;
  }

  if (currNode->_object != QGuardedPtr<KstObject>(o)) {
//    kstdDebug() << "Tried to remove KstObject from naming tree:" << o->tag().tagString() << ", but the object is not in the tree" << endl;
    return false;
  } else {
    currNode->_object = NULL;
    QStringList::ConstIterator i = tag.end();
    while (i != tag.begin() && currNode->_object.isNull() && currNode->_children.isEmpty()) {
      --i;
      KstObjectTreeNode<T> *lastNode = currNode->_parent;
      lastNode->_children.remove(*i);
//      kstdDebug() << "Removed naming tree node:" << currNode->fullTag().join(KstObjectTag::tagSeparator) << endl;
      if (index) {
        (*index)[*i].remove(currNode);
        if ((*index)[*i].isEmpty()) {
          (*index).remove(*i);
        }
      }
      delete currNode;
      currNode = lastNode;
    }
    kstdDebug() << "Removed KstObject from naming tree:" << o->tag().tagString() << endl;
    return true;
  }
}

template <class T>
void KstObjectTreeNode<T>::clear() {
  _tag = QString::null;
  _parent = NULL;
  _object = NULL;

  // delete children
  for (QMapIterator<QString, KstObjectTreeNode*> i = _children.begin(); i != _children.end(); ++i) {
    delete (i.data());
  }
  _children.clear();
}


template <class T>
bool KstObjectCollection<T>::addObject(T *o) {
  _list.append(o);
  KstObjectTreeNode<T> *n = _root.addDescendant(o, &_index);
  return (n != NULL);
}


template <class T>
bool KstObjectCollection<T>::removeObject(T *o) {
  _list.remove(o);
  bool ok = _root.removeDescendant(o, &_index);

  return ok;
}

template <class T>
void KstObjectCollection<T>::doRename(T *o, KstObjectTag newTag) {
  _root.removeDescendant(o, &_index);
  o->KstObject::setTagName(newTag);
  _root.addDescendant(o, &_index);
}


template <class T>
KstSharedPtr<T> KstObjectCollection<T>::retrieveObject(QStringList tag) {
  kstdDebug() << "Retrieving object with tag: " << tag.join(KstObjectTag::tagSeparator) << endl;

  if (tag.isEmpty()) {
    return NULL;
  }

  if (_index.contains(tag.first()) && _index[tag.first()].count() == 1) {
    // the first tag element is unique, so use the index
    kstdDebug() << "  first tag element (" << tag.first() << ") is unique in index" << endl;

    KstObjectTreeNode<T> *n = _index[tag.first()].first();
    if (n) {
      tag.pop_front();
      n = n->descendant(tag);
    }
    if (n) {
      kstdDebug() << "  found node, returning object " << (void*) n->object() << endl;
      return n->object();
    }
  }

  // search through the tree
  KstObjectTreeNode<T> *n = _root.descendant(tag);
  if (n) {
    kstdDebug() << "  found node, returning object " << (void*) n->object() << endl;
    return n->object();
  } else {
    kstdDebug() << "  node not found" << endl;
    return NULL;
  }
}

template <class T>
KstSharedPtr<T> KstObjectCollection<T>::retrieveObject(KstObjectTag tag) {
  if (!tag.isValid()) {
    return NULL;
  }

  return retrieveObject(tag.fullTag());
}

// KstObjectList compatibility
template <class T>
void KstObjectCollection<T>::append(T *o) {
  addObject(o);
}

template <class T>
void KstObjectCollection<T>::remove(T *o) {
  removeObject(o);
}

template <class T>
void KstObjectCollection<T>::clear() {
  _root.clear();
  _index.clear();
  _list.clear();
}

template <class T>
QStringList KstObjectCollection<T>::tagNames() const {
  return _list.tagNames();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::removeTag(const QString& x) {
  // find object in tree
  T *obj = retrieveObject(KstObjectTag::fromString(x));

  if (obj) {
    // remove object from tree
    _root.removeDescendant(obj, &_index);

    // remove object from list
    typename KstObjectList<KstSharedPtr<T> >::Iterator it = _list.find(obj);
    if (it != _list.end()) {
      return _list.remove(it);
    }
  }
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::findTag(const QString& x) {
  T *obj = retrieveObject(KstObjectTag::fromString(x));
  if (obj) {
    return _list.find(obj);
  }
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::ConstIterator KstObjectCollection<T>::findTag(const QString& x) const {
  T *obj = retrieveObject(KstObjectTag::fromString(x));
  if (obj) {
    return _list.find(obj);
  }
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::begin() {
  return _list.begin();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::ConstIterator KstObjectCollection<T>::begin() const {
  return _list.begin();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::end() {
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::ConstIterator KstObjectCollection<T>::end() const {
  return _list.end();
}

#endif
// vim: ts=2 sw=2 et
