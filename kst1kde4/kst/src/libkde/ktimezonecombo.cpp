/*
    Copyright (c) 2005 The University of Toronto
    Copyright (c) 2010 The University of British Columbia

    Based on code Copyright (C) 2005, S.R.Haque <srhaque@iee.org>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QComboBox>
#include <QListWidget>
#include <QVector>

#include "kst_export.h"
#include "ktimezonecombo.h"
#include "ksttimezones.h"

class KTimezoneCombo::Private {
  public:
    QVector<int> _offsets;
    QStringList _names;
};


KTimezoneCombo::KTimezoneCombo(QWidget *parent, const char *name, KstTimezones *db)
: QComboBox(parent), d(new Private) {
  bool userDb = db != 0L;
  if (!userDb) {
    db = new KstTimezones;
  }
/* xxx
  if (listBox()) {
    listBox()->setVScrollBarMode(QScrollView::AlwaysOn);
    listBox()->setColumnMode(QListBox::FixedNumber);
    listBox()->setRowMode(QListBox::Variable);
  }
*/
  insertItem(0, "UTC");
  const KstTimezones::ZoneMap zones = db->allZones();
  d->_offsets.resize(zones.count()+1);
  d->_offsets[0] = 0;
  d->_names += "UTC";
  int i = 0;

  for (KstTimezones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it) {
    int offset = -(*it)->offset();
    d->_offsets[++i] = offset;
    int hours = offset / 3600;
    int minutes = 100 * offset / 3600 % 100;
    bool negative = false;
    if (hours < 0) {
      hours *= -1;
      negative = true;
    }
    if (minutes < 0) {
      minutes *= -1;
    }
    QString offnum;
    if (hours < 10) {
      offnum += '0';
    }
    offnum += QString::number(hours);
    if (minutes < 10) {
      offnum += '0';
    }
    offnum += QString::number(minutes);
    if ((*it)->name() != "UTC") {
      insertItem(0, tr("%3 (UTC%1%2)").arg(negative ? '-' : '+').arg(offnum).arg((*it)->name()));
      d->_names += (*it)->name();
    }
  }

  if (!userDb) {
    delete db;
  }
}


KTimezoneCombo::~KTimezoneCombo() {
  delete d;
  d = 0;
}


int KTimezoneCombo::offset() const {
  return d->_offsets[currentIndex()];
}


QString KTimezoneCombo::tzName() const {
  if (currentIndex() == 0) {
    return currentText();
  } else {
    return d->_names[currentIndex()];
  }
}


void KTimezoneCombo::setTimezone(const QString& tz) {
  int idx = d->_names.indexOf(tz);

  if (idx != -1) {
    setCurrentIndex(idx);
  } else {
    setCurrentIndex(0);
    setItemText(currentIndex(), tz);
  }
}

#include "ktimezonecombo.moc"
