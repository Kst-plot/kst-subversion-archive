/***************************************************************************
                                 rwlock.cpp 
                             -------------------
    begin                : Feb 21, 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "rwlock.h"

KstRWLock::KstRWLock()
: _readCount(0), _writeCount(0), _waitingReaders(0), _waitingWriters(0) {
}


KstRWLock::~KstRWLock() {
}


void KstRWLock::readLock() const {
  QMutexLocker lock(&_mutex);

  Qt::HANDLE me = QThread::currentThreadId();

  if (_writeCount > 0 && _writeLocker == me) {

  } else {
    QMap<Qt::HANDLE, int>::Iterator it = _readLockers.find(me);

    if (it != _readLockers.end() && *it > 0) {
      // thread already has another read lock
    } else {
      while (_writeCount > 0 || _waitingWriters) {  // writer priority otherwise
        ++_waitingReaders;
        _readerWait.wait(&_mutex);
        --_waitingReaders;
      }
    }
  }

  _readLockers[me] = _readLockers[me] + 1;
  ++_readCount;
}


void KstRWLock::writeLock() const {
  QMutexLocker lock(&_mutex);

  Qt::HANDLE me = QThread::currentThreadId();

  if (_readCount > 0) {
    QMap<Qt::HANDLE, int>::Iterator it = _readLockers.find(me);
    if (it != _readLockers.end() && *it > 0) {
      //
      // cannot acquire a write lock if I already have a read lock -- ERROR
      //

      qFatal("[1] Thread 0x%X tried to write lock KstRWLock 0x%X while holding a read lock\n", (int)QThread::currentThread(), (int)this );

      return;
    }
  }

  while (_readCount > 0 || (_writeCount > 0 && _writeLocker != me)) {
    ++_waitingWriters;
    _writerWait.wait(&_mutex);
    --_waitingWriters;
  }
  _writeLocker = me;
  ++_writeCount;
}


void KstRWLock::unlock() const {
  QMutexLocker lock(&_mutex);

  Qt::HANDLE me = QThread::currentThreadId();

  if (_readCount > 0) {
    QMap<Qt::HANDLE, int>::Iterator it = _readLockers.find(me);
    if (it == _readLockers.end()) {
      //
      // read locked but not by me -- ERROR
      //
      
      qFatal("[1] Thread 0x%X tried to unlock KstRWLock 0x%X (read locked) without holding the lock\n", (int)QThread::currentThread(), (int)this );
      
      return;
    } else {
      --_readCount;
      if (*it == 1) {
        _readLockers.remove(*it);
      } else {
        --(*it);
      }
    }
  } else if (_writeCount > 0) {
    if (_writeLocker != me) {
      //
      // write locked but not by me -- ERROR
      //

      qFatal("[2] Thread 0x%X tried to unlock KstRWLock 0x%X (write locked) without holding the lock\n", (int)QThread::currentThread(), (int)this );

      return;
    } else {
      --_writeCount;
    }
  } else if (_readCount == 0 && _writeCount == 0) {
    //
    // not locked -- ERROR
    //
    
    qFatal("[3] Thread 0x%X tried to unlock KstRWLock 0x%X (unlocked) without holding the lock\n", (int)QThread::currentThread(), (int)this );

    return;
  }

  if (_readCount == 0 && _writeCount == 0) {
    // no locks remaining
    if (_waitingWriters) {
      _writerWait.wakeOne();
    } else if (_waitingReaders) {
      _readerWait.wakeAll();
    }
  }
}


KstRWLock::LockStatus KstRWLock::lockStatus() const {
  QMutexLocker lock(&_mutex);

  if (_writeCount > 0) {
    return WRITELOCKED;
  } else if (_readCount > 0) {
    return READLOCKED;
  } else {
    return UNLOCKED;
  }
}


KstRWLock::LockStatus KstRWLock::myLockStatus() const {
  QMutexLocker lock(&_mutex);

  Qt::HANDLE me = QThread::currentThreadId();

  if (_writeCount > 0 && _writeLocker == me) {
    return WRITELOCKED;
  } else if (_readCount > 0 && _readLockers.find(me) != _readLockers.end()) {
    return READLOCKED;
  } else {
    return UNLOCKED;
  }
}
