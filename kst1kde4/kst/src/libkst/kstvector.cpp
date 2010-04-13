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

#include <QTextDocument>

#include "defaultprimitivenames.h"
#include "kstdatacollection.h"
#include "kstmath.h"
#include "kstscalar.h"
#include "kstvector.h"

static int anonymousVectorCounter = 1;

#define INITSIZE 1

KstVector::KstVector(KstObjectTag in_tag, int size, KstObject *provider, bool isScalarList)
: KstPrimitive(provider), _nsum(0) {
  _editable = false;
  _numShifted = 0;
  _numNew = 0;
  _saveData = false;
  _isScalarList = isScalarList;

  _saveable = false;

  if (size <= 0) {
    size = INITSIZE;
  }

  if (!in_tag.isValid()) {
    QString nt = QObject::tr("Anonymous Vector %1");

    do {
      KstObject::setTagName(KstObjectTag(nt.arg(anonymousVectorCounter++), in_tag.context()));
    } while (KstData::self()->vectorTagNameNotUnique(tagName(), false));
  } else {
    KstObject::setTagName(KST::suggestUniqueVectorTag(in_tag));
  }

  _v = static_cast<double*>(KST::malloc(size * sizeof(double)));
  if (!_v) {
    _v = static_cast<double*>(KST::malloc(sizeof(double)));
    _size = 1;
  } else {
    _size = size;
  }
  _is_rising = false;

  createScalars();
  blank();

  KST::vectorList.lock().writeLock();
  KST::vectorList.append(this);
  KST::vectorList.lock().unlock();
}


KstVector::KstVector(const QDomElement& e)
: KstPrimitive(), _nsum(0) {
  QDomNode n = e.firstChild();
  KstObjectTag in_tag = KstObjectTag::invalidTag;
  QByteArray qba;
  int sz = INITSIZE;  

  _v = 0L;
  _size = 0;
  _editable = false;
  _numShifted = 0;
  _numNew = 0;
  _isScalarList = false;
  _saveable = false;
  _saveData = false;

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        in_tag = KstObjectTag::fromString(e.text());
      } else if (e.tagName() == "data") {
        QByteArray qcs(e.text().toAscii());
        QByteArray qbca = QByteArray::fromBase64(qcs);
        
        qba = qUncompress(qbca);
        sz = qMax((size_t)(INITSIZE), qba.size()/sizeof(double));
      }
    }
    n = n.nextSibling();
  }

  if (!in_tag.isValid()) {
    QString nt = QObject::tr("Anonymous Vector %1");

    do {
      KstObject::setTagName(KstObjectTag(nt.arg(anonymousVectorCounter++), in_tag.context()));
    } while (KstData::self()->vectorTagNameNotUnique(tagName(), false));
  } else {
    KstObject::setTagName(KST::suggestUniqueVectorTag(in_tag));
  }

  createScalars();
  resize(sz, true);

  if (!qba.isEmpty()) {
    QDataStream qds(&qba, QIODevice::ReadOnly);
    
    _saveable = true;
    _saveData = true;

    for (int i = 0; !qds.atEnd(); ++i) {
      qds >> _v[i];
    }
  }

  _is_rising = false;

  KST::vectorList.lock().writeLock();
  KST::vectorList.append(this);
  KST::vectorList.lock().unlock();
}


KstVector::~KstVector() {
  KST::scalarList.lock().writeLock();
  KST::scalarList.setUpdateDisplayTags(false);

  ScalarCollection::iterator it = _scalars.begin();
  
  while (it != _scalars.end()) {
    KST::scalarList.remove(it.value().data());
    it = _scalars.erase(it);
  }

  KST::scalarList.setUpdateDisplayTags(true);
  KST::scalarList.lock().unlock();

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

#define RETURN_FIRST_NON_HOLE               \
    for (int i = 0; i < _size; ++i) {       \
      if (_v[i] == _v[i]) {                 \
        return _v[i];                       \
      }                                     \
    }                                       \
    return 0.;

#define RETURN_LAST_NON_HOLE                \
    for (int i = _size - 1; i >= 0; --i) {  \
      if (_v[i] == _v[i]) {                 \
        return _v[i];                       \
      }                                     \
    }                                       \
    return 0.;

#define FIND_LEFT(val, idx)                 \
    for (; idx >= 0; --idx) {               \
      if (_v[idx] == _v[idx]) {             \
        val = _v[idx]; break;               \
      }                                     \
    }

#define FIND_RIGHT(val, idx)                \
    for (; idx < _size; ++idx) {            \
      if (_v[idx] == _v[idx]) {             \
        val = _v[idx]; break;               \
      }                                     \
    }


#define GENERATE_INTERPOLATION              \
  assert(_size > 0);                        \
  /** Limits checks - optional? **/         \
  if (in_i <= 0 || _size == 1) {            \
    RETURN_FIRST_NON_HOLE                   \
  }                                         \
                                            \
  if (in_i >= ns_i - 1) {                   \
    RETURN_LAST_NON_HOLE                    \
  }                                         \
                                            \
  /** speedup check **/                     \
  if (ns_i == _size) {                      \
    if (_v[in_i] == _v[in_i]) {             \
      return _v[in_i];                      \
    }                                       \
    double left = 0., right = 0.;           \
    int leftIndex = in_i, rightIndex = in_i;\
    FIND_LEFT(left, leftIndex)              \
    FIND_RIGHT(right, rightIndex)           \
    if (leftIndex == -1) {                  \
      return right;                         \
    }                                       \
    if (rightIndex == _size) {              \
      return left;                          \
    }                                       \
    return left + (right - left) * double(in_i - leftIndex) / double(rightIndex - leftIndex); \
  }                                         \
  abort(); /* FIXME */                      \
  double indexScaleFactor = double(_size - 1) / double(ns_i - 1); \
  double fj = in_i * indexScaleFactor; /* scaled index */ \
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
double KstVector::interpolateNoHoles(int in_i, int ns_i) const {
  GENERATE_INTERPOLATION
}


double kstInterpolateNoHoles(double *_v, int _size, int in_i, int ns_i) {
  GENERATE_INTERPOLATION
}

#undef FIND_LEFT
#undef FIND_RIGHT
#undef RETURN_LAST_NON_HOLE
#undef RETURN_FIRST_NON_HOLE
#undef GENERATE_INTERPOLATION

double KstVector::value(int i) {
  if (i < 0 || i >= _size) { // can't look before beginning or past end
    return 0.0;
  }
  return _v[i];
}

void KstVector::createScalars() {
  if (!_isScalarList) {
    _min = _max = _mean = _minPos = 0.0;
    _minIndex = -1;
    _maxIndex = -1;

    KstWriteLocker sl(&KST::scalarList.lock());
    KST::scalarList.setUpdateDisplayTags(false);
    KstScalarPtr sp;

    _scalars.insert("max", sp = new KstScalar(KstObjectTag("Max", tag()), this));
    _scalars.insert("min", sp = new KstScalar(KstObjectTag("Min", tag()), this));
    _scalars.insert("maxindex", sp = new KstScalar(KstObjectTag("MaxIndex", tag()), this));
    _scalars.insert("minindex", sp = new KstScalar(KstObjectTag("MinIndex", tag()), this));
    _scalars.insert("last", sp = new KstScalar(KstObjectTag("Last", tag()), this));
    _scalars.insert("first", sp = new KstScalar(KstObjectTag("First", tag()), this));
    _scalars.insert("mean", sp = new KstScalar(KstObjectTag("Mean", tag()), this));
    _scalars.insert("sigma", sp = new KstScalar(KstObjectTag("Sigma", tag()), this));
    _scalars.insert("rms", sp = new KstScalar(KstObjectTag("Rms", tag()), this));
    _scalars.insert("ns", sp = new KstScalar(KstObjectTag("NS", tag()), this));
    _scalars.insert("sum", sp = new KstScalar(KstObjectTag("Sum", tag()), this));
    _scalars.insert("sumsquared", sp = new KstScalar(KstObjectTag("SumSquared", tag()), this));
    _scalars.insert("minpos", sp = new KstScalar(KstObjectTag("MinPos", tag()), this));

    KST::scalarList.setUpdateDisplayTags(true);
  }
}

void KstVector::renameScalars() {
  if (!_isScalarList) {
    KstWriteLocker sl(&KST::scalarList.lock());
    KST::scalarList.setUpdateDisplayTags(false);

    _scalars["max"]->setTagName(KstObjectTag("Max", tag()));
    _scalars["min"]->setTagName(KstObjectTag("Min", tag()));
    _scalars["maxindex"]->setTagName(KstObjectTag("MaxIndex", tag()));
    _scalars["minindex"]->setTagName(KstObjectTag("MinIndex", tag()));
    _scalars["last"]->setTagName(KstObjectTag("Last", tag()));
    _scalars["first"]->setTagName(KstObjectTag("First", tag()));
    _scalars["mean"]->setTagName(KstObjectTag("Mean", tag()));
    _scalars["sigma"]->setTagName(KstObjectTag("Sigma", tag()));
    _scalars["rms"]->setTagName(KstObjectTag("Rms", tag()));
    _scalars["ns"]->setTagName(KstObjectTag("NS", tag()));
    _scalars["sum"]->setTagName(KstObjectTag("Sum", tag()));
    _scalars["sumsquared"]->setTagName(KstObjectTag("SumSquared", tag()));
    _scalars["minpos"]->setTagName(KstObjectTag("MinPos", tag()));

    KST::scalarList.setUpdateDisplayTags(true);
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
      _scalars["mean"]->setValue(_mean = KST::NOPOINT);
      _scalars["sigma"]->setValue(_max - _min);
      _scalars["rms"]->setValue(sqrt(_scalars["sumsquared"]->value()));
    }
  }
}


const ScalarCollection& KstVector::scalars() const {
  return _scalars;
}


double* KstVector::realloced(double *memptr, int newSize) {
  double *old = _v;
  _v = memptr;
  if (newSize < _size) {
    _numNew = newSize; // all new if we shrunk the vector
  } else {
    _numNew = newSize - _size;
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


void KstVector::blank() {
  setDirty();
  _ns_min = _ns_max = 0.0;

  for (int i = 0; i < _size; ++i) {
    _v[i] = KST::NOPOINT;
  }

  updateScalars();
}


bool KstVector::resize(int sz, bool reinit) {
  writeLock();

  if (sz > 0) {
    _v = static_cast<double*>(KST::realloc(_v, sz*sizeof(double)));
    if (!_v) {
      return false;
    }

    if (reinit && _size < sz) {
      for (int i = _size; i < sz; i++) {
        _v[i] = KST::NOPOINT;
      }
    }

    _size = sz;
    updateScalars();
  }

  setDirty();
  unlock();

  return true;
}


KstObject::UpdateType KstVector::internalUpdate(KstObject::UpdateType providerRC) {
  int i, i0;
  double sum, sum2, last, first, v;
  double last_v;
  double dv2 = 0.0, dv, no_spike_max_dv;

  _max = _min = sum = sum2 = _minPos = last = first = KST::NOPOINT;
  _maxIndex = -1;
  _minIndex = -1;
  _nsum = 0;

  if (_size > 0) {
    _is_rising = true;

    // FIXME: expensive!!  Can we somehow cache this at least?
    for (i = 0; i < _size && !finite(_v[i]); i++) {
      // do nothing
    }

    if (i == _size) {
      if (!_isScalarList) {
	_scalars["sum"]->setValue(sum);
        _scalars["sumsquared"]->setValue(sum2);
        _scalars["max"]->setValue(_max);
        _scalars["min"]->setValue(_min);
        _scalars["maxindex"]->setValue(_maxIndex);
        _scalars["minindex"]->setValue(_minIndex);
        _scalars["minpos"]->setValue(_minPos);
        _scalars["last"]->setValue(last);
        _scalars["first"]->setValue(first);
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
    _maxIndex = i0;
    _minIndex = i0;
    sum = sum2 = 0.0;

    if (_v[i0] > 0.0) {
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
          _maxIndex = i;
        } else if (v < _min) {
          _min = v;
          _minIndex = i;
        }
        if (v < _minPos && v > 0.0) {
          _minPos = v;
        }
      } else {
        _is_rising = false;
      }
    }

    no_spike_max_dv = 7.0*sqrt(dv2/double(_nsum));

    _ns_max = _ns_min = last_v = _v[i0];

    last = _v[_size-1];
    first = _v[0];

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
      _maxIndex = 0;
      _minIndex = 0;
    } else {
      _scalars["sum"]->setValue(sum);
      _scalars["sumsquared"]->setValue(sum2);
      _scalars["max"]->setValue(_max);
      _scalars["min"]->setValue(_min);
      _scalars["maxindex"]->setValue(_maxIndex);
      _scalars["minindex"]->setValue(_minIndex);
      _scalars["minpos"]->setValue(_minPos);
      _scalars["last"]->setValue(last);
      _scalars["first"]->setValue(first);
    }

    updateScalars();

    return setLastUpdateResult(providerRC);
  }

  return setLastUpdateResult(NO_CHANGE);
}



void KstVector::save(QTextStream &ts, const QString& indent, bool saveAbsolutePosition) {
  Q_UNUSED(saveAbsolutePosition)
  ts << indent << "<tag>" << Qt::escape(tag().tagString()) << "</tag>" << endl;
  if (_saveData) {
    QByteArray qba(length()*sizeof(double), '\0');
    QDataStream qds(&qba, QIODevice::WriteOnly);

    for (int i = 0; i < length(); i++) {
      qds << _v[i];
    }

    ts << indent << "<data>" << qCompress(qba).toBase64() << "</data>" << endl;
  }
}


void KstVector::setTagName(const KstObjectTag& newTag) {
  if (newTag == tag()) {
    return;
  }

  KstWriteLocker l(&KST::vectorList.lock());

  KST::vectorList.doRename(this, newTag);

  renameScalars();
}


void KstVector::setNewAndShift(int inNew, int inShift) {
  _numNew = inNew;
  _numShifted = inShift;
}


QString KstVector::label() const {
  return _label; // default
}


QString KstVector::fileLabel() const {
  return QString::null;
}


double* KstVector::value() const {
  return _v;
}


void KstVector::newSync() {
  _numNew = _numShifted = 0;
}


KstVectorPtr KstVector::generateVector(double x0, double x1, int n, const KstObjectTag& tag) {
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

  QString t = tag.tag();
  if (t.isEmpty()) {
    t = KST::suggestVectorName("X(" + QString::number(x0) + ".." + QString::number(x1) + ")");
  }

  KstVectorPtr xv;
  new KstVector(KstObjectTag(t, tag.context()), n);
  xv->_saveable = false;

  for (int i = 0; i < n; i++) {
    xv->value()[i] = x0 + double(i) * (x1 - x0) / double(n - 1);
  }

  xv->_scalars["min"]->setValue(x0);
  xv->_scalars["max"]->setValue(x1);
  xv->_scalars["minindex"]->setValue(0);
  xv->_scalars["maxindex"]->setValue(n-1);
  xv->updateScalars();

  return xv;
}


void KstVector::setLabel(const QString& label_in) {
  _label = label_in;
}


int KstVector::getUsage() const {
  QHash<QString, KstScalarPtr>::const_iterator it;
  int adj = 0;

  for (it=_scalars.begin(); it!=_scalars.end(); ++it) {
    adj += (*it)->getUsage() - 1;
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


bool KstVector::saveData() const {
  return _saveData;
}


void KstVector::setSaveData(bool save) {
  _saveData = save;
}

int KstVector::indexNearX(double x, int NS) {
  if (isRising()) {
    // monotonically rising: we can do a binary search
    // should be reasonably fast
    int i_top = NS - 1;
    int i_bot = 0;

    // don't pre-check for x outside of the curve since this is not
    // the common case.  It will be correct - just slightly slower...
    while (i_bot + 1 < i_top) {
      int i0 = (i_top + i_bot)/2;
      double rX = interpolate(i0, NS);
      if (x < rX) {
        i_top = i0;
      } else {
        i_bot = i0;
      }
    }
    double xt = interpolate(i_top, NS);
    double xb = interpolate(i_bot, NS);
    if (xt - x < x - xb) {
      return i_top;
    } else {
      return i_bot;
    }
  } else {
    // Oh Oh... not monotonically rising - we have to search the entire curve!
    // May be unbearably slow for large vectors
    double rX = interpolate(0, NS);
    double dx0 = fabs(x - rX);
    int i0 = 0;

    for (int i = 1; i < NS; ++i) {
      rX = interpolate(i, NS);
      double dx = fabs(x - rX);
      if (dx < dx0) {
        dx0 = dx;
        i0 = i;
      }
    }
    return i0;
  }
}

#undef INITSIZE

#include "kstvector.moc"
