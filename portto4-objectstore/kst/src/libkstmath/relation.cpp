/***************************************************************************
                   relation.cpp: base class for a curve
                             -------------------
    begin                : June 2003
    copyright            : (C) 2003 University of Toronto
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   Permission is granted to link with any opensource library             *
 *                                                                         *
 ***************************************************************************/

#include "relation.h"
#include "datacollection.h"
#include "debug.h"
#include "kst_i18n.h"
#include "objectstore.h"

#include <QXmlStreamWriter>

namespace Kst {

const QString Relation::staticTypeString = I18N_NOOP("Relation");

Relation::Relation(ObjectStore *store, const QDomElement& e) : Object() {
  commonConstructor();
}


Relation::Relation(ObjectStore *store, const ObjectTag& tag) : Object(tag) {
  commonConstructor();
}


void Relation::commonConstructor() {
  _ns_maxx = _ns_minx = _ns_maxy = _ns_miny = MaxX = MinX = MinPosX = MeanX = MaxY = MinY = MinPosY = 0.0;
  NS = 0;

  _ignoreAutoScale = false;
  _parsedLegendTag = 0L;
}


Relation::~Relation() {
  delete _parsedLegendTag;
  _parsedLegendTag = 0L;
}


void Relation::save(QXmlStreamWriter &s) {
  Q_UNUSED(s)
}


bool Relation::deleteDependents() {
  Data::self()->removeCurveFromPlots(this);
  return true;
}


void Relation::setIgnoreAutoScale(bool ignoreAutoScale) {
  setDirty();
  _ignoreAutoScale = ignoreAutoScale;
}

void Relation::updateParsedLegendTag() {
  delete _parsedLegendTag;
  if (_legendText.isEmpty()) {
    _parsedLegendTag = Label::parse(tag().tagString(), false, false); // FIXME: should this be displayString?
  } else {
    _parsedLegendTag = Label::parse(legendText(), true, false);
  }
}

Label::Parsed *Relation::parsedLegendTag() {
  if (!_parsedLegendTag) {
    updateParsedLegendTag();
  }
  return _parsedLegendTag;
}

void Relation::setLegendText(const QString& theValue) {
  _legendText = theValue; 
  updateParsedLegendTag(); 
  emit modifiedLegendEntry();
}


void Relation::writeLockInputsAndOutputs() const {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  #ifdef LOCKTRACE
  qDebug() << (void*)this << this->tag().tagString() << ") Relation::writeLockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << endl;
  #endif

  QList<PrimitivePtr> inputs;
  QList<PrimitivePtr> outputs;

  QList<StringPtr> sl = _inputStrings.values();
  for (QList<StringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    inputs += (*i).data();
  }
  sl = _outputStrings.values();
  for (QList<StringPtr>::Iterator i = sl.begin(); i != sl.end(); ++i) {
    outputs += (*i).data();
  }

  QList<ScalarPtr> sc = _inputScalars.values();
  for (QList<ScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    inputs += (*i).data();
  }
  sc = _outputScalars.values();
  for (QList<ScalarPtr>::Iterator i = sc.begin(); i != sc.end(); ++i) {
    outputs += (*i).data();
  }

  QList<VectorPtr> vl = _inputVectors.values();
  for (QList<VectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    inputs += (*i).data();
  }
  vl = _outputVectors.values();
  for (QList<VectorPtr>::Iterator i = vl.begin(); i != vl.end(); ++i) {
    outputs += (*i).data();
  }

  QList<MatrixPtr> ml = _inputMatrices.values();
  for (QList<MatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    inputs += (*i).data();
  }
  ml = _outputMatrices.values();
  for (QList<MatrixPtr>::Iterator i = ml.begin(); i != ml.end(); ++i) {
    outputs += (*i).data();
  }

  qSort(inputs);
  qSort(outputs);

  QList<PrimitivePtr>::ConstIterator inputIt = inputs.begin();
  QList<PrimitivePtr>::ConstIterator outputIt = outputs.begin();

  while (inputIt != inputs.end() || outputIt != outputs.end()) {
    if (inputIt != inputs.end() && (outputIt == outputs.end() || (void*)(*inputIt) < (void*)(*outputIt))) {
      // do input
      if (!(*inputIt)) {
        qWarning() << "Input for data object " << this->tag().displayString() << " is invalid." << endl;
      }
#ifdef LOCKTRACE
      qDebug() << (void*)this << this->tag().tagString() << ") KstDataObject::writeLockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": write locking input \"" << (*inputIt)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*inputIt) << ")" << endl;
#endif
      (*inputIt)->writeLock();
      ++inputIt;
    } else {
      // do output
      if (!(*outputIt)) {
        qWarning() << "Output for data object " << this->tag().displayString() << " is invalid." << endl;
      }
#ifdef LOCKTRACE
      qDebug() << (void*)this << this->tag().tagString() << ") KstDataObject::writeLockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": write locking output \"" << (*outputIt)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*outputIt) << ")" << endl;
#endif
      if ((*outputIt)->provider() != this) {
//        Debug::self()->log(i18n("KstDataObject::writeLockInputsAndOutputs() by tid=%1: write locking output %2 (not provider) -- this is probably an error. Please email kst@kde.org with details.").arg(reinterpret_cast<qint64>(QThread::currentThread())).arg((*outputIt)->tagName()), Debug::Error);
      }
      (*outputIt)->writeLock();
      ++outputIt;
    }
  }
}


void Relation::unlockInputsAndOutputs() const {
  #ifdef LOCKTRACE
  qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << endl;
  #endif

  for (MatrixMap::ConstIterator i = _outputMatrices.begin(); i != _outputMatrices.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Output matrix for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking output matrix \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (MatrixMap::ConstIterator i = _inputMatrices.begin(); i != _inputMatrices.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Input matrix for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking input matrix \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (VectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Output vector for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking output vector \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (VectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Input vector for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking input vector \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (ScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Output scalar for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking output scalar \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (ScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Input scalar for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking input scalar \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (StringMap::ConstIterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Output string for data object " << this->tag().displayString() << " is invalid." << endl;
    }
   #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking output string \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }

  for (StringMap::ConstIterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    if (!(*i)) {
      qWarning() << "Input string for data object " << this->tag().displayString() << " is invalid." << endl;
    }
    #ifdef LOCKTRACE
    qDebug() << (void*)this << this->tag().tagString() << ") Relation::unlockInputsAndOutputs() by tid=" << (int)QThread::currentThread() << ": unlocking input string \"" << (*i)->tag().tagString() << "\" (" << (void*)((KstRWLock*)*i) << ")" << endl;
    #endif
    (*i)->unlock();
  }
}

}
// vim: ts=2 sw=2 et
