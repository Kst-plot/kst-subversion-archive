/***************************************************************************
                   ksttoplevelview.h: class for the toplevel view
                             -------------------
    begin                : Mar 11, 2004
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

#ifndef KSTTOPLEVELVIEW_H
#define KSTTOPLEVELVIEW_H

#include <qcursor.h>
#include <qguardedptr.h>

class KPopupMenu;
class KstViewWidget;

class KstTopLevelView : public KstViewObject {
  friend class KstViewWidget;
  Q_OBJECT
  public:
    KstTopLevelView(QWidget *parent = 0L, const char *name = 0L, WFlags w = 0);
    KstTopLevelView(const QDomElement& e, QWidget *parent = 0L, const char *name = 0L, WFlags w = 0);
    virtual ~KstTopLevelView();

    virtual void save(QTextStream& ts, const QString& indent = QString::null);

    void release(); // Release this from it's window/view.  When you call this,
                    // you'd better be deleting this object in the next line.
    KstViewWidget *widget() const;

    virtual void resize(const QSize& size);
    virtual void paint(KstPaintType type, QPainter& p);

    void updateAlignment(KstPaintType type, QPainter& p);
    void paint(KstPaintType type);

    void clearFocus();
    enum ViewMode { LayoutMode = 0, DisplayMode, Unknown = 15 };
    ViewMode viewMode() const { return _mode; }
    void setViewMode(ViewMode v);

    // Are we tracking (rubber band / move / resize)
    bool tracking() const;
    bool trackingIsMove() const;

    KstViewObjectList& selectionList() { return _selectionList; }
    KstViewObjectPtr pressTarget() const { return _pressTarget; }

    template<class T> KstSharedPtr<T> createPlot(const QString& name, bool doCleanup = true);

    virtual bool removeChild(KstViewObjectPtr obj, bool recursive = false);
    virtual void cleanup(int cols = -1);

    bool mouseGrabbed() const { return _mouseGrabbed; }
    KstViewObjectPtr mouseGrabber() const { return _mouseGrabber; }
    bool grabMouse(KstViewObjectPtr me);
    void releaseMouse(KstViewObjectPtr me);

  private slots:
    void menuClosed();
    void makeSameWidth();
    void makeSameHeight();
    void makeSameSize();
    void alignLeft();
    void alignRight();
    void alignTop();
    void alignBottom();
    void packVertically();
    void packHorizontally();
    void groupSelection();
    void cleanupAction();

  protected:
    void resized(const QSize& size);
    void cancelMouseOperations();
    void updateFocus(const QPoint& pos);
    bool handlePress(const QPoint& pos, bool shift = false);
    void pressMove(const QPoint& pos, bool shift = false);
    void releasePress(const QPoint& pos, bool shift = false);
    void setCursorFor(const QPoint& pos, const QRect& objGeom);
    bool popupMenu(KPopupMenu *menu, const QPoint& pos);
    void checkPosition( KstViewObjectPtr pObject, QPoint point );
    QRect newSize(const QRect& oldSize, int direction, const QPoint& pos);

    // Called as a response to drag re-entering widget()
    void restartMove();

  private:
    void commonConstructor();
    // If no children, returns (0,0)
    QSize averageChildSize();

    QGuardedPtr<KstViewWidget> _w;
    bool _focusOn : 1;
    bool _mouseGrabbed : 1;
    ViewMode _mode : 4;
    signed int _pressDirection : 5;
    QCursor _cursor;
    QPoint _moveOffset;
    QPoint _moveOffsetSticky;
    KstViewObjectPtr _pressTarget, _hoverFocus;
    QRect _prevBand;
    KstViewObjectList _selectionList;
    KstViewObjectPtr _mouseGrabber;
};

typedef KstSharedPtr<KstTopLevelView> KstTopLevelViewPtr;
typedef KstObjectList<KstTopLevelViewPtr> KstTopLevelViewList;

template<class T>
KstSharedPtr<T> KstTopLevelView::createPlot(const QString& name, bool doCleanup) {
  T *plot = new T(name);
  if (_onGrid) {
    // FIXME: make this more powerful, preserve columns
    appendChild(plot);
    if (doCleanup) {
      this->cleanup(-1); // GCC 2.95/ppc bug.  Don't touch!!!
    }
  } else {
    QSize sz = averageChildSize();
    if (sz != QSize(0, 0)) {
      plot->resize(sz);
    } else {
      plot->resize(size());
    }
    // First look at the overall clip mask.  If there are gaps, take the
    // biggest one and use that location.
    QRegion r = _lastClipRegion;
    QMemArray<QRect> rects = r.rects();
    if (!rects.isEmpty()) {
      QRect maxRect(0, 0, 0, 0);
      for (QMemArray<QRect>::ConstIterator i = rects.begin(); i != rects.end(); ++i) {
        if ((*i).width() * (*i).height() > maxRect.width() * maxRect.height()) {
          maxRect = *i;
        }
      }
      if (maxRect.left() + plot->geometry().width() > geometry().width()) {
        maxRect.moveLeft(geometry().width() - plot->geometry().width());
      }
      if (maxRect.top() + plot->geometry().height() > geometry().height()) {
        maxRect.moveTop(geometry().height() - plot->geometry().height());
      }
      plot->move(maxRect.topLeft());
    } else {
      // If no gaps, then look at the top object and place relative to it.  It
      // would probably be better to iterate back->front with complete masks
      // but that's more complicated and not worth the effort at this time.
      r = QRegion(geometry());
      r -= QRegion(_children.last()->geometry());
      rects = r.rects();
      if (rects.isEmpty()) {
        plot->move(QPoint(0, 0));
      } else {
        QRect maxRect(0, 0, 0, 0);
        for (QMemArray<QRect>::ConstIterator i = rects.begin(); i != rects.end(); ++i) {
          if ((*i).width() * (*i).height() > maxRect.width() * maxRect.height()) {
            maxRect = *i;
          }
        }
        if (maxRect.left() + plot->geometry().width() > geometry().width()) {
          maxRect.moveLeft(geometry().width() - plot->geometry().width());
        }
        if (maxRect.top() + plot->geometry().height() > geometry().height()) {
          maxRect.moveTop(geometry().height() - plot->geometry().height());
        }
        plot->move(maxRect.topLeft());
      }
    }
    appendChild(plot);
  }
  return plot;
}


#endif
// vim: ts=2 sw=2 et
