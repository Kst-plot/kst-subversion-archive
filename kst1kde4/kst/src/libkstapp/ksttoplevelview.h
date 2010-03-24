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

#include <QCursor>
#include <QPointer>

#include "kstviewobject.h"

class KstGfxMouseHandler;
class KstViewWidget;
class Kst2DPlot;
typedef QExplicitlySharedDataPointer<Kst2DPlot> Kst2DPlotPtr;

class KstTopLevelView : public KstViewObject {
  friend class KstViewWidget;
  Q_OBJECT
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
  Q_PROPERTY(double marginLeft READ marginLeft WRITE setMarginLeft)
  Q_PROPERTY(double marginRight READ marginRight WRITE setMarginRight)
  Q_PROPERTY(double marginTop READ marginTop WRITE setMarginTop)
  Q_PROPERTY(double marginBottom READ marginBottom WRITE setMarginBottom)

  public:
    KstTopLevelView(QWidget *parent = 0L, const char *name = 0L, Qt::WindowFlags w = 0);
    KstTopLevelView(const QDomElement& e, QWidget *parent = 0L, const char *name = 0L, Qt::WindowFlags w = 0);
    KstTopLevelView(const KstTopLevelView& tlv);
    virtual ~KstTopLevelView();

    virtual KstViewObject* copyObjectQuietly() const;

    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    QMap<QString, QVariant > widgetHints(const QString& propertyName) const;

    void applyDefaults();
    void release(); // Release this from it's window/view.  When you call this,
                    // you'd better be deleting this object in the next line.
    KstViewWidget *widget() const;

    virtual void resize(const QSize& size);
    virtual void paint(KstPainter& p, const QRegion& bounds);
    virtual void setBackgroundColor(const QColor& color);
    virtual QColor backgroundColor() const;

    double marginLeft() const { return _margins.left; }
    double marginRight() const { return _margins.right; }
    double marginTop() const { return _margins.top; }
    double marginBottom() const { return _margins.bottom; }
    void setMarginLeft(double left) { _margins.left = left; }
    void setMarginRight(double right) { _margins.right = right; }
    void setMarginTop(double top) { _margins.top = top; }
    void setMarginBottom(double bottom) { _margins.bottom = bottom; }

    void updateAlignment(KstPainter& p);
    void paint(KstPainter::PaintType type);
    void paint(KstPainter::PaintType type, const QRegion& boundry);
    void paintSelf(KstPainter& p, const QRegion& bounds);

    void clearFocus();
    enum ViewMode { LayoutMode = 0, DisplayMode, CreateMode, LabelMode, Unknown = 15 };
    ViewMode viewMode() const { return _mode; }
    void setViewMode(ViewMode v, const QString& createType = QString::null);

    // Are we tracking (rubber band / move / resize)
    bool tracking() const;
    bool trackingIsMove() const;

    KstViewObjectList& selectionList() { return _selectionList; }
    KstViewObjectPtr pressTarget() const { return _pressTarget; }
    void clearPressTarget() { _pressTarget = 0L; }

    Kst2DPlotPtr createPlotObject(const QString& name, bool doCleanup = true);

    bool mouseGrabbed() const { return _mouseGrabbed; }
    KstViewObjectPtr mouseGrabber() const { return _mouseGrabber; }
    bool grabMouse(KstViewObjectPtr me);
    void releaseMouse(KstViewObjectPtr me);

    // save defaults for mouse mode from a given object
    void saveDefaults(KstViewObjectPtr object);
    void restoreDefaults(KstViewObjectPtr object);

    bool tiedZoomPrev(const QString& plotName);
    bool tiedZoomMode(int zoom, bool flag, double center, int mode, int modeExtra, const QString& plotName);
    bool tiedZoom(bool x, double xmin, double xmax, bool y, double ymin, double ymax, const QString& plotName);

  public slots:
    void cleanupDefault();
    void cleanupCustom();

  private slots:
    void menuClosed();
    void condenseXAxis();
    void condenseYAxis();
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

  protected:
    void resized(const QSize& size);
    void cancelMouseOperations();
    void deleteSelectedObjects();
    void updateFocus(const QPoint& pos);
    bool handlePress(const QPoint& pos, bool shift = false);
    bool handleDoubleClick(const QPoint& pos, bool shift = false);

    // press move handlers
    void pressMove(const QPoint& pos, bool shift = false, bool alt = false);
    void pressMoveLayoutMode(const QPoint& pos, bool shift = false, bool alt = false);
    // helpers for pressMoveLayoutMode
    void pressMoveLayoutModeMove(const QPoint& pos, bool shift = false, bool snapToBorder = true);
    void pressMoveLayoutModeResize(const QPoint& pos, bool maintainAspect = false, bool snapToBorder = true);
    void pressMoveLayoutModeSelect(const QPoint& pos);
    void pressMoveLayoutModeEndPoint(const QPoint& pos, bool maintainAspect = false, bool snapToBorder = true);
    void pressMoveLayoutModeCenteredResize(const QPoint& pos, bool maintainAspect = false, bool snapToBorder = true);

    // release press handlers 
    void releasePress(const QPoint& pos, bool shift = false);
    void releasePressLayoutMode(const QPoint& pos, bool shift = false);
    // helpers for releasePressLayoutMode
    void releasePressLayoutModeMove(const QPoint& pos, bool shift = false);
    void releasePressLayoutModeResize(const QPoint& pos, bool shift = false);
    void releasePressLayoutModeSelect(const QPoint& pos, bool shift = false);
    void releasePressLayoutModeEndPoint(const QPoint& pos, bool shift = false);
    void releasePressLayoutModeCenteredResize(const QPoint& pos, bool shift = false);

    void setCursorFor(const QPoint& pos, KstViewObjectPtr p);
    bool popupMenu(QMenu *menu, const QPoint& pos);
    void correctPosition(KstViewObjectPtr pObject, QPoint point);
    QRect newSize(const QRect& originalSize, const QRect& bounds, int direction, const QPoint& pos, bool maintainAspect = false);
    QRect newSizeCentered(const QRect& oldSize, const QRect& bounds, int direction, const QPoint& pos, bool maintainAspect);
    QRect correctWidthForRatio(const QRect& oldRect, double ratio, int direction);
    QRect correctHeightForRatio(const QRect& oldRect, double ratio, int direction, int origRight, int origLeft);
    void moveSnapToBorders(int *xMin, int *yMin, const KstViewObjectPtr &obj, const QRect &r) const;
    void resizeSnapToBorders(int *xMin, int *yMin, const KstViewObjectPtr& obj, const QRect &r, int direction) const;
    QRect resizeCenteredSnapToObjects(const QRect& r, const QRect& bounds, int direction);
    QRect resizeSnapToObjects(const QRect& r, int direction);
    void pointSnapToBorders(int *xMin, int *yMin, const KstViewObjectPtr &obj, const QPoint &p) const;
    QPoint pointSnapToObjects(const QPoint& p);
    // Called as a response to drag re-entering widget()
    void restartMove();

    KstGfxMouseHandler *handlerForObject(const QString& objType);

  private:
    void commonConstructor();
    // If no children, returns (0,0)
    QSize averageChildSize() const;

    QPointer<KstViewWidget> _w;
    bool _focusOn : 1;
    bool _mouseGrabbed : 1;
    bool _mouseMoved : 1; // true even if mouse moved back to original position
    ViewMode _mode : 9;
    signed int _pressDirection : 7;
    QCursor _cursor;
    QCursor _endpointCursor; // for storage of the custom hotpoint cursor
    QPoint _moveOffset;
    QPoint _moveOffsetSticky;
    KstViewObjectPtr _pressTarget;
    KstViewObjectPtr _hoverFocus;
    KstViewObjectPtr _prevContainer;
    QRect _prevBand;
    KstViewObjectList _selectionList;
    KstViewObjectPtr _mouseGrabber;
    KstGfxMouseHandler *_activeHandler;
    QMap<QString,KstGfxMouseHandler*> _handlers;
};

typedef QExplicitlySharedDataPointer<KstTopLevelView> KstTopLevelViewPtr;
typedef KstObjectList<KstTopLevelViewPtr> KstTopLevelViewList;

#endif

