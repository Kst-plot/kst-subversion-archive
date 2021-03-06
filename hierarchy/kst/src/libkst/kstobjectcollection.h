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

// NAMEDEBUG: 0 for no debug, 1 for some debug, 2 for more debug, 3 for all debug
#define NAMEDEBUG 0

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

    KstObjectTreeNode<T> *descendant(const QStringList& tag);
    const KstObjectTreeNode<T> *descendant(const QStringList& tag) const;
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
    void doRename(T *o, const KstObjectTag& newTag);

    KstSharedPtr<T> retrieveObject(QStringList tag) const;
    KstSharedPtr<T> retrieveObject(const KstObjectTag& tag) const;
    bool tagExists(const QString& tag) const;
    bool tagExists(const KstObjectTag& tag) const;

    // get the shortest unique tag in in_tag
    KstObjectTag shortestUniqueTag(const KstObjectTag& in_tag) const;
    // get the minimum number of tag components of in_tag necessary for a unique tag
    unsigned int componentsForUniqueTag(const KstObjectTag& in_tag) const;

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
    typename KstObjectList<KstSharedPtr<T> >::Iterator findTag(const KstObjectTag& x);
    typename KstObjectList<KstSharedPtr<T> >::ConstIterator findTag(const KstObjectTag& x) const;
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
    QValueList<KstObjectTreeNode<T> *> relatedNodes(T *obj);
    void relatedNodesHelper(T *o, KstObjectTreeNode<T> *n, QValueList<KstObjectTreeNode<T> *>& nodes);

    // must be called AFTER the object is added to the index, while holding a write lock
    void updateDisplayComponents(T *obj);
    void updateDisplayComponents(QValueList<KstObjectTreeNode<T> *> nodes);

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
const KstObjectTreeNode<T> *KstObjectTreeNode<T>::descendant(const QStringList& tag) const {
  const KstObjectTreeNode<T> *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    currNode = currNode->child(*i);
    if (!currNode) {
      return NULL;
    }
  }

  return currNode;
}


template <class T>
KstObjectTreeNode<T> *KstObjectTreeNode<T>::descendant(const QStringList& tag) {
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
#if NAMEDEBUG > 0
    kstdDebug() << "Tried to add KstObject to naming tree " << (void*)this << ": \"" << o->tag().tagString() << "\", but there's already an object with that name" << endl;
#endif
    return NULL;
  } else {
    currNode->_object = o;
#if NAMEDEBUG > 0
    kstdDebug() << "Added KstObject to naming tree " << (void*)this << ": \"" << o->tag().tagString() << "\"" << endl;
#endif
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
#if NAMEDEBUG > 0
      kstdDebug() << "Tried to remove KstObject from naming tree: \"" << o->tag().tagString() << "\", but the node is not in the tree" << endl;
#endif
      return false;
    }
    currNode = nextNode;
  }

  if (currNode->_object != QGuardedPtr<KstObject>(o)) {
#if NAMEDEBUG > 0
    kstdDebug() << "Tried to remove KstObject from naming tree: \"" << o->tag().tagString() << "\", but the object is not in the tree" << endl;
#endif
    return false;
  } else {
    currNode->_object = NULL;
    QStringList::ConstIterator i = tag.end();
    while (i != tag.begin() && currNode->_object.isNull() && currNode->_children.isEmpty()) {
      --i;
      KstObjectTreeNode<T> *lastNode = currNode->_parent;
      lastNode->_children.remove(*i);
#if NAMEDEBUG > 1
      kstdDebug() << "Removed naming tree node: \"" << currNode->fullTag().join(KstObjectTag::tagSeparator) << "\"" << endl;
#endif
      if (index) {
        (*index)[*i].remove(currNode);
        if ((*index)[*i].isEmpty()) {
          (*index).remove(*i);
        }
      }
      delete currNode;
      currNode = lastNode;
    }
#if NAMEDEBUG > 0
    kstdDebug() << "Removed KstObject from naming tree: \"" << o->tag().tagString() << "\"" << endl;
#endif
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
  if (!o) {
    return false;
  }

  _list.append(o);

  QValueList<KstObjectTreeNode<T> *> relNodes = relatedNodes(o);

  KstObjectTreeNode<T> *n = _root.addDescendant(o, &_index);

  if (n) {
    updateDisplayComponents(o);
    updateDisplayComponents(relNodes);
    return true;
  } else {
    // TODO: handle failed insert?
    return false;
  }
}


template <class T>
bool KstObjectCollection<T>::removeObject(T *o) {
  if (!o) {
    return false;
  }

  if (!_list.contains(o)) {
#if NAMEDEBUG > 1
    kstdDebug() << "Trying to delete a non-existant object from the collection: " << o->tag().tagString() << endl;
#endif
    return false;
  }

#if NAMEDEBUG > 1
    kstdDebug() << "Removing object from the collection: " << o->tag().tagString() << endl;
#endif

#if NAMEDEBUG > 2
    kstdDebug() << "  fetching related nodes" << endl;
#endif
  QValueList<KstObjectTreeNode<T> *> relNodes = relatedNodes(o);

#if NAMEDEBUG > 2
    kstdDebug() << "  removing object from tree" << endl;
#endif
  bool ok = _root.removeDescendant(o, &_index);

  if (ok) {
#if NAMEDEBUG > 2
    kstdDebug() << "  updating display components" << endl;
#endif
    updateDisplayComponents(relNodes);

#if NAMEDEBUG > 2
    kstdDebug() << "  removing object from list" << endl;
#endif
    _list.remove(o);
  }

  return ok;
}


// Rename a KstObject in the collection.
//
// Updates the display components of all related objects. This can be somewhat
// expensive, but it shouldn't happen very often.
template <class T>
void KstObjectCollection<T>::doRename(T *o, const KstObjectTag& newTag) {
  if (!o) {
    return;
  }

  QValueList<KstObjectTreeNode<T> *> relNodes = relatedNodes(o);

  _root.removeDescendant(o, &_index);

  o->KstObject::setTagName(newTag);

  if (_root.addDescendant(o, &_index)) {
    relNodes += relatedNodes(o);  // TODO: remove duplicates
    updateDisplayComponents(o);
    updateDisplayComponents(relNodes);
  } else {
    // TODO: handle failed insert
  }
}


template <class T>
KstSharedPtr<T> KstObjectCollection<T>::retrieveObject(QStringList tag) const {
#if NAMEDEBUG > 1
  kstdDebug() << "Retrieving object with tag: \"" << tag.join(KstObjectTag::tagSeparator) << "\"" << endl;
#endif

  if (tag.isEmpty()) {
    return NULL;
  }

  if (_index.contains(tag.first()) && _index[tag.first()].count() == 1) {
    // the first tag element is unique, so use the index
#if NAMEDEBUG > 2
    kstdDebug() << "  first tag element (\"" << tag.first() << "\") is unique in index" << endl;
#endif

    KstObjectTreeNode<T> *n = _index[tag.first()].first();
    if (n) {
      tag.pop_front();
      n = n->descendant(tag);
    }
    if (n) {
#if NAMEDEBUG > 1
      kstdDebug() << "  found node, returning object " << (void*) n->object() << endl;
#endif
      return n->object();
    }
  }

  // search through the tree
  const KstObjectTreeNode<T> *n = _root.descendant(tag);
  if (n) {
#if NAMEDEBUG > 1
    kstdDebug() << "  found node, returning object " << (void*) n->object() << endl;
#endif
    return n->object();
  } else {
#if NAMEDEBUG > 1
    kstdDebug() << "  node not found" << endl;
#endif
    return NULL;
  }
}

template <class T>
KstSharedPtr<T> KstObjectCollection<T>::retrieveObject(const KstObjectTag& tag) const {
  if (!tag.isValid()) {
    return NULL;
  }

  return retrieveObject(tag.fullTag());
}

template <class T>
bool KstObjectCollection<T>::tagExists(const QString& tag) const {
  return (_index.contains(tag) && _index[tag].count() > 0);
}

template <class T>
bool KstObjectCollection<T>::tagExists(const KstObjectTag& tag) const {
  const KstObjectTreeNode<T> *n = _root.descendant(tag.fullTag());
  return (n != NULL);
}

template <class T>
KstObjectTag KstObjectCollection<T>::shortestUniqueTag(const KstObjectTag& tag) const {
  QStringList in_tag = tag.fullTag();
  QStringList out_tag;

  QStringList::ConstIterator it = in_tag.end();
  if (it == in_tag.begin()) {
    return KstObjectTag::invalidTag;
  }

  // add components starting from the end until a unique tag is found
  do {
    --it;
    out_tag.prepend(*it);
    if (_index.contains(*it) && _index[*it].count() == 1) {
      // found unique tag
      break;
    }
  } while (it != in_tag.begin());

  return KstObjectTag(out_tag);
}

template <class T>
unsigned int KstObjectCollection<T>::componentsForUniqueTag(const KstObjectTag& tag) const {
  QStringList in_tag = tag.fullTag();
  unsigned int components = 0;

  QStringList::ConstIterator it = in_tag.end();
  if (it == in_tag.begin()) {
    // tag is empty
    return components;
  }

  // add components starting from the end until a unique tag is found
  do {
    --it;
    components++;
    if (_index.contains(*it) && _index[*it].count() == 1) {
      // found unique tag
      break;
    }
  } while (it != in_tag.begin());

  return components;
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
#if NAMEDEBUG > 0
  kstdDebug () << "Clearing object collection " << (void*) this << endl;
#endif
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
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::findTag(const KstObjectTag& x) {
  T *obj = retrieveObject(x);
  if (obj) {
    return _list.find(obj);
  } else {
    // For historical compatibility:
    // previously, output vectors of equations, PSDs, etc. were named PSD1-ABCDE-freq
    // now, they are PSD1-ABCDE/freq
    QString newTag = x.tagString();
    newTag.replace(newTag.findRev('-'), 1, KstObjectTag::tagSeparator);
    obj = retrieveObject(KstObjectTag::fromString(newTag));
    if (obj) {
      return _list.find(obj);
    }
  }
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::Iterator KstObjectCollection<T>::findTag(const QString& x) {
  return findTag(KstObjectTag::fromString(x));
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::ConstIterator KstObjectCollection<T>::findTag(const KstObjectTag& x) const {
  T *obj = retrieveObject(x);
  if (obj) {
    return _list.find(obj);
  } else {
    // For historical compatibility:
    // previously, output vectors of equations, PSDs, etc. were named PSD1-ABCDE-freq
    // now, they are PSD1-ABCDE/freq
    QString newTag = x.tagString();
    newTag.replace(newTag.findRev('-'), 1, KstObjectTag::tagSeparator);
    obj = retrieveObject(KstObjectTag::fromString(newTag));
    if (obj) {
      return _list.find(obj);
    }
  }
  return _list.end();
}

template <class T>
typename KstObjectList<KstSharedPtr<T> >::ConstIterator KstObjectCollection<T>::findTag(const QString& x) const {
  return findTag(KstObjectTag::fromString(x));
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


// must be called AFTER the object is added to the index, while holding a write lock
template <class T>
void KstObjectCollection<T>::updateDisplayComponents(T *obj) {
  if (!obj) {
    return;
  }

  KstObjectTag tag = obj->tag();

  if (!_index.contains(tag.tag())) {
    return;
  }

  unsigned int nc = componentsForUniqueTag(tag);
  if (tag.uniqueDisplayComponents() != nc) {
#if NAMEDEBUG > 2
    kstdDebug() << "Changing display components on \"" << tag.tagString() << "\" from " << tag.uniqueDisplayComponents() << " to " << nc << endl;
#endif
//    KstWriteLocker l(obj);
    obj->tag().setUniqueDisplayComponents(nc);
  }
}

template <class T>
void KstObjectCollection<T>::updateDisplayComponents(QValueList<KstObjectTreeNode<T> *> nodes) {
  for (typename QValueList<KstObjectTreeNode<T> *>::Iterator i = nodes.begin(); i != nodes.end(); ++i) {
    updateDisplayComponents((*i)->object());
  }
}


// recursion helper
template <class T>
void KstObjectCollection<T>::relatedNodesHelper(T *o, KstObjectTreeNode<T> *n, QValueList<KstObjectTreeNode<T> *>& nodes) {

  if (n->object() && n->object() != o && !nodes.contains(n)) {
#if NAMEDEBUG > 2
          kstdDebug() << "Found related node to \"" << o->tag().tagString() << "\": \"" << n->object()->tag().tagString() << "\"" << endl; 
#endif
    nodes << n;
  }

  if (!n->children().isEmpty()) {
    // non-leaf node, so recurse
    QMap<QString, KstObjectTreeNode<T> *> children = n->children();
    for (typename QMap<QString, KstObjectTreeNode<T> *>::ConstIterator i = children.begin(); i != children.end(); ++i) {
      relatedNodesHelper(o, *i, nodes);
    }
  }
}

// Find the nodes with KstObjects which are affected by the addition or removal
// of an object with the given tag.
//
// There should not be any duplicates in the returned list.
template <class T>
QValueList<KstObjectTreeNode<T> *> KstObjectCollection<T>::relatedNodes(T *o) {
  QValueList<KstObjectTreeNode<T> *> nodes;

  if (!o) {
    return nodes;
  }

#if NAMEDEBUG > 2
  kstdDebug() << "Looking for related nodes to \"" << o->tag().tagString() << "\"" << endl; 
#endif

  QStringList ft = o->tag().fullTag();

  for (QStringList::ConstIterator i = ft.begin(); i != ft.end(); ++i) {
    if (_index.contains(*i)) {
      QValueList<KstObjectTreeNode<T> *> nodeList = _index[*i];
      for (typename QValueList<KstObjectTreeNode<T> *>::ConstIterator i2 = nodeList.begin(); i2 != nodeList.end(); ++i2) {
        relatedNodesHelper(o, *i2, nodes);
      }
    }
  }

  return nodes;
}


#endif
// vim: ts=2 sw=2 et
