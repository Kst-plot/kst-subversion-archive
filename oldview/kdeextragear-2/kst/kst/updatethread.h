/***************************************************************************
                               updatethread.h
                              ----------------
    begin                : Dec 29 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>

class KstDoc;

class UpdateThread : public QThread {
  public:
    UpdateThread(KstDoc*);
    virtual ~UpdateThread();

    void setPaused(bool paused);
    void setFinished(bool finished);
    void setUpdateTime(int updateTime);
    void forceUpdate();

  protected:
    virtual void run();

    bool doUpdates(bool force = false);

  private:
    bool _paused, _done;
    QWaitCondition _waitCondition;
    QMutex _statusMutex;
    KstDoc *_doc;
    int _updateCounter;
    int _updateTime;
    bool _force;
};


#endif

// vim: ts=2 sw=2 et
