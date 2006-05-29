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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <qdeepcopy.h>
#include <qstylesheet.h>

#include "ksdebug.h"
#include <klocale.h>
#include "kstdatacollection.h"
#include "defaultprimitivenames.h"
#include "kstmath.h"
#include "kstvector.h"

static int anonymousVectorCounter = 1;

// Use 1 for a simple for() loop
#define ZERO_MEMORY 2

#define INITSIZE 1

/** Create a vector */
KstVector::KstVector(const QString& name, int size, bool isScalarList)
: KstObject(), _nsum(0), _scalars(isScalarList ? 0 : 11) {
  //kstdDebug() << "+++ CREATING VECTOR: " << (void*) this << endl;
  KstObject::setTagName(name);
  _editable = false;
  NumShifted = 0;
  NumNew = 0;
  _isScalarList = isScalarList;
  _label = QString::null;

  _saveable = false;

  if (size <= 0) {
    size = INITSIZE;
  }

  if (name.isEmpty()) {
    QString nt = i18n("Anonymous Vector %1");

    do {
      KstObject::setTagName(nt.arg(anonymousVectorCounter++));
    } while (KstData::self()->vectorTagNameNotUnique(tagName(), false));
  } else {
    while (KstData::self()->vectorTagNameNotUnique(tagName(), false)) {
      KstObject::setTagName(tagName() + '\'');
    }
  }

  _v = static_cast<double*>(KST::malloc(size*sizeof(double)));
  if (!_v) { // Malloc failed
    _v = static_cast<double*>(KST::malloc(sizeof(double)));
    _size = 1;
  } else {
    _size = size;
  }
  _is_rising = false;

  CreateScalars();
  zero();
}


KstVector::~KstVector() {
  //kstdDebug() << "+++ DELETING VECTOR: " << (void*) this << endl;
  KST::scalarList.lock().writeLock();
  for (QDictIterator<KstScalar> it(_scalars); it.current(); ++it) {
    KST::scalarList.remove(it.current());
    it.current()->_KShared_unref();
  }
  KST::scalarList.lock().writeUnlock();

  if (_v) {
    free(_v);
    _v = 0;
  }
}


#define GENERATE_INTERPOLATION              \
  assert(_size > 0);                        \
  /** Limits checks - optional? **/         \
  if (in_i < 0 || _size == 1) {             \
    return _v[0];                           \
  }                                         \
                                            \
  if (in_i >= ns_i - 1) {                   \
    return _v[_size - 1];                   \
  }                                         \
                                            \
  /** speedup check **/                     \
  if (ns_i == _size) { /* no extrapolating or decimating needed */  \
    return _v[in_i];                        \
  }                                         \
                                            \
  double fj = in_i * double(_size - 1) / double(ns_i-1); /* scaled index */ \
                                            \
  int j = int(floor(fj)); /* index of sample one lower */ \
  assert(j+1 < _size && j >= 0);            \
  if (_v[j + 1] != _v[j + 1] || _v[j] != _v[j]) { \
    return KST::NOPOINT;                    \
  }                                         \
                                            \
  double fdj = fj - float(j); /* fdj is fraction between _v[j] and _v[j+1] */ \
                                            \
  return _v[j + 1] * fdj + _v[j] * (1.0 - fdj);


// FIXME: optimize me - possible that floor() (especially) and isnan() are
//        expensive here.
/** Return v[i], i is sample number, interpolated to have ns_i total
    samples in vector */
double KstVector::interpolate(int in_i, int ns_i) const {
  GENERATE_INTERPOLATION
}


double kstInterpolate(double *_v, int _size, int in_i, int ns_i) {
  GENERATE_INTERPOLATION
}

#undef GENERATE_INTERPOLATION

double KstVector::value(int i) {
  if (i < 0 || i >= _size) { // can't look before beginning or past end
    return 0.0;
  }
  return _v[i];
}

void KstVector::CreateScalars() {
  if (!_isScalarList) {
    _min = _max = _mean = _minPos = 0.0;
    KstScalarPtr sp;
    _scalars.insert("max", sp = new KstScalar(tagName() + "-Max"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("min", sp = new KstScalar(tagName() + "-Min"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("last", sp = new KstScalar(tagName() + "-Last"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("mean", sp = new KstScalar(tagName() + "-Mean"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("sigma", sp = new KstScalar(tagName() + "-Sigma"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("rms", sp = new KstScalar(tagName() + "-Rms"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("ns", sp = new KstScalar(tagName() + "-NS"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("sum", sp = new KstScalar(tagName() + "-Sum"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("sumsquared", sp = new KstScalar(tagName() + "-SumSquared"));
    sp->_KShared_ref();
    sp->setProvider(this);
    _scalars.insert("minpos", sp = new KstScalar(tagName() + "-MinPos"));
    sp->_KShared_ref();
    sp->setProvider(this);
  }
}

void KstVector::RenameScalars() {
  if (!_isScalarList) {
    _scalars["max"]->setTagName(tagName() + "-Max");
    _scalars["min"]->setTagName(tagName() + "-Min");
    _scalars["last"]->setTagName(tagName() + "-Last");
    _scalars["mean"]->setTagName(tagName() + "-Mean");
    _scalars["sigma"]->setTagName(tagName() + "-Sigma");
    _scalars["rms"]->setTagName(tagName() + "-Rms");
    _scalars["ns"]->setTagName(tagName() + "-NS");
    _scalars["sum"]->setTagName(tagName() + "-Sum");
    _scalars["sumsquared"]->setTagName(tagName() + "-SumSquared");
    _scalars["minpos"]->setTagName(tagName() + "-MinPos");
  }
}

void KstVector::updateScalars() {
  if (!_isScalarList) {
    _scalars["ns"]->setValue(_size);

    if (_nsum >= 2) {
      double sum = _scalars["sum"]->value();
      double sumsq = _scalars["sumsquared"]->value();
      _scalars["mean"]->setValue(_mean = sum/double(_nsum));
      _scalars["sigma"]->setValue(sqrt((sumsq - sum * sum / double(_nsum)) / double(_nsum-1)));
      _scalars["rms"]->setValue(sqrt(sumsq/double(_nsum)));
    } else {
      _scalars["sigma"]->setValue(_max - _min);
      _scalars["rms"]->setValue(sqrt(_scalars["sumsquared"]->value()));
      _scalars["mean"]->setValue(_mean = 0);
    }
  }
}


const QDict<KstScalar>& KstVector::scalars() const {  
  return _scalars;
}


double* KstVector::realloced(double *memptr, int newSize) {
  double *old = _v;
  _v = memptr;
  if (newSize < _size) {
    NumNew = newSize; // all new if we shrunk the vector
  } else {
    NumNew = newSize - _size;
  }
  _size = newSize;
  updateScalars();
  return old;
}


void KstVector::zero() {
  setDirty();
  _ns_min = _ns_max = 0.0;
  memset(_v, 0, sizeof(double)*_size);
  updateScalars();
}


bool KstVector::resize(int sz, bool reinit) {
  //kstdDebug() << "resizing to: " << sz << endl;
  if (sz > 0) {
    _v = static_cast<double*>(KST::realloc(_v, sz*sizeof(double)));
    if (!_v) {
      return false;
    }
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
#else
    abort();  // avoid unpleasant surprises
#endif
    _size = sz;
    updateScalars();
  }

  setDirty();
  return true;
}


KstObject::UpdateType KstVector::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  KstObject::UpdateType providerRC = NO_CHANGE;

  if (update_counter > 0) {
    KstObjectPtr prov = KstObjectPtr(_provider);
    if (prov) {
      prov->writeLock();
      providerRC = prov->update(update_counter);
      prov->writeUnlock();
      if (!force && providerRC == KstObject::NO_CHANGE) {
        return setLastUpdateResult(providerRC);
      }
    }
  }

  KstObject::UpdateType rc = internalUpdate(providerRC);
  setDirty(false);
  return rc;
}


KstObject::UpdateType KstVector::internalUpdate(KstObject::UpdateType providerRC) {
  int i, i0;
  double sum, sum2, last, v;
  double last_v;
  double dv2 = 0.0, dv, no_spike_max_dv;
  _max = _min = sum = sum2 = _minPos = last = 0.0;
  _nsum = 0;
  if (_size > 0) {
    _is_rising = true;

    // FIXME: expensive!!  Can we somehow cache this at least?
    for (i = 0; i < _size && !finite(_v[i]); i++)
      /* Do Nothing */ ;
    if (i == _size) {
      if (!_isScalarList) {
        _scalars["sum"]->setValue(sum);
        _scalars["sumsquared"]->setValue(sum2);
        _scalars["max"]->setValue(_max);
        _scalars["min"]->setValue(_min);
        _scalars["minpos"]->setValue(_minPos);
        _scalars["last"]->setValue(last);
      }
      _ns_max = _ns_min = 0;

      updateScalars();
      return setLastUpdateResult(providerRC);
    }

    i0 = i;

    if (i0 > 0) {
      _is_rising = false;
    }

    _max = _min = _v[i0];
    sum = sum2 = 0.0;

    if (_v[i0] > 0) {
      _minPos = _v[i0];
    } else {
      _minPos = 1.0E300;
    }

    last_v = _v[i0];
    for (i = i0; i < _size; i++) {
      v = _v[i]; // get rid of redirections

      if (finite(v)) {
        dv = v - last_v;
        dv2 += dv*dv;

        if (v <= last_v) {
          if (i != i0) {
            _is_rising = false;
          }
        }

        last_v = v;

        _nsum++;
        sum += v;
        sum2 += v*v;

        if (v > _max) {
          _max = v;
        } else if (v < _min) {
          _min = v;
        }
        if (v < _minPos && v > 0) {
            _minPos = v;
        }
      } else {
        _is_rising = false;
      }
    }

    no_spike_max_dv = 7.0*sqrt(dv2/double(_nsum));

    _ns_max = _ns_min = last_v = _v[i0];

    last = _v[_size-1];

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
          if (i < _size) {
            last_v = _v[i];
          }
          i++;
        }
      }
    }

    if (_isScalarList) {
      _max = _min = _minPos = 0.0;
    } else {
      _scalars["sum"]->setValue(sum);
      _scalars["sumsquared"]->setValue(sum2);
      _scalars["max"]->setValue(_max);
      _scalars["min"]->setValue(_min);
      _scalars["minpos"]->setValue(_minPos);
      _scalars["last"]->setValue(last);
    }

    updateScalars();

    return setLastUpdateResult(providerRC);
  }

  return setLastUpdateResult(NO_CHANGE);
}

void KstVector::save(QTextStream &ts, const QString& indent, bool saveAbsolutePosition) {
  Q_UNUSED(saveAbsolutePosition)
    Q_UNUSED(ts)
    Q_UNUSED(indent)
    abort();
}


void KstVector::setTagName(const QString& newTag) {
  KstObject::setTagName(newTag);
  RenameScalars();
}


void KstVector::setNewAndShift(int inNew, int inShift) {
  NumNew = inNew;
  NumShifted = inShift;
}


QString KstVector::label() const {
  return _label; // default
}


QString KstVector::fileLabel() const {
  return QString::null;
}


double *const KstVector::value() const {
  return _v;
}


void KstVector::newSync() {
  NumNew = NumShifted = 0;
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
    t = KST::suggestVectorName("X(" + QString::number(x0) + ".." + QString::number(x1) + ")");
  }

  KstVectorPtr xv = new KstVector(t, n);
  KST::addVectorToList(xv);
  xv->_saveable = false;

  for (int i = 0; i < n; i++) {
    xv->value()[i] = x0 + double(i) * (x1 - x0) / (n - 1);
  }

  xv->_scalars["min"]->setValue(x0);
  xv->_scalars["max"]->setValue(x1);
  xv->updateScalars();

  return xv;
}


void KstVector::setProvider(KstObject* obj) {
  _provider = obj;
}


void KstVector::setLabel(const QString& label_in) {
  _label = label_in;
}


int KstVector::getUsage() const {
  int adj = 0;
  for (QDictIterator<KstScalar> it(_scalars); it.current(); ++it) {
    adj += it.current()->getUsage() - 1;
  }
  return KstObject::getUsage() + adj;
}


bool KstVector::saveable() const {
  return _saveable;
}


bool KstVector::editable() const {
  return _editable;
}


void KstVector::setEditable(bool editable) {
  _editable = editable;
}


#include "kstvector.moc"
// vim: et sw=2 ts=2
