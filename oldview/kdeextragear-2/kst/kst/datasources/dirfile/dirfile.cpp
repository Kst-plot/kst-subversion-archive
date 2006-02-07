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

#include "dirfile.h"
#include "getdata.h"
#include "getdata_struct.h"

 
DirFileSource::DirFileSource(const QString& filename, const QString& type)
: KstDataSource(filename, type) {
  if (init()) {
    _valid = true;
  }
}


DirFileSource::~DirFileSource() {
}


bool DirFileSource::init() {
  int err = 0;

  _frameCount = 0;
  struct FormatType *ft = GetFormat(_filename.latin1(), &err);

  if (err == GD_E_OK) {
    for (int i = 0; i < ft->n_lincom; i++) {
      _fieldList.append(ft->lincomEntries[i].field);
    }

    for (int i = 0; i < ft->n_linterp; i++) {
      _fieldList.append(ft->linterpEntries[i].field);
    }

    for (int i = 0; i < ft->n_bit; i++) {
      _fieldList.append(ft->bitEntries[i].field);
    }

    for (int i = 0; i < ft->n_raw; i++) {
      _fieldList.append(ft->rawEntries[i].field);
    }
  }
  return update() == KstObject::UPDATE;
}


KstObject::UpdateType DirFileSource::update(int u) {
  Q_UNUSED(u)
  int err = 0;
  int newNF = GetNFrames(_filename.latin1(), &err, 0L);
  bool isnew = newNF != _frameCount;

  _frameCount = newNF;

  return isnew ? KstObject::UPDATE : KstObject::NO_CHANGE;
}


int DirFileSource::readField(double *v, const QString& field, int s, int n) {
  int err = 0;

  if (n < 0) {
    return GetData(_filename.latin1(), field.left(16).latin1(),
                   s, 0, /* 1st sframe, 1st samp */
                   0, 1, /* num sframes, num samps */
                   'd', (void*)v,
                   &err);
  } else {
    return GetData(_filename.latin1(), field.left(16).latin1(),
                   s, 0, /* 1st sframe, 1st samp */
                   n, 0, /* num sframes, num samps */
                   'd', (void*)v,
                   &err);
  }
}


bool DirFileSource::isValidField(const QString& field) const {
  int err = 0;
  GetSamplesPerFrame(_filename.latin1(), field.left(16).latin1(), &err);
  return err == 0;
}


int DirFileSource::samplesPerFrame(const QString &field) {
  int err = 0;
  return GetSamplesPerFrame(_filename.latin1(), field.left(16).latin1(), &err);
}


int DirFileSource::frameCount() const {
  return _frameCount;
}


QString DirFileSource::fileType() const {
  return "Directory of Binary Files";
}


void DirFileSource::save(QTextStream &ts) {
  KstDataSource::save(ts);
}

#include <kdebug.h>

extern "C" {
KstDataSource *create_dirfile(const QString& filename, const QString& type) {
  return new DirFileSource(filename, type);
}

QStringList provides_dirfile() {
  QStringList rc;
  // create the stringlist
  rc += "Directory of Binary Files";
  return rc;
}

bool understands_dirfile(const QString& filename) {
  // FIXME: GetNFrames causes a memory error here.  I think it is due to
  // the lfilename parameter.
  int err = 0;
  int frameCount = GetNFrames(filename.latin1(), &err, 0L);
  if (frameCount > 0 && err == GD_E_OK) {
    return true;
  }

  kdDebug() << "Don't understand.  FrameCount=" << frameCount << " err=" << err << endl;
  return false;
}

}

// vim: ts=2 sw=2 et
