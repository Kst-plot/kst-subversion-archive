/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2012  Barth Netterfield                               *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "circledimensionstab.h"
#include "plotitem.h"
#include <QDebug>

namespace Kst {
CircleDimensionsTab::CircleDimensionsTab(ViewItem* viewItem, QWidget *parent)
  : DialogTab(parent), _viewItem(viewItem) {
  setupUi(this);

  connect(_lockPosToData, SIGNAL(clicked(bool)), this, SLOT(fillDimensions(bool)));
}

void CircleDimensionsTab::fillDimensions(bool lock_pos_to_data) {
  if (lock_pos_to_data) {
    _x->setRange(-1E308, 1E308);
    _y->setRange(-1E308, 1E308);
    _radius->setRange(0,1E308);
    _x->setValue(_viewItem->dataRelativeRect().center().x());
    _y->setValue(_viewItem->dataRelativeRect().center().y());
    _radius->setValue(0.5*_viewItem->dataRelativeRect().width());
  } else {
    _x->setRange(0, 1);
    _y->setRange(0, 1);
    _radius->setRange(0,1);
    _x->setValue(_viewItem->relativeCenter().x());
    _y->setValue(_viewItem->relativeCenter().y());
    _radius->setValue(0.5*_viewItem->relativeWidth());
  }
}

void CircleDimensionsTab::setupDimensions() {

  fillDimensions(_viewItem->dataPosLockable() && _viewItem->lockPosToData());

  _lockPosToData->setChecked(_viewItem->lockPosToData());
  if (_viewItem->dataPosLockable()) {
      _lockPosToData->show();
  } else {
      _lockPosToData->hide();
  }
}


void CircleDimensionsTab::modified() {
  emit tabModified();
}


void CircleDimensionsTab::clearTabValues() {
  _radius->clear();
  _lockPosToData->setCheckState(Qt::PartiallyChecked);
}


void CircleDimensionsTab::enableSingleEditOptions(bool enabled) {
  _x->setEnabled(enabled);
  _y->setEnabled(enabled);
}


bool CircleDimensionsTab::radiusDirty() const {
  return (!_radius->text().isEmpty());
}


bool CircleDimensionsTab::lockPosToDataDirty() const {
  return _lockPosToData->checkState() != Qt::PartiallyChecked;
}

}
