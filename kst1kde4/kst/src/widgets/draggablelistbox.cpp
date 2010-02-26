/***************************************************************************
                             draggablelistbox.cpp
                             -------------------
    begin                : Jul 19, 2004
    copyright            : (C) 2004 The University of Toronto
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
#include "draggablelistbox.h"

DraggableListBox::DraggableListBox(QWidget *parent, const char *name)
: QListWidget(parent), _pressPos(-1, -1), _dragEnabled(false) {
}


DraggableListBox::~DraggableListBox() {
}


bool DraggableListBox::dragEnabled() const {
  return _dragEnabled;
}


void DraggableListBox::setDragEnabled(bool en) {
  _dragEnabled = en;
}


QMimeData *DraggableListBox::dragObject() {
  return 0L;
}


void DraggableListBox::mousePressEvent(QMouseEvent *e) {
  if (_dragEnabled) {
    _pressPos = QPoint(-1, -1);

    if (e->button() & Qt::LeftButton /* xxx && !isRubberSelecting()*/) {
      QListWidgetItem *item;

      if ((item = itemAt(e->pos()))) {
        setCurrentItem(item);
        if (!item->isSelected()) {
          if (!(e->buttons() & Qt::ControlModifier)) {
            clearSelection();
          }
          item->setSelected(true);
        }
        _pressPos = e->pos();
        e->accept();
        return;
      }
    }
  }

  QListWidget::mousePressEvent(e);
}


void DraggableListBox::mouseReleaseEvent(QMouseEvent *e) {
  _pressPos = QPoint(-1, -1);

  QListWidget::mouseReleaseEvent(e);
}


void DraggableListBox::mouseMoveEvent(QMouseEvent *e) {
  if (_dragEnabled && (e->buttons() & Qt::LeftButton) && _pressPos != QPoint(-1, -1)) {
    QPoint delta = e->pos() - _pressPos;

    if (delta.manhattanLength() > QApplication::startDragDistance()) {
      _pressPos = QPoint(-1, -1);
      startDrag();
    }
    e->accept();
  } else {
    QListWidget::mouseMoveEvent(e);
  }
}


void DraggableListBox::startDrag() {
  QMimeData *o = dragObject();
  QDrag drag(this);

  drag.setMimeData(o);
  if (o) {
    drag.start();
  }
}

bool DraggableListBox::up() {
  bool bRetVal = false;
  int i;

  if (count() > 1) {
    for (i=1; i<count(); i++) {
      if (item(i)->isSelected()) {
        insertItem(i-1, takeItem(i));
        i--;

        while (i < count( ) && item(i)->isSelected( )) {
          ++i;
        }

        bRetVal = true;
      }
    }
  }

  return bRetVal;
}

bool DraggableListBox::down() {
  bool bRetVal = false;
  int i;

  if (count() > 1) {
    for (i = count()-2; i>=0; i--) {
      if (item(i)->isSelected()) {
        insertItem(i+1, takeItem(i));
        i++;

        while (i > 0 && item(i)->isSelected( )) {
          --i;
        }

        bRetVal = true;
      }
    }
  }

  return bRetVal;
}

//#include "draggablelistbox.moc"

