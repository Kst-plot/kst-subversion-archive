/*
 *  Copyright 2004, The University of Toronto
 *  Licensed under GPL.
 */
#ifndef SLISTENER_H
#define SLISTENER_H
#include <qobject.h>

class SListener : public QObject {
  Q_OBJECT
  public:
    SListener();
    virtual ~SListener();
  public slots:
    void trigger();

    int _trigger;
};

#endif
// vim: ts=2 sw=2 et
