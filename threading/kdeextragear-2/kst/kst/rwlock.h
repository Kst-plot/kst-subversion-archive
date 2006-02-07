/***************************************************************************
                                  rwlock.h 
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

#ifndef RWLOCK_H
#define RWLOCK_H

#include <qsemaphore.h>

class KstRWLock {
  public:
    KstRWLock();
    virtual ~KstRWLock();

    virtual void readLock() const;
    virtual void readUnlock() const;

    virtual void writeLock() const;
    virtual void writeUnlock() const;

  protected:
    mutable QSemaphore _sem;
};


class KstReadLocker {
  public:
    KstReadLocker(KstRWLock *l) : _l(l) { l->readLock(); }
    ~KstReadLocker() { _l->readUnlock(); }
  private:
    KstRWLock *_l;
};


class KstWriteLocker {
  public:
    KstWriteLocker(KstRWLock *l) : _l(l) { l->writeLock(); }
    ~KstWriteLocker() { _l->writeUnlock(); }
  private:
    KstRWLock *_l;
};

#endif
// vim: ts=2 sw=2 et
