/***************************************************************************
                                 rwlock.cpp 
                             -------------------
    begin                : Feb 21, 2004
    copyright            : (C) 2004 The University of toronto
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

#include "ksdebug.h"

//#define LOCKTRACE

#ifdef ONE_LOCK_TO_RULE_THEM_ALL
QMutex KstRWLock::_mutex(true);
#endif

KstRWLock::KstRWLock()
: _readCount(0), _writeCount(0), _waitingReaders(0), _waitingWriters(0) {
}


KstRWLock::~KstRWLock() {
}


void KstRWLock::readLock() const {
#ifndef ONE_LOCK_TO_RULE_THEM_ALL
  QMutexLocker lock(&_mutex);
  
#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::readLock() by tid=" << (int)QThread::currentThread() << endl;
#endif

  Qt::HANDLE me = QThread::currentThread();

  if (_writeCount > 0 && _writeLocker == me) {
    // thread already has a write lock
#ifdef LOCKTRACE
    kstdDebug() << "Thread " << (int)QThread::currentThread() << " has a write lock on KstRWLock " << (void*)this << ", getting a read lock" << endl;
#endif
  } else {
    while (_writeCount > 0 || _waitingWriters) {
      ++_waitingReaders;
      _readerWait.wait(&_mutex);
      --_waitingReaders;
    }
  }

  _readLockers[me] = _readLockers[me] + 1;
  ++_readCount;

#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::readLock() done by tid=" << (int)QThread::currentThread() << endl;
#endif
#else
  _mutex.lock();
#endif
}


void KstRWLock::writeLock() const {
#ifndef ONE_LOCK_TO_RULE_THEM_ALL
  QMutexLocker lock(&_mutex);

#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::writeLock() by tid=" << (int)QThread::currentThread() << endl;
#endif

  Qt::HANDLE me = QThread::currentThread();

  if (_readCount > 0) {
    QMap<Qt::HANDLE, int>::Iterator it = _readLockers.find(me);
    if (it != _readLockers.end() && it.data() > 0) {
      // cannot acquire a write lock if I already have a read lock -- ERROR
      kstdFatal() << "Thread " << (int)QThread::currentThread() << " tried to write lock KstRWLock " << (void*)this << " while holding a read lock" << endl;
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

#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::writeLock() done by tid=" << (int)QThread::currentThread() << endl;
#endif
#else
  _mutex.lock();
#endif
}


void KstRWLock::unlock() const {
#ifndef ONE_LOCK_TO_RULE_THEM_ALL
  QMutexLocker lock(&_mutex);

#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::unlock() by tid=" << (int)QThread::currentThread() << endl;
#endif

  Qt::HANDLE me = QThread::currentThread();

  if (_readCount > 0) {
    QMap<Qt::HANDLE, int>::Iterator it = _readLockers.find(me);
    if (it == _readLockers.end()) {
      // read locked but not by me -- ERROR
      kstdFatal() << "Thread " << (int)QThread::currentThread() << " tried to unlock KstRWLock " << (void*)this << " (read locked) without holding the lock" << endl;
      return;
    } else {
      --_readCount;
      if (it.data() == 1) {
        _readLockers.remove(it);
      } else {
        --(it.data());
      }
    }
  } else if (_writeCount > 0) {
    if (_writeLocker != me) {
      // write locked but not by me -- ERROR
      kstdFatal() << "Thread " << (int)QThread::currentThread() << " tried to unlock KstRWLock " << (void*)this << " (write locked) without holding the lock" << endl;
      return;
    } else {
      --_writeCount;
    }
  } else if (_readCount == 0 && _writeCount == 0) {
    // not locked -- ERROR
    kstdFatal() << "Thread " << (int)QThread::currentThread() << " tried to unlock KstRWLock " << (void*)this << " (unlocked) without holding the lock" << endl;
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

#ifdef LOCKTRACE
  kstdDebug() << (void*)this << " KstRWLock::unlock() done by tid=" << (int)QThread::currentThread() << endl;
#endif
#else
  _mutex.unlock();
#endif
}


// vim: ts=2 sw=2 et
