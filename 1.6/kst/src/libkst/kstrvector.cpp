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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <qstylesheet.h>

#include "ksdebug.h"
#include <klocale.h>

#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstrvector.h"
#include "kstmath.h"

// _reqNumberFrames <=0 means read from _reqStartingFrame to end of File
// _reqNumberFrames < means start at EndOfFile-_reqNumberFrames.
//
// _reqNumberFrames _reqStartingFrame   Action
//  < 1             >=0                 read from _reqStartingFrame to end of file
//  < 1             < 0                 illegal: fixed in checkIntegrity
//    1             ??                  illegal: fixed in checkIntegrity
//  > 1             < 0                 read the last _reqNumberFrames frames from the file
//  > 1             >=0                 read _reqNumberFrames frames starting at frame _reqStartingFrame

/** Create a KstRVector: raw data from a file */
KstRVector::KstRVector(KstDataSourcePtr in_file, const QString &in_field,
                       KstObjectTag in_tag,
                       int in_f0, int in_n, int skip, bool in_DoSkip,
                       bool in_DoAve)
: KstVector(in_tag) {
  commonRVConstructor(in_file, in_field, in_f0, in_n, skip,
      in_DoSkip, in_DoAve);
}


KstRVector::KstRVector(const QDomElement &e, const QString &o_file,
                       int o_n, int o_f, int o_s, bool o_ave)
: KstVector(e) {
  KstDataSourcePtr in_file, in_provider;
  QString in_field;
  int in_f0 = 0;
  int in_n = -1;
  int in_skip = 0;
  bool in_DoSkip = false;
  bool in_DoAve = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "provider") {
        KST::dataSourceList.lock().readLock();
        in_provider = *KST::dataSourceList.findTag(e.text());
        KST::dataSourceList.lock().unlock();
      } else if (e.tagName() == "filename") {
        if (!in_provider) {
          KST::dataSourceList.lock().readLock();
          if (o_file == "|") {
            in_file = *KST::dataSourceList.findFileName(e.text());
          } else {
            in_file = *KST::dataSourceList.findFileName(o_file);
          }
          KST::dataSourceList.lock().unlock();
        }
      } else if (e.tagName() == "field") {
        in_field = e.text();
      } else if (e.tagName() == "start") {
        in_f0 = e.text().toInt();
      } else if (e.tagName() == "num") {
        in_n = e.text().toInt();
      } else if (e.tagName() == "skip") {
        in_skip = e.text().toInt();
        in_DoSkip = in_skip > 0;
      } else if (e.tagName() == "doAve") {
        in_DoAve = true;
        in_DoSkip = true;
        if (in_skip < 1) {
          in_skip = 1;
        }
      }
    }
    n = n.nextSibling();
  }

  if (in_provider) {
    // provider overrides filename
    in_file = in_provider;
  }
  if (in_file) {
    // use datasource as tag context for this RVector
    // allow unique vector names to be displayed at top-level
    setTagName(KstObjectTag(tag().tag(), in_file->tag(), false));
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

  commonRVConstructor(in_file, in_field, in_f0, in_n, in_skip, in_DoSkip, in_DoAve);
}


void KstRVector::commonRVConstructor(KstDataSourcePtr in_file,
                                     const QString &field, int reqStartingFrame,
                                     int reqNumberFrames, int skip, bool doSkip,
                                     bool doAve) {
  _saveable = true;
  _dontUseSkipAccel = false;
  _numSamples = 0;
  _scalars["sum"]->setValue(0.0);
  _scalars["sumsquared"]->setValue(0.0);
  _startingFrame = _numberOfFrames = 0; // nothing read yet

  _nAveReadBuf = 0;
  _aveReadBuf = 0L;

  _file = in_file;
  _reqStartingFrame = reqStartingFrame;
  _reqNumberFrames = reqNumberFrames;
  _skip = skip;
  _doSkip = doSkip;
  _doAve = doAve;
  _field = field;

  if (_doSkip && _skip < 1) {
    _skip = 1;
  }

  if (_reqNumberFrames <= 0 && _reqStartingFrame < 0) {
    _reqStartingFrame = 0;
  }

  if (_file) {
    _samplesPerFrame = _file->samplesPerFrame(_field);
  }

  _dirty = true;

  if (!in_file) {
    KstDebug::self()->log(i18n("Data file for vector %1 was not opened.").arg(tagName()), KstDebug::Warning);
  }
}


void KstRVector::change(KstDataSourcePtr in_file, const QString &in_field,
                        KstObjectTag in_tag,
                        int reqStartingFrame, int reqNumberFrames,
                        int skip, bool doSkip, bool doAve) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  _skip = skip;
  _doSkip = doSkip;
  _doAve = doAve;
  if (_doSkip && _skip < 1) {
    _skip = 1;
  }

  _dontUseSkipAccel = false;
  _file = in_file;
  _reqStartingFrame = reqStartingFrame;
  _reqNumberFrames = reqNumberFrames;
  _field = in_field;
  if (in_tag != tag()) {
    setTagName(in_tag);
  }

  if (_file) {
    _file->writeLock();
  }
  reset();
  if (_file) {
    _file->unlock();
  }

  if (_reqNumberFrames <= 0 && _reqStartingFrame < 0) {
    _reqStartingFrame = 0;
  }

}


void KstRVector::changeFile(KstDataSourcePtr in_file) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  if (!in_file) {
    KstDebug::self()->log(i18n("Data file for vector %1 was not opened.").arg(tagName()), KstDebug::Warning);
  }
  _file = in_file;
  if (_file) {
    _file->writeLock();
  }
  setTagName(KstObjectTag(tag().tag(), _file->tag(), false));
  reset();
  if (_file) {
    _file->unlock();
  }
}


void KstRVector::changeFrames(int reqStartingFrame, int reqNumberFrames,
                              int skip, bool doSkip, bool doAve) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  if (_file) {
    _file->writeLock();
  }
  reset();
  if (_file) {
    _file->unlock();
  }
  _skip = skip;
  _doSkip = doSkip;
  _doAve = doAve;
  if (_doSkip && _skip < 1) {
    _skip = 1;
  }

  _reqStartingFrame = reqStartingFrame;
  _reqNumberFrames = reqNumberFrames;

  if (_reqNumberFrames <= 0 && _reqStartingFrame < 0) {
    _reqStartingFrame = 0;
  }
}


void KstRVector::setFromEnd() {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  _reqStartingFrame = -1;
  if (_reqNumberFrames < 2) {
    _reqNumberFrames = numFrames();
    if (_reqNumberFrames < 2) {
      _reqStartingFrame = 0;
    }
  }
}


KstRVector::~KstRVector() {
  _file = 0;

  if (_aveReadBuf) {
    free(_aveReadBuf);
    _aveReadBuf = 0L;
  }
}


bool KstRVector::readToEOF() const {
  return _reqNumberFrames <= 0;
}


bool KstRVector::countFromEOF() const {
  return _reqStartingFrame < 0;
}


/** Return Starting Frame of Vector */
int KstRVector::startFrame() const {
  return _startingFrame;
}


/** Return frames per skip to read */
int KstRVector::skip() const {
  return _doSkip ? _skip : 0;
}


bool KstRVector::doSkip() const {
  return _doSkip;
}


bool KstRVector::doAve() const {
  return _doAve;
}


/** Return frames held in Vector */
int KstRVector::numFrames() const {
  return _numberOfFrames;
}


int KstRVector::reqNumFrames() const {
  return _reqNumberFrames;
}


int KstRVector::reqStartFrame() const {
  return _reqStartingFrame;
}


void KstRVector::save(QTextStream &ts, const QString& indent, bool saveAbsolutePosition) {
  if (_saveData) {
    // This is ugly.  Really we need a way to change vector types at runtime.
    ts << indent << "<avector>" << endl;
    KstVector::save(ts, indent + "  ", saveAbsolutePosition);
    ts << indent << "</avector>" << endl;
  } else if (_file) {
    ts << indent << "<vector>" << endl;
    KstVector::save(ts, indent + "  ", saveAbsolutePosition);
    _file->readLock();
    ts << indent << "  <provider>" << QStyleSheet::escape(_file->tag().tagString()) << "</provider>" << endl;
    ts << indent << "  <filename>" << QStyleSheet::escape(_file->fileName()) << "</filename>" << endl;
    _file->unlock();

    ts << indent << "  <field>" << _field << "</field>" << endl;
    if (saveAbsolutePosition) {
      ts << indent << "  <start>" << _startingFrame << "</start>" << endl;
      ts << indent << "  <num>" << _numberOfFrames << "</num>" << endl;
    } else {
      ts << indent << "  <start>" << _reqStartingFrame << "</start>" << endl;
      ts << indent << "  <num>" << _reqNumberFrames << "</num>" << endl;
    }

    if (doSkip()) {
      ts << indent << "  <skip>" << _skip << "</skip>" << endl;
      if (doAve()) {
        ts << indent << "  <doAve/>" << endl;
      }
    }
    ts << indent << "</vector>" << endl;
  }
}


QString KstRVector::filename() const {
  QString rc;
  if (_file) {
    _file->readLock();
    rc = _file->fileName();
    _file->unlock();
  }
  return rc;
}


const QString& KstRVector::field() const {
  return _field;
}


QString KstRVector::label() const {
  bool ok;
  QString label;

  _field.toInt(&ok);
  if (ok && _file) {
    _file->readLock();
    if (_file->fileType() == "ASCII") {
      label = i18n("Column %1").arg(_field);
    } else {
      label = _field;
    }
    _file->unlock();
  } else {
    label = _field;
  }

  return label;
}


void KstRVector::reset() { // must be called with a lock
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  _dontUseSkipAccel = false;
  if (_file) {
    _samplesPerFrame = _file->samplesPerFrame(_field);
  }
  _startingFrame = _numberOfFrames = 0;
  resize(0);
  _numSamples = 0;
  _dirty = true;
}


void KstRVector::checkIntegrity() {
  if (_doSkip && _skip < 1) {
    _skip = 1;
  }

  if (_dirty) {
    reset();
  }

  // if it looks like we have a new file, reset
  if (_file && (_samplesPerFrame != _file->samplesPerFrame(_field) || _file->frameCount(_field) < _numberOfFrames)) {
    reset();
  }

  // check for illegal _numberOfFrames and _startingFrame values
  if (_reqNumberFrames < 1 && _reqStartingFrame < 0) {
    _reqStartingFrame = 0; // for this illegal case, read the whole file
  }

  if (_reqNumberFrames == 1) {
    _reqNumberFrames = 2;
  }
}


KstObject::UpdateType KstRVector::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();
  setDirty(false);
  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  if (_file) {
    _file->writeLock();
  }

  KstObject::UpdateType rc = doUpdate(force);
  if (_file) {
    _file->unlock();
  }

  setDirty(false);
  return setLastUpdateResult(rc);
}

// Some things to consider about the following routine...
// Frames:
//    Some data sources have data divided into frames.  Each field
//    has a fixed number of samples per frame.  For some (eg, ascii files)
//    each frame has 1 sample.  For others (eg, dirfiles) you may have more.
//    Different fields in the same data source may have different samples per frame.
//    Within a data source, it is assumed that the first sample of each frame is
//    simultaneous between fields.
// Last Frame Read:
//    Only read the first sample of the last frame read, in cases where there are more
//    than one sample per frame.   This allows for more sensible association of vectors
//    into curves, when the X and Y vectors have different numbers of samples per frame.
//    The rule is that we assume is that the first sample of each frame is simultaneous.
// Skip reading:  
//    -'Skip' means read 1 sample each 'Skip' frames (not read one sample,
//     then skip 'Skip' samples or something else).
//    -In order that the data are not re-drawn each time a new sample arrives, and to
//     ensure the re-usability (via shifting) of previously read data, and to make a
//     region of data look the same regardless of the choice of startingFrame, all samples
//     read with skip enabled are read on 'skip boundries'... ie, the first samples of
//     frame 0, Skip, 2*Skip... N*skip, and never M*Skip+1.

KstObject::UpdateType KstRVector::doUpdate(bool force) {
  int i, k, shift, nRead=0;
  int ave_nread;
  int newStartingFrame, newNumberOfFrames;
  bool startPastEOF = false;

  checkIntegrity();

  if (_doSkip && _skip < 2 && _samplesPerFrame == 1) {
    _doSkip = false;
  }

  if (!_file) {
    return NO_CHANGE;
  }

  // set newNumberOfFrames and newStartingFrame
  int frameCount = _file->frameCount(_field);
  if (_reqNumberFrames < 1) { // read to end of file
    newStartingFrame = _reqStartingFrame;
    newNumberOfFrames = frameCount - newStartingFrame;
  } else if (_reqStartingFrame < 0) { // count back from end of file
    newNumberOfFrames = frameCount;
    if (newNumberOfFrames > _reqNumberFrames) {
      newNumberOfFrames = _reqNumberFrames;
    }
    newStartingFrame = frameCount - newNumberOfFrames;
  } else {
    newStartingFrame = _reqStartingFrame;
    newNumberOfFrames = _reqNumberFrames;
    if (newStartingFrame + newNumberOfFrames > frameCount) {
      newNumberOfFrames = frameCount - newStartingFrame;
    }
    if (newNumberOfFrames <= 0) {
      // Tried to read starting past the end.
      newStartingFrame = 0;
      newNumberOfFrames = 1;
      startPastEOF = true;
    }
  }

  if (_doSkip) {
    // change newStartingFrame and newNumberOfFrames so they both lie on skip boundaries
    if (newStartingFrame != 0) {
      newStartingFrame = ((newStartingFrame-1)/_skip+1)*_skip;
    }
    newNumberOfFrames = (newNumberOfFrames/_skip)*_skip;
  }

  if (_numberOfFrames == newNumberOfFrames && _startingFrame == newStartingFrame && !force) {
    return NO_CHANGE;
  }

  // shift vector if necessary
  if (newStartingFrame < _startingFrame || newStartingFrame >= _startingFrame + _numberOfFrames) { // No useful data around.
    reset();
  } else { // shift stuff rather than re-read
    if (_doSkip) {
      shift = (newStartingFrame - _startingFrame)/_skip;
      _numberOfFrames -= (newStartingFrame - _startingFrame);
      _numSamples = _numberOfFrames/_skip;
    } else {
      shift = _samplesPerFrame*(newStartingFrame - _startingFrame);
      _numberOfFrames -= (newStartingFrame - _startingFrame);
      _numSamples = (_numberOfFrames-1)*_samplesPerFrame+1;
    }

    // FIXME: use memmove()
    for (i = 0; i < _numSamples; i++) {
      _v[i] = _v[i+shift];
    }
  }

  if (_doSkip) {
    // reallocate V if necessary
    //kstdDebug() << "newNumberOfFrames = " << newNumberOfFrames << " and skip = " << Skip << " so newNumberOfFrames/Skip+1 = " << (newNumberOfFrames / Skip + 1) << endl;
    if (newNumberOfFrames / _skip != _size) {
      bool rc = resize(newNumberOfFrames/_skip);
      if (!rc) {
        // FIXME: handle failed resize
      }
    }
    // for debugging: _dontUseSkipAccel = true;
    if (!_dontUseSkipAccel) {
      int rc;
      int lastRead = -1;
      if (_doAve) {
        // We don't support boxcar inside data sources yet.
        _dontUseSkipAccel = true;
      } else {
        rc = _file->readField(_v + _numSamples, _field, newStartingFrame, (newNumberOfFrames - _numberOfFrames)/_skip, _skip, &lastRead);
        if (rc != -9999) {
          //kstdDebug() << "USED SKIP FOR READ - " << _field << " - rc=" << rc << " for Skip=" << Skip << " s=" << newStartingFrame << " n=" << (newNumberOfFrames - _numberOfFrames)/Skip << endl;
          if (rc >= 0) {
            nRead = rc;
          } else {
            nRead = 0;
          }
        } else {
          _dontUseSkipAccel = true;
        }
      }
    }
    if (_dontUseSkipAccel) {
      nRead = 0;
      /** read each sample from the File */
      //kstdDebug() << "NF = " << NF << " numsamples = " << _numSamples << " newStartingFrame = " << newStartingFrame << endl;
      double *t = _v + _numSamples;
      int new_nf_Skip = newNumberOfFrames - _skip;
      if (_doAve) {
        for (i = _numberOfFrames; new_nf_Skip >= i; i += _skip) {
          /* enlarge AveReadBuf if necessary */
          if (_nAveReadBuf < _skip * _samplesPerFrame) {
            _nAveReadBuf = _skip * _samplesPerFrame;
            _aveReadBuf = static_cast<double*>(realloc(_aveReadBuf, _nAveReadBuf*sizeof(double)));
            if (!_aveReadBuf) {
              // FIXME: handle failed resize
            }
          }
          ave_nread = _file->readField(_aveReadBuf, _field, newStartingFrame+i, _skip);
          for (k = 1; k < ave_nread; k++) {
            _aveReadBuf[0] += _aveReadBuf[k];
          }
          if (ave_nread > 0) {
            *t = _aveReadBuf[0]/double(ave_nread);
            nRead++;
          }
          ++t;
        }
      } else {
        for (i = _numberOfFrames; new_nf_Skip >= i; i += _skip) {
          //kstdDebug() << "readField " << _field << " start=" << newStartingFrame + i << " n=-1" << endl;
          nRead += _file->readField(t++, _field, newStartingFrame + i, -1);
        }
      }
    }
  } else {
    // reallocate V if necessary
    if ((newNumberOfFrames - 1)*_samplesPerFrame + 1 != _size) {
      bool rc = resize((newNumberOfFrames - 1)*_samplesPerFrame + 1);
      if (!rc) {
        // FIXME: handle failed resize
        abort();
      }
    }

    if (_numberOfFrames > 0) {
      _numberOfFrames--; /* last frame read was only partially read... */
    }

    // read the new data from file
    if (startPastEOF) {
      _v[0] = KST::NOPOINT;
      nRead = 1;
    } else if (_file->samplesPerFrame(_field) > 1) {
      assert(newNumberOfFrames - _numberOfFrames - 1 > 0 || newNumberOfFrames - _numberOfFrames - 1 == -1 || force);
      assert(newStartingFrame + _numberOfFrames >= 0);
      assert(newStartingFrame + newNumberOfFrames - 1 >= 0);
      nRead = _file->readField(_v+_numberOfFrames*_samplesPerFrame, _field, newStartingFrame + _numberOfFrames, newNumberOfFrames - _numberOfFrames - 1);
      nRead += _file->readField(_v+(newNumberOfFrames-1)*_samplesPerFrame, _field, newStartingFrame + newNumberOfFrames - 1, -1);
    } else {
      //kstdDebug() << "Reading into _v=" << (void*)_v << " which has size " << _size << " and starting at offset " << NF*_samplesPerFrame << " for s=" << newStartingFrame + NF << " and n=" << newNumberOfFrames - NF << endl;
      assert(newStartingFrame + _numberOfFrames >= 0);
      if (newNumberOfFrames - _numberOfFrames > 0 || newNumberOfFrames - _numberOfFrames == -1) {
        nRead = _file->readField(_v+_numberOfFrames*_samplesPerFrame, _field, newStartingFrame + _numberOfFrames, newNumberOfFrames - _numberOfFrames);
      }
    }
  }

  NumNew = _size - _numSamples;
  _numberOfFrames = newNumberOfFrames;
  _startingFrame = newStartingFrame;
  _numSamples += nRead;

  // if for some reason (eg, realtime reading an nfs mounted
  // dirfile) not all of the data was read, the data will never
  // be read; the samples will be filled in with the last data
  // point read, and a complete reset will be requested.
  // This is bad - I think it will be worthwhile
  // to add blocking w/ timeout to KstFile.
  // As a first fix, mount all nsf mounted dirfiles with "-o noac"
  _dirty = false;
  if (_numSamples != _size && !(_numSamples == 0 && _size == 1)) {
    //kstdDebug() << "SET DIRTY since _numSamples = " << _numSamples << " but _size = " << _size << endl;
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

  return KstVector::internalUpdate(UPDATE);
}


/** Returns intrinsic samples per frame */
int KstRVector::samplesPerFrame() const {
  return _samplesPerFrame;
}


/** return true if it has a valid file and field, or false otherwise */
bool KstRVector::isValid() const {
  if (_file) {
    _file->readLock();
    bool rc = _file->isValidField(_field);
    _file->unlock();
    return rc;
  }
  return false;
}


int KstRVector::fileLength() const {
  if (_file) {
    _file->readLock();
    int rc = _file->frameCount(_field);
    _file->unlock();

    return rc;
  }

  return 0;
}


void KstRVector::reload() {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  if (_file) {
    _file->writeLock();
    if (_file->reset()) { // try the efficient way first
      reset();
    } else { // the inefficient way
      KstDataSourcePtr newsrc = KstDataSource::loadSource(_file->fileName(), _file->fileType());
      assert(newsrc != _file);
      if (newsrc) {
        _file->unlock();
        KST::dataSourceList.lock().writeLock();
        KST::dataSourceList.remove(_file);
        _dontUseSkipAccel = false;
        _file = newsrc;
        _file->writeLock();
        KST::dataSourceList.append(_file);
        KST::dataSourceList.lock().unlock();
        reset();
      }
    }
    _file->unlock();
  }
}


KstDataSourcePtr KstRVector::dataSource() const {
  return _file;
}


KstRVectorPtr KstRVector::makeDuplicate() const {
  QString newTag = tag().tag() + "'";

  return new KstRVector(_file, _field, KstObjectTag(newTag, tag().context()), _reqStartingFrame, _reqNumberFrames, _skip, _doSkip, _doAve);
}

