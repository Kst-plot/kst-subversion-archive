/***************************************************************************
                     ascii.cpp  -  ASCII file data source
                             -------------------
    begin                : Fri Oct 17 2003
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

#include "ascii.h"

#include <kdebug.h>

#include <qfile.h>
#include <qregexp.h>

#include <ctype.h>
#include <stdlib.h>


AsciiSource::AsciiSource(const QString& filename, const QString& type)
  : KstDataSource(filename, type) {
    if (!type.isEmpty() && type != "ASCII") {
      return;
    }

    _rowIndex = 0L;
    if (initFile()) {
      _valid = true;
    }
}


AsciiSource::~AsciiSource() {
  if (_rowIndex) {
    free(_rowIndex);
    _rowIndex = 0L;
    _numLinesAlloc = 0;
  }
}


bool AsciiSource::initFile() {
  if (!_rowIndex) {
    _rowIndex = (int *)malloc(32768 * sizeof(int));
    _rowIndex[0] = 0;
    _numLinesAlloc = 32768;
    _numFrames = 0;
    _byteLength = 0;
  }

  // Allow files to start out empty (stdin, for instance)
  //return update() == KstObject::UPDATE;
  update();
  return true;
}


#define MAXBUFREADLEN 32768
KstObject::UpdateType AsciiSource::update(int u) {
  Q_UNUSED(u)
  static char *tmpbuf = 0L;
  QFile file(_filename);

  if (file.exists()) {
    _byteLength = file.size();
  } else {
    _valid = false;
    return KstObject::NO_CHANGE;
  }

  if (!file.open(IO_ReadOnly)) {
    // quietly fail - no data to be had here
    _valid = false;
    return KstObject::NO_CHANGE;
  }

  if (!tmpbuf) {
    tmpbuf = new char[MAXBUFREADLEN];
  }

  int bufstart, bufread;
  bool new_data = false;
  do {
    /* Read the tmpbuffer, starting at row_index[_numFrames] */
    if (_byteLength - _rowIndex[_numFrames] > MAXBUFREADLEN) {
      bufread = MAXBUFREADLEN;
    } else {
      bufread = _byteLength - _rowIndex[_numFrames];
    }

    bufstart = _rowIndex[_numFrames];
    file.at(bufstart); // expensive?
    file.readBlock(tmpbuf, bufread);

    bool is_comment = false, has_dat = false;
    for (int i = 0; i < bufread; i++) {
      // Statistically numbers should be more common than comments, so let's
      // check for them first
      // if isdigit() generates a function call, it would be better to write
      // this inline, but I'm not convinced that compilers are that stupid
      // yet.  Profiling demo.kst shows this code path is hit > 10^5 times.
      if (isdigit(tmpbuf[i])) {
        if (!is_comment) {
          has_dat = true;
        }
      } else if (tmpbuf[i] == '\n' ||
                 tmpbuf[i] == '\r') {
        if (has_dat) {
          _numFrames++;
          if (_numFrames >= _numLinesAlloc) {
            _numLinesAlloc += 32768;
            _rowIndex = (int *)realloc(_rowIndex, _numLinesAlloc*sizeof(int));
          }
          new_data = true;
        }
        _rowIndex[_numFrames] = bufstart + i + 1;
        has_dat = is_comment = false;
      } else if (tmpbuf[i] == '#' || tmpbuf[i] == '!' || tmpbuf[i] == '/' ||
                 tmpbuf[i] == ';' || tmpbuf[i] == 'c') {
        is_comment = true;
      }
    }
  } while (bufread == MAXBUFREADLEN);

  file.close();

  return new_data ? KstObject::UPDATE : KstObject::NO_CHANGE;
}


int AsciiSource::readField(double *v, const QString& field, int s, int n) {
  bool ok;

  if (n < 0) {
    n = 1; /* n < 0 means read one sample, not frame - irrelavent here */
  }

  if (field == "INDEX") {
    for (int i = 0; i < n; i++) {
      v[i] = double(s + i);
    }
    return n;
  }

  int col = (int)field.toUInt(&ok);
  if (!ok) {
    return 0;
  }

  int bufstart = _rowIndex[s];
  int bufread = _rowIndex[s + n] - bufstart;

  QFile file(_filename);
  if (!file.open(IO_ReadOnly)) {
    _valid = false;
    return 0;
  }

  char *tmpbuf = new char[bufread];

  file.at(bufstart);
  file.readBlock(tmpbuf, bufread);

  memset( v, 0, n * sizeof( double ) );
  
  for (int i = 0; i < n; i++, s++) {
    bool done = false;
    bool incol = false;
    int i_col = 0;
    for (int ch = _rowIndex[s] - bufstart; !done && ch < bufread; ch++) {
      if (isspace(tmpbuf[ch])) {
        if (tmpbuf[ch] == '\n' ||
            tmpbuf[ch] == '\r' ) {
          done = true;
        }
        else
        {
          incol = false;
        }
      } else if (tmpbuf[ch] == '#' || tmpbuf[ch] == '!' ||
          tmpbuf[ch] == '/' || tmpbuf[ch] == ';' ||
          tmpbuf[ch] == 'c') {
        done = true;
      } else {
        if (!incol) {
          incol = true;
          i_col++;
          if (i_col == col) {
            done = true;
            v[i] = atof(tmpbuf + ch);
          }
        }
      }
    }
  }

  delete[] tmpbuf;

  file.close();

  return n;
}


bool AsciiSource::isValidField(const QString& field) const {
  bool ok;

  if (field == "INDEX") {
    return true;
  }

  field.toUInt(&ok);

  return ok;
}


int AsciiSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 1;
}


int AsciiSource::frameCount() const {
  return _numFrames;
}


QString AsciiSource::fileType() const {
  return "ASCII";
}


// FIXME: refactor to cache the number of columns and just update it in
//        ::update() as needed
QStringList AsciiSource::fieldList() const {
  QStringList rc;
  QFile file(_filename);
  bool done = false;
  QString line;

  if (!file.open(IO_ReadOnly)) {
    return rc;
  }

  QRegExp re("^\\s*[#/c!].*");
  while (!file.atEnd() && !done) {
      int rc = file.readLine(line, 1000);
      if (rc > 1 && !re.exactMatch(line)) {
        done = true;
      }
  }

  file.close();

  int cnt = QStringList::split(QRegExp("\\s"), line, false).count();
  for (int i = 1; i <= cnt; ++i) {
    rc += QString::number(i);
  }

  return rc;
}


void AsciiSource::save(QTextStream &ts) {
  // FIXME
  KstDataSource::save(ts);
}


extern "C" {
KstDataSource *create_ascii(const QString& filename, const QString& type) {
  return new AsciiSource(filename, type);
}

QStringList provides_ascii() {
  QStringList rc;
  rc += "ASCII";
  return rc;
}

/** understands_ascii: returns true iff:
    the first line which is neither empty nor a comment
    (a comment starts with #, /, c, or !)
    is a number.  **/
// FIXME: might it be better to be more lenient and ignore lines with
//        ascii free text?
bool understands_ascii(const QString& filename) {
  QFile f(filename);

  if (f.open(IO_ReadOnly)) {
    QString s;
    Q_LONG rc = 0;
    bool done = false;

    while (!done) {
      rc = f.readLine(s, 1000);
      if (rc <= 0) {
        done = true;
      } else if (rc == 1) {
        // empty line; do nothing
      } else if (QRegExp("^\\s*[\\#\\/\\c\\!].*").exactMatch(s)) {
        // comment; do nothing
      } else if (QRegExp("^[\\s]*(([Nn][Aa][Nn]|(\\-\\+)?[Ii][Nn][Ff]|[0-9\\+\\-\\.eE]+)[\\s]*)+").exactMatch(s)) {
        // a number - this may be an ascii file - assume that it is
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}
}

// vim: ts=2 sw=2 et


