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

// application specific includes
#include "palette.h"
#include "settings.h"
#include <QVector>
#include <qapplication.h>
#include <math_kst.h>

namespace Kst {


// Default palette.
static const char *const KstColors[] = { "red",
                                      "blue",
                                      "green",
                                      "black",
                                      "magenta",
                                      "steelblue",
                                      "#501010",
                                      "#105010"
                                      };
static const int KstColorsCount = sizeof(KstColors) / sizeof(char*);

static const QString KstColorsName = "Kst Colors";
static const QString KstGrayscaleName = "Kst Grayscale";


QStringList Palette::getPaletteList() { 
  QStringList paletteList;

  //TODO Populate a shared list of colors to return here.
  paletteList.append(KstColorsName);
  paletteList.append(KstGrayscaleName);

  return paletteList;
}

Palette::Palette() {
  createPalette();
}


Palette::Palette(const QString &paletteName) {
  createPalette(paletteName);
}


Palette::~Palette() {
}


QString Palette::paletteName() const {
  return _paletteName;
}

  
int Palette::colorCount() const {
  return _count;
}

PaletteData Palette::paletteData() const {
  return _palette;
}

QColor Palette::color(const int colorId) const {
  return _palette[colorId];
}


void Palette::createPalette(const QString &paletteName) {
  //TODO Get Palette details from a palette name parameter when a shared list exists.
  _palette.clear();
  if (paletteName.isEmpty()) {
    _paletteName = KstColorsName;
  } else {
    //TODO when a proper shared list exists, validate that the palette exists.
    _paletteName = paletteName;
  }

  if (_paletteName == KstColorsName) {
    for (int i = 0; i < KstColorsCount; i++) {
      _palette.insert(i, QColor(KstColors[i]));
    }
  } else {
    for (int i = 0; i < 255; i++) {
      _palette.insert(i, QColor(i, i, i));
    }
  }
  _count = _palette.count();
}


void Palette::addColor(const QColor& color) {
  _palette.insert(_count, QColor(color));
  ++_count;
}

}
// vim: ts=2 sw=2 et
