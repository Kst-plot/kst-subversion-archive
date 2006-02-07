/***************************************************************************
                          kstrvector.cpp  -  description
                             -------------------
    begin                : Fri Sep 22 2000
    copyright            : (C) 2000 by cbn
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include "rwlock.h"
#include "enodes.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstrvector.h"

/** Create a KstRVector: raw data from a file */
KstRVector::KstRVector(KstDataSourcePtr in_file, const QString &in_field,
                       const QString &in_tag, int in_f0, int in_n,
                       int skip, bool in_DoSkip, bool in_DoAve)
: KstVector() {
  setTagName(in_tag);
  commonRVConstructor(in_file, in_field, in_f0, in_n, skip,
		      in_DoSkip, in_DoAve);
}

KstRVector::KstRVector(QDomElement &e, const QString &o_file,
                       int o_n, int o_f, int o_s, bool o_ave)
: KstVector() {
  EventMonitorEntry* pEvent;
  KstDataSourcePtr in_file;
  QString in_field;
  int in_f0 = 0;
  int in_n = -1;
  int in_skip = 0;
  bool in_DoSkip = false;
  bool in_DoAve = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "filename") {
        KST::dataSourceList.lock().readLock();
        if (o_file == "|") {
          in_file = *KST::dataSourceList.findFileName(e.text());
        } else {
          in_file = *KST::dataSourceList.findFileName(o_file);
        }
        KST::dataSourceList.lock().readUnlock();
      } else if (e.tagName() == "field") {
        in_field = e.text();
      } else if (e.tagName() == "start") {
        in_f0 = e.text().toInt();
      } else if (e.tagName() == "num") {
        in_n = e.text().toInt();
      } else if (e.tagName() == "skip") {
        in_skip = e.text().toInt();
        in_DoSkip = true;
      } else if (e.tagName() == "doAve") {
        in_DoAve = true;
        in_DoSkip = true;
        if (in_skip < 1) {
          in_skip = 1;
        }
      }
      else if( e.tagName() == "event") {
        pEvent = new EventMonitorEntry(e);
        _eventMonitors.append( *pEvent );
        delete pEvent;
      }
    }
    n = n.nextSibling();
  }

  if (o_n > -2) {
    in_n = o_n;
  }
  if (o_f > -2) {
    in_f0 = o_f;
  }
  if (o_s > -1) {
    in_DoSkip = true;
    if (o_s > 0) {
      in_skip = o_s;
      in_DoAve |= o_ave;
    } else {
      in_skip = 0;
    }
  }
  /* Call the common constructor */
  commonRVConstructor(in_file, in_field, in_f0, in_n, in_skip, in_DoSkip, in_DoAve);
}

void KstRVector::commonRVConstructor(KstDataSourcePtr in_file,
                                     const QString &in_field, int in_f0,
                                     int in_n, int in_skip, bool in_DoSkip,
                                     bool in_DoAve) {
  _numSamples = 0;
  NumUsed = 0;
  _scalars["sum"]->setValue(0.0);
  _scalars["sumsquared"]->setValue(0.0);
  F0 = NF = 0; // nothing read yet

  N_AveReadBuf = 0;
  AveReadBuf = 0L;

  _file = in_file;
  ReqF0 = in_f0;
  ReqNF = in_n;
  Skip = in_skip;
  DoSkip = in_DoSkip;
  DoAve = in_DoAve;
  Field = in_field;

  if (DoSkip && Skip < 1) {
    Skip = 1;
  }

  // ReqNF <=0 means read to end of file
  // ReqF0 < 0 means count back ReqNF from end of file
  if (ReqNF <= 0 && ReqF0 < 0) {
    ReqF0 = 0;
  }

  if (_file) {
    SPF = _file->samplesPerFrame(Field);
  }

  _dirty = true;

  if (!in_file) {
    KstDebug::self()->log(i18n("Data file for vector %1 was not opened.").arg(tagName()), KstDebug::Warning);
  }

  update(-1);
}

void KstRVector::setEventMonitors( EventMonitors* pEventMonitors ) {  
  int i;
  
  _eventLock.writeLock();  
  for( i=0; i<(int)_eventMonitors.size(); i++ ) {      
    delete _eventMonitors[i].getExpression( );
    _eventMonitors[i].setExpression( NULL );
  }  
  _eventMonitors = *pEventMonitors;
  _eventLock.writeUnlock();  
}

void KstRVector::change(KstDataSourcePtr in_file, const QString &in_field,
                        const QString &in_tag,
                        int in_f0, int in_n,
                        int in_skip, bool in_DoSkip,
                        bool in_DoAve) {
  Skip = in_skip;
  DoSkip = in_DoSkip;
  DoAve = in_DoAve;
  if (DoSkip && Skip < 1) {
    Skip = 1;
  }

  _file = in_file;
  ReqF0 = in_f0;
  ReqNF = in_n;
  Field = in_field;
  setTagName(in_tag);

  reset();
  // ReqNF <=0 means read to end of file
  // ReqF0 < 0 means count back ReqNF from end of file
  if (ReqNF <= 0 && ReqF0 < 0) {
    ReqF0 = 0;
  }

}

void KstRVector::changeFile(KstDataSourcePtr in_file) {
  if (!in_file) {
    KstDebug::self()->log(i18n("Data file for vector %1 was not opened.").arg(tagName()), KstDebug::Warning);
  }
  _file = in_file;
  reset();
}

void KstRVector::changeFrames(int in_f0, int in_n,
                              int in_skip, bool in_DoSkip,
                              bool in_DoAve) {
  reset();
  Skip = in_skip;
  DoSkip = in_DoSkip;
  DoAve = in_DoAve;
  if (DoSkip) {
    if (Skip < 1)
      Skip = 1;
  }

  ReqF0 = in_f0;
  ReqNF = in_n;

  // ReqNF <=0 means read to end of file
  // ReqF0 < 0 means count back ReqNF from end of file
  if (ReqNF <= 0) {
    if (ReqF0 < 0)
      ReqF0 = 0;
  }
}

KstRVector::~KstRVector() {
  int i;
  
  _file = 0;

  if (AveReadBuf != NULL) {
    free(AveReadBuf);

   AveReadBuf = NULL;
  }
  
  for( i=0; i<(int)_eventMonitors.size(); i++ ) {      
    delete _eventMonitors[i].getExpression( );
    _eventMonitors[i].setExpression( NULL );
  }
}


  // ReqNF <=0 means read to end of file
  // ReqF0 < 0 means count back ReqNF from end of file

bool KstRVector::readToEOF() const {
  return ReqNF <= 0;
}

bool KstRVector::countFromEOF() const {
  return ReqF0 < 0;
}

/** Return Starting Frame of Vector */
int KstRVector::startFrame() const {
  return F0;
}

/** Return frames per skip to read */
int KstRVector::skip() const {
  return DoSkip ? Skip : 0;
}

bool KstRVector::doSkip() const {
  return DoSkip;
}

bool KstRVector::doAve() const {
  return DoAve;
}

/** Return frames held in Vector */
int KstRVector::numFrames() const {
  return NF;
}

int KstRVector::reqNumFrames() const {
  return ReqNF;
}

int KstRVector::reqStartFrame() const {
  return ReqF0;
}

/** Save vector information */
void KstRVector::save(QTextStream &ts) {
  int i;
  
  if (!_file) {
    return;
  }

  ts << "  <tag>" << tagName() << "</tag>" << endl;
  ts << "  <filename>" << _file->fileName() << "</filename>" << endl;
  ts << "  <field>" << Field << "</field>" << endl;
  ts << "  <start>" << ReqF0 << "</start>" << endl;
  ts << "  <num>" << ReqNF << "</num>" << endl;
  if (doSkip()) {
    ts << "  <skip>" << Skip << "</skip>" << endl;
    if (doAve()) {
      ts << "  <doAve/>" << endl;
    }
  }
  if( _eventMonitors.size() > 0 ) {
    for( i=0; i<(int)_eventMonitors.size(); i++ ) {
      _eventMonitors[i].save(ts);
    }
  }
}

/** return the name of the file */
QString KstRVector::filename() const {
  return _file ? _file->fileName() : QString::null;
}

/** return the field */
QString KstRVector::getField() const {
  return Field;
}

QString KstRVector::label() const {
  bool ok;
  QString label;

  Field.toInt(&ok);
  if (ok && _file && _file->fileType() == "ASCII") {
    label = i18n("Column %1").arg(Field);
  } else {
    label = Field;
  }

  return label;
}

void KstRVector::reset() {
  SPF = _file ? _file->samplesPerFrame(Field) : SPF;
  F0 = NF = 0;
  resize(0);
  _numSamples = 0;
  _dirty = true;
}

void KstRVector::checkIntegrity() {
  if (DoSkip && Skip < 1) {
    Skip = 1;
  }

  /* if dirty, reset */
  if (_dirty) {
    reset();
  }

  /* if it looks like we have a new file, reset */
  if (_file && (SPF != _file->samplesPerFrame(Field.latin1()) || _file->frameCount() < NF)) {
    reset();
  }

  // check for illeagal NF and F0 values
  if (ReqNF < 1 && ReqF0 < 0) {
    ReqF0 = 0; // for this illegal case, read the whole file
  }

  if (ReqNF == 1) {
    ReqNF = 2;
  }
}


/** Update an RVECTOR */
/** if update_counter < 1, the update will happen */
/** if update_counter == LastUpdateCouner, the update won't happen */

KstObject::UpdateType KstRVector::update(int update_counter) {
  if (KstObject::checkUpdateCounter(update_counter)) {
    return NO_CHANGE;
  }

  return doUpdate();
}

KstObject::UpdateType KstRVector::doUpdate(bool force) {
  KstObject::UpdateType  update;
  int i, j, k, shift, n_read=0;
  int ave_nread;
  int new_f0, new_nf, tmp_fn;

  checkIntegrity();

  if (!_file) {
    return NO_CHANGE;
  }

  /*******************************/
  /**** Set new_nf and new_f0 ****/
  // ReqNF <=0 means read from ReqF0 to end of File
  // ReqF0 < means start at EndOfFile-ReqNF.

  // ReqNF      ReqF0      Action
  //  < 1        >=0       read from ReqF0 to end of file
  //  < 1        < 0       illegal: fixed in checkIntegrity
  //    1         ??       illegal: fixed in checkIntegrity
  //  > 1        < 0       read the last ReqNF frames from the file
  //  > 1        >=0       Read ReqNF frames starting at frame ReqF0
  if (ReqNF < 1) { // read to end of file
    new_f0 = ReqF0;
    new_nf = _file->frameCount() - new_f0;
  } else if (ReqF0 < 0) { // count back from end of file
    new_nf = _file->frameCount();
    if (new_nf > ReqNF) {
      new_nf = ReqNF;
    }
    new_f0 = _file->frameCount() - new_nf;
  } else {
    new_f0 = ReqF0;
    new_nf = ReqNF;
    if (new_f0 + new_nf > _file->frameCount()) {
      new_nf = _file->frameCount() - new_f0;
    }
    if (new_nf <= 0) {
      new_nf = new_f0 = 0;
    }
  }

  if (DoSkip) {
    /*********************************/
    /* change new_f0 and new_nf so they both lie on skip boundaries */
    tmp_fn = ((new_f0+new_nf)/Skip) * Skip;
    new_f0 = (((new_f0-1)/Skip)+1) * Skip;
    new_nf = tmp_fn-new_f0;
  }

  if (NF == new_nf && F0 == new_f0 && !force) {
    return NO_CHANGE;
  }

  /***********************************/
  /**** Shift vector if necessary ****/
  if (new_f0 < F0 || new_f0 >= F0 + NF) { // No useful data around.
    reset();
  } else { // shift stuff rather than re-read
    if (DoSkip) {
      shift = (new_f0 - F0)/Skip;
      NF -= (new_f0 - F0);
      _numSamples = NF/Skip;
    } else {
      shift = SPF*(new_f0 - F0);
      NF -= (new_f0 - F0);
      _numSamples = (NF-1)*SPF+1;
    }

    for (i = 0; i < _numSamples; i++) {
      _v[i] = _v[i+shift];
    }
  }

  if (DoSkip) {
    /**** Reallocate V if necessary ****/
    if (new_nf / Skip != _size) {
      resize(new_nf/Skip);
    }
    n_read = 0;
    /** read each sample from the File */
    for (i = NF, j = 0; i < new_nf; i += Skip, j++) {
      if (DoAve) {
        /* enlarge AveReadBuf if necessary */
        if (N_AveReadBuf < Skip*SPF) {
          N_AveReadBuf = Skip*SPF;
          AveReadBuf = static_cast<double*>
            (realloc(AveReadBuf, N_AveReadBuf*sizeof(double)));
        }
        ave_nread = _file->readField(AveReadBuf, Field, new_f0+i, Skip);
        for (k = 1; k < ave_nread; k++) {
          AveReadBuf[0] += AveReadBuf[k];
        }
        if (ave_nread > 0) {
          _v[_numSamples + j] = AveReadBuf[0]/double(ave_nread);
          n_read++;
        }
      } else {
        n_read += _file->readField(_v + _numSamples + j, Field, new_f0 + i, -1);
      }
    }
  } else {
    /**** Reallocate V if necessary ****/
    if ((new_nf - 1)*SPF + 1 != _size) {
      resize((new_nf - 1)*SPF + 1);
    }

    if (NF > 0) {
      NF--; /* last frame read was only partially read... */
    }

    /**** Read the new data from File ****/
    n_read = _file->readField(_v+NF*SPF, Field, new_f0 + NF, new_nf - NF - 1);
    n_read += _file->readField(_v+(new_nf-1)*SPF, Field, new_f0 + new_nf-1, -1);
  }

  NumNew = _size - _numSamples;
  NF = new_nf;
  F0 = new_f0;
  _numSamples += n_read;

  // if for some reason (eg, realtime reading an nfs mounted
  // dirfile) not all of the data was read, the data will never
  // be read; the samples will be filled in with the last data
  // point read, and a complete reset will be requested.
  // This is bad - I think it will be worthwhile
  // to add blocking w/ timeout to KstFile.
  // As a first fix, mount all nsf mounted dirfiles with "-o noac"
  _dirty = false;
  if (_numSamples != _size) {
    _dirty = true;
    for (i = _numSamples; i < _size; i++) {
      _v[i] = _v[0];
    }
  }

  if (NumNew > _size) {
    NumNew = _size;
  }
  if (NumShifted > _size) {
    NumShifted = _size;
  }

  update = KstVector::update(-1);
  
  if( update == UPDATE ) {
    //
    // check for events...
    //
    Equation::Context ctx;
    
    ctx.sampleCount = NumNew;
    _eventLock.writeLock();  
    for( i=0; i<(int)_eventMonitors.size(); i++ ) {      
      if( _eventMonitors[i].needToEvaluate() ) {
        if( _eventMonitors[i].compile() ) {
          for (ctx.i = new_f0; ctx.i < _size; ++ctx.i) {
            ctx.x = (double)ctx.i;
            _eventMonitors[i].evaluate( ctx );
          }
        }
      }
    }
    _eventLock.writeUnlock();  
  }
  
  return update;
}


/** Returns intrinsic samples per frame */
int KstRVector::samplesPerFrame() const {
  return SPF;
}

/** return true if it has a valid file and field, or false otherwise */
bool KstRVector::isValid() const {
  return _file && _file->isValidField(Field);
}

int KstRVector::fileLength() const {
  return _file ? _file->frameCount() : 0;
}

void KstRVector::reload() {
  if (!_file) {
    return;
  }

  // FIXME: inefficient
  KstDataSourcePtr newsrc = KstDataSource::loadSource(_file->fileName(), _file->fileType());
  if (newsrc) {
    KST::dataSourceList.lock().writeLock();
    KST::dataSourceList.remove(_file);
    KST::dataSourceList.append(newsrc);
    KST::dataSourceList.lock().writeUnlock();
    _file = newsrc;
    reset();
  }
}

// vim: ts=2 sw=2 et
