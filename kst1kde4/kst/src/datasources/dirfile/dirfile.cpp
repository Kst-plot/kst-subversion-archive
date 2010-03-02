/***************************************************************************
                 dirfile.cpp  -  data source for dirfiles
                             -------------------
    begin                : Tue Oct 21 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dirfile.h"

#ifdef HAVE_EXTERNAL_GETDATA
#include <getdata.h>
#include <getdata_struct.h>
#define FIELD_LENGTH 1024
#else
#include "old_getdata.h"
#include "old_getdata_struct.h"
#endif

#include "kstdebug.h"


DirFileSource::DirFileSource(KConfig *cfg, const QString& filename, const QString& type)
: KstDataSource(cfg, filename, type) {
  if (init()) {
    _valid = true;
  }
}


DirFileSource::~DirFileSource() {
}


bool DirFileSource::reset() {
  _fieldList.clear();
  init();

  return true;
}


bool DirFileSource::init() {
  int err = 0;

  _frameCount = 0;
  FormatType *ft = GetFormat(_filename.toLatin1(), &err);

  if (err == GD_E_OK) {
    _fieldList.append("INDEX");
    for (int i = 0; i < ft->n_lincom; i++) {
      _fieldList.append(ft->lincomEntries[i].field);
    }

    for (int i = 0; i < ft->n_multiply; i++) {
      _fieldList.append(ft->multiplyEntries[i].field);
    }

    for (int i = 0; i < ft->n_linterp; i++) {
      _fieldList.append(ft->linterpEntries[i].field);
    }

    for (int i = 0; i < ft->n_bit; i++) {
      _fieldList.append(ft->bitEntries[i].field);
    }

    for (int i = 0; i < ft->n_phase; i++) {
      _fieldList.append(ft->phaseEntries[i].field);
    }

    for (int i = 0; i < ft->n_raw; i++) {
      _fieldList.append(ft->rawEntries[i].field);
    }

    _writable = true;
  } else {
    char error[200];

    GetDataErrorString(error, 200);
    KstDebug::self()->log(error, KstDebug::Error);
  }

  return update() == KstObject::UPDATE;
}


KstObject::UpdateType DirFileSource::update(int u) {
  if (KstObject::checkUpdateCounter(u)) {
    return lastUpdateResult();
  }

  int err = 0;
  int newNF;

  newNF = GetNFrames(_filename.toLatin1(), &err, 0L);
  if (newNF < 0) {
    newNF = 0;
  }
  bool isnew = newNF != _frameCount;

  _frameCount = newNF;

  updateNumFramesScalar();

  return setLastUpdateResult(isnew ? KstObject::UPDATE : KstObject::NO_CHANGE);
}


int DirFileSource::readField(double *v, const QString& field, int s, int n) {
  int err = GD_E_OK;
  int read;

  if (n < 0) {
    read = GetData(_filename.toLatin1(), field.left(FIELD_LENGTH).toLatin1(),
                   s, 0, /* 1st sframe, 1st samp */
                   0, 1, /* num sframes, num samps */
                   'd', (void*)v,
                   &err);
  } else {
    read = GetData(_filename.toLatin1(), field.left(FIELD_LENGTH).toLatin1(),
                   s, 0, /* 1st sframe, 1st samp */
                   n, 0, /* num sframes, num samps */
                   'd', (void*)v,
                   &err);
  }

  if (err != GD_E_OK) {
    if (_errors.find(field) == 0L) {
      char error[200];

      _errors.insert(field, (int*)1L);

      GetDataErrorString(error, 200);
      KstDebug::self()->log(error, KstDebug::Error);
    }
  }

  return read;
}


int DirFileSource::writeField(const double *v, const QString& field, int s, int n) {
  int err = 0;

  return PutData(_filename.toLatin1(), field.left(FIELD_LENGTH).toLatin1(),
      s, 0, /* 1st sframe, 1st samp */
      n, 0, /* num sframes, num samps */
      'd', (void*)v,
      &err);
}


bool DirFileSource::isValidField(const QString& field) const {
  int err = GD_E_OK;
  GetSamplesPerFrame(_filename.toLatin1(), field.left(FIELD_LENGTH).toLatin1(), &err);

  if (err != GD_E_OK) {
    char error[200];

    GetDataErrorString(error, 200);
    KstDebug::self()->log(error, KstDebug::Error);
  }

  return err == GD_E_OK;
}


int DirFileSource::samplesPerFrame(const QString &field) {
  int samples = 0;
  int err = GD_E_OK;

  samples = GetSamplesPerFrame(_filename.toLatin1(), field.left(FIELD_LENGTH).toLatin1(), &err);

  if (err != GD_E_OK) {
    char error[200];

    GetDataErrorString(error, 200);
    KstDebug::self()->log(error, KstDebug::Error);
  }

  return samples;
}


int DirFileSource::frameCount(const QString& field) const {
  Q_UNUSED(field)
  return _frameCount;
}


bool DirFileSource::isEmpty() const {
  return _frameCount < 1;
}


QString DirFileSource::fileType() const {
  return "Directory of Binary Files";
}


void DirFileSource::save(QTextStream &ts, const QString& indent) {
  KstDataSource::save(ts, indent);
}

//#include <kdebug.h>

extern "C" {
KstDataSource *create_dirfile(KConfig *cfg, const QString& filename, const QString& type) {
  return new DirFileSource(cfg, filename, type);
}

QStringList provides_dirfile() {
  QStringList rc;
  rc += "Directory of Binary Files";
  return rc;
}

int understands_dirfile(KConfig*, const QString& filename) {
  // FIXME: GetNFrames causes a memory error here.  I think it is due to
  // the lfilename parameter.
  int err = 0;
  int frameCount = GetNFrames(filename.toLatin1(), &err, 0L);
  if (frameCount > 0 && err == GD_E_OK) {
    return 98;
  }

  //kdDebug() << "Don't understand.  filename = [" << filename << "] FrameCount=" << frameCount << " err=" << err << endl;
  return 0;
}


QStringList fieldList_dirfile(KConfig*, const QString& filename, const QString& type, QString *typeSuggestion, bool *complete) {
  Q_UNUSED(type)
  int err = 0;
  struct FormatType *ft = GetFormat(filename.toLatin1(), &err);
  QStringList fieldList;

  if (complete) {
    *complete = true;
  }

  if (typeSuggestion) {
    *typeSuggestion = "Directory of Binary Files";
  }

  if (err == GD_E_OK) {
    fieldList.append("INDEX");
    for (int i = 0; i < ft->n_lincom; i++) {
      fieldList.append(ft->lincomEntries[i].field);
    }

    for (int i = 0; i < ft->n_multiply; i++) {
      fieldList.append(ft->multiplyEntries[i].field);
    }

    for (int i = 0; i < ft->n_linterp; i++) {
      fieldList.append(ft->linterpEntries[i].field);
    }

    for (int i = 0; i < ft->n_bit; i++) {
      fieldList.append(ft->bitEntries[i].field);
    }

    for (int i = 0; i < ft->n_phase; i++) {
      fieldList.append(ft->phaseEntries[i].field);
    }

    for (int i = 0; i < ft->n_raw; i++) {
      fieldList.append(ft->rawEntries[i].field);
    }
  } else {
    char error[200];

    GetDataErrorString(error, 200);
    KstDebug::self()->log(error, KstDebug::Error);
  }

  return fieldList;
}

}

KST_KEY_DATASOURCE_PLUGIN(dirfile)

