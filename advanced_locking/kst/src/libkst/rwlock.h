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
#include <qmap.h>
#include <qthread.h>

#include "kstwaitcondition.h"

#include "kst_export.h"

class KST_EXPORT KstRWLock {
  public:
    KstRWLock();
    virtual ~KstRWLock();

    virtual void readLock() const;
    virtual void writeLock() const;

    virtual void unlock() const;

  protected:
    mutable QMutex _mutex;
    mutable KstWaitCondition _readerWait, _writerWait;

    mutable int _readCount, _writeCount;
    mutable int _waitingReaders, _waitingWriters;

    mutable Qt::HANDLE _writeLocker;
    mutable QMap<Qt::HANDLE, int> _readLockers;

    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};


class KST_EXPORT KstReadLocker {
  public:
    KstReadLocker(KstRWLock *l) : _l(l) { _l->readLock(); }
    ~KstReadLocker() { _l->unlock(); }
  private:
    KstRWLock *_l;
    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};


class KST_EXPORT KstWriteLocker {
  public:
    KstWriteLocker(KstRWLock *l) : _l(l) { _l->writeLock(); }
    ~KstWriteLocker() { _l->unlock(); }
  private:
    KstRWLock *_l;
    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.
};

#endif
// vim: ts=2 sw=2 et
