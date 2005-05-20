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

#include <qmutex.h>
#include <qsemaphore.h>

#include "kst_export.h"

class KST_EXPORT KstRWLock {
  public:
    KstRWLock();
    virtual ~KstRWLock();

    virtual void readLock() const;
    virtual void readUnlock() const;

    virtual void writeLock() const;
    virtual void writeUnlock() const;

  protected:
    mutable QSemaphore _sem;
    mutable QMutex _writeLock;
    mutable int _writeRecursion;
    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};


class KST_EXPORT KstReadLocker {
  public:
    KstReadLocker(KstRWLock *l) : _l(l) { l->readLock(); }
    ~KstReadLocker() { _l->readUnlock(); }
  private:
    KstRWLock *_l;
    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};


class KST_EXPORT KstWriteLocker {
  public:
    KstWriteLocker(KstRWLock *l) : _l(l) { l->writeLock(); }
    ~KstWriteLocker() { _l->writeUnlock(); }
  private:
    KstRWLock *_l;
    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};

#endif
// vim: ts=2 sw=2 et
