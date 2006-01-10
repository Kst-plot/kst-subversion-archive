/***************************************************************************
                                kstpainter.h
                             -------------------
    begin                : Nov 25, 2005
    copyright            : (C) 2005 The University of Toronto
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

#ifndef KSTPAINTER_H
#define KSTPAINTER_H

#include <qpainter.h>


class KstPainter : public QPainter {
  public:
    // KstPaintType - hints to paint to allow optimizations
    // P_PAINT: Neither 'data' nor 'plot' needs to change
    // P_PLOT: data didn't change
    // P_UPDATE: at least one curve was updated
    /* FIXME: Define P_PLOT better, and do we even need P_ZOOM, P_PLOT and
       P_PAINT?  They should all do the same thing.
     */
    enum PaintType { P_PAINT = 0, P_PLOT, P_ZOOM, P_UPDATE, P_PRINT, P_EXPORT };

    KstPainter(PaintType t = P_PAINT);
    virtual ~KstPainter();

    // Defalut: P_PAINT
    void setType(PaintType t);
    PaintType type() const;

    // True if UI elements such as focus rect should be drawn.  default: false
    bool drawInlineUI() const;
    void setDrawInlineUI(bool draw);

    inline QRegion& uiMask() { return _uiMask; }

    bool makingMask() const;
    void setMakingMask(bool making);

  private:
    PaintType _type;
    bool _drawInlineUI;
    bool _makingMask;
    QRegion _uiMask;
};

#endif
// vim: ts=2 sw=2 et
