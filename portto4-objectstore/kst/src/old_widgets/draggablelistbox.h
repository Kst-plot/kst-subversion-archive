/***************************************************************************
                             draggablelistbox.h
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

#ifndef DRAGGABLELISTBOX_H
#define DRAGGABLELISTBOX_H

#include <q3listbox.h>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QMouseEvent>

#include "kst_export.h"

class DraggableListBox : public Q3ListBox {
  Q_OBJECT
  public:
    DraggableListBox(QWidget *parent = 0L, const char *name = 0L);
    virtual ~DraggableListBox();

    virtual Q3DragObject *dragObject();

    bool dragEnabled() const;
    virtual void setDragEnabled(bool enabled);

  public slots:
    virtual void up();
    virtual void down();
    
  protected:
    virtual void startDrag();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);

  private:
    QPoint _pressPos;
    bool _dragEnabled;
} KST_EXPORT;

#endif
// vim: ts=2 sw=2 et