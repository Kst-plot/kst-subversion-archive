/***************************************************************************
                             binnedmap.cpp
                             -------------------
    begin                : 04/13/06
    copyright            : (C) 2007 C. Barth Netterfield
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
#include "binnedmap.h"

#include <QTextDocument>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include <kstdatacollection.h>

#include "binnedmapdialog_i.h"


static const QString& VECTOR_X = KGlobal::staticQString("Vector X");
static const QString& VECTOR_Y = KGlobal::staticQString("Vector Y");
static const QString& VECTOR_Z = KGlobal::staticQString("Vector Z");

static const QString& MAP = KGlobal::staticQString("Binned Map");
static const QString& HITSMAP = KGlobal::staticQString("Hits Map");

static const QString& XMIN = KGlobal::staticQString("xMin");
static const QString& XMAX = KGlobal::staticQString("xMax");
static const QString& YMIN = KGlobal::staticQString("yMin");
static const QString& YMAX = KGlobal::staticQString("yMax");
static const QString& NX = KGlobal::staticQString("nX");
static const QString& NY = KGlobal::staticQString("nY");

static const QString& AUTOBIN = KGlobal::staticQString("autobin");

KST_KEY_DATAOBJECT_PLUGIN( binnedmap )

K_EXPORT_COMPONENT_FACTORY( kstobject_binnedmap, KGenericFactory<BinnedMap>( "kstobject_binnedmap" ) )

BinnedMap::BinnedMap( QObject */*parent*/, const QStringList &/*args*/ )
    : KstDataObject() {
  _typeString = i18n("Plugin");
  _type = "Plugin";

  setXMin(-1.0);
  setYMin(-1.0);
  setXMax(1.0);
  setYMax(1.0);
  setNX(1);
  setNY(1);
  setAutoBin(false);
}

BinnedMap::~BinnedMap() {
}

bool BinnedMap::isValid() const {
  if (X() && Y() && Z()) {
    return true;
  }

  return false;
}

QString BinnedMap::xTag() const {
  KstVectorPtr v = X();

  if (v) {
    return v->tagName();
  }

  return QString::null;
}

QString BinnedMap::yTag() const {
  KstVectorPtr v = Y();

  if (v) {
    return v->tagName();
  }

  return QString::null;
}

QString BinnedMap::zTag() const {
  KstVectorPtr v = Z();

  if (v) {
    return v->tagName();
  }

  return QString::null;
}

QString BinnedMap::mapTag() const {
  KstMatrixPtr m = map();

  if (m) {
    return m->tagName();
  }

  return QString::null;
}

QString BinnedMap::hitsMapTag() const {
  KstMatrixPtr m = hitsMap();

  if (m) {
    return m->tagName();
  }

  return QString::null;
}

KstVectorPtr BinnedMap::X() const {
  return *_inputVectors.find(VECTOR_X);
}

KstVectorPtr BinnedMap::Y() const {
  return *_inputVectors.find(VECTOR_Y);
}

KstVectorPtr BinnedMap::Z() const {
  return *_inputVectors.find(VECTOR_Z);
}

KstMatrixPtr BinnedMap::map() const {
  return *_outputMatrices.find(MAP);
}

KstMatrixPtr BinnedMap::hitsMap() const {
  return *_outputMatrices.find(HITSMAP);
}

void BinnedMap::setX(KstVectorPtr new_x) {
  if (new_x) {
    _inputVectors[VECTOR_X] = new_x;
  } else {
    _inputVectors.remove(VECTOR_X);
  }
  setDirty();
}

void BinnedMap::setY(KstVectorPtr new_y) {
  if (new_y) {
    _inputVectors[VECTOR_Y] = new_y;
  } else {
    _inputVectors.remove(VECTOR_Y);
  }
  setDirty();
}

void BinnedMap::setZ(KstVectorPtr new_z) {
  if (new_z) {
    _inputVectors[VECTOR_Z] = new_z;
  } else {
    _inputVectors.remove(VECTOR_Z);
  }
  setDirty();
}

void BinnedMap::setMap(const QString &name) {
  QString tname;
  KstMatrixPtr m;

  if (name.isEmpty()) {
    tname = i18n("map");
  } else {
    tname = name;
  }
  
  KstWriteLocker blockMatrixUpdates(&KST::matrixList.lock());
  
  m = new KstMatrix(KstObjectTag(tname, tag()), this);
  _outputMatrices.insert(MAP, m);
}

void BinnedMap::setHitsMap(const QString &name) {
  QString tname;
  KstMatrixPtr m;

  if (name.isEmpty()) {
    tname = i18n("hits map");
  } else {
    tname = name;
  }

  KstWriteLocker blockMatrixUpdates(&KST::matrixList.lock());

  m = new KstMatrix(KstObjectTag(tname, tag()), this);
  _outputMatrices.insert(HITSMAP, m);
}

KstObject::UpdateType BinnedMap::update(int updateCounter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  if (!isValid()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  if (!map() || !hitsMap()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool depUpdated = force;

  writeLockInputsAndOutputs();

  depUpdated = UPDATE == X()->update(updateCounter) || depUpdated;
  depUpdated = UPDATE == Y()->update(updateCounter) || depUpdated;
  depUpdated = UPDATE == Z()->update(updateCounter) || depUpdated;

  if (depUpdated) {
    binnedmap();

    map()->setDirty();
    map()->update(updateCounter);
    hitsMap()->setDirty();
    hitsMap()->update(updateCounter);
  }
  unlockInputsAndOutputs();

  return setLastUpdateResult(depUpdated ? UPDATE : NO_CHANGE);
}


void BinnedMap::binnedmap() {
  KstVectorPtr x = *_inputVectors.find(VECTOR_X);
  KstVectorPtr y = *_inputVectors.find(VECTOR_Y);
  KstVectorPtr z = *_inputVectors.find(VECTOR_Z);
  KstMatrixPtr map = *_outputMatrices.find(MAP);
  KstMatrixPtr hitsMap = *_outputMatrices.find(HITSMAP);
  KstScalarPtr autobin = *_inputScalars.find(AUTOBIN);

  if (autobin) {
    if (autobin->value() != 0.0) {
      _autoBin = true;
    } else {
      _autoBin = false;
    }
  }

  if (_autoBin) {
    double minx, miny, maxx, maxy;
    int nx, ny;

    autoSize(X(), Y(), &nx, &minx, &maxx, &ny, &miny, &maxy);

    setNX(nx);
    setNY(ny);
    setXMin(minx);
    setXMax(maxx);
    setYMin(miny);
    setYMax(maxy);
  } else {
    KstScalarPtr xmin = *_inputScalars.find(XMIN);
    KstScalarPtr xmax = *_inputScalars.find(XMAX);
    KstScalarPtr ymin = *_inputScalars.find(YMIN);
    KstScalarPtr ymax = *_inputScalars.find(YMAX);
    KstScalarPtr nx = *_inputScalars.find(NX);
    KstScalarPtr ny = *_inputScalars.find(NY);

    if (xmin) {
      _xMin = xmin->value();
    }
    if (xmax) {
      _xMax = xmax->value();
    }
    if (ymin) {
      _yMin = ymin->value();
    }
    if (ymax) {
      _yMax = ymax->value();
    }
    if (nx) {
      _nx = (int)nx->value();
    }
    if (ny) {
      _ny = (int)ny->value();
    }
  }

  bool needsresize = false;

  if (_nx < 2) {
    _nx = 2;
    needsresize = true;
  }
  if (_ny < 2) {
    _ny = 2;
    needsresize = true;
  }

  if ((map->xNumSteps() != _nx) || (map->yNumSteps() != _ny) ||
       (map->minX() != _xMin) || (map->minY() != _yMin)) {
    needsresize = true;
  }

  if (map->xStepSize() != (_xMax - _xMin)/double(_nx-1)) {
    needsresize = true;
  }
  if (map->yStepSize() != (_yMax - _yMin)/double(_ny-1)) {
    needsresize = true;
  }

  if (needsresize) {
    map->change(map->tag(), _nx, _ny, _xMin, _yMin, 
                (_xMax - _xMin)/double(_nx-1), (_yMax - _yMin)/double(_ny-1));
    map->resize(_nx, _ny);
    hitsMap->change(hitsMap->tag(), _nx, _ny, _xMin, _yMin,
                (_xMax - _xMin)/double(_nx-1), (_yMax - _yMin)/double(_ny-1));
    hitsMap->resize(_nx, _ny);
  }

  map->zero();
  hitsMap->zero();

  int ns = z->length(); // the z vector defines the number of points.
  double n,p, x0, y0, z0;

  for (int i=0; i<ns; i++) {
    x0 = x->interpolate(i, ns);
    y0 = y->interpolate(i, ns);
    z0 = z->interpolate(i, ns);
    p = map->value(x0, y0)+z0;
    map->setValue(x0, y0, p);
    n = hitsMap->value(x0, y0)+1;
    hitsMap->setValue(x0, y0, n);
  }

  for (int i=0; i<_nx; i++) {
    for (int j=0; j<_ny; j++) {
      p = map->valueRaw(i, j);
      n = hitsMap->valueRaw(i, j);
      if (n>0) {
        map->setValueRaw(i, j, p/n);
      } else {
        map->setValueRaw(i, j, KST::NOPOINT);
      }
    }
  }
}


QString BinnedMap::propertyString() const {
  return "Binned Map";
}


KstDataObjectPtr BinnedMap::makeDuplicate(KstDataObjectDataObjectMap&) {
  return KstDataObjectPtr();
}


void BinnedMap::showNewDialog() {
  BinnedMapDialogI *dialog;

  dialog = new BinnedMapDialogI;
  dialog->show();
}


void BinnedMap::showEditDialog() {
  BinnedMapDialogI *dialog;

  dialog = new BinnedMapDialogI;
  dialog->showEdit(tagName());
}


void BinnedMap::load(const QDomElement &e) {
  QDomNode n = e.firstChild();

  setAutoBin(false);

  while (!n.isNull()) {
    QDomElement e = n.toElement();

    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(KstObjectTag::fromString(e.text()));
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "omatrix") {
        KstWriteLocker blockMatrixUpdates(&KST::matrixList.lock());
        KstMatrixPtr m;

        m = new KstMatrix(KstObjectTag(e.text(), tag()), this);
        _outputMatrices.insert(e.attribute("name"), m);
      } else if (e.tagName() == "minX") {
        setXMin(e.text().toDouble());
      } else if (e.tagName() == "maxX") {
        setXMax(e.text().toDouble());
      } else if (e.tagName() == "minY") {
        setYMin(e.text().toDouble());
      } else if (e.tagName() == "maxY") {
        setYMax(e.text().toDouble());
      } else if (e.tagName() == "nX") {
        setNX(e.text().toInt());
      } else if (e.tagName() == "nY") {
        setNY(e.text().toInt());
      } else if (e.tagName() == "autoBin") {
        setAutoBin(true);
      }
    }
    n = n.nextSibling();
  }
}

void BinnedMap::save(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  KstVectorMap::iterator iv;
  KstMatrixMap::Iterator im;

  ts << indent << "<plugin name=\"Binned Map\">" << endl;
  ts << l2 << "<tag>" << Qt::escape(tagName()) << "</tag>" << endl;

  for (iv = _inputVectors.begin(); iv != _inputVectors.end(); ++iv) {
    ts << l2 << "<ivector name=\"" << Qt::escape(iv.key()) << "\">"
        << Qt::escape((*iv)->tagName())
        << "</ivector>" << endl;
  }

  for (im = _outputMatrices.begin(); im != _outputMatrices.end(); ++im) {
    ts << l2 << "<omatrix name=\"" << Qt::escape(im.key());
    ts << "\">" << Qt::escape((*im)->tagName())
        << "</omatrix>" << endl;
  }
  ts << 12 << "<minX>" << xMin() << "</minX>" << endl;
  ts << 12 << "<maxX>" << xMax() << "</maxX>" << endl;
  ts << 12 << "<minY>" << yMin() << "</minY>" << endl;
  ts << 12 << "<maxY>" << yMax() << "</maxY>" << endl;
  ts << 12 << "<nX>" << nX() << "</nX>" << endl;
  ts << 12 << "<nY>" << nY() << "</nY>" << endl;
  if (autoBin()) {
    ts << 12 << "<autoBin/>" << endl;
  }

  ts << indent << "</plugin>" << endl;
}

void BinnedMap::setXMin(double xmin) {
  if (_inputScalars.find(XMIN) != _inputScalars.end()) {
    _inputScalars[XMIN]->setValue(xmin);
  }
  _xMin = xmin;
}

void BinnedMap::setXMax(double xmax) {
  if (_inputScalars.find(XMAX) != _inputScalars.end()) {
    _inputScalars[XMAX]->setValue(xmax);
  }
  _xMax = xmax;
}

void BinnedMap::setYMin(double ymin) {
  if (_inputScalars.find(YMIN) != _inputScalars.end()) {
    _inputScalars[YMIN]->setValue(ymin);
  }
  _yMin = ymin;
}

void BinnedMap::setYMax(double ymax) {
  if (_inputScalars.find(YMAX) != _inputScalars.end()) {
    _inputScalars[YMAX]->setValue(ymax);
  }
  _yMax = ymax;
}

double BinnedMap::xMin() const {
  return _xMin;
}

double BinnedMap::xMax() const {
  return _xMax;
}

double BinnedMap::yMin() const {
  return _yMin;
}

double BinnedMap::yMax() const {
  return _yMax;
}

void BinnedMap::setNX(int nx) {
  if (_inputScalars.find(NX) != _inputScalars.end()) {
    _inputScalars[NX]->setValue(nx);
  }
  _nx = nx;
}

void BinnedMap::setNY(int ny) {
  if (_inputScalars.find(NY) != _inputScalars.end()) {
    _inputScalars[NY]->setValue(ny);
  }
  _ny = ny;
}

int BinnedMap::nX() const {
  return _nx;
}

int BinnedMap::BinnedMap::nY() const {
  return _ny;
}

void BinnedMap::setAutoBin(bool autobin) {
  if (_inputScalars.find(AUTOBIN) != _inputScalars.end()) {
    if (autobin) {
      _inputScalars[AUTOBIN]->setValue(1.0);
    } else {
      _inputScalars[AUTOBIN]->setValue(0.0);
    }
  }
  _autoBin = autobin;
}

bool BinnedMap::autoBin() const {
  return _autoBin;
}

void BinnedMap::autoSize(KstVectorPtr x, KstVectorPtr y, int *nx, double *minx, double *maxx, int *ny, double *miny, double *maxy) {

  // use a very simple guess at nx and ny... One could imagine something more
  // clever in principle.
  *nx = *ny = (int)sqrt(x->length())/2;
  if (*nx < 2) {
    *nx = 2;
  }
  if (*ny < 2) {
    *ny = 2;
  }

  *minx = x->min();
  *maxx = x->max();
  *miny = y->min();
  *maxy = y->max();
}


#include "binnedmap.moc"
