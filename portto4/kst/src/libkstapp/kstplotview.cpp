/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kstplotview.h"
#include "kstmainwindow.h"
#include "kstapplication.h"

#include <QDebug>
#include <QUndoStack>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

KstPlotView::KstPlotView()
    : QGraphicsView(kstApp->mainWindow()),
      _currentPlotItem(0), _mouseMode(Default) {

  _undoStack = new QUndoStack(this);
  setScene(new QGraphicsScene(this));
  scene()->installEventFilter(this);
}


KstPlotView::~KstPlotView() {
}


QUndoStack *KstPlotView::undoStack() const {
  return _undoStack;
}


KstPlotItem *KstPlotView::currentPlotItem() const {
  return _currentPlotItem;
}


KstPlotView::MouseMode KstPlotView::mouseMode() const {
  return _mouseMode;
}


void KstPlotView::setMouseMode(MouseMode mode) {

  if (_mouseMode == Create)
    _creationPolygon.clear();

  _mouseMode = mode;
}


QPolygonF KstPlotView::creationPolygon() const {
  return _creationPolygon;
}


bool KstPlotView::eventFilter(QObject *obj, QEvent *event) {
  if (obj != scene())
    return QGraphicsView::eventFilter(obj, event);

  switch (event->type()) {
  case QEvent::GraphicsSceneMousePress:
    {
      QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);
      if (_mouseMode == Create) {
        _creationPolygon << e->buttonDownScreenPos(Qt::LeftButton);
        emit creationPolygonChanged();
        return false;
      }
    }
  case QEvent::GraphicsSceneMouseRelease:
  case QEvent::GraphicsSceneMouseMove:
  default:
    return QGraphicsView::eventFilter(obj, event);
  }
}

#include "kstplotview.moc"

// vim: ts=2 sw=2 et
