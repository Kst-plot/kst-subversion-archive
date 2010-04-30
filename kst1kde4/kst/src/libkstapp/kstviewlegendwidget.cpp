/***************************************************************************
                      kstviewlegendwidget.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
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

#include <QCheckBox>

#include "kst_export.h"
#include "kstviewlegendwidget.h"

KstViewLegendWidget::KstViewLegendWidget(QWidget* parent, const char* name, Qt::WindowFlags fl) : QDialog(parent, fl) {
  setupUi(this);

  connect(DisplayedCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(DisplayedCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeDisplayedCurve()));
  connect(_add, SIGNAL(clicked()), this, SLOT(addDisplayedCurve()));
  connect(_up, SIGNAL(clicked()), DisplayedCurveList, SLOT(up()));
  connect(_down, SIGNAL(clicked()), DisplayedCurveList, SLOT(down()));
  connect(_up, SIGNAL(clicked()), DisplayedCurveList, SIGNAL(changed()));
  connect(_down, SIGNAL(clicked()), DisplayedCurveList, SIGNAL(changed()));

  _up->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
  _up->setEnabled(false);
  _down->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
  _down->setEnabled(false);
  _add->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
  _add->setEnabled(false);
  _remove->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
  _remove->setEnabled(false);

  _up->setToolTip(QObject::tr("Shortcut: Alt+Up"));
  _down->setToolTip(QObject::tr("Shortcut: Alt+Down"));
  _add->setToolTip(QObject::tr("Shortcut: Alt+s"));
  _remove->setToolTip(QObject::tr("Shortcut: Alt+r"));

  _changedFgColor = false;
  _changedBgColor = false;
}


KstViewLegendWidget::~KstViewLegendWidget() {
}


void KstViewLegendWidget::updateButtons() {
  bool selected = false;
  uint count = AvailableCurveList->count();
  uint i;

  for (i = 0; i < count; i++) {
    if (AvailableCurveList->item(i)->isSelected()) {
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

  for (i = 0; i < count; i++) {
    if (DisplayedCurveList->item(i)->isSelected()) {
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


void KstViewLegendWidget::removeDisplayedCurve() {
  uint count = DisplayedCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (DisplayedCurveList->item(i)->isSelected()) {
        AvailableCurveList->addItem(DisplayedCurveList->item(i)->text());
        DisplayedCurveList->removeItemWidget(DisplayedCurveList->item(i));
      }
    }
    updateButtons();

    emit changed();
  }

  TrackContents->setChecked(false);
}


void KstViewLegendWidget::addDisplayedCurve() {
  uint count = AvailableCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (AvailableCurveList->item(i)->isSelected()) {
        DisplayedCurveList->addItem(AvailableCurveList->item(i)->text());
        AvailableCurveList->removeItemWidget(AvailableCurveList->item(i));
      }
    }
    updateButtons();

    emit changed();
  }

  TrackContents->setChecked(false);
}


void KstViewLegendWidget::changedFgColor( ) {
  _changedFgColor = true;
}


void KstViewLegendWidget::changedBgColor( ) {
  _changedBgColor = true;
}

#include "kstviewlegendwidget.moc"
