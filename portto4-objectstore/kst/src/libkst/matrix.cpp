/***************************************************************************
                   matrix.cpp: 2D matrix type for kst
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *   copyright : (C) 2004  University of British Columbia                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include <QXmlStreamWriter>

#include "matrix.h"

//#include <stdlib.h>
//#include <math.h>

#include "debug.h"
#include "kst_i18n.h"
#include "math_kst.h"
#include "datacollection.h"
#include "objectstore.h"


// used for resizing; set to 1 for loop zeroing, 2 to use memset
#define ZERO_MEMORY 2

namespace Kst {

const QString Matrix::staticTypeString = I18N_NOOP("Matrix");

Matrix::Matrix(ObjectStore *store, ObjectTag tag, Object *provider, uint nX, uint nY, double minX, double minY, double stepX, double stepY)
    : Primitive(store, tag, provider) {

  _nX = nX;
  _nY = nY;
  _NS = _nX * _nY;
  _NRealS = 0;
  _minX = minX;
  _minY = minY;
  _stepX = stepX;
  _stepY = stepY;
  _z = 0L;
  _zSize = 0;

  _editable = false;
  _saveable = false;

  createScalars(store);
  setDirty();
}


Matrix::~Matrix() {
  // TODO: get rid of the stat scalars
#if 0
  scalarList.lock().writeLock();
  scalarList.setUpdateDisplayTags(false);
  for (QHash<QString, Scalar*>::Iterator iter = _statScalars.begin(); iter != _statScalars.end(); ++iter) {
    scalarList.remove(iter.value());
    iter.value()->_KShared_unref();
  }
  scalarList.setUpdateDisplayTags(true);
  scalarList.lock().unlock();
#endif

  if (_z) {
    free(_z);
    _z = 0L;
  }
}


const QString& Matrix::typeString() const {
  return staticTypeString;
}


int Matrix::sampleCount() const {
  return _nX*_nY;
}


double Matrix::value(double x, double y, bool* ok) const {
  int x_index = (int)floor((x - _minX) / (double)_stepX);
  int y_index = (int)floor((y - _minY) / (double)_stepY);

  return valueRaw(x_index, y_index, ok);
}


double Matrix::valueRaw(int x, int y, bool* ok) const {
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


int Matrix::zIndex(int x, int y) const {
  if (x >= _nX || x < 0 || y >= _nY || y < 0) {
    return -1;
  }
  int index = x * _nY + y;
  if (index >= _zSize || index < 0 ) {
    return -1;
  }
  return index;
}


bool Matrix::setValue(double x, double y, double z) {
  int x_index = (int)floor((x - _minX) / (double)_stepX);
  int y_index = (int)floor((y - _minY) / (double)_stepY);
  return setValueRaw(x_index, y_index, z);
}


bool Matrix::setValueRaw(int x, int y, double z) {
  int index = zIndex(x,y);
  if (index < 0) {
    return false;
  }
  _z[index] = z;
  return true;
}

double Matrix::minValue() const {
  return _statScalars["min"]->value();
}


double Matrix::maxValue() const {
  return _statScalars["max"]->value();
}

double Matrix::minValueNoSpike() const {
  // FIXME: it is expensive to calcNoSpikeRange
  // so we have chosen here to only call it expicitly
  // and no attempt is made to check if it is still up to date...
  // It would be better to have these calls call
  // calcNoSpikeRange iff the values were obsolete.
  return _minNoSpike;
}

double Matrix::maxValueNoSpike() const {
  // FIXME: it is expensive to calcNoSpikeRange
  // so we have chosen here to only call it expicitly
  // and no attempt is made to check if it is still up to date...
  // It would be better to have these calls call
  // calcNoSpikeRange iff the values were obsolete.

  return _maxNoSpike;
}

void Matrix::calcNoSpikeRange(double per) {
  double *min_list, *max_list, min_of_max, max_of_min;
  int n_list;
  int max_n = 50000; // the most samples we will look at...
  double n_skip;
  double x=0;
  int n_notnan;

  int i,j, k;

  // count number of points which aren't nans.
  for (i=n_notnan=0; i<_NS; i++) {
    if (!KST_ISNAN(_z[i])) {
      n_notnan++;
    }
  }

  if (n_notnan==0) {
    _minNoSpike = 0;
    _maxNoSpike = 0;

    return;
  }

  per *= (double)n_notnan/(double)_NS;
  max_n *= int((double)_NS/(double)n_notnan);

  n_skip = (double)_NS/max_n;
  if (n_skip<1.0) n_skip = 1.0;

  n_list = int(double(_NS)*per/n_skip);

  min_list = (double *)malloc(n_list * sizeof(double));
  max_list = (double *)malloc(n_list * sizeof(double));


  // prefill the list
  for (i=0; i<n_list; i++) {
    j = int(i*n_skip);
    min_list[i] = 1E+300;
    max_list[i] = -1E+300;
  }
  min_of_max = -1E+300;
  max_of_min = 1E+300;

  i = n_list;
  for (j=0; j<_NS; j=int(i*n_skip), i++) {
    if (_z[j] < max_of_min) { // member for the min list
      // replace max of min with the new value
      for (k=0; k<n_list; k++) {
        if (min_list[k]==max_of_min) {
          x = min_list[k] = _z[j];
          break;
        }
      }
      max_of_min = x;
      // find the new max_of_min
      for (k=0; k<n_list; k++) {
        if (min_list[k] > max_of_min) {
          max_of_min = min_list[k];
        }
      }
    }
    if (_z[j] > min_of_max) { // member for the max list
      //printf("******** z: %g  min_of_max: %g\n", _z[j], min_of_max);
      // replace min of max with the new value
      for (k=0; k<n_list; k++) {
        if (max_list[k]==min_of_max) {
          x = max_list[k] = _z[j];
          break;
        }
      }
      // find the new min_of_max
      min_of_max = x;
      for (k=0; k<n_list; k++) {
        if (max_list[k] < min_of_max) {
          min_of_max = max_list[k];
        }
      }
    }
  }

  // FIXME: this needs a z spike insensitive algorithm...
  // it should use the update counter to see if it needs
  // to calculate it.  A call to either this or to
  // minValueNoSpike should trigger the calculation
  // which will be slow.
  _minNoSpike = max_of_min;
  _maxNoSpike = min_of_max;

  free(min_list);
  free(max_list);
}

double Matrix::meanValue() const {
  return _statScalars["mean"]->value();
}

double Matrix::minValuePositive() const {
  return _statScalars["minpos"]->value();
}

int Matrix::numNew() const {
  return _numNew;
}


void Matrix::resetNumNew() {
  _numNew = 0;
}


QString Matrix::label() const {
  return _label;
}


void Matrix::zero() {
  for (int i = 0; i < _zSize; i++) {
    _z[i] = 0.0;
  }
  setDirty();
  updateScalars();
}


void Matrix::blank() {
  for (int i = 0; i < _zSize; ++i) {
    _z[i] = NOPOINT;
  }
  setDirty();
  updateScalars();
}


int Matrix::getUsage() const {
  int scalarUsage = 0;
  for (QHash<QString, Scalar*>::ConstIterator it = _statScalars.begin(); it != _statScalars.end(); ++it) {
    scalarUsage += it.value()->getUsage() - 1;
  }
  return Object::getUsage() + scalarUsage;
}


Object::UpdateType Matrix::internalUpdate(Object::UpdateType providerUpdateType) {
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


const QHash<QString, Scalar*>& Matrix::scalars() const {
  return _statScalars;
}


void Matrix::setLabel(const QString& newLabel) {
  _label = newLabel;
}


void Matrix::setXLabel(const QString& newLabel) {
  _xLabel = newLabel;
}


void Matrix::setYLabel(const QString& newLabel) {
  _yLabel = newLabel;
}


QString Matrix::xLabel() const {
  return _xLabel;
}


QString Matrix::yLabel() const {
  return _yLabel;
}


bool Matrix::editable() const {
  return _editable;
}


void Matrix::setEditable(bool editable) {
  _editable = editable;
}


void Matrix::createScalars(ObjectStore *store) {
  Q_ASSERT(store);
  _statScalars.insert("max", store->createObject<Scalar>(ObjectTag("Max", tag())));
  _statScalars["max"]->setProvider(this);
  _statScalars["max"]->_KShared_ref();
  _statScalars.insert("min", store->createObject<Scalar>(ObjectTag("Min", tag())));
  _statScalars["min"]->setProvider(this);
  _statScalars["min"]->_KShared_ref();
  _statScalars.insert("mean", store->createObject<Scalar>(ObjectTag("Mean", tag())));
  _statScalars["mean"]->setProvider(this);
  _statScalars["mean"]->_KShared_ref();
  _statScalars.insert("sigma", store->createObject<Scalar>(ObjectTag("Sigma", tag())));
  _statScalars["sigma"]->setProvider(this);
  _statScalars["sigma"]->_KShared_ref();
  _statScalars.insert("rms", store->createObject<Scalar>(ObjectTag("Rms", tag())));
  _statScalars["rms"]->setProvider(this);
  _statScalars["rms"]->_KShared_ref();
  _statScalars.insert("ns", store->createObject<Scalar>(ObjectTag("NS", tag())));
  _statScalars["ns"]->setProvider(this);
  _statScalars["ns"]->_KShared_ref();
  _statScalars.insert("sum", store->createObject<Scalar>(ObjectTag("Sum", tag())));
  _statScalars["sum"]->setProvider(this);
  _statScalars["sum"]->_KShared_ref();
  _statScalars.insert("sumsquared", store->createObject<Scalar>(ObjectTag("SumSquared", tag())));
  _statScalars["sumsquared"]->setProvider(this);
  _statScalars["sumsquared"]->_KShared_ref();
  _statScalars.insert("minpos", store->createObject<Scalar>(ObjectTag("MinPos", tag())));
  _statScalars["minpos"]->setProvider(this);
  _statScalars["minpos"]->_KShared_ref();
}


void Matrix::updateScalars() {
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


bool Matrix::resizeZ(int sz, bool reinit) {
  //kdDebug() << "resizing to: " << sz << endl;
  if (sz >= 1) {
    _z = static_cast<double*>(Kst::realloc(_z, sz*sizeof(double)));
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


#if 0
bool Matrix::resize(int xSize, int ySize, bool reinit) {
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
#endif


// Resize the matrix to xSize x ySize, maintaining the values in the current
// positions. If reinit is set, new entries will be initialized to 0.
bool Matrix::resize(int xSize, int ySize, bool reinit) {
  if (xSize <= 0 || ySize <= 0) {
    return false;
  }

  // NOTE: _zSize is assumed to correctly represent the state of _z, while _nX
  // and _nY are the desired (but not necessarily actual) current size of the
  // matrix
  bool valid = (_zSize == _nX * _nY); // is the current matrix properly initialized?

  int sz = xSize * ySize;
  if (sz > _zSize) {
    // array is getting bigger, so resize before moving
    _z = static_cast<double*>(Kst::realloc(_z, sz*sizeof(double)));
    if (!_z) {
      qCritical() << "Matrix resize failed";
      return false;
    }
  }

  if (valid && ySize != _nY && _nY > 0) {
    // move old values to new spots
    int source = 0;
    int target = 0;
    for (int row=1; row < qMin(xSize, _nX); ++row) {
      source += _nY;
      target += ySize;
      memmove(_z + target, _z + source, qMin(ySize, _nY)*sizeof(double));
      if (reinit && ySize > _nY) {
        // initialize memory in new column(s) of previous row vacated by memmove
        memset(_z + source, 0, (ySize - _nY)*sizeof(double));
      }
    }
  }

  if (sz < _zSize) {
    // array is getting smaller, so resize after moving
    _z = static_cast<double*>(Kst::realloc(_z, sz*sizeof(double)));
    if (!_z) {
      qCritical() << "Matrix resize failed";
      return false;
    }
  }

  if (reinit && _zSize < sz) {
    // initialize new memory after old values
    int newMemStart = ((_nX - 1) * ySize) + _nY;
    memset(_z + newMemStart, 0, (sz - newMemStart)*sizeof(double));
  }

  _nX = xSize;
  _nY = ySize;
  _NS = _nX * _nY;
  _zSize = sz;

  updateScalars();
  setDirty(true);

  return true;
}


void Matrix::save(QXmlStreamWriter &s) {
  Q_UNUSED(s)
  // no saving
}


bool Matrix::saveable() const {
  return _saveable;
}


void Matrix::change(uint nX, uint nY, double minX, double minY, double stepX, double stepY) {
  _nX = nX;
  _nY = nY;
  _stepX = stepX;
  _stepY = stepY;
  _minX = minX;
  _minY = minY;

  setDirty();
}

}
// vim: ts=2 sw=2 et
