/***************************************************************************
                          kstvector.cpp  -  description
                             -------------------
    begin                : Fri Sep 22 2000
    copyright            : (C) 2000-2002 by cbn
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

#include <kdebug.h>

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "kstvector.h"
#include "kstdatacollection.h"
#include "kstdoc.h"

// Use 1 for a simple for() loop
#define ZERO_MEMORY 2

#ifdef NAN
double KST::NOPOINT = NAN;
#else
double KST::NOPOINT = 0.0/0.0; // NaN
#endif

#define INITSIZE 1

// FIXME: optimize scalar access

/** Create a vector */
KstVector::KstVector(const QString& name, int size)
: KstObject(), _nsum(0), _scalars(11) {
  //kdDebug() << "+++ CREATING VECTOR: " << (void*) this << endl;
  _tag = name;
  NumShifted = 0;
  NumNew = 0;

  if (size <= 0) {
    size = INITSIZE;
  }

  if (_tag.isEmpty()) {
    QString nt = "Anonymous Vector %1";
    int i = 0;
    // FIXME: make me more efficient
    do {
      _tag = nt.arg(i++);
    } while (KST::vectorTagNameNotUnique(_tag, false));
  } else {
    while (KST::vectorTagNameNotUnique(_tag, false)) {
      _tag += "'";
    }
  }

  _v = static_cast<double*>(malloc(size*sizeof(double)));
  _size = size;
  zero();
  _is_rising = false;

  CreateScalars();
  KST::vectorList.lock().writeLock();
  KST::vectorList.append(this);
  KST::vectorList.lock().writeUnlock();
}

KstVector::~KstVector() {
  //kdDebug() << "+++ DELETING VECTOR: " << (void*) this << endl;

  KST::scalarList.lock().writeLock();
  for (QDictIterator<KstScalar> it(_scalars); it.current(); ++it) {
    KST::scalarList.remove(it.current());
    it.current()->_KShared_unref();
  }
  KST::scalarList.lock().writeUnlock();

  free(_v);
  _v = 0;
}


// FIXME: optimize me - possible that floor() (especially) and isnan() are
//        expensive here.
/** Return v[i], i is sample number, interpolated to have ns_i total
    samples in vector */
double KstVector::interpolate(int in_i, int ns_i) {
  double fj, fdj;
  int j;

  /** Limits checks - optional? **/
  if (in_i < 0)
    return _v[0];

  if (in_i >= ns_i - 1)
    return _v[_size - 1];

  /** speedup check **/
  if (ns_i == _size) { /* no extrapolating or decimating needed */
    return _v[in_i];
  }

  fj = in_i * double(_size - 1) / double(ns_i-1); //scaled index

  j = int(floor(fj));	       // index of sample one lower

  // This is optimized to avoid unnecessary isnan calls!
  if (_v[j + 1] != _v[j + 1]) {
    if (isnan(_v[j + 1])) {
      return KST::NOPOINT;
    }
  }
  if (_v[j] != _v[j]) {
    if (isnan(_v[j])) {
      return KST::NOPOINT;
    }
  }

  fdj = fj - float(j);     // fdj is fraction between _v[j] and _v[j+1]

  return _v[j + 1] * fdj + _v[j] * (1.0 - fdj);
}


double KstVector::value(int i) {
  if (i < 0) {//can't look before begining
    return 0.0;
  }

  if (i >= _size) {// can't look past end
    return 0.0;
  }
  return _v[i];
}

void KstVector::CreateScalars() {
  _scalars.insert("max", new KstScalar(_tag + "-Max"));
  _scalars["max"]->_KShared_ref();
  _scalars.insert("min", new KstScalar(_tag + "-Min"));
  _scalars["min"]->_KShared_ref();
  _scalars.insert("mean", new KstScalar(_tag + "-Mean"));
  _scalars["mean"]->_KShared_ref();
  _scalars.insert("sigma", new KstScalar(_tag + "-Sigma"));
  _scalars["sigma"]->_KShared_ref();
  _scalars.insert("rms", new KstScalar(_tag + "-Rms"));
  _scalars["rms"]->_KShared_ref();
  _scalars.insert("ns", new KstScalar(_tag + "-NS"));
  _scalars["ns"]->_KShared_ref();
  _scalars.insert("sum", new KstScalar(_tag + "-Sum"));
  _scalars["sum"]->_KShared_ref();
  _scalars.insert("sumsquared", new KstScalar(_tag + "-SumSquared"));
  _scalars["sumsquared"]->_KShared_ref();
  _scalars.insert("minpos", new KstScalar(_tag + "-MinPos"));
  _scalars["minpos"]->_KShared_ref();
}

void KstVector::RenameScalars() {
  _scalars["max"]->setTagName(_tag + "-Max");
  _scalars["min"]->setTagName(_tag + "-Min");
  _scalars["mean"]->setTagName(_tag + "-Mean");
  _scalars["sigma"]->setTagName(_tag + "-Sigma");
  _scalars["rms"]->setTagName(_tag + "-Rms");
  _scalars["ns"]->setTagName(_tag + "-NS");
  _scalars["sum"]->setTagName(_tag + "-Sum");
  _scalars["sumsquared"]->setTagName(_tag + "-SumSquared");
  _scalars["minpos"]->setTagName(_tag + "-MinPos");
}

void KstVector::UpdateScalars() {
  _scalars["ns"]->setValue(_size);

  if (_nsum >= 2) {
    _scalars["mean"]->setValue(_scalars["sum"]->value()/double(_nsum));
    _scalars["sigma"]->setValue( sqrt(
      (_scalars["sumsquared"]->value() - _scalars["sum"]->value()*_scalars["sum"]->value()/double(_nsum))/ double(_nsum-1) ) );
    _scalars["rms"]->setValue(sqrt(_scalars["sumsquared"]->value()/double(_nsum)));
  } else {
    _scalars["sigma"]->setValue(_scalars["max"]->value() - _scalars["min"]->value());
    _scalars["rms"]->setValue(sqrt(_scalars["sumsquared"]->value()));
    _scalars["mean"]->setValue(0);
  }
}

double* KstVector::realloced(double *memptr, int newSize) {
  double *old = _v;
  _v = memptr;
  _size = newSize;
  return old;
}


void KstVector::zero() {
  for (int i = 0; i < _size; i++) {
    _v[i] = 0.0;
  }
}


void KstVector::resize(int sz, bool reinit) {
  //kdDebug() << "resizing to: " << sz << endl;
  if (sz > 1) {
    _v = static_cast<double*>(realloc(_v, sz*sizeof(double)));
#ifdef ZERO_MEMORY
    if (reinit && _size < sz) {
#if ZERO_MEMORY == 2
      memset(&_v[_size], 0, (sz - _size)*sizeof(double));
#else
      for (int i = _size; i < sz; i++) {
        _v[i] = 0.0;
      }
#endif
    }
#endif
    _size = sz;
  }
}


KstObject::UpdateType KstVector::update(int update_counter) {
  int i, i0;
  double max, min, sum, sum2, minpos, v;
  double last_v;
  double dv2=0.0, dv, no_spike_max_dv;

  max = min = sum = sum2 = minpos = 0.0;

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  _nsum = 0;
  if (_size > 0) {
    _is_rising = true;

    for (i = 0; i < _size && !finite(_v[i]); i++)
      /* Do Nothing */ ;

    if (i==_size) {
      _scalars["sum"]->setValue(sum);
      _scalars["sumsquared"]->setValue(sum2);
      _scalars["max"]->setValue(max);
      _scalars["min"]->setValue(min);
      _scalars["minpos"]->setValue(minpos);
      _ns_max = _ns_min = 0;

      UpdateScalars();
      return UPDATE;
    }

    i0 = i;

    if (i0>0) {
      _is_rising = false;
    }

    max = min = sum = sum2 = _v[i0];
    if (_v[i0]>0)
      minpos = _v[i0];
    else
      minpos = 1.0E300;

    last_v = _v[i0];
    for (i=i0; i < _size; i++) {
      v = _v[i]; // get rid of redirections

      if (finite(v)) {

        dv = v - last_v;
        dv2 += dv*dv;

        if (v<=last_v) {
          if (i!=i0) {
            _is_rising = false;
          }
        }
        last_v = v;

        _nsum++;
        sum += v;
        sum2 += v*v;

        if (v > max) {
          max = v;
        } else {
          if (v < min) {
            min = v;
          }
          if (v < minpos && v > 0) {
              minpos = v;
          }
        }
      } else {
        _is_rising = false;
      }
    }

    no_spike_max_dv = 7.0*sqrt(dv2/double(_nsum));

    _ns_max = _ns_min = last_v = _v[i0];

    for (i = i0; i < _size; i++) {
      v = _v[i]; // get rid of redirections
      if (finite(v)) {
        if (fabs(v - last_v) < no_spike_max_dv) {
          if (v > _ns_max) {
            _ns_max = v;
          } else if (v < _ns_min) {
            _ns_min = v;
          }
          last_v = v;
        } else {
          i += 20;
          if (i < _size)
            last_v = _v[i];
          i++;
        }
      }
    }

    _scalars["sum"]->setValue(sum);
    _scalars["sumsquared"]->setValue(sum2);
    _scalars["max"]->setValue(max);
    _scalars["min"]->setValue(min);
    _scalars["minpos"]->setValue(minpos);

    UpdateScalars();
    return UPDATE;
  }

  return NO_CHANGE;
}

void KstVector::save(QTextStream &ts) {
  Q_UNUSED(ts);

assert(0);
}


void KstVector::setTagName(const QString& newTag) {
  KstObject::setTagName(newTag);
  RenameScalars();
}

void KstVector::SetNewAndShift(int inNew, int inShift) {
  NumNew = inNew;
  NumShifted = inShift;
}

QString KstVector::label() const {
  return QString::null;
}

QString KstVector::fileLabel() const {
  return QString::null;
}

double *const KstVector::value() const {
  return _v;
}

double KstVector::min() const {
  return _scalars["min"]->value();
}

double KstVector::max() const {
  return _scalars["max"]->value();
}

double KstVector::mean() const {
  return _scalars["mean"]->value();
}

double KstVector::minPos() const {
  return _scalars["minpos"]->value();
}

int KstVector::sampleCount() const {
  return _size;
}

int KstVector::numNew() const {
  return NumNew;
}

int KstVector::numShift() const {
  return NumShifted;
}

void KstVector::newSync() {
  NumNew = NumShifted = 0;
}

int KstVector::length() const {
  return _size;
}

KstVectorPtr KstVector::generateVector(double x0, double x1, int n, const QString &tag) {
  if (n < 2) {
    n = 2;
  }

  if (x0 > x1) {
    double tx;
    tx = x0;
    x0 = x1;
    x1 = tx;
  }

  if (x0 == x1) {
    x1 = x0 + 0.1;
  }

  QString t = tag;
  if (t.isEmpty()) {
    KST::vectorList.lock().readLock();
    t = "V" + QString::number(KST::vectorList.count() + 1) + "-" + "X(" + QString::number(x0) + ".." + QString::number(x1) + ")";
    KST::vectorList.lock().readUnlock();
  }

  while (KST::vectorTagNameNotUnique(t, false)) {
    t += "'";
  }

  KstVectorPtr xv = new KstVector(t, n);

  for (int i = 0; i < n; i++) {
    xv->value()[i] = x0 + double(i) * (x1 - x0) / (n - 1);
  }

  xv->_scalars["min"]->setValue(x0);
  xv->_scalars["max"]->setValue(x1);
  xv->UpdateScalars();

return xv;
}

// vim: et sw=2 ts=2
