/***************************************************************************
                  kstdataobject.cpp: base class for data objects
                             -------------------
    begin                : May 20, 2003
    copyright            : (C) 2003 by C. Barth Netterfield
                           (C) 2003 The University of Toronto
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

#include <assert.h>

#include <QTimer>

#include <klibloader.h>
#include <kparts/componentfactory.h>

#include "kstdataobject.h"
#include "kstdataplugin.h"
#include "kstdebug.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"

KstDataObject::KstDataObject() : KstObject() {
  _curveHints = new KstCurveHintList;
  _isInputLoaded = false;
  _isRecursed = false;
}

KstDataObject::KstDataObject(const QDomElement& e) : KstObject() {
  Q_UNUSED(e)
  _curveHints = new KstCurveHintList;
  _isInputLoaded = false;
  _isRecursed = false;
}


KstDataObject::~KstDataObject() {
  {
    KstStringMap::Iterator it;
    
    KST::stringList.lock().writeLock();
    for (it = _outputStrings.begin(); it != _outputStrings.end(); ++it) {
      KST::stringList.remove((*it).data());
    }
    KST::stringList.lock().unlock();
  }
  
  {
    KstScalarMap::Iterator it;
    
    KST::scalarList.lock().writeLock();
    for (it = _outputScalars.begin(); it != _outputScalars.end(); ++it) {
      KST::scalarList.remove((*it).data());
    }
    KST::scalarList.lock().unlock();
  }
  
  {
    KstVectorMap::Iterator it;
    
    KST::vectorList.lock().writeLock();
    for (it = _outputVectors.begin(); it != _outputVectors.end(); ++it) {
      KST::vectorList.remove((*it).data());
    }
    KST::vectorList.lock().unlock();
  }
  
  {
    KstMatrixMap::Iterator it;
    
    KST::matrixList.lock().writeLock();
    for (it = _outputMatrices.begin(); it != _outputMatrices.end(); ++it) {
      KST::matrixList.remove((*it).data());
    }
    KST::matrixList.lock().unlock();
  }
  
  delete _curveHints;
}

static QMap<QString, KstDataObjectPtr> pluginInfo;
void KstDataObject::cleanupForExit() {
  pluginInfo.clear();
}

/* xxx
KstDataObjectPtr KstDataObject::createPlugin(KService::Ptr service) {
  int err = 0;
  KstDataObject *object =
      KParts::ComponentFactory::createInstanceFromService<KstDataObject>(service, 0, "",
      QStringList(), &err);

  QExplicitlySharedDataPointer<KST::Plugin> p = new KST::DataObjectPlugin(service);

  if (object && p->key()) {
    const QString name = service->property("Name").toString();
    const QString description = service->property("Comment").toString();
    const QString author = service->property("X-Kst-Plugin-Author").toString();
    const QString version = service->property("X-Kst-Plugin-Version").toString();
    const QString library = service->library();
    Q_ASSERT( !name.isEmpty() );
    Q_ASSERT( !library.isEmpty() );
    object->setName(name);
    object->setAuthor(author);
    object->setDescription(description);
    object->setVersion(version);
    object->setLibrary(library);

    KstDebug::self()->log(QObject::tr("Loaded data-object plugin %1.").arg(service->name()));
    return object;
  }

  KstDebug::self()->log(QObject::tr("Could not load data-object plugin %1.").arg(service->name()), KstDebug::Error);

  return KstDataObjectPtr( );
}
*/

// Scans for plugins and stores the information for them
void KstDataObject::scanPlugins() {
  KstDebug::self()->log(QObject::tr("Scanning for data-object plugins."));

  pluginInfo.clear();
/* xxx
  KService::List sl = KServiceType::offers("Kst Data Object");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    if (KstDataObjectPtr object = createPlugin(*it)) {
      pluginInfo.insert((*it)->name(), KstDataObjectPtr(object));
    }
  }
*/
}


KstPluginInfoList KstDataObject::pluginInfoList() {
  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }

  KstPluginInfoList list;

  QMap<QString, KstDataObjectPtr>::ConstIterator it = pluginInfo.begin();
  for (; it != pluginInfo.end(); ++it) {
    list.insert(it.key(), (*it)->kind());
  }

  return list;
}


KstDataObjectPtr KstDataObject::plugin(const QString& name) {
  if (pluginInfo.contains(name)) {
    return pluginInfo[name];
  }

  return KstDataObjectPtr();
}


KstDataObjectPtr KstDataObject::createPlugin(const QString& name) {
/* xxx
  KService::List sl = KServiceType::offers("Kst Data Object");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    if ((*it)->name() != name) {
      continue;
    } else if (KstDataObjectPtr object = createPlugin(*it)) {
      return object;
    }
  }
*/
  return KstDataObjectPtr();
}


double *KstDataObject::vectorRealloced(KstVectorPtr v, double *memptr, int newSize) const {
  if (!v) {
    return 0L;
  }

  // One would think this needs special locking, but it results in deadlock
  // in complicated object hierarchies such as filtered vectors.  Therefore if
  // you call vectorRealloced() and v is not locked by you already, you'd
  // better lock it!
  return v->realloced(memptr, newSize);
}


void KstDataObject::load(const QDomElement &e) {
  Q_UNUSED(e)
}


void KstDataObject::save(QTextStream& ts, const QString& indent) {
  Q_UNUSED(ts)
  Q_UNUSED(indent)
}


bool KstDataObject::loadInputs() {
  QLinkedList<QPair<QString,QString> >::Iterator i;
  bool rc = true;

  KST::vectorList.lock().readLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {    
    KstVectorList::iterator it = KST::vectorList.findTag((*i).second);

    if (it != KST::vectorList.end()) {
      _inputVectors.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(QObject::tr("Unable to find required vector [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::vectorList.lock().unlock();

  KST::scalarList.lock().readLock();
  for (i = _inputScalarLoadQueue.begin(); i != _inputScalarLoadQueue.end(); ++i) {
    KstScalarList::Iterator it = KST::scalarList.findTag((*i).second);
    
    if (it != KST::scalarList.end()) {
      _inputScalars.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(QObject::tr("Unable to find required scalar [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::scalarList.lock().unlock();

  KST::stringList.lock().readLock();
  for (i = _inputStringLoadQueue.begin(); i != _inputStringLoadQueue.end(); ++i) {
    KstStringList::Iterator it = KST::stringList.findTag((*i).second);
    
    if (it != KST::stringList.end()) {
      _inputStrings.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(QObject::tr("Unable to find required string [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::stringList.lock().unlock();
  
  KST::matrixList.lock().readLock();
  for (i = _inputMatrixLoadQueue.begin(); i != _inputMatrixLoadQueue.end(); ++i) {
    KstMatrixList::Iterator it = KST::matrixList.findTag((*i).second);
    
    if (it != KST::matrixList.end()) {
      _inputMatrices.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(QObject::tr("Unable to find required matrix [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::matrixList.lock().unlock();

  _inputVectorLoadQueue.clear();
  _inputScalarLoadQueue.clear();
  _inputStringLoadQueue.clear();
  _inputMatrixLoadQueue.clear();

  setDirty();

  _isInputLoaded = true;

  return rc;
}


int KstDataObject::getUsage() const {
  int rc = 0;

  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    if (*i) {
      rc += (*i)->getUsage() - 1;
    }
  }

  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    if (*i) {
      rc += (*i)->getUsage() - 1;
    }
  }

  for (KstStringMap::ConstIterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    if (*i) {
      rc += (*i)->getUsage() - 1;
    }
  }

  for (KstMatrixMap::ConstIterator i = _outputMatrices.begin(); i != _outputMatrices.end(); ++i) {
    if (*i) {
      rc += (*i)->getUsage() - 1;  
    }
  }

  return KstObject::getUsage() + rc;
}


void KstDataObject::showDialog(bool isNew) {
  if (isNew) {
    QTimer::singleShot(0, this, SLOT(showNewDialog()));
  } else {
    QTimer::singleShot(0, this, SLOT(showEditDialog()));
  }
}


void KstDataObject::readLock() const {
  KstObject::readLock();
}


void KstDataObject::writeLock() const {
  KstObject::writeLock();
}


void KstDataObject::unlock() const {
  KstObject::unlock();
}


void KstDataObject::writeLockInputsAndOutputs() const {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);
/* xxx
  QList<KstPrimitivePtr> inputs;
  QList<KstPrimitivePtr> outputs;
  QList<KstStringPtr> sl;
  QList<KstScalarPtr> sc;
  QList<KstVectorPtr> vl;
  QList<KstMatrixPtr> ml;

  sl = _inputStrings.values();
  for (QList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    inputs += (*i).data();
  }
  sl = _outputStrings.values();
  for (QList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    outputs += (*i).data();
  }

  sc = _inputScalars.values();
  for (QList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    inputs += (*i).data();
  }
  sc = _outputScalars.values();
  for (QList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    outputs += (*i).data();
  }

  vl = _inputVectors.values();
  for (QList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    inputs += (*i).data();
  }
  vl = _outputVectors.values();
  for (QList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    outputs += (*i).data();
  }

  ml = _inputMatrices.values();
  for (QList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    inputs += (*i).data();
  }
  ml = _outputMatrices.values();
  for (QList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    outputs += (*i).data();
  }

  qSort(inputs);
  qSort(outputs);

  QList<KstPrimitivePtr>::ConstIterator inputIt = inputs.begin();
  QList<KstPrimitivePtr>::ConstIterator outputIt = outputs.begin();

  while (inputIt != inputs.end() || outputIt != outputs.end()) {
    if (inputIt != inputs.end() && (outputIt == outputs.end() || (void*)(*inputIt) < (void*)(*outputIt))) {
      // do input
      if (!(*inputIt)) {
// xxx        kstdFatal() << "Input for data object " << this->tag().displayString() << " is invalid." << endl;
      }
      (*inputIt)->writeLock();
      ++inputIt;
    } else {
      // do output
      if (!(*outputIt)) {
// xxx        kstdFatal() << "Output for data object " << this->tag().displayString() << " is invalid." << endl;
      }
      if ((*outputIt)->provider() != this) {
        KstDebug::self()->log(QObject::tr("(%1) KstDataObject::writeLockInputsAndOutputs() by tid=%2: write locking output %3 (not provider) -- this is probably an error. Please email kst@kde.org with details.").arg(this->type()).arg((int)QThread::currentThread()).arg((*outputIt)->tagName()), KstDebug::Error);
      }
      (*outputIt)->writeLock();
      ++outputIt;
    }
  }
*/
}


void KstDataObject::unlockInputsAndOutputs() const {
  for (KstMatrixMap::ConstIterator i = _outputMatrices.begin(); i != _outputMatrices.end(); ++i) {
    (*i)->unlock();
  }

  for (KstMatrixMap::ConstIterator i = _inputMatrices.begin(); i != _inputMatrices.end(); ++i) {
    (*i)->unlock();
  }

  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->unlock();
  }

  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->unlock();
  }

  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->unlock();
  }

  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->unlock();
  }

  for (KstStringMap::ConstIterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    (*i)->unlock();
  }

  for (KstStringMap::ConstIterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    (*i)->unlock();
  }
}


bool KstDataObject::isValid() const {
  return true;
}


const KstCurveHintList* KstDataObject::curveHints() const {
  return _curveHints;
}


bool KstDataObject::deleteDependents() {
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = KST::dataObjectList;
  KST::dataObjectList.lock().unlock();
  
  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
    bool user = (*i)->uses(KstObjectPtr(this));
    
    if (!user) {
      for (KstVectorMap::Iterator j = _outputVectors.begin(); !user && j != _outputVectors.end(); ++j) {
        user = (*i)->uses(KstObjectPtr((*j).data()));
      }
      for (KstScalarMap::Iterator j = _outputScalars.begin(); !user && j != _outputScalars.end(); ++j) {
        user = (*i)->uses(KstObjectPtr((*j).data()));
      }
      for (KstStringMap::Iterator j = _outputStrings.begin(); !user && j != _outputStrings.end(); ++j) {
        user = (*i)->uses(KstObjectPtr((*j).data()));
      }
    }

    if (user) {
      KstDataObjectPtr dop = *i;
      
      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.erase(i);
      KST::dataObjectList.lock().unlock();

      dop->deleteDependents();
    }
  }

  return true;
}


bool KstDataObject::duplicateDependents(QMap<KstDataObjectPtr, KstDataObjectPtr> &duplicatedMap) {
  //
  // work with a copy of the data object list
  //
  
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = KST::dataObjectList;
  KST::dataObjectList.lock().unlock();

  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) { 
    if ((*i)->uses(KstObjectPtr(this))) {
      if (duplicatedMap.contains(*i)) {
        (duplicatedMap[*i])->replaceDependency(KstDataObjectPtr(this), duplicatedMap.value(KstDataObjectPtr(this)));
      } else {
        KstDataObjectPtr newObject = (*i)->makeDuplicate(duplicatedMap);

        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(newObject);
        KST::dataObjectList.lock().unlock();
        
        (duplicatedMap[*i])->replaceDependency(KstDataObjectPtr(this), duplicatedMap.value(KstDataObjectPtr(this)));
        (*i)->duplicateDependents(duplicatedMap);
      }
    }
  }

  return true;
}


void KstDataObject::replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject) {
  //
  // find all connections from this object to old object
  //

  for (KstVectorMap::Iterator j = oldObject->outputVectors().begin(); j != oldObject->outputVectors().end(); ++j) {
    for (KstVectorMap::Iterator k = _inputVectors.begin(); k != _inputVectors.end(); ++k) {
      if (*j == *k) {
        // replace input with the output from newObject
        _inputVectors[k.key()] = (newObject->outputVectors())[j.key()]; 
      }
    }
    
    // also replace dependencies on vector stats
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      for (ScalarCollection::const_iterator l = (*j)->scalars().begin(); l != (*j)->scalars().end(); ++l) {
        if (*l == *k) {
          _inputScalars[k.key()] = (((newObject->outputVectors())[j.key()])->scalars())[l.key()];
        }
      }
    }
  }

  for (KstMatrixMap::Iterator j = oldObject->outputMatrices().begin(); j != oldObject->outputMatrices().end(); ++j) {
    for (KstMatrixMap::Iterator k = _inputMatrices.begin(); k != _inputMatrices.end(); ++k) {
      if (*j == *k) {
        // replace input with the output from newObject
        _inputMatrices[k.key()] = (newObject->outputMatrices())[j.key()]; 
      }
    }
    
    // also replace dependencies on matrix stats
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      for (ScalarCollection::const_iterator l=(*j)->scalars().begin(); l != (*j)->scalars().end(); ++l) {
        if (*l == *k) {
          _inputScalars[k.key()] = (((newObject->outputMatrices())[j.key()])->scalars())[l.key()];
        }
      }
    }
  }

  for (KstScalarMap::Iterator j = oldObject->outputScalars().begin(); j != oldObject->outputScalars().end(); ++j) {
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      if (*j == *k) {
        // replace input with the output from newObject
        _inputScalars[k.key()] = (newObject->outputScalars())[j.key()];
      }
    }
  }

  for (KstStringMap::Iterator j = oldObject->outputStrings().begin(); j != oldObject->outputStrings().end(); ++j) {
    for (KstStringMap::Iterator k = _inputStrings.begin(); k != _inputStrings.end(); ++k) {
      if (*j == *k) {
        // replace input with the output from newObject
        _inputStrings[k.key()] = (newObject->outputStrings())[j.key()];
      }
    }
  }
}


void KstDataObject::replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector) {
  for (KstVectorMap::Iterator j = _inputVectors.begin(); j != _inputVectors.end(); ++j) {
    if (*j == oldVector) {
      _inputVectors[j.key()] = newVector;
    }
  }

  for (KstScalarMap::Iterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
    for (ScalarCollection::const_iterator l = oldVector->scalars().begin(); l != oldVector->scalars().end(); ++l) {
      if (*l == *j) {
        _inputScalars[j.key()] = (newVector->scalars())[l.key()];
      }
    }
  }
}


void KstDataObject::replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix) {
  KstMatrixMap::Iterator j;

  for (j = _inputMatrices.begin(); j != _inputMatrices.end(); ++j) {
    if (*j == oldMatrix) {
      _inputMatrices[j.key()] = newMatrix;
    }
  }

  ScalarCollection::const_iterator it;
  
  for (KstScalarMap::Iterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
    for (it =oldMatrix->scalars().begin(); it != oldMatrix->scalars().end(); ++it) {
      if ((*it) == (*j)) {
        _inputScalars[j.key()] = (newMatrix->scalars())[it.key()];
      }
    }
  }
}


bool KstDataObject::uses(KstObjectPtr p) const {
  KstVectorPtr v(kst_cast<KstVector>(p));
  KstMatrixPtr m(kst_cast<KstMatrix>(p));
  KstDataObjectPtr o(kst_cast<KstDataObject>(p));
  ScalarCollection::const_iterator it;
  
  if (v) {
    for (KstVectorMap::ConstIterator j = _inputVectors.begin(); j != _inputVectors.end(); ++j) {
      if (*j == v) {
        return true;
      }
    }
      
    for (KstScalarMap::ConstIterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
      for (it = v->scalars().begin(); it != v->scalars().end(); ++it) {
        if (*it == *j) {
          return true;
        }
      }
    }
  } else if (m) {
    for (KstMatrixMap::ConstIterator j = _inputMatrices.begin(); j != _inputMatrices.end(); ++j) {
      if (*j == m) {
        return true;
      }
    }

    for (KstScalarMap::ConstIterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
      for (it = m->scalars().begin(); it != m->scalars().end(); ++it) {
        if (*it != *j) {
          return true;
        }
      }
    }
  } else if (o) {
    // check all connections from this object to p
    for (KstVectorMap::Iterator j = o->outputVectors().begin(); j != o->outputVectors().end(); ++j) {
      for (KstVectorMap::ConstIterator k = _inputVectors.begin(); k != _inputVectors.end(); ++k) {
        if (*j == *k) {
          return true;
        }
      }

      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        for (it = (*j)->scalars().begin(); it != (*j)->scalars().end(); ++it) {
          if (*it == *k) {
            return true;
          }
        }
      }
    }

    for (KstMatrixMap::Iterator j = o->outputMatrices().begin(); j != o->outputMatrices().end(); ++j) {
      for (KstMatrixMap::ConstIterator k = _inputMatrices.begin(); k != _inputMatrices.end(); ++k) {
        if (*j == *k) {
          return true;
        }
      }
      
      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        for (it = (*j)->scalars().begin(); it != (*j)->scalars().end(); ++it) {
          if (*it == *k) {
            return true;
          }
        }
      }
    }

    for (KstScalarMap::Iterator j = o->outputScalars().begin(); j != o->outputScalars().end(); ++j) {
      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        if (*j == *k) {
          return true;
        }
      }
    }

    for (KstStringMap::Iterator j = o->outputStrings().begin(); j != o->outputStrings().end(); ++j) {
      for (KstStringMap::ConstIterator k = _inputStrings.begin(); k != _inputStrings.end(); ++k) {
        if (*j == *k) {
          return true;
        }
      }
    }
  }
  
  return false;
}


bool KstDataObject::recursion(KstDataObjectDataObjectMap& objectsInUse) {
  bool recurses = false;

  objectsInUse.insert(KstDataObjectPtr(this), KstDataObjectPtr(this));

  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    if ((*it)->uses(KstDataObjectPtr(this))) {
      if (objectsInUse.find(*it) == objectsInUse.end()) {
        if ((*it)->recursion(objectsInUse)) {
          recurses = true;

          break;
        }
      } else {
        recurses = true;

        break;
      }
    }
  }

  objectsInUse.remove(KstDataObjectPtr(this));

  return recurses;
}


bool KstDataObject::recursion() {
  KstDataObjectDataObjectMap objectsInUse;
  bool recurses = false;

  recurses = recursion(objectsInUse);

  return recurses;
}


void KstDataObject::setRecursed( bool isRecursed ) {
  _isRecursed = isRecursed;
}


bool KstDataObject::recursed() const {
  return _isRecursed;
}

