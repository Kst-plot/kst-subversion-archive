/***************************************************************************
                      kstcolorsequence.h  -  Part of KST
                             -------------------
    begin                : Mon Jul 07 2003
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

#ifndef _KST_CS_H
#define _KST_CS_H

#include <qcolor.h>
#include <kstaticdeleter.h>
#include "kstbasecurve.h"

class KPalette;

class KstColorSequence {
  friend class KStaticDeleter<KstColorSequence>;
  public:
    enum ColorMode { MonoChrome, GrayScale, Color };
    static QColor next();
    static QColor next(const QColor& badColor);
    static QColor next(const KstBaseCurveList& Curves, const QColor& badColor);
    static bool colorsTooClose(const QColor& color, const QColor& badColor);
    static ColorMode colorMode();
    static void setColorMode(ColorMode mode);

  private:
    KstColorSequence();
    ~KstColorSequence();
    static KstColorSequence* _self;
    KPalette* _pal;
    int _count;
    int _ptr;  // pointer to the next color
    ColorMode _mode;
};

#endif

// vim: ts=2 sw=2 et
