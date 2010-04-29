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

#include <QHash>
#include <QMultiHash>

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
class KstObjectNameIndex : public QMultiHash<QString, QLinkedList<KstObjectTreeNode<T> *> > {
};


template <class T>
class KstObjectTreeNode : public QObject {
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
    QPointer<T> _object;
    KstObjectTreeNode<T> *_parent;
    QMap<QString, KstObjectTreeNode<T> *> _children;
};


template <class T>
class KstObjectCollection {
  public:
    KstObjectCollection();

    bool addObject(T *o);
    bool removeObject(T *o);
    void doRename(T *o, const KstObjectTag& newTag);

    QExplicitlySharedDataPointer<T> retrieveObject(QStringList tag) const;
    QExplicitlySharedDataPointer<T> retrieveObject(const KstObjectTag& tag) const;
    bool tagExists(const QString& tag) const;
    bool tagExists(const KstObjectTag& tag) const;

    // get the shortest unique tag in in_tag
    KstObjectTag shortestUniqueTag(const KstObjectTag& in_tag) const;
    // get the minimum number of tag components of in_tag necessary for a unique tag
    unsigned int componentsForUniqueTag(const KstObjectTag& in_tag) const;

    void setUpdateDisplayTags(bool u);

    KstObjectTreeNode<T> *nameTreeRoot() { return &_root; }
    bool isEmpty() const { return _list.isEmpty(); }
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::size_type count() const { return _list.count(); }
    void append(T *o);
    void remove(T *o);
    void clear();
    QStringList tagNames() const;
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator removeTag(const QString& x);
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator findTag(const QString& x);
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator findTag(const QString& x) const;
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator findTag(const KstObjectTag& x);
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator findTag(const KstObjectTag& x) const;
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator begin();
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator begin() const;
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator end();
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator end() const;
    KstObjectList<QExplicitlySharedDataPointer<T> >& list() { return _list; } // FIXME: this should be const, but it will break KstObjectSubList
    QExplicitlySharedDataPointer<T>& operator[](int i) { return _list[i]; }
    const QExplicitlySharedDataPointer<T>& operator[](int i) const { return _list[i]; }
    KstRWLock& lock() const { return _list.lock(); }

  private:
    QLinkedList<KstObjectTreeNode<T> *> relatedNodes(T *obj);
    void relatedNodesHelper(T *o, KstObjectTreeNode<T> *n, QHash<long, KstObjectTreeNode<T>*>& nodes);

    // must be called AFTER the object is added to the index, while holding a write lock
    void updateAllDisplayTags();
    void updateDisplayTag(T *obj);
    void updateDisplayTags(QLinkedList<KstObjectTreeNode<T> *> nodes);

    bool _updateDisplayTags;

    KstObjectTreeNode<T> _root;
    KstObjectNameIndex<T> _index;
    KstObjectList<QExplicitlySharedDataPointer<T> > _list; // owns the objects
};


/******************************************************************************/

// FIXME: this should probably return either a const list or another KstObjectCollection
template<class T, class S>
const KstObjectList<QExplicitlySharedDataPointer<S> > kstObjectSubList(KstObjectCollection<T>& coll) {
  KstObjectList<QExplicitlySharedDataPointer<T> > list = coll.list();
  KstObjectList<QExplicitlySharedDataPointer<S> > rc;
  typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;
  
  list.lock().readLock();

  for (it = list.begin(); it != list.end(); ++it) {
    S *x = dynamic_cast<S*>((*it).data());

    if (x != 0L) {
      QExplicitlySharedDataPointer<S> item(x);
      
      rc.append(item);
    }
  }

  list.lock().unlock();

  return rc;
}

/******************************************************************************/


template <class T>
KstObjectTreeNode<T>::KstObjectTreeNode(const QString& tag) : 
  _tag(tag), _object(NULL), _parent(NULL) {
}


template <class T>
KstObjectTreeNode<T>::~KstObjectTreeNode() {
  for (typename QMap<QString, KstObjectTreeNode<T>*>::iterator i = _children.begin(); i != _children.end(); ++i) {
    delete(i.value());
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
        if (index->contains(*i)) {
          QLinkedList<KstObjectTreeNode<T> *> l;

          l = index->take(*i);
          l.append(nextNode);
          index->insert(*i, l);
        } else {
          QLinkedList<KstObjectTreeNode<T> *>* pl;

          pl = new QLinkedList<KstObjectTreeNode<T> *>;
          pl->append(nextNode);
          index->insert(*i, *pl);
        }
      }
    }
    currNode = nextNode;
  }

  if (currNode->_object) {

    return NULL;
  } else {
    currNode->_object = o;

    return currNode;
  }
}


template <class T>
bool KstObjectTreeNode<T>::removeDescendant(T *obj, KstObjectNameIndex<T> *index) {
  if (!obj) {
    return false;
  }

  QStringList tag = obj->tag().fullTag();

  KstObjectTreeNode<T> *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    KstObjectTreeNode<T> *nextNode = currNode->child(*i);
    if (!nextNode) {
      return false;
    }
    currNode = nextNode;
  }

  if (currNode->_object != QPointer<KstObject>(obj)) {
    return false;
  } else {
    currNode->_object = NULL;
    QStringList::ConstIterator i = tag.end();
    while (i != tag.begin() && currNode->_object.isNull() && currNode->_children.isEmpty()) {
      --i;
      KstObjectTreeNode<T> *lastNode = currNode->_parent;

      lastNode->_children.remove(*i);

      if (index) {
        if (index->contains(*i)) {
          QLinkedList<KstObjectTreeNode<T> *> l = index->take(*i);

          l.removeAll(currNode);
          index->insert(*i, l);
        }
      }

      delete currNode;
      currNode = lastNode;
    }

    return true;
  }
}

template <class T>
void KstObjectTreeNode<T>::clear() {
  QMapIterator<QString, KstObjectTreeNode*> i;

  _tag = QString::null;
  _parent = NULL;
  _object = NULL;

  //
  // delete children
  //

  for (i = _children.begin(); i != _children.end(); ++i) {
    delete (i);
  }
  
  _children.clear();
}


/******************************************************************************/


template <class T>
KstObjectCollection<T>::KstObjectCollection() : _updateDisplayTags(true)
{
}


template <class T>
bool KstObjectCollection<T>::addObject(T *o) {
  if (!o) {
    return false;
  }

  QExplicitlySharedDataPointer<T> p(o);
  QLinkedList<KstObjectTreeNode<T> *> relNodes;
  
  _list.append(p);

  if (_updateDisplayTags) {
    relNodes = relatedNodes(o);
  }

  KstObjectTreeNode<T> *n = _root.addDescendant(o, &_index);

  if (n) {
    if (_updateDisplayTags) {
      updateDisplayTag(o);
      updateDisplayTags(relNodes);
    }
    return true;
  } else {
    // TODO: handle failed insert?
    return false;
  }
}


template <class T>
bool KstObjectCollection<T>::removeObject(T *o) {
  bool ok = false;

  if (o) {
    QExplicitlySharedDataPointer<T> p(o);
    
    if (_list.contains(p)) {
      QLinkedList<KstObjectTreeNode<T> *> relNodes;
      
      if (_updateDisplayTags) {
        relNodes = relatedNodes(o);
      }

      ok = _root.removeDescendant(o, &_index);

      if (ok) {
        if (_updateDisplayTags) {
          updateDisplayTags(relNodes);
        }

        _list.removeAll(p);
      }
    }
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

  QLinkedList<KstObjectTreeNode<T> *> relNodes;
  
  if (_updateDisplayTags) {
    relNodes = relatedNodes(o);
  }

  _root.removeDescendant(o, &_index);

  o->KstObject::setTag(newTag);

  if (_root.addDescendant(o, &_index)) {
    if (_updateDisplayTags) {
      relNodes += relatedNodes(o);  // TODO: remove duplicates
      updateDisplayTag(o);
      updateDisplayTags(relNodes);
    }
  } else {
    // TODO: handle failed insert
  }
}


template <class T>
QExplicitlySharedDataPointer<T> KstObjectCollection<T>::retrieveObject(QStringList tag) const {
  if (tag.isEmpty()) {
    return QExplicitlySharedDataPointer<T>();
  }

  if (_index.contains(tag.first()) && _index.value(tag.first()).count() == 1) {
    KstObjectTreeNode<T> *n = _index.value(tag.first()).first();

    if (n) {
      tag.pop_front();
      n = n->descendant(tag);
    }
    
    if (n) {
      return QExplicitlySharedDataPointer<T>(n->object());
    }
  }

  //
  // search through the tree...
  //
  
  const KstObjectTreeNode<T> *n = _root.descendant(tag);
  
  if (n) {
    return QExplicitlySharedDataPointer<T>(n->object());
  } else {
    return QExplicitlySharedDataPointer<T>();
  }
}


template <class T>
QExplicitlySharedDataPointer<T> KstObjectCollection<T>::retrieveObject(const KstObjectTag& tag) const {
  if (!tag.isValid()) {
    return QExplicitlySharedDataPointer<T>();
  }

  return retrieveObject(tag.fullTag());
}

template <class T>
bool KstObjectCollection<T>::tagExists(const QString& tag) const {
  return (_index.contains(tag) && _index.value(tag).count() > 0);
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
    if (_index.contains(*it) && _index.value(*it).count() == 1) {
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
    if (_index.contains(*it) && _index.value(*it).count() == 1) {
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
  _root.clear();
  
  //
  // need to manually delete the contents of _index,
  //  as setAutoDelete() is no longer supported...
  //
  typename QMultiHash<QString, QLinkedList<KstObjectTreeNode<T> *> >::iterator it;

  for (it= _index.begin(); it != _index.end(); ++it) {
    delete(it);
  }
  
  _index.clear();

  _list.clear();
}

template <class T>
QStringList KstObjectCollection<T>::tagNames() const {
  return _list.tagNames();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator KstObjectCollection<T>::removeTag(const QString& x) {
  //
  // find object in tree
  //

  T *obj = retrieveObject(KstObjectTag::fromString(x));

  if (obj) {
    //
    // remove object from tree
    //
    
    _root.removeDescendant(obj, &_index);

    //
    // remove object from list
    //
    
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it = _list.find(obj);

    if (it != _list.end()) {
      return _list.remove(it);
    }
  }
  return _list.end();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator KstObjectCollection<T>::findTag(const KstObjectTag& x) {
  T *obj;
  
  obj = retrieveObject(x).data();
  if (obj) {
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;
    
    for (it=_list.begin(); it!=_list.end(); ++it) {
      if ((*it).data() == obj) {
        return it;
      }
    }
  } else {
    //
    // for historical compatibility:
    // previously, output vectors of equations, PSDs, etc. 
    //  were named PSD1-ABCDE-freq now, they are PSD1-ABCDE/freq
    //
    QString newTag = x.tagString();

    newTag.replace(newTag.lastIndexOf('-'), 1, KstObjectTag::tagSeparator);
    obj = retrieveObject(KstObjectTag::fromString(newTag)).data();
    if (obj) {
      typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;
      
      for (it=_list.begin(); it!=_list.end(); ++it) {
        if ((*it).data() == obj) {
          return it;
        }
      }
    }
  }
  
  return _list.end();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator KstObjectCollection<T>::findTag(const QString& x) {
  return findTag(KstObjectTag::fromString(x));
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator KstObjectCollection<T>::findTag(const KstObjectTag& x) const {
  T *obj;
  
  obj = retrieveObject(x).data();
  if (obj) {
    typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator it;
    
    for (it=_list.begin(); it!=_list.end(); ++it) {
      if ((*it).data() == obj) {
        return it;
      }
    }
  } else {
    // For historical compatibility:
    // previously, output vectors of equations, PSDs, etc. were named PSD1-ABCDE-freq
    // now, they are PSD1-ABCDE/freq
    QString newTag = x.tagString();
    
    newTag.replace(newTag.lastIndexOf('-'), 1, KstObjectTag::tagSeparator);
    obj = retrieveObject(KstObjectTag::fromString(newTag));
    if (obj) {
      return _list.find(obj);
    }
  }
  return _list.end();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator KstObjectCollection<T>::findTag(const QString& x) const {
  return findTag(KstObjectTag::fromString(x));
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator KstObjectCollection<T>::begin() {
  return _list.begin();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator KstObjectCollection<T>::begin() const {
  return _list.begin();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::Iterator KstObjectCollection<T>::end() {
  return _list.end();
}

template <class T>
typename KstObjectList<QExplicitlySharedDataPointer<T> >::ConstIterator KstObjectCollection<T>::end() const {
  return _list.end();
}

template <class T>
void KstObjectCollection<T>::setUpdateDisplayTags(bool u) {
  if (u && !_updateDisplayTags) {
    //
    // turning on _updateDisplayTags, so do an update
    //
    
    updateAllDisplayTags();
  }

  _updateDisplayTags = u;
}

// update the display tags for all the objects in the collection
template <class T>
void KstObjectCollection<T>::updateAllDisplayTags() {
  Q_ASSERT(lock().myLockStatus() == KstRWLock::WRITELOCKED);

  for (typename KstObjectList<QExplicitlySharedDataPointer<T> >::iterator i = _list.begin(); i != _list.end(); ++i) {
    updateDisplayTag(i->data());
  }
}

// must be called AFTER the object is added to the index, while holding a write lock
template <class T>
void KstObjectCollection<T>::updateDisplayTag(T *obj) {
  if (!obj) {
    return;
  }

  KstObjectTag tag = obj->tag();

  if (!_index.contains(tag.tag())) {
    return;
  }

  unsigned int nc = componentsForUniqueTag(tag);
  if (tag.uniqueDisplayComponents() != nc) {
    obj->tag().setUniqueDisplayComponents(nc);
  }
}

template <class T>
void KstObjectCollection<T>::updateDisplayTags(QLinkedList<KstObjectTreeNode<T> *> nodes) {
  for (typename QLinkedList<KstObjectTreeNode<T> *>::Iterator i = nodes.begin(); i != nodes.end(); ++i) {
    updateDisplayTag((*i)->object());
  }
}


template <class T>
void KstObjectCollection<T>::relatedNodesHelper(T *o, KstObjectTreeNode<T> *n, QHash<long, KstObjectTreeNode<T>* >& nodes) {
  if (n->object()) {
    if (n->object() != o) {
      if (!nodes.contains((int)n)) {
        nodes.insert((int)n, n);
      }
    }
  }

  if (!n->children().isEmpty()) {
    //
    // non-leaf node, so recurse
    //

    QMap<QString, KstObjectTreeNode<T> *> children = n->children();
    typename QMap<QString, KstObjectTreeNode<T> *>::ConstIterator i;

    for (i = children.begin(); i != children.end(); ++i) {
      relatedNodesHelper(o, *i, nodes);
    }
  }
}

// Find the nodes with KstObjects which are affected by the addition or removal
// of an object with the given tag.
//
// There should not be any duplicates in the returned list.
template <class T>
QLinkedList<KstObjectTreeNode<T> *> KstObjectCollection<T>::relatedNodes(T *o) {
  QHash<long, KstObjectTreeNode<T>*> nodes;
  QLinkedList<KstObjectTreeNode<T>*> outNodes;

  if (!o) {
    return outNodes;
  }

  QStringList ft = o->tag().fullTag();
  QStringList::ConstIterator i;
  typename QHash<long, KstObjectTreeNode<T>*>::iterator it;
  
  for (i = ft.begin(); i != ft.end(); ++i) {
    if (_index.contains(*i)) {
      typename QLinkedList<KstObjectTreeNode<T> *>::ConstIterator i2;
      QLinkedList<KstObjectTreeNode<T> *> nodeList = _index.value(*i);

      for (i2 = nodeList.begin(); i2 != nodeList.end(); ++i2) {
        relatedNodesHelper(o, *i2, nodes);
      }
    }
  }

  for (it=nodes.begin(); it!=nodes.end(); ++it) {
    outNodes << *it;
  }

  return outNodes;
}

#endif
