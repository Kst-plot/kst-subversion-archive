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

#define MAXREADERS 30

//#define LOCKTRACE

#ifdef LOCKTRACE
#include <stdio.h>
#include <qthread.h>
#endif


KstRWLock::KstRWLock()
: _sem(MAXREADERS), _writeLock(true), _writeRecursion(0) {
}


KstRWLock::~KstRWLock() {
}


void KstRWLock::readLock() const {
  #ifdef LOCKTRACE
  printf("%p Read lock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  #endif
  _sem++;
  #ifdef LOCKTRACE
  printf("%p Read locked %d/%d\n", (void*)this, _sem.available(), _sem.total());
  #endif
}


void KstRWLock::readUnlock() const {
  #ifdef LOCKTRACE
  printf("%p Read unlock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  #endif
  _sem--;
  #ifdef LOCKTRACE
  printf("%p Read unlocked %d/%d\n", (void*)this, _sem.available(), _sem.total());
  #endif
}


void KstRWLock::writeLock() const {
  #ifdef LOCKTRACE
  kstdDebug() << (void*) this << " Write lock " << _sem.available() << "/" << _sem.total() << " tid=" << (int)QThread::currentThread() << endl;
  #endif
  _writeLock.lock();
  if (_writeRecursion == 0) {
    _sem += MAXREADERS;
  }
  _writeRecursion++;
  #ifdef LOCKTRACE
  kstdDebug() << (void*) this << " Write locked " << _sem.available() << "/" << _sem.total() << " tid=" << (int)QThread::currentThread() << endl;
  #endif
}


void KstRWLock::writeUnlock() const {
  #ifdef LOCKTRACE
  kstdDebug() << (void*) this << " Write unlock " << _sem.available() << "/" << _sem.total() << " tid=" << (int)QThread::currentThread() << endl;
  #endif
  _writeRecursion--;
  if (_writeRecursion == 0) {
    _sem -= MAXREADERS;
  }
  #ifdef LOCKTRACE
  kstdDebug() << (void*) this << " Write unlocked " << _sem.available() << "/" << _sem.total() << " tid=" << (int)QThread::currentThread() << endl;
  #endif
  _writeLock.unlock();
}


// vim: ts=2 sw=2 et
