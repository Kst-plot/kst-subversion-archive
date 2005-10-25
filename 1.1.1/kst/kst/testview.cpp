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

#include "testview.h"
#include "ksttoplevelview.h"
#include "kst2dplot.h"
#include "kstviewlabel.h"
#include "kstviewwidget.h"

#include <kdebug.h>

#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>

TestView::TestView(QWidget *parent)
: QWidget(parent, "test view") {
  QVBoxLayout *vb = new QVBoxLayout(this);
  _view = new KstTopLevelView(this, "test toplevel view");
  _view->setTagName("Top Level View");
  QSizePolicy pol = _view->widget()->sizePolicy();
  pol.setVerStretch(2);
  _view->widget()->setSizePolicy(pol);
  vb->addWidget(_view->widget());

  QFrame *f = new QFrame(this);
  f->setLineWidth(1);

  QHBoxLayout *hb = new QHBoxLayout(f);
  QPushButton *p = new QPushButton("New 2d plot", f);
  connect(p, SIGNAL(clicked()), this, SLOT(new2dPlot()));
  hb->addWidget(p);
  p = new QPushButton("New label", f);
  connect(p, SIGNAL(clicked()), this, SLOT(newLabel()));
  hb->addWidget(p);
  p = new QPushButton("Cleanup", f);
  connect(p, SIGNAL(clicked()), this, SLOT(cleanup()));
  hb->addWidget(p);
  p = new QPushButton("Clear", f);
  connect(p, SIGNAL(clicked()), this, SLOT(clear()));
  hb->addWidget(p);
  QCheckBox *c = new QCheckBox("Layout mode", f);
  connect(c, SIGNAL(toggled(bool)), this, SLOT(layoutMode(bool)));
  hb->addWidget(c);

  hb->addItem(new QSpacerItem(10,10));

  vb->addWidget(f);
  _pos = QPoint(0, 0);
}


TestView::~TestView() {
}


void TestView::new2dPlot() {
  Kst2DPlot *p = new Kst2DPlot;
  p->setTagName(QString("Plot %1").arg(_view->children().count()));
  p->setBorderColor(Qt::black);
  p->setBorderWidth(2);
  p->setMargin(4);
  p->setPadding(2);
  p->resize(QSize(50, 50));
  p->move(_pos);
  _pos += QPoint(24, 12);
  _view->appendChild(p);
  _view->paint(P_PLOT);
}


void TestView::newLabel() {
  KstViewLabel *p = new KstViewLabel("This \\tau\\epsilon\\sigma\\tau \\\\ \\alfa\\\\\\sum_\\inf 2^n A_{m x n} x^n^n^n^n^n^n^n");
  p->setTagName(QString("Label %1").arg(_view->children().count()));
  p->adjustSizeForText();
  p->move(_pos);
  p->setRotation(30.0);
  _pos += QPoint(24, 12);
  _view->appendChild(p);
  _view->paint(P_PLOT);
}


void TestView::cleanup() {
  _view->cleanup();
  _view->paint(P_PAINT, true);
}


void TestView::clear() {
  _view->clearChildren();
  _view->paint(P_PAINT, true);
}


void TestView::layoutMode(bool on) {
  _view->setViewMode(on ? KstTopLevelView::LayoutMode : KstTopLevelView::DisplayMode);
}


#include "testview.moc"

// vim: ts=2 sw=2 et
