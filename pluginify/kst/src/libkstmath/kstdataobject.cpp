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

#include "kstvcurve.h"

#include "kstdataobjectcollection.h"
#include <kstdatacollection.h>
#include "kstdataobject.h"
#include "kstdebug.h"

#include <qdeepcopy.h>
#include <qtimer.h>

#include <assert.h>

#include <kdebug.h>

#include <klibloader.h>
#include <klocale.h>
#include <kservicetype.h>
#include <kstdataobject.h>
#include <kparts/componentfactory.h>

#include "ksdebug.h"
#include <klocale.h>

//#define LOCKTRACE

KstDataObject::KstDataObject() : KstObject() {
  //kstdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
  _curveHints = new KstCurveHintList;
}

KstDataObject::KstDataObject(const QDomElement& e) : KstObject() {
  Q_UNUSED(e)
  //kstdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
  _curveHints = new KstCurveHintList;
}


KstDataObject::~KstDataObject() {
  // Remove our slave vectors, scalars, and strings, and matrices
  KST::stringList.lock().writeLock();
  for (KstStringMap::Iterator it = _outputStrings.begin();
                               it != _outputStrings.end();
                                                      ++it) {
    KST::stringList.remove(it.data());
  }
  KST::stringList.lock().unlock();

  KST::scalarList.lock().writeLock();
  for (KstScalarMap::Iterator it = _outputScalars.begin();
                               it != _outputScalars.end();
                                                      ++it) {
    KST::scalarList.remove(it.data());
  }
  KST::scalarList.lock().unlock();

  KST::vectorList.lock().writeLock();
  for (KstVectorMap::Iterator it = _outputVectors.begin();
                               it != _outputVectors.end();
                                                      ++it) {
    KST::vectorList.remove(it.data());
  }
  KST::vectorList.lock().unlock();
  
  KST::matrixList.lock().writeLock();
  for (KstMatrixMap::Iterator it = _outputMatrices.begin();
       it != _outputMatrices.end();
       ++it) {
    KST::matrixList.remove(it.data());       
  }
  KST::matrixList.lock().unlock();
  //kstdDebug() << "+++ DESTROYING DATA OBJECT: " << (void*)this << endl;
  delete _curveHints;
}

static QMap<QString, KstDataObjectPtr> pluginInfo;
void KstDataObject::cleanupForExit() {
  pluginInfo.clear(); //FIXME?
}

// Scans for plugins and stores the information for them
static void scanPlugins() {

  KstDebug::self()->log(i18n("Scanning for data-object plugins."));

  pluginInfo.clear(); //FIXME?

  KService::List sl = KServiceType::offers("Kst Data Object");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    int err = 0;
    KService::Ptr service = ( *it );
    KstDataObject *object =
        KParts::ComponentFactory::createInstanceFromService<KstDataObject>( service, 0, "",
                                                                            QStringList(), &err );
    if ( object ) {
      pluginInfo.insert( service->name(), KstDataObjectPtr( object ) );
      kdDebug() << "SUCCESS! " << service->name() << endl;
    }
    else
    {
      kdDebug() << "FAILURE! " << k_funcinfo << " " << service->name() << " error=" << err << endl;
      kdDebug() << "KLibLoader::lastErrorMessage! " << KLibLoader::self()->lastErrorMessage() << endl;
    }
  }
}

QStringList KstDataObject::pluginList() {
  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }
  return pluginInfo.keys();
}

KstDataObjectPtr KstDataObject::plugin( const QString &name )
{
    if ( pluginInfo.contains( name ) )
        return pluginInfo[ name ];
    else
        return 0;
}

KstDataObjectPtr KstDataObject::createPlugin( const QString &name, const QDomElement &e )
{
  KService::List sl = KServiceType::offers("Kst Data Object");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    int err = 0;
    KService::Ptr service = ( *it );
    if ( service->name() != name )
      continue;
    KstDataObject *object =
        KParts::ComponentFactory::createInstanceFromService<KstDataObject>( service, 0, "",
        QStringList(), &err );
    object->load( e );
    if ( object ) {
      pluginInfo.insert( service->name(), KstDataObjectPtr( object ) );
      kdDebug() << "SUCCESS! " << service->name() << endl;
    }
    else
    {
      kdDebug() << "FAILURE! " << k_funcinfo << " " << service->name() << " error=" << err << endl;
      kdDebug() << "KLibLoader::lastErrorMessage! " << KLibLoader::self()->lastErrorMessage() << endl;
    }
    return object;
  }
  return 0;
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
  bool rc = true;
  QValueList<QPair<QString,QString> >::Iterator i;
  
  KST::vectorList.lock().readLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    KstVectorList::Iterator it = KST::vectorList.findTag((*i).second);
    if (it != KST::vectorList.end()) {
      assert(*it);
      _inputVectors.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(i18n("Unable to find required vector [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
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
      KstDebug::self()->log(i18n("Unable to find required scalar [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
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
      KstDebug::self()->log(i18n("Unable to find required string [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
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
      KstDebug::self()->log(i18n("Unable to find required matrix [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::matrixList.lock().unlock();

  _inputVectorLoadQueue.clear();
  _inputScalarLoadQueue.clear();
  _inputStringLoadQueue.clear();
  _inputMatrixLoadQueue.clear();
  
  setDirty();

  return rc;
}

int KstDataObject::getUsage() const {
  int rc = 0;

  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;
    }
  }

  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;
    }
  }

  for (KstStringMap::ConstIterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;
    }
  }
  
  for (KstMatrixMap::ConstIterator i = _outputMatrices.begin(); i != _outputMatrices.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;  
    }  
  }

  return KstObject::getUsage() + rc;
}


void KstDataObject::showDialog() {
  QTimer::singleShot(0, this, SLOT(_showDialog()));
}


void KstDataObject::readLock() const {
  #ifdef LOCKTRACE
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << endl;
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << ": read locking myself" << endl;
  #endif
  KstObject::readLock();

  QValueList<KstPrimitivePtr> inputs;
  QValueList<KstPrimitivePtr> outputs;

  QValueList<KstStringPtr> sl = _inputStrings.values();
  for (QValueList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    inputs += (*i).data();
  }
  sl = _outputStrings.values();
  for (QValueList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstScalarPtr> sc = _inputScalars.values();
  for (QValueList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    inputs += (*i).data();
  }
  sc = _outputScalars.values();
  for (QValueList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstVectorPtr> vl = _inputVectors.values();
  for (QValueList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    inputs += (*i).data();
  }
  vl = _outputVectors.values();
  for (QValueList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstMatrixPtr> ml = _inputMatrices.values();
  for (QValueList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    inputs += (*i).data();
  }
  ml = _outputMatrices.values();
  for (QValueList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    outputs += (*i).data();
  }

  qHeapSort(inputs);
  qHeapSort(outputs);

  QValueList<KstPrimitivePtr>::ConstIterator inputIt = inputs.begin();
  QValueList<KstPrimitivePtr>::ConstIterator outputIt = outputs.begin();

  while (inputIt != inputs.end() || outputIt != outputs.end()) {
    if (inputIt != inputs.end() && (outputIt == outputs.end() || (void*)(*inputIt) < (void*)(*outputIt))) {
      // do input
      #ifdef LOCKTRACE
//      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << ": read locking input " << (*inputIt)->type() << " " << (void*)(*i) << endl;
      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << ": read locking input " << (void*)(*inputIt) << endl;
      #endif
      (*inputIt)->readLock();
      ++inputIt;
    } else {
      // do output
      #ifdef LOCKTRACE
//      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << ": read locking output " << (*outputIt)->type() << " " << (void*)(*i) << endl;
      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::readLock() by tid=" << (int)QThread::currentThread() << ": read locking output " << (void*)(*outputIt) << endl;
      #endif
      if ((*outputIt)->provider() != this) {
        KstDebug::self()->log(i18n("(%1) KstDataObject::readLock() by tid=%2: read locking output %3 (not provider) -- this is probably an error. Please email kst@kde.org with details.").arg(this->type()).arg((int)QThread::currentThread()).arg((*outputIt)->tagName()), KstDebug::Error);
      }
      (*outputIt)->readLock();
      ++outputIt;
    }
  }
}


// Note to self, this is nasty.  Some objects may lock while others don't,
// leading to very complex deadlocks or loss of consistency.
// Might want to guard these with another lock yet.
void KstDataObject::writeLock() const {
  #ifdef LOCKTRACE
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << endl;
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << ": write locking myself" << endl;
  #endif
  KstRWLock::writeLock();

  QValueList<KstPrimitivePtr> inputs;
  QValueList<KstPrimitivePtr> outputs;

  QValueList<KstStringPtr> sl = _inputStrings.values();
  for (QValueList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    inputs += (*i).data();
  }
  sl = _outputStrings.values();
  for (QValueList<KstStringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstScalarPtr> sc = _inputScalars.values();
  for (QValueList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    inputs += (*i).data();
  }
  sc = _outputScalars.values();
  for (QValueList<KstScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstVectorPtr> vl = _inputVectors.values();
  for (QValueList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    inputs += (*i).data();
  }
  vl = _outputVectors.values();
  for (QValueList<KstVectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    outputs += (*i).data();
  }
  
  QValueList<KstMatrixPtr> ml = _inputMatrices.values();
  for (QValueList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    inputs += (*i).data();
  }
  ml = _outputMatrices.values();
  for (QValueList<KstMatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    outputs += (*i).data();
  }

  qHeapSort(inputs);
  qHeapSort(outputs);

  QValueList<KstPrimitivePtr>::ConstIterator inputIt = inputs.begin();
  QValueList<KstPrimitivePtr>::ConstIterator outputIt = outputs.begin();

  while (inputIt != inputs.end() || outputIt != outputs.end()) {
    if (inputIt != inputs.end() && (outputIt == outputs.end() || (void*)(*inputIt) < (void*)(*outputIt))) {
      // do input
      #ifdef LOCKTRACE
//      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << ": read locking input " << (*inputIt)->type() << " " << (void*)(*i) << endl;
      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << ": read locking input " << (void*)(*inputIt) << endl;
      #endif
      (*inputIt)->readLock();
      ++inputIt;
    } else {
      // do output
      #ifdef LOCKTRACE
//      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << ": write locking output " << (*outputIt)->type() << " " << (void*)(*i) << endl;
      kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::writeLock() by tid=" << (int)QThread::currentThread() << ": write locking output " << (void*)(*outputIt) << endl;
      #endif
      if ((*outputIt)->provider() != this) {
        KstDebug::self()->log(i18n("(%1) KstDataObject::writeLock() by tid=%2: write locking output %3 (not provider) -- this is probably an error. Please email kst@kde.org with details.").arg(this->type()).arg((int)QThread::currentThread()).arg((*outputIt)->tagName()), KstDebug::Error);
      }
      (*outputIt)->writeLock();
      ++outputIt;
    }
  }
}


void KstDataObject::unlock() const {
  #ifdef LOCKTRACE
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << endl;
  #endif

  for (KstMatrixMap::ConstIterator i = _outputMatrices.begin(); i != _outputMatrices.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking output matrix " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstMatrixMap::ConstIterator i = _inputMatrices.begin(); i != _inputMatrices.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking input matrix " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking output vector " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking input vector " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking output scalar " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking input scalar " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstStringMap::ConstIterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
   #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking output string " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  for (KstStringMap::ConstIterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    #ifdef LOCKTRACE
    kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking input string " << (void*)(*i) << endl;
    #endif
    (*i)->unlock();
  }

  #ifdef LOCKTRACE
  kstdDebug() << (void*)this << " (" << this->type() << ") KstDataObject::unlock() by tid=" << (int)QThread::currentThread() << ": unlocking myself" << endl;
  #endif
  KstRWLock::unlock();
}


bool KstDataObject::isValid() const {
  return true;
}


const KstCurveHintList* KstDataObject::curveHints() const {
  return _curveHints;
}


bool KstDataObject::deleteDependents() {
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
  KST::dataObjectList.lock().unlock();
  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
    bool user = (*i)->uses(this);
    if (!user) {
      for (KstVectorMap::Iterator j = _outputVectors.begin(); !user && j != _outputVectors.end(); ++j) {
        user = (*i)->uses(j.data().data());
      }
      for (KstScalarMap::Iterator j = _outputScalars.begin(); !user && j != _outputScalars.end(); ++j) {
        user = (*i)->uses(j.data().data());
      }
      for (KstStringMap::Iterator j = _outputStrings.begin(); !user && j != _outputStrings.end(); ++j) {
        user = (*i)->uses(j.data().data());
      }
    }
    if (user) {
      KstDataObjectPtr dop = *i;
      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.remove(dop);
      KST::dataObjectList.lock().unlock();
      dop->deleteDependents();
    }
  } 

  return true;
}


bool KstDataObject::duplicateDependents(QMap<KstDataObjectPtr, KstDataObjectPtr> &duplicatedMap) {
  // work with a copy of the data object list
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
  KST::dataObjectList.lock().unlock();
  
  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) { 
    if ((*i)->uses(this)) {
      if (duplicatedMap.contains(*i)) {
        (duplicatedMap[*i])->replaceDependency(this, duplicatedMap[this]);
      } else {
        KstDataObjectPtr newObject = (*i)->makeDuplicate(duplicatedMap);
        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(newObject.data());
        KST::dataObjectList.lock().unlock();
        (duplicatedMap[*i])->replaceDependency(this, duplicatedMap[this]);
        (*i)->duplicateDependents(duplicatedMap);
      }
    }
  }
  return true;
}


void KstDataObject::replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject) {
  
  // find all connections from this object to old object
  
  // vectors
  for (KstVectorMap::Iterator j = oldObject->outputVectors().begin(); j != oldObject->outputVectors().end(); ++j) {
    for (KstVectorMap::Iterator k = _inputVectors.begin(); k != _inputVectors.end(); ++k) {
      if (j.data().data() == k.data().data()) {
        // replace input with the output from newObject
        _inputVectors[k.key()] = (newObject->outputVectors())[j.key()]; 
      }
    }
    // also replace dependencies on vector stats
    QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      for (; scalarDictIter.current(); ++scalarDictIter) {
        if (scalarDictIter.current() == k.data()) {
          _inputScalars[k.key()] = (((newObject->outputVectors())[j.key()])->scalars())[scalarDictIter.currentKey()];
        }  
      }
    }
  }
  
  // matrices
  for (KstMatrixMap::Iterator j = oldObject->outputMatrices().begin(); j != oldObject->outputMatrices().end(); ++j) {
    for (KstMatrixMap::Iterator k = _inputMatrices.begin(); k != _inputMatrices.end(); ++k) {
      if (j.data().data() == k.data().data()) {
        // replace input with the output from newObject
        _inputMatrices[k.key()] = (newObject->outputMatrices())[j.key()]; 
      }
    }
    // also replace dependencies on matrix stats
    QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      for (; scalarDictIter.current(); ++scalarDictIter) {
        if (scalarDictIter.current() == k.data()) {
          _inputScalars[k.key()] = (((newObject->outputMatrices())[j.key()])->scalars())[scalarDictIter.currentKey()];
        }  
      }
    }
  }

  // scalars
  for (KstScalarMap::Iterator j = oldObject->outputScalars().begin(); j != oldObject->outputScalars().end(); ++j) {
    for (KstScalarMap::Iterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
      if (j.data().data() == k.data().data()) {
        // replace input with the output from newObject
        _inputScalars[k.key()] = (newObject->outputScalars())[j.key()];  
      }
    } 
  }
  
  // strings 
  for (KstStringMap::Iterator j = oldObject->outputStrings().begin(); j != oldObject->outputStrings().end(); ++j) {
    for (KstStringMap::Iterator k = _inputStrings.begin(); k != _inputStrings.end(); ++k) {
      if (j.data().data() == k.data().data()) {
        // replace input with the output from newObject
        _inputStrings[k.key()] = (newObject->outputStrings())[j.key()];  
      }
    }
  }
}


void KstDataObject::replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector) {
  for (KstVectorMap::Iterator j = _inputVectors.begin(); j != _inputVectors.end(); ++j) {
    if (j.data() == oldVector) {
      _inputVectors[j.key()] = newVector;  
    }      
  }
  
  QDictIterator<KstScalar> scalarDictIter(oldVector->scalars());
  for (KstScalarMap::Iterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
    for (; scalarDictIter.current(); ++ scalarDictIter) {
      if (scalarDictIter.current() == j.data()) {
        _inputScalars[j.key()] = (newVector->scalars())[scalarDictIter.currentKey()];
      }  
    }
  }
}


void KstDataObject::replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix) {
  for (KstMatrixMap::Iterator j = _inputMatrices.begin(); j != _inputMatrices.end(); ++j) {
    if (j.data() == oldMatrix) {
      _inputMatrices[j.key()] = newMatrix;  
    }      
  }
  
  QDictIterator<KstScalar> scalarDictIter(oldMatrix->scalars());
  for (KstScalarMap::Iterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
    for (; scalarDictIter.current(); ++ scalarDictIter) {
      if (scalarDictIter.current() == j.data()) {
        _inputScalars[j.key()] = (newMatrix->scalars())[scalarDictIter.currentKey()];
      }  
    }
  }
}


bool KstDataObject::uses(KstObjectPtr p) const {
  
  KstVectorPtr v = kst_cast<KstVector>(p);
  if (v) {
    for (KstVectorMap::ConstIterator j = _inputVectors.begin(); j != _inputVectors.end(); ++j) {
      if (j.data() == v) {
        return true;
      }
    }
    QDictIterator<KstScalar> scalarDictIter(v->scalars());
    for (KstScalarMap::ConstIterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
      for (; scalarDictIter.current(); ++scalarDictIter) {
        if (scalarDictIter.current() == j.data()) {
          return true;  
        }  
      }
    }
  } else if (KstMatrixPtr matrix = kst_cast<KstMatrix>(p)) {
    for (KstMatrixMap::ConstIterator j = _inputMatrices.begin(); j != _inputMatrices.end(); ++j) {
      if (j.data() == matrix) {
        return true;
      }
    }
    QDictIterator<KstScalar> scalarDictIter(matrix->scalars());
    for (KstScalarMap::ConstIterator j = _inputScalars.begin(); j != _inputScalars.end(); ++j) {
      for (; scalarDictIter.current(); ++scalarDictIter) {
        if (scalarDictIter.current() == j.data()) {
          return true;  
        }  
      }
    }
  } else if (KstDataObjectPtr obj = kst_cast<KstDataObject>(p) ) {
    // check all connections from this object to p
    for (KstVectorMap::Iterator j = obj->outputVectors().begin(); j != obj->outputVectors().end(); ++j) {
      for (KstVectorMap::ConstIterator k = _inputVectors.begin(); k != _inputVectors.end(); ++k) {
        if (j.data() == k.data()) {
          return true;
        }
      }
      // also check dependencies on vector stats
      QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        for (; scalarDictIter.current(); ++scalarDictIter) {
          if (scalarDictIter.current() == k.data()) {
            return true;
          }  
        }
      }
    }
  
    for (KstMatrixMap::Iterator j = obj->outputMatrices().begin(); j != obj->outputMatrices().end(); ++j) {
      for (KstMatrixMap::ConstIterator k = _inputMatrices.begin(); k != _inputMatrices.end(); ++k) {
        if (j.data() == k.data()) {
          return true;
        }
      }
      // also check dependencies on vector stats
      QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        for (; scalarDictIter.current(); ++scalarDictIter) {
          if (scalarDictIter.current() == k.data()) {
            return true;
          }  
        }
      }
    }
    
    for (KstScalarMap::Iterator j = obj->outputScalars().begin(); j != obj->outputScalars().end(); ++j) {
      for (KstScalarMap::ConstIterator k = _inputScalars.begin(); k != _inputScalars.end(); ++k) {
        if (j.data() == k.data()) {
          return true;
        }
      } 
    }
  
    for (KstStringMap::Iterator j = obj->outputStrings().begin(); j != obj->outputStrings().end(); ++j) {
      for (KstStringMap::ConstIterator k = _inputStrings.begin(); k != _inputStrings.end(); ++k) {
        if (j.data() == k.data()) {
          return true;
        }
      }
    }
  }
  return false;
}


#include "kstdataobject.moc"
// vim: ts=2 sw=2 et
