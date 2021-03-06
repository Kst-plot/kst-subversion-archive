/***************************************************************************
                   kstviewwidget.cpp: widget for the toplevel view
                             -------------------
    begin                : Mar 27, 2004
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

#include <assert.h>

#include "kst.h"
#include "kstviewobjectimagedrag.h"
#include "ksttimers.h"
#include "kstviewwidget.h"
#include "kstviewwindow.h"
#include "plotmimesource.h"

KstViewWidget::KstViewWidget(KstTopLevelViewPtr view, QWidget *parent, const char *name, Qt::WindowFlags w)
: QWidget(parent, w), _view(view) {
  setDragEnabled(true);
  setDropEnabled(true);
  setMouseTracking(true);
// xxx  setFocusPolicy(QWidget::StrongFocus);
// xxx  setBackgroundMode(Qt::NoBackground);
  setMinimumSize(40, 25);
  _vo_datamode = 0L;
  _lastViewMode = KstTopLevelView::LayoutMode;
}


KstViewWidget::~KstViewWidget() {
  if (_menu) {
    delete _menu;
  }
}

/* xxx
QDragObject *KstViewWidget::dragObject() {
  KMultipleDrag *drag = new KMultipleDrag(this);
  QStringList plots;
  KstViewObjectList vol;
  QString window = static_cast<KstViewWindow*>(parent())->caption();

  if (_view->selectionList().isEmpty()) {
    if (_view->pressTarget()) {
      plots.append(_view->pressTarget()->tagName());
      vol.append(_view->pressTarget());
    } else {
      for (size_t i=0; i<_view->children().size(); i++) {
        plots.append(_view->children()[i]->tagName());
        vol.append(_view->children()[i]);
      }
    }
  } else {
    for (size_t i=0; i<_view->selectionList().size(); i++) {
      plots.append(_view->selectionList()[i]->tagName());
      vol.append(_view->selectionList()[i]);
    }
  }

  drag->addDragObject(new PlotMimeSource(window, plots, 0L));
  KstViewObjectImageDrag *imd = new KstViewObjectImageDrag(this);
  imd->setObjects(vol);
  drag->addDragObject(imd);

  return drag;
}
*/

void KstViewWidget::enterEvent(QEvent *e) {
  if (_view->viewMode() != KstTopLevelView::DisplayMode) {
    if (!_menu && !_view->tracking()) {
      _view->clearFocus();
    }
  }
  QWidget::enterEvent(e);
}


void KstViewWidget::focusOutEvent(QFocusEvent *e) {
  if (_view->viewMode() == KstTopLevelView::LayoutMode && _lastViewMode != KstTopLevelView::LayoutMode) {
    _view->setViewMode(_lastViewMode);
    _lastViewMode = KstTopLevelView::LayoutMode;
  }
  QWidget::focusOutEvent(e);
}


void KstViewWidget::leaveEvent(QEvent *e) {
  QWidget::leaveEvent(e);
  if (_view->viewMode() != KstTopLevelView::DisplayMode) {
    if (_menu) {
      return;
    }
    if (!_view->tracking()) {
      _view->clearFocus();
    } else {
      if (_view->trackingIsMove() && _dragEnabled) {
/* xxx
        QDragObject *d = dragObject();
        if (d) {
          // First cancel any operations
          _view->cancelMouseOperations();
          _view->paint(KstPainter::P_PAINT);
          d->drag();
        }
*/
      }
    }
  }
}


KstViewObjectPtr KstViewWidget::findChildFor(const QPoint& pos) {
  KstViewObjectPtr rc;
  KstViewObjectPtr search;

  search = _view.data();
  while (search) {
    search = search->findChild(pos);
    if (search && search->mouseHandler()) {
      rc = search;
    }
  }

  return rc;
}


void KstViewWidget::dropped(QDropEvent *e) {
  dropEvent(e);
}


// Optimize: cache previous child here to make it faster? How to invalidate?
void KstViewWidget::mouseMoveEvent(QMouseEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(e->pos());
    }

    if (KstApp::inst()->dataMode()) {
      if (vo.data() != _vo_datamode) {
        _vo_datamode = vo.data();
        paint();
      }
    } else {
      _vo_datamode = 0L;
    }

    if (vo) {
      vo->mouseMoveEvent(this, e);
    } else {
      setCursor(QCursor(Qt::ArrowCursor));
    }
  } else {
    if ((e->modifiers() & Qt::MouseButtonMask) == 0) {
      switch (_view->viewMode()) {
        case KstTopLevelView::LabelMode:
          setCursor(QCursor(Qt::IBeamCursor));
          break;
        case KstTopLevelView::CreateMode:
          setCursor(QCursor(Qt::ArrowCursor));
          break;
        default:
          break;
      }
      _view->updateFocus(e->pos());
      e->accept();
    } else if (e->modifiers() & Qt::LeftButton) {
      _view->pressMove(e->pos(), e->modifiers() & Qt::ShiftModifier, e->modifiers() & Qt::AltModifier);
      e->accept();
    }
  }
}


void KstViewWidget::mousePressEvent(QMouseEvent *e) {
  if (_menu) {
    QWidget::mousePressEvent(e);

    return;
  }

  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;
    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(e->pos());
    }
    if (vo) {
      vo->mousePressEvent(this, e);
    }

    return;
  }

  //
  // layout/graphics mode...
  //

  if (e->button() & Qt::LeftButton) {
    setCursor(QCursor(Qt::ArrowCursor));
    if (_view->handlePress(e->pos(), e->modifiers() & Qt::ShiftModifier)) {
      e->accept();

      return;
    } else if (_view->tracking()) {
      e->accept();

      return;
    }
  }

  QWidget::mousePressEvent(e);
}


void KstViewWidget::mouseDoubleClickEvent(QMouseEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(e->pos());
    }
    if (vo) {
      vo->mouseDoubleClickEvent(this, e);
    }
    return;
  }

  //
  // layout/graphics mode...
  //

  if (e->button() & Qt::LeftButton) {
    //
    // let the toplevel view handle it...
    //

    if (_view->handleDoubleClick(e->pos(), e->modifiers() & Qt::ShiftModifier)) {
      e->accept();

      return;
    }
  }

  QWidget::mouseDoubleClickEvent(e);
}


void KstViewWidget::wheelEvent(QWheelEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(e->pos());
    }
    if (vo) {
      vo->wheelEvent(this, e);
    }

    return;
  }
}


void KstViewWidget::mouseReleaseEvent(QMouseEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(e->pos());
    }
    if (vo) {
      vo->mouseReleaseEvent(this, e);
    }

    return;
  }

  if ((e->modifiers() & Qt::ShiftModifier) && (e->button() & Qt::LeftButton) && !_view->tracking()) {
    _view->releasePress(e->pos(), true);
  } else if (e->button() & Qt::LeftButton) {
    _view->releasePress(e->pos(), e->modifiers() & Qt::ShiftModifier);
    e->accept();
  } else {
    if (e->modifiers() & Qt::LeftButton && _view->tracking()) {
      e->accept();

      return;
    }
    _view->updateFocus(e->pos());
    QWidget::mouseReleaseEvent(e);
  }
}


void KstViewWidget::contextMenuEvent(QContextMenuEvent *e) {
  if (e->modifiers() & Qt::LeftButton || _view->tracking()) {
    e->ignore();

    return;
  }

  if (_view->mouseGrabber()) {
    _view->releaseMouse(_view->mouseGrabber());
  }

  //
  // do not allow recursive menus
  //
  if (_menu) {
    e->ignore();
    return;
  }

  _menu = new QMenu(this);
//
  bool rc = _view->popupMenu(_menu, e->pos());
  if (rc && _menu->actions().count() > 0) {
    _menu->popup(mapToGlobal(e->pos()));
    connect(_menu, SIGNAL(aboutToHide()), this, SLOT(menuHiding()));
  } else {
    delete _menu;
  }

  e->accept();
}


void KstViewWidget::paint(const QRegion& region) {
  QPaintEvent e(region);

  paintEvent(&e);
}


void KstViewWidget::paint() {
  paintEvent(0L);
}


// Note: e can be null
void KstViewWidget::paintEvent(QPaintEvent *e) {
  if (e) {  // Resize/expose/etc triggered by X11
    QRegion r = e->region();
    if (r.isEmpty()) {
      _view->paint(KstPainter::P_PAINT);
    } else {
      _view->paint(KstPainter::P_PAINT, e->region());
    }
  } else { // explicitly forced paint in the code
    _view->paint(KstPainter::P_ZOOM);
  }
}


void KstViewWidget::dragEnterEvent(QDragEnterEvent *e) {
  if (e->provides(PlotMimeSource::mimeType())) {
    if (e->source() == this) {
      QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, 0, 0);

// xxx      QApplication::sendEvent(this, &keyEvent);
      _view->restartMove();
    } else {
// xxx      e->acceptAction(true);
    }
  } else if (_view->viewMode() != KstTopLevelView::LayoutMode) {
    KstViewObjectPtr vo;

    vo = findChildFor(e->pos());
    if (vo) {
      vo->dragEnterEvent(this, e);
    } else {
      e->setAccepted(false);
    }
  }

  QWidget::dragEnterEvent(e);
}


void KstViewWidget::dragMoveEvent(QDragMoveEvent *e) {
  if (_dragEnabled && e->provides(PlotMimeSource::mimeType())) {
    e->setAccepted(true);
  } else if (_view->viewMode() != KstTopLevelView::LayoutMode) {
    KstViewObjectPtr vo = findChildFor(e->pos());
    if (vo) {
      vo->dragMoveEvent(this, e);
    } else {
      e->setAccepted(false);
    }
  }
  QWidget::dragMoveEvent(e);
}


void KstViewWidget::dragLeaveEvent(QDragLeaveEvent *e) {
  // How to deal with drags to other windows/views/apps
  QWidget::dragLeaveEvent(e);
}


void KstViewWidget::dropEvent(QDropEvent *e) {
  if (e->source() != this && e->provides(PlotMimeSource::mimeType())) {
    // FIXME: support both copy and move
    KstViewObjectPtr vo;

    vo = findChildFor(e->pos());
    if (vo) {
/* xxx
      if (vo->paste(e)) {
        e->acceptAction(true);
        KstApp::inst()->updateViewManager(true);
        _view->paint(KstPainter::P_PAINT);
      }
*/
    } else {
/* xxx
      if (viewObject()->paste(e)) {
        e->acceptAction(true);
        KstApp::inst()->updateViewManager(true);
        _view->paint(KstPainter::P_PAINT);
      }
*/
    }
  } else if (_view->viewMode() != KstTopLevelView::LayoutMode) {
    KstViewObjectPtr vo;

    vo = findChildFor(e->pos());
    if (vo) {
      vo->dropEvent(this, e);
      return;
    }
    QWidget::dropEvent(e);
  }
}


void KstViewWidget::resizeEvent(QResizeEvent *e) {
  _view->resized(e->size());
  QWidget::resizeEvent(e);
}


void KstViewWidget::keyPressEvent(QKeyEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(mapFromGlobal(QCursor::pos()));
    }
    if (vo) {
      vo->keyPressEvent(this, e);
    }
    return;
  } else if (_view->viewMode() == KstTopLevelView::LayoutMode) {
    Qt::KeyboardModifiers s = e->modifiers();

    if (e->key() == Qt::Key_Escape) {
      _view->cancelMouseOperations();
      return;
    } else if (e->key() == Qt::Key_A && (s & Qt::ControlModifier) && _view->viewMode() == KstTopLevelView::LayoutMode) {
      if (s & Qt::ShiftModifier) {
        _view->unselectAll();
      } else {
        _view->selectAll();
      }
      paint();
      return;
    } else if (e->key() == Qt::Key_Delete) {
      _view->deleteSelectedObjects();
      return;
    } else if (e->key() == Qt::Key_Control && (_view->viewMode() != KstTopLevelView::LayoutMode)) {
      _lastViewMode = _view->viewMode();
      _view->setViewMode(KstTopLevelView::LayoutMode);
      return;
    }
  }

  QWidget::keyPressEvent(e);
}


void KstViewWidget::keyReleaseEvent(QKeyEvent *e) {
  if (_view->viewMode() == KstTopLevelView::DisplayMode) {
    KstViewObjectPtr vo;

    // Note: should mouse grabbers get keyboard input?
    if (_view->mouseGrabbed()) {
      vo = _view->mouseGrabber();
    } else {
      vo = findChildFor(mapFromGlobal(QCursor::pos()));
    }
    if (vo) {
      vo->keyReleaseEvent(this, e);
    }
    return;
  } else if (e->key() == Qt::Key_Control && _view->viewMode() == KstTopLevelView::LayoutMode) {
    _view->setViewMode(_lastViewMode);
    _lastViewMode = KstTopLevelView::LayoutMode;
  }

  QWidget::keyReleaseEvent(e);
}


void KstViewWidget::setDropEnabled(bool en) {
  _dropEnabled = en;
  setAcceptDrops(en);
}


void KstViewWidget::setDragEnabled(bool en) {
  _dragEnabled = en;
}


KstTopLevelViewPtr KstViewWidget::viewObject() const {
  return _view;
}


void KstViewWidget::menuHiding() {
  QTimer::singleShot(0, this, SLOT(menuHidden()));

  if (_view->viewMode() != KstTopLevelView::DisplayMode) {
    _view->updateFocus(mapFromGlobal(QCursor::pos()));
  }

  //
  // for convenience, let's update the dialogs...
  //

  QTimer::singleShot(0, KstApp::inst(), SLOT(updateVisibleDialogs()));
}


void KstViewWidget::menuHidden() {
  delete _menu;
}


#include "kstviewwidget.moc"
