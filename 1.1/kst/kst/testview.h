/***************************************************************************
    copyright            : (C) 2003 The University of Toronto
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <qpoint.h>
#include <qwidget.h>
#include "ksttoplevelview.h"

class TestView : public QWidget {
  Q_OBJECT
  public:
    TestView(QWidget *parent);
    virtual ~TestView();

    KstTopLevelViewPtr view() const { return _view; }
  private slots:
    void new2dPlot();
    void newLabel();
    void cleanup();
    void clear();
    void layoutMode(bool on);

  private:
    KstTopLevelViewPtr _view;
    QPoint _pos;
};


#endif
// vim: ts=2 sw=2 et
