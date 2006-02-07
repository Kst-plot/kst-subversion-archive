/***************************************************************************
                    stdinsource.cpp  -  data source for stdin
                             -------------------
    begin                : Fri Oct 31 2003
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

#include "stdinsource.h"
#include <kdebug.h>
#include <ktempfile.h>

#include <stdlib.h>
#include <unistd.h>

 
KstStdinSource::KstStdinSource()
: KstDataSource("stdin", "stdin") {
  _file = new KTempFile();
  _filename = _file->name();
  update();
  _src = KstDataSource::loadSource(_filename, "ASCII");
  if (_src && _src->isValid()) {
    _valid = true;
  }
}


KstStdinSource::~KstStdinSource() {
  _file->close();
  _file->unlink();
  delete _file;
  _file = 0L;
}


KstObject::UpdateType KstStdinSource::update(int u) {
  fd_set rfds;
  struct timeval tv;
  int retval;
  char instr[4097];
  int i = 0;
  bool new_data = false;

  FILE *fp = _file->fstream();

  if (!fp) {
    return KstObject::NO_CHANGE;
  }

  do {
    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    /* Wait up to 0 seconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);

    new_data = false;
    if (retval) {
      char *fgs = fgets(instr, 4096, stdin);
      if (fgs && fp) {
        fputs(instr, fp);
        new_data = true;
      }
    }
  } while (++i < 100000 && new_data);

  fflush(fp);

  if (new_data) {
    return _src ? _src->update(u) : KstObject::NO_CHANGE;
  } else {
    return KstObject::NO_CHANGE;
  }
}


int KstStdinSource::readField(double *v, const QString& field, int s, int n) {
  if (isValid()) {
    return _src->readField(v, field, s, n);
  }
  return -1;
}


bool KstStdinSource::isValidField(const QString& field) const {
  if (isValid()) {
    return _src->isValidField(field);
  }
  return false;
}


int KstStdinSource::samplesPerFrame(const QString &field) {
  if (isValid()) {
    return _src->samplesPerFrame(field);
  }
  return 0;
}


int KstStdinSource::frameCount() const {
  if (isValid()) {
    return _src->frameCount();
  }
  return 0;
}


QString KstStdinSource::fileType() const {
  if (isValid()) {
    return _src->fileType();
  }
  return QString::null;
}


void KstStdinSource::save(QTextStream &ts) {
  if (isValid()) {
    return _src->save(ts);
  }
  KstDataSource::save(ts);
}


bool KstStdinSource::isValid() const {
    return _valid && _src && _src->isValid();
}


// vim: ts=2 sw=2 et
