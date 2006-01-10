/***************************************************************************
                   kstmatrix.cpp: 2D matrix type for kst
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 University of British Columbia
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

#include <stdlib.h>
#include <math.h>

#include <qdeepcopy.h>
#include <qstylesheet.h>
  
#include <klocale.h>

#include "kstdatacollection.h"
#include "kstdataobject.h"
#include "kstdebug.h"
#include "ksdebug.h"
#include "kstmatrix.h"
#include "kstmath.h"

// used for resizing; set to 1 for loop zeroing, 2 to use memset
#define ZERO_MEMORY 2

static int anonymousMatrixCounter = 1;

KstMatrix::KstMatrix(const QString &in_tag, uint nX, uint nY, double minX, double minY, double stepX, double stepY) : KstObject() {
 
  _nX = nX;
  _nY = nY;
  _minX = minX;
  _minY = minY;
  _stepX = stepX;
  _stepY = stepY;
  _z = 0L;
  _zSize = 0;
  
  _editable = false;
  _saveable = false;
  
  // must create scalars before setting tag name
  createScalars();
  setTagName(in_tag);
  if (in_tag.isEmpty()) {
    QString nt = i18n("Anonymous Matrix %1");

    do {
      KstObject::setTagName(nt.arg(anonymousMatrixCounter++));
    } while (KST::matrixTagNameNotUnique(tagName(), false));
  } else {
    while (KST::matrixTagNameNotUnique(tagName(), false)) {
      KstObject::setTagName(tagName() + '\'');
    }
  }
  setDirty();
}
    
KstMatrix::~KstMatrix() {
  // get rid of the stat scalars
  KST::scalarList.lock().writeLock();
  for (QDictIterator<KstScalar> iter(_statScalars); iter.current(); ++iter) {
    KST::scalarList.remove(iter.current());
    iter.current()->_KShared_unref();  
  }
  KST::scalarList.lock().writeUnlock();

  if (_z) {
    free(_z);
    _z = 0L;  
  }  
}


int KstMatrix::sampleCount() const {
  return _nX*_nY;  
}
    

double KstMatrix::value(double x, double y, bool* ok) {
  int x_index = (int)floor((x - _minX) / (double)_stepX);
  int y_index = (int)floor((y - _minY) / (double)_stepY);

  return valueRaw(x_index, y_index, ok);
}
    

double KstMatrix::valueRaw(int x, int y, bool* ok) {
  int index = zIndex(x,y);
  if ((index < 0) || !finite(_z[index]) || KST_ISNAN(_z[index])) {
    if (ok) {
      (*ok) = false;
    }
    return 0.0;
  }
  if (ok) {
    (*ok) = true;  
  }
  return _z[index];
}


int KstMatrix::zIndex(int x, int y) {
  if (x >= _nX || x < 0 || y >= _nY || y < 0) {
    return -1;
  }
  int index = x * _nY + y;
  if (index >= _zSize || index < 0 ) {
    return -1;
  }
  return index;
}


bool KstMatrix::setValue(double x, double y, double z) {
  int x_index = (int)floor((x - _minX) / (double)_stepX);
  int y_index = (int)floor((y - _minY) / (double)_stepY);
  return setValueRaw(x_index, y_index, z);
}


bool KstMatrix::setValueRaw(int x, int y, double z) {
  int index = zIndex(x,y);
  if (index < 0) {
    return false;  
  }
  _z[index] = z;
  return true;
}
    

double KstMatrix::minValue() const {
  return _statScalars["min"]->value();  
}


double KstMatrix::maxValue() const {
  return _statScalars["max"]->value();  
}
    

double KstMatrix::minValueNoSpike() const {
  return _minZNoSpike;
}


double KstMatrix::maxValueNoSpike() const {
  return _maxZNoSpike;
}
  
  
double KstMatrix::meanValue() const {
  return _statScalars["mean"]->value();
}
    

double KstMatrix::minValuePositive() const {
  return _statScalars["minpos"]->value();  
}
    

int KstMatrix::numNew() const {
  return _numNew;  
}
    
    
void KstMatrix::resetNumNew() {
  _numNew = 0;  
}

    
QString KstMatrix::label() const {
  return _label;
}

    
void KstMatrix::zero() {
  _minZNoSpike = _maxZNoSpike = 0.0;
  for (int i = 0; i < _zSize; i++) {
    _z[i] = 0.0;  
  }
  setDirty();
  updateScalars();
}
    

void KstMatrix::setProvider(KstObject* obj) {
  _provider = obj;  
}
    

int KstMatrix::getUsage() const {
  int scalarUsage = 0;
  for (QDictIterator<KstScalar> it(_statScalars); it.current(); ++it) {
    scalarUsage += it.current()->getUsage() - 1;
  }
  return KstObject::getUsage() + scalarUsage;
}


KstObject::UpdateType KstMatrix::update(int update_counter) {
  
  bool force = dirty();
  
  setDirty(false);
  
  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();  
  }
  
  KstObject::UpdateType providerUpdateType = NO_CHANGE;
  
  if (update_counter > 0) {
    KstObjectPtr provider = KstObjectPtr(_provider);
    if (provider) {
      providerUpdateType = provider->update(update_counter);
      if (!force && providerUpdateType == KstObject::NO_CHANGE) {
        return setLastUpdateResult(providerUpdateType);  
      }  
    } else if (force) {
      providerUpdateType = UPDATE;  
    }
  }
  
  // do the update if necessary
  KstObject::UpdateType rc = internalUpdate(providerUpdateType);
  setDirty(false);
  return rc;
}


KstObject::UpdateType KstMatrix::internalUpdate(KstObject::UpdateType providerUpdateType) {
  // calculate stats
  _NS = _nX * _nY;

  if (_zSize > 0) {
    double min = NAN;
    double max = NAN;
    double minpos = NAN;
    double sum = 0.0, sumsquared = 0.0;
    bool initialized = false;
    
    _NRealS = 0;    
    
    for (int i = 0; i < _zSize; i++) {
      if (finite(_z[i]) && !KST_ISNAN(_z[i])) {
        if (!initialized) {
          min = _z[i];
          max = _z[i];
          minpos = (_z[0] > 0) ? _z[0] : 1.0E300;
          initialized = true;
          _NRealS++;
        } else {
          if (min > _z[i]) {
            min = _z[i];
          }
          if (max < _z[i]) {
            max = _z[i];
          }
          if (minpos > _z[i] && _z[i] > 0) {
            minpos = _z[i];
          }
          sum += _z[i];
          sumsquared += _z[i] * _z[i];
        
          _NRealS++;
        }
      }
    }
    _statScalars["sum"]->setValue(sum);
    _statScalars["sumsquared"]->setValue(sumsquared);
    _statScalars["max"]->setValue(max);
    _statScalars["min"]->setValue(min);
    _statScalars["minpos"]->setValue(minpos);
    
    updateScalars();
    
    return setLastUpdateResult(providerUpdateType);
  } 
  return setLastUpdateResult(NO_CHANGE);
}
    
    
void KstMatrix::setTagName(const QString& newTag) {
  KstObject::setTagName(newTag);
  renameScalars();
}
    
 
const QDict<KstScalar>& KstMatrix::scalars() const {
  return _statScalars;
}
    
    
KstObjectPtr KstMatrix::provider() const {
  return KstObjectPtr(_provider);  
}
    
   
bool KstMatrix::deleteDependents() {
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
  KST::dataObjectList.lock().readUnlock();
  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
    bool user = (*i)->uses(this);
    if (user) {
      KstDataObjectPtr dop = *i;
      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.remove(dop);
      KST::dataObjectList.lock().writeUnlock();
      dop->deleteDependents();
    }
  }
  return true;
}
    
bool KstMatrix::duplicateDependents(QMap<KstSharedPtr<KstDataObject>, KstSharedPtr<KstDataObject> > &duplicatedMap,
                                    QMap<KstMatrixPtr, KstMatrixPtr> &duplicatedMatrices) {
  // work with a copy of the data object list
  KST::dataObjectList.lock().readLock();
  KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
  KST::dataObjectList.lock().readUnlock();

  for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
    if ((*i)->uses(this)) {
      if (duplicatedMap.contains(*i)) {
        (duplicatedMap[*i])->replaceDependency(this, duplicatedMatrices[this]);  
      } else {
        KstDataObjectPtr newObject = (*i)->makeDuplicate(duplicatedMap);
        newObject->replaceDependency(this, duplicatedMatrices[this]);
        if (newObject) {
          KST::dataObjectList.lock().writeLock();
          KST::dataObjectList.append(newObject.data());
          KST::dataObjectList.lock().writeUnlock();
          (*i)->duplicateDependents(duplicatedMap);
        }
      }
    }
  }

  return true;
}
    

void KstMatrix::setLabel(const QString& newLabel) {
  _label = newLabel;
}


void KstMatrix::setXLabel(const QString& newLabel) {
  _xLabel = newLabel;  
}


void KstMatrix::setYLabel(const QString& newLabel) {
  _yLabel = newLabel;
}


QString KstMatrix::xLabel() const {
  return _xLabel;
}


QString KstMatrix::yLabel() const {
  return _yLabel;
}


bool KstMatrix::editable() const {
  return _editable;  
}


void KstMatrix::setEditable(bool editable) {
  _editable = editable;  
}


void KstMatrix::createScalars() {
  _statScalars.insert("max", new KstScalar(tagName() + "-Max"));
  _statScalars["max"]->_KShared_ref();
  _statScalars.insert("min", new KstScalar(tagName() + "-Min"));
  _statScalars["min"]->_KShared_ref();
  _statScalars.insert("mean", new KstScalar(tagName() + "-Mean"));
  _statScalars["mean"]->_KShared_ref();
  _statScalars.insert("sigma", new KstScalar(tagName() + "-Sigma"));
  _statScalars["sigma"]->_KShared_ref();
  _statScalars.insert("rms", new KstScalar(tagName() + "-Rms"));
  _statScalars["rms"]->_KShared_ref();
  _statScalars.insert("ns", new KstScalar(tagName() + "-NS"));
  _statScalars["ns"]->_KShared_ref();
  _statScalars.insert("sum", new KstScalar(tagName() + "-Sum"));
  _statScalars["sum"]->_KShared_ref();
  _statScalars.insert("sumsquared", new KstScalar(tagName() + "-SumSquared"));
  _statScalars["sumsquared"]->_KShared_ref();
  _statScalars.insert("minpos", new KstScalar(tagName() + "-MinPos"));
  _statScalars["minpos"]->_KShared_ref();
}


void KstMatrix::renameScalars() {
  _statScalars["max"]->setTagName(tagName() + "-Max");
  _statScalars["min"]->setTagName(tagName() + "-Min");
  _statScalars["mean"]->setTagName(tagName() + "-Mean");
  _statScalars["sigma"]->setTagName(tagName() + "-Sigma");
  _statScalars["rms"]->setTagName(tagName() + "-Rms");
  _statScalars["ns"]->setTagName(tagName() + "-NS");
  _statScalars["sum"]->setTagName(tagName() + "-Sum");
  _statScalars["sumsquared"]->setTagName(tagName() + "-SumSquared");
  _statScalars["minpos"]->setTagName(tagName() + "-MinPos");
}


void KstMatrix::updateScalars() {  
  _statScalars["ns"]->setValue(_NS);
  if (_NRealS >= 2) {
    _statScalars["mean"]->setValue(_statScalars["sum"]->value()/double(_NRealS));
    _statScalars["sigma"]->setValue( sqrt(
        (_statScalars["sumsquared"]->value() - _statScalars["sum"]->value()*_statScalars["sum"]->value()/double(_NRealS))/ double(_NRealS-1) ) );
    _statScalars["rms"]->setValue(sqrt(_statScalars["sumsquared"]->value()/double(_NRealS)));
  } else {
    _statScalars["sigma"]->setValue(_statScalars["max"]->value() - _statScalars["min"]->value());
    _statScalars["rms"]->setValue(sqrt(_statScalars["sumsquared"]->value()));
    _statScalars["mean"]->setValue(0);
  }
}


bool KstMatrix::resizeZ(int sz, bool reinit) {
  //kdDebug() << "resizing to: " << sz << endl;
  if (sz >= 1) {
    _z = static_cast<double*>(KST::realloc(_z, sz*sizeof(double)));
    if (!_z) {
      return false;
    }
#ifdef ZERO_MEMORY
    if (reinit && _zSize < sz) {
#if ZERO_MEMORY == 2
      memset(&_z[_zSize], 0, (sz - _zSize)*sizeof(double));
      
#else
      for (int i = _zSize; i < sz; i++) {
        _z[i] = 0.0;
      }
#endif
    }
#else
    abort();  // avoid unpleasant surprises
#endif
    _zSize = sz;
    updateScalars();
  }

  setDirty();
  return true;
}


bool KstMatrix::resize(int xSize, int ySize, bool reinit) {
  int oldNX = _nX;
  int oldNY = _nY;
  _nX = xSize;
  _nY = ySize;
  if (resizeZ(xSize*ySize, reinit)) {
    return true;  
  } else {
    _nX = oldNX;
    _nY = oldNY;
    return false;  
  }
}


void KstMatrix::save(QTextStream &ts, const QString& indent) {
  Q_UNUSED(ts);
  Q_UNUSED(indent);

  // no saving
}


bool KstMatrix::saveable() const {
  return _saveable;  
}


void KstMatrix::change(const QString tag, uint nX, uint nY, double minX, double minY, double stepX, double stepY) {
  setTagName(tag);
  _nX = nX;
  _nY = nY;
  _stepX = stepX;
  _stepY = stepY;
  _minX = minX;
  _minY = minY;
  
  setDirty();  
}


#include "kstmatrix.moc"
// vim: ts=2 sw=2 et
