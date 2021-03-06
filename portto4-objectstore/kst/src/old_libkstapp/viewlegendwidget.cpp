/***************************************************************************
                   viewlegendwidget.cpp
                             -------------------
    begin                : 02/28/07
    copyright            : (C) 2007 The University of Toronto
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

#include "viewlegendwidget.h"

#include <kiconloader.h>

#include <kst_export.h>

ViewLegendWidget::ViewLegendWidget(QWidget *parent)
    : QWidget(parent) {
  setupUi(this);

//  connect(, SIGNAL(), this, SLOT());
}


ViewLegendWidget::~ViewLegendWidget() {}


void ViewLegendWidget::init() {
  connect(DisplayedCurveList, SIGNAL(clicked(Q3ListBoxItem*)),
          this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(clicked(Q3ListBoxItem*)),
          this, SLOT(updateButtons()));
  connect(DisplayedCurveList, SIGNAL(doubleClicked(Q3ListBoxItem*)),
          this, SLOT(removeDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(doubleClicked(Q3ListBoxItem*)),
          this, SLOT(addDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(selectionChanged()),
          this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged()),
          this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()),
          this, SLOT(removeDisplayedCurve()));
  connect(_add, SIGNAL(clicked()),
          this, SLOT(addDisplayedCurve()));
  connect(_up, SIGNAL(clicked()),
          DisplayedCurveList, SLOT(up()));
  connect(_down, SIGNAL(clicked()),
          DisplayedCurveList, SLOT(down()));

  _up->setIcon(BarIcon("up"));
  _up->setEnabled(false);
  _down->setIcon(BarIcon("down"));
  _down->setEnabled(false);
  _add->setIcon(BarIcon("forward"));
  _add->setEnabled(false);
  _remove->setIcon(BarIcon("back"));
  _remove->setEnabled(false);
  _up->setToolTip(i18n("Shortcut: Alt+Up"));
  _down->setToolTip(i18n("Shortcut: Alt+Down"));
  _add->setToolTip(i18n("Shortcut: Alt+s"));
  _remove->setToolTip(i18n("Shortcut: Alt+r"));

  _thisLegend->setChecked(true);

}


void ViewLegendWidget::updateButtons() {
  bool selected = false;
  uint count = AvailableCurveList->count();

  for (uint i = 0; i < count; i++) {
    if (AvailableCurveList->isSelected(i)) {
      selected = true;
    }
  }
  if (selected && !_add->isEnabled()) {
    _add->setEnabled(true);
  } else if (!selected && _add->isEnabled()) {
    _add->setEnabled(false);
  }

  selected = false;
  count = DisplayedCurveList->count();
  for (uint i = 0; i < count; i++) {
    if (DisplayedCurveList->isSelected(i)) {
      selected = true;
    }
  }
  if (selected && !_remove->isEnabled()) {
    _remove->setEnabled(true);
  } else if (!selected && _remove->isEnabled()) {
    _remove->setEnabled(false);
  }
  if (selected && !_up->isEnabled()) {
    _up->setEnabled(true);
    _down->setEnabled(true);
  } else if (!selected && _up->isEnabled()) {
    _up->setEnabled(false);
    _down->setEnabled(false);
  }

}


void ViewLegendWidget::removeDisplayedCurve() {
  uint count = DisplayedCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (DisplayedCurveList->isSelected(i)) {
        AvailableCurveList->insertItem(DisplayedCurveList->text(i));
        DisplayedCurveList->removeItem(i);
      }
    }
    updateButtons();
    emit changed();
  }
  TrackContents->setChecked(false);
}


void ViewLegendWidget::addDisplayedCurve() {
  uint count = AvailableCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (AvailableCurveList->isSelected(i)) {
        DisplayedCurveList->insertItem(AvailableCurveList->text(i));
        AvailableCurveList->removeItem(i);
      }
    }
    updateButtons();
    emit changed();
  }
  TrackContents->setChecked(false);
}

#include "viewlegendwidget.moc"

// vim: ts=2 sw=2 et
