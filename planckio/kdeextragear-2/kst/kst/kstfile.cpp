/***************************************************************************
                          kstfile.cpp  -  A file class for KST
                             -------------------
    begin                : Thu Aug 24 2000
    copyright            : (C) 2000 by Barth Netterfield
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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>

#include <qfile.h>
#include <ktempfile.h>
#include <klocale.h>
#include <kdebug.h>

#include "kstdatacollection.h"

#include "creaddata_cpp.h"
#include "getdata_struct.h"
#include "getdata_cpp.h"

#include "kstfile.h"

#include "plancktoi.h"

/************************************************************************/
/*                                                                      */
/*                Public Methods                                        */
/*                                                                      */
/************************************************************************/
KstFile::KstFile(const QString &filename_in, KstFileType newType)
: KstObject() {
  commonConstructor(filename_in, newType);
}

KstFile::KstFile(QDomElement &e)
: KstObject() {
  QString filename_in = "not_set";

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "filename")
         filename_in = e.text();
    }
    n = n.nextSibling();
  }

  /* Call the common constructor */
 commonConstructor(filename_in, UNKNOWN);
}

void KstFile::commonConstructor(const QString &filename_in,
                                const KstFileType newType) {

  NumUsed = NumFrames = 0;
  IsStdin = false;
  IsIndirect = false;
  ByteLength = NumLinesAlloc = 0;
  BytePerFrame = FramePerFile = 0;

  /* determine if this is an indirect file                               */
  /* an indirect file lists the name of the actual file to get data from */
  /* This is often useful in real-time data aquisition                   */
  if (filename_in.right(4) == ".cur") {
    IndirectFilename = filename_in;
    IsIndirect = true;
    setFilename("");
    readIFile();
  } else {
    IsIndirect = 0;
    setFilename(filename_in);
  }

  if (Filename == "stdin") {
    StdinFile = new KTempFile();
    setFilename(StdinFile->name());
    IsStdin = true;
    UpdateStdin();
  }

  Type = newType;
  init();
}


KstFile::~KstFile(){
  if (IsStdin) {
    StdinFile->close();
    StdinFile->unlink();
  }

  switch (Type) {
  case ASCII:
    free(RowIndex);
    break;
  case PLANCK:
    _planck = 0L; // deletes it
    break;
  case FRAME:
  case DIRFILE:
  case EMPTY:
  case UNKNOWN:
  default:
    break;
  }
}

/** Reads a field from the file.  Data is returned in the
double Array V[].  Returns number of samples read.  If n<0, read 1 sample,
not entire frames */
int KstFile::readField(double *V, const QString &field, int s, int n){
  if (s > NumFrames - 1)
    return 0;
  if (s + n > NumFrames)
    n = NumFrames - s;

  switch (Type) {
  case ASCII:
    return asciiReadField(V, field, s, n);
  case FRAME:
    return frameReadField(V, field, s, n);
  case DIRFILE:
    return dirfileReadField(V, field, s, n);
  case EMPTY:
  case UNKNOWN:
    return 0;
  case PLANCK:
    return planckReadField(V, field, s, n);
  default:
    kdWarning() << "kst internal error: unknown type in ReadField" << endl;
    return 0;
  }
}

/** Returns the size of the file as of last update */
int KstFile::numFrames() const {
  return Type == EMPTY ? 0 : NumFrames;
}

bool KstFile::isValidField(const QString &field) const {
  switch (Type) {
  case ASCII:
    return asciiIsValidField(field);
  case FRAME:
    return frameIsValidField(field);
  case DIRFILE:
    return dirfileIsValidField(field);
  case EMPTY:
  case UNKNOWN:
    return false;
  case PLANCK:
    return _fieldList.contains(field) || field.lower() == "index";
  default:
    kdWarning() << "kst internal error: unknown type in isValidField" << endl;
    return false;
  }
}


/** Returns samples per frame for field <field>.  For
ascii column data, this is always 1.  For frame data
this could greater than 1. */
int KstFile::samplesPerFrame(const QString &field){

  switch (Type) {
  case ASCII:
    return 1;
  case FRAME:
    return frameSampsPerFrame(field);
  case DIRFILE:
    return dirfileSampsPerFrame(field);
  case EMPTY:
  case UNKNOWN:
    return 0;
  case PLANCK:
    return 1;
  default:
    kdWarning() << "kst internal error: unknown type in samplesPerFrame" << endl;
    return 0;
  }
}

/** Updates the size of the file.  For ASCII files, also reads and writes
    to temp binary file.  Return 1 if there was
    new data */
KstObject::UpdateType KstFile::update(int) {

  if (IsIndirect) {
    if (readIFile() == 1)
      init();
  }

  if (IsStdin)
    UpdateStdin();

  if (Type == EMPTY)
    init();

  switch (Type) {
  case ASCII:
    return asciiUpdate() ? UPDATE : NO_CHANGE;
  case FRAME:
    return frameUpdate() ? UPDATE : NO_CHANGE;
  case DIRFILE:
    return dirfileUpdate() ? UPDATE : NO_CHANGE;
  case EMPTY:
  case UNKNOWN:
    return NO_CHANGE;
  case PLANCK:
    return planckUpdate() ? UPDATE : NO_CHANGE;
  default:
    kdFatal() << "kst internal error: unknown type in update" << endl;
  }
  return NO_CHANGE;
}

/** Returns the file name.  It is stored in a separate static variable,
    so changes to this are ignored */
QString KstFile::fileName() const {
  if (IsIndirect)
    return IndirectFilename;
  else if (IsStdin)
    return "stdin";

  return Filename;
}

/** Returns the file type or an error message in a static string */
QString KstFile::fileType() const {
  switch (Type) {
  case ASCII:
    return i18n("ASCII");
  case FRAME:
    return i18n("Binary Frame");
  case DIRFILE:
    return i18n("Directory of Binary Files");
  case EMPTY:
    return i18n("Empty");
  case UNKNOWN:
    return i18n("Unknown Type");
  case PLANCK:
    return i18n("PLANCK I/O");
  default:
    break;
  }

  kdWarning() << "kst internal error: unknown type in FileType" << endl;
  return "???";
}

/** Save file description info into stream ts */
void KstFile::save(QTextStream &ts) {
  ts << "  <filename>";
  if (IsIndirect) {
    ts << IndirectFilename;
  } else {
    ts << Filename;
  }
  ts << "</filename>" << endl;
}

/** return a qstringlist of valid fields for this file type */
QStringList KstFile::fieldList() const {
  return _fieldList;
}

/************************************************************************/
/*                                                                      */
/*                Private Methods: General                              */
/*                                                                      */
/************************************************************************/
/** Determines the file type : returns file type */
int KstFile::determineType(){
  int error_code;
  int i, nread;
  char readbuf[1024];

  if (Filename.startsWith("group:")) {
    Type = PLANCK;
    return Type;
  }

  // FIXME: GetNFrames causes a memory error here.  I think it is due to
  // the lfilename parameter.
  /**********************************************************/
  /* check to see if the file is a dirfile file (directory) */
  NumFrames = GetNFrames(CFilename, &error_code, NULL);
  if (NumFrames > 0 && error_code == GD_E_OK) {
    Type = DIRFILE;
    return Type;
  }

  /**********************************************************/
  /* verify that the file exists; determine length in bytes */
  QFile the_file(Filename);

  if (the_file.exists()) {
    ByteLength = the_file.size();
  } else {
    Type = EMPTY;
    return Type;
  }

  /******************************************************************/
  /* test to see if the file is a creaddata recognized binary frame */
  CReadData(CFilename,"INDEX",
	    0,0, /* 1st sframe, 1st samp */
	    1,0, /* num sframes, num samps */
	    'n',(void*)NULL,
	    &error_code);
  if (error_code == E_OK) {
    Type = FRAME;
    return Type;
  }

  /********************************************/
  /* test to see if the file is ascii collums */
  /* This is pretty unreliable so do it last  */
  /* Just search the first 1024 bytes for a   */
  /* non character.                           */
  the_file.open(IO_ReadOnly);
  nread = the_file.readBlock(readbuf, 1024);

  bool non_ascii = false;
  for (i = 0; i < nread; i++) {
    if (!(isprint(readbuf[i]) || readbuf[i]=='\r' || readbuf[i]=='\n' ||
        readbuf[i]=='\t')) {
      non_ascii = true;
    }
  }
  the_file.close();

  if (!non_ascii) {
    Type = ASCII;
    return Type;
  } else {
    Type = UNKNOWN;
    return Type;
  }
}

/** Read Indirect File */
/** Return 1 if filename changes */
int KstFile::readIFile() {
  QFile ifile(IndirectFilename);

  if (ifile.exists()) {
    ifile.open(IO_ReadOnly);
    QString new_filename;
    ifile.readLine(new_filename, 1024);
    if (new_filename.stripWhiteSpace() == Filename) {
      return 0;
    } else {
      setFilename(new_filename.stripWhiteSpace());
      return 1;
    }
  } else {
    Type = EMPTY;
    setFilename("");
    return 0;
  }
}


/** Initializes files */
void KstFile::init() {
  _fieldList.clear();

  if (Type == UNKNOWN || Type == EMPTY) {
    determineType();
  }
  switch (Type) {
  case ASCII:
    asciiInitFile();
    break;
  case FRAME:
    frameInitFile();
    break;
  case DIRFILE:
    dirfileInitFile();
    break;
  case EMPTY:
  case UNKNOWN:
    return;
  case PLANCK:
    planckInitFile();
    return;
  default:
    kdFatal() << "kst internal error: unknown type in inializer" << endl;
  }
}

bool KstFile::UpdateStdin() {
  fd_set rfds;
  struct timeval tv;
  int retval;
  char instr[4097];
  int i = 0;
  char *fgs = NULL;
  bool new_data = false;
  FILE *fp = NULL;

  fp = StdinFile->fstream();

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
      fgs = fgets(instr, 4096, stdin);
      if (fgs && fp) {
        fputs(instr, fp);
        new_data = true;
      }
    }
  } while (++i < 100000 && new_data);

  if (fp) fflush(fp);

  return new_data;
}

/************************************************************************/
/*                                                                      */
/*               Private Methods: ASCII Files                           */
/*                                                                      */
/************************************************************************/

/** Initializations for ascii files: */
bool KstFile::asciiInitFile(){
  RowIndex = (int *)malloc(32768 * sizeof(int));
  NumLinesAlloc = 32768;
  NumFrames = 0;
  RowIndex[0] = 0;

  return asciiUpdate();
}


/** Update an Ascii file: read lines and fill row index array
 Return 1 if there is new data. */
#define MAXBUFREADLEN 32768
bool KstFile::asciiUpdate() {
  static char *tmpbuf=NULL;
  int i_buf;
  bool is_comment, has_dat;
  int bufstart, bufread;
  bool new_data = false;

  QFile ascii_file(Filename);

  if (ascii_file.exists()) {
    ByteLength = ascii_file.size();
  } else {
    Type = EMPTY;
    return false;
  }

  if (!ascii_file.open(IO_ReadOnly)) {
    // quietly fail - no data to be had here
    Type = EMPTY;
    return false;
  }

  if (tmpbuf == NULL)
    tmpbuf = (char *)malloc(MAXBUFREADLEN*sizeof(char));

  do {
    /* Read the tmpbuffer, starting at row_index[NumFrames] */
    if (ByteLength-RowIndex[NumFrames]>MAXBUFREADLEN) bufread = MAXBUFREADLEN;
    else bufread = ByteLength-RowIndex[NumFrames];

    bufstart = RowIndex[NumFrames];
    ascii_file.at(bufstart);
    ascii_file.readBlock(tmpbuf, bufread);

    is_comment = has_dat = false;
    for (i_buf = 0; i_buf < bufread; i_buf++) {
      if (tmpbuf[i_buf] == '\n') {
        if (has_dat) {
          NumFrames++;
          if (NumFrames >= NumLinesAlloc) {
            RowIndex = (int *)realloc(RowIndex,
                                      (NumLinesAlloc + 32768)*sizeof(int));
            NumLinesAlloc += 32768;
          }
          new_data = true;
        }
        RowIndex[NumFrames] = bufstart + i_buf+1;
        has_dat = is_comment = false;
      // Statistically numbers should be more common than comments, so let's
      // check for them first
      // if isdigit() generates a function call, it would be better to write
      // this inline, but I'm not convinced that compilers are that stupid yet.
      // Profiling shows this code path gets hit 10^5+ times.
      } else if (isdigit(tmpbuf[i_buf])) {
        if (!is_comment) {
          has_dat = true;
        }
      } else if (tmpbuf[i_buf] == '#' || tmpbuf[i_buf] == '!' ||
                 tmpbuf[i_buf] == '/' || tmpbuf[i_buf] == ';' ||
                 tmpbuf[i_buf] == 'c') {
        is_comment = true;
      }
    }
  } while (bufread == MAXBUFREADLEN);

  ascii_file.close();

  return new_data;
}

/** Read a field from an Ascii file */
int KstFile::asciiReadField(double *V, const QString &field, int s, int n) {
  int i_char, i;
  char *tmpbuf;
  int bufread;
  int bufstart, col;
  bool done;
  bool incol, ok;
  int i_col;

  if (n < 0)
    n = 1; /* n<0 means read one sample, not frame - irrelevant here */

  if (field == "INDEX") {
    for (i = 0; i < n; i++) {
      V[i] = (double) s+i;
    }
    return n;
  }

  col = (int)field.toUInt(&ok);
  if (!ok) return 0;

  bufstart = RowIndex[s];
  bufread = RowIndex[s+n] - RowIndex[s];

  QFile ascii_file(Filename);
  if (!ascii_file.open(IO_ReadOnly)) {
    Type = EMPTY;
    return 0;
  }

  tmpbuf = new char[bufread];

  ascii_file.at(bufstart);
  ascii_file.readBlock(tmpbuf, bufread);

  for (i = 0; i < n; i++, s++) {
    done = false;
    incol = false;
    i_col = 0;
    for (i_char = RowIndex[s] - bufstart; !done; i_char++) {
      if (tmpbuf[i_char] == '#' || tmpbuf[i_char] == '!' ||
          tmpbuf[i_char] == '/' || tmpbuf[i_char] == ';' ||
          tmpbuf[i_char] == 'c') {
        done = true;
        V[i] = 0;
      } else if (tmpbuf[i_char] == '\n') {
        done = true;
        V[i] = 0;
      } else if (isspace(tmpbuf[i_char])) {
        incol = false;
      } else {
        if (!incol) {
          incol = true;
          i_col++;
          if (i_col == col) {
            done = true;
            V[i] = atof(tmpbuf + i_char);
          }
        }
      }
    }
  }

  delete[] tmpbuf;

  ascii_file.close();

  return n;
}

bool KstFile::asciiIsValidField(const QString &field) const {
  bool ok;

  if (field == "INDEX")
    return true;

  field.toUInt(&ok);

  return ok;
}

/************************************************************************/
/*                                                                      */
/*               Private Methods: Frame Files                           */
/*                                                                      */
/************************************************************************/

/** Initialization for Frame Files */
bool KstFile::frameInitFile(){
  int buf[4], error_code;
  int len;
  char ext[3];
  char *tmpstr=NULL;

  ReadData(CFilename, (const char *)"FFINFO",
	    0,0, /* 1st sframe, 1st samp */
	    0,2, /* num sframes, num samps */
	    'i',(void*)buf,
	    &error_code);

  if (error_code!=E_OK) {
    kdFatal() << "Problem reading FFINFO in from Frame Type File" << endl;
  }

  BytePerFrame = buf[0];
  FramePerFile = buf[1];
  NumFrames=0;

  /* split out file name and extension */
  len = strlen(CFilename);
  ext[0] = CFilename[len-2];
  ext[1] = CFilename[len-1];
  ext[2] = '\0';

  strncpy(RootFileName, CFilename, 252);
  RootFileName[252] = '\0';

  if (isxdigit(ext[0]) && isxdigit(ext[1])) {
    RootFileName[len-2] = '\0';
    RootExt = strtol(ext, &tmpstr, 16);
    MaxExt = RootExt;
  } else {
    MaxExt = RootExt = -1;
  }
  return frameUpdate();
}

/** Update Frame Data file: determine length return true if new data */
bool KstFile::frameUpdate() {
  QString tmpfilename;
  struct stat stat_buf;
  int done = 0;
  int dec = 0;
  int newN;
  bool isnew;

  if (MaxExt<0) { // no hex number extension: only this file
    if (stat(CFilename, &stat_buf) != 0) { // file is gone
      newN = 0;
    } else {                               // file exists
      newN = stat_buf.st_size/BytePerFrame;
    }
  } else {
    do {
      tmpfilename.sprintf("%s%2.2x", RootFileName, MaxExt);
      if (stat(QFile::encodeName(tmpfilename).data(), &stat_buf) != 0) { // no file with MaxExt
	if (MaxExt > RootExt) {  // deleted (?) check the next one down
	  MaxExt--;
	  dec = 1;
	} else {                      // All files have been deleted
	  stat_buf.st_size = 0;
	  done = 1;
	}
      } else {
	if (stat_buf.st_size == BytePerFrame*FramePerFile) { // Full file
	  if (dec) { // already checked next one up: it is empty
	    done = 1;
	  } else {
	    MaxExt++;
	  }
	} else {
	  done = 1;
	}
      }
    } while (!done);
    newN = (MaxExt - RootExt)*FramePerFile + stat_buf.st_size/BytePerFrame;
  }

  isnew = NumFrames != newN;
  NumFrames = newN;

  return isnew;
}

/** Returns true if the field is valid, or false if it is not */
bool KstFile::frameIsValidField(const QString &field) const {
  int error_code;

  CReadData(CFilename,field.left(8).latin1(),
            0,0, /* 1st sframe, 1st samp */
            1,0, /* num sframes, num samps */
            'n',(void*)NULL,
            &error_code);

  return error_code == 0;
}

/** determine samples per frame by doing a NULL read to the file */
int KstFile::frameSampsPerFrame(const QString &field) {
  int spf, error_code;

  spf = CReadData(CFilename,field.left(8).latin1(),
                  0,0, /* 1st sframe, 1st samp */
                  1,0, /* num sframes, num samps */
                  'n',(void*)NULL,
                  &error_code);

  return spf;
}


/** Read a field from a frame file.  The request is for a number of
frames:  the number of samples is n * SampsPerFrame(field).  if n<0,
read just one sample, not an entire frame.  The number
of samples read is returned */
int KstFile::frameReadField(double *V, const QString &field,
                            int s, int n){
  int error_code;
  int n_read;

  if (n < 0) {
    n_read = CReadData(CFilename, field.left(8).latin1(),
                       s,0, /* 1st sframe, 1st samp */
                       0,1, /* num sframes, num samps */
                       'd',(void*)V,
                       &error_code);
  } else {
    n_read = CReadData(CFilename, field.left(8).latin1(),
                       s,0, /* 1st sframe, 1st samp */
                       n,0, /* num sframes, num samps */
                       'd',(void*)V,
                       &error_code);
  }

  return n_read;
}

/************************************************************************/
/*                                                                      */
/*               Private Methods: Dirfile Files                           */
/*                                                                      */
/************************************************************************/

/** Initialization for Dirfile Files */
bool KstFile::dirfileInitFile(){
  struct FormatType *F;
  int error_code=0;

  NumFrames = 0;
  F = GetFormat(CFilename, &error_code);
  if (error_code == GD_E_OK) {
    int i;


    for (i=0; i<F->n_lincom; i++) {
      _fieldList.append(QString(F->lincomEntries[i].field));
    }
    for (i=0; i<F->n_linterp; i++) {
      _fieldList.append(QString(F->linterpEntries[i].field));
    }
    for (i=0; i<F->n_bit; i++) {
      _fieldList.append(QString(F->bitEntries[i].field));
    }
    for (i=0; i<F->n_raw; i++) {
      _fieldList.append(QString(F->rawEntries[i].field));
    }
  }
  return dirfileUpdate();
}

/** Update Dirfile Data file: determine length return 1 if new data */
bool KstFile::dirfileUpdate(){
  int error_code;
  int newNF = GetNFrames(CFilename, &error_code, NULL);
  bool isnew = newNF != NumFrames;

  NumFrames = newNF;

  return isnew;
}

/** Returns true if the field is valid, or false if it is not */
bool KstFile::dirfileIsValidField(const QString &field) const {
  int error_code;
  /*int i =*/
  GetSamplesPerFrame(CFilename, field.left(16).latin1(), &error_code);

  return error_code == 0;
}

/** determine samples per frame */
int KstFile::dirfileSampsPerFrame(const QString &field) {
  int error_code;

  int spf = GetSamplesPerFrame(CFilename, field.left(16).latin1(), &error_code);

  return spf;
}


/** Read a field from a dirfile file.  The request is for a number of
frames:  the number of samples is n * SampsPerFrame(field).  if n<0,
read just one sample, not an entire frame.  The number
of samples read is returned */
int KstFile::dirfileReadField(double *V, const QString &field,
                              int s, int n){
  int error_code;
  int n_read;

  if (n < 0) {
    n_read = GetData(CFilename, field.left(16).latin1(),
                       s,0, /* 1st sframe, 1st samp */
                       0,1, /* num sframes, num samps */
                       'd',(void*)V,
                       &error_code);
  } else {
    n_read = GetData(CFilename, field.left(16).latin1(),
                       s,0, /* 1st sframe, 1st samp */
                       n,0, /* num sframes, num samps */
                       'd',(void*)V,
                       &error_code);
  }

  return n_read;
}

void KstFile::setFilename(QString new_name) {
  Filename = new_name;
  EncodedFilename = QFile::encodeName(Filename);
  CFilename = EncodedFilename.data();
}


bool KstFile::planckUpdate() {
  // FIXME: group is hardcoded
  return _planck->updateGroup(QStringList::split(':', Filename)[2]);
}


bool KstFile::planckInitFile() {
  _planck = new Planck::TOI;
  _planck->setSource(Filename);
  if (!_planck->isValid()) {
    return false;
  }

  QStringList fields = QStringList::split(':', Filename);
  // precondition: fields.size() == 3  <-- guaranteed by isValid() above
  _fieldList = _planck->fields(fields[2]);
  // FIXME: hardcoded the last field!
  QSize s = _planck->range(fields[2], _fieldList[_fieldList.size() - 1]);
  NumFrames = s.height() - s.width();
  return true;
}


int KstFile::planckReadField(double *V, const QString& field, int s, int n) {
  if (field.lower() == "index") {
    for (int i = 0; i < n; ++i) {
      V[i] = float(s+i);
    }
    return n > 0 ? n : 0;
  }

  if (n == 0 || _planck.data() == 0 || !_planck->isValid() || !_fieldList.contains(field)) {
    kdDebug() << ">>>>>> planckReadField: something isn't right." << endl;
    return 0;
  } 

  QStringList fields = QStringList::split(':', Filename); // FIXME: this is garbage
  // precondition: fields.size() == 3  <-- guaranteed by isValid() above

  // FIXME: store this in the class
  QSize sz = _planck->range(fields[2], field);

  if (n < 0) {
    n = 1;
  }

  if (s + sz.width() >= sz.height() - 1) {
    return 0;
  }

  return _planck->readObject(fields[2], field, V, sz.width() + s, sz.width() + s + n - 1);
}


// vim: ts=2 sw=2 et
