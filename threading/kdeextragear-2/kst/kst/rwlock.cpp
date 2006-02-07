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
#include <assert.h>
#include <stdio.h>

#define MAXREADERS 30

KstRWLock::KstRWLock()
: _sem(MAXREADERS) {
}


KstRWLock::~KstRWLock() {
}


void KstRWLock::readLock() const {
  //printf("%p Read lock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  _sem++;
  //printf("%p Read locked %d/%d\n", (void*)this, _sem.available(), _sem.total());
}


void KstRWLock::readUnlock() const {
  //printf("%p Read unlock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  _sem--;
  //printf("%p Read unlocked %d/%d\n", (void*)this, _sem.available(), _sem.total());
}


void KstRWLock::writeLock() const {
  //printf("%p Write lock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  _sem += MAXREADERS;
  //printf("%p Write locked %d/%d\n", (void*)this, _sem.available(), _sem.total());
}


void KstRWLock::writeUnlock() const {
  //printf("%p Write unlock %d/%d\n", (void*)this, _sem.available(), _sem.total());
  _sem -= MAXREADERS;
  //printf("%p Write unlocked %d/%d\n", (void*)this, _sem.available(), _sem.total());
}


// vim: ts=2 sw=2 et
