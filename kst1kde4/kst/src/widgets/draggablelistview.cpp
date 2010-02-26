/***************************************************************************
                            draggablelistview.cpp
                             -------------------
    begin                : Jul 19, 2006
    copyright            : (C) 2006 The University of Toronto
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

#include <QApplication>
#include <QMouseEvent>
#include "draggablelistview.h"

DraggableListView::DraggableListView(QWidget *parent, const char *name)
: QListView(parent), _pressPos(-1, -1), _dragEnabled(false) {
}


DraggableListView::~DraggableListView() {
}


bool DraggableListView::dragEnabled() const {
  return _dragEnabled;
}


void DraggableListView::setDragEnabled(bool en) {
  _dragEnabled = en;
}


QMimeData *DraggableListView::dragObject() {
  return 0L;
}


void DraggableListView::mousePressEvent(QMouseEvent *e) {
  if (_dragEnabled) {
    _pressPos = QPoint(-1, -1);

    if (e->button() & Qt::LeftButton) {
/* xxx
      QListViewItem *item;
      if ((item = itemAt(e->pos()))) {
        setCurrentItem(item);
        if (!item->isSelected()) {
          if (!(e->state() & Qt::ControlButton)) {
            clearSelection();
          }
          setSelected(item, true);
        }
        _pressPos = e->pos();
        e->accept();
        return;
      }
*/
    }
  }

  QListView::mousePressEvent(e);
}


void DraggableListView::mouseReleaseEvent(QMouseEvent *e) {
  _pressPos = QPoint(-1, -1);
  QListView::mouseReleaseEvent(e);
}


void DraggableListView::mouseMoveEvent(QMouseEvent *e) {
  if (_dragEnabled && ( e->buttons() & Qt::LeftButton ) && _pressPos != QPoint(-1, -1)) {
    QPoint delta = e->pos() - _pressPos;
    if (delta.manhattanLength() > QApplication::startDragDistance()) {
      _pressPos = QPoint(-1, -1);
      startDrag();
    }
    e->accept();
  } else {
    QListView::mouseMoveEvent(e);
  }
}


void DraggableListView::startDrag() {
  QMimeData *o = dragObject();
  QDrag drag(this);

  drag.setMimeData(o);
  if (o) {
    drag.start();
  }
}


