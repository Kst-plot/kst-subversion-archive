/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "kcolorcollection.h"

#include "colorpalettewidget.h"

ColorPaletteWidget::ColorPaletteWidget(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  refresh();

  connect(_palette, SIGNAL(highlighted(QString)), this, SLOT(updatePalette(QString)));
  connect(_palette, SIGNAL(activated(QString)), this, SLOT(updatePalette(QString)));
}


ColorPaletteWidget::~ColorPaletteWidget() {
}


void ColorPaletteWidget::updatePalette( const QString &palette ) {
  int height = _palette->height();
  int width = 7 * height;
  QPixmap pix(width, height);
  QPainter p(&pix);

  p.fillRect(p.window(), QColor("white"));

  if (!palette.isEmpty()) {
    KColorCollection* colors = 0L;
    QColor color;
    int numColors = 0;
    int size = 1;
    int step = 1;
    int pos = 0;
    int i;
  
    colors = new KColorCollection(palette);
    if (colors) {
      numColors = colors->count();
  
      if (numColors > 0) {
        size = width / numColors;
        if (size == 0) {
          size = 1;
          step = numColors / width;
        }
      }

      for (i=0; i<numColors; i+=step) {
        color = colors->color(i);
        p.fillRect(pos*size, 0, size, height, QBrush(color));
        ++pos;
      }  
      _paletteDisplay->setPixmap(pix);
  
      delete colors;
    }
  }
}


QString ColorPaletteWidget::selectedPalette() {
  return _palette->currentText();
}


void ColorPaletteWidget::refresh() {
  QStringList palList = KColorCollection::installedCollections();
  int index;

  _palette->clear();
  palList.sort();
  _palette->addItems(palList);
  
  index = _palette->findText("Kst Spectrum 1021");
  if (index == -1) {
    index = _palette->findText("Kst Grayscale 256");
  }

  if (index != -1) {
    _palette->setCurrentIndex(index);
  }
}


void ColorPaletteWidget::refresh( const QString & palette ) {
  QStringList palList = KColorCollection::installedCollections();
  int i;
  
  _palette->clear();
  palList.sort();
  _palette->addItems(palList);

  for (i = 0; i < _palette->count(); ++i) {
    if (_palette->itemText(i) == palette) {
      break;
    }
  }
  if (i == _palette->count()) {
    i = 0;
  }

  _palette->setCurrentIndex(i);
}


int ColorPaletteWidget::currentPaletteIndex() {
  return _palette->currentIndex();
}
