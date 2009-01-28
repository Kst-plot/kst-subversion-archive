/***************************************************************************
                       kstfieldselect_i.cpp  -  Part of KST
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
#include <assert.h>

// include files for Qt
#include <qpushbutton.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qradiobutton.h>
#include <qregexp.h>

// include files for KDE
#include "ksdebug.h"
#include <klistview.h>
#include <kmessagebox.h>
#include <kmultipledrag.h>
#include <kstandarddirs.h>

// application specific includes
#include "fieldselect.h"
#include "kstfieldselect_i.h"
#include "vectorlistview.h"

KstFieldSelectI::KstFieldSelectI(QWidget* parent, const char* name, bool modal, WFlags fl)
: FieldSelect(parent, name, modal, fl) {
  connect(_OK, SIGNAL(clicked()), this, SLOT(OKFieldSelect()));
  connect(_Cancel, SIGNAL(clicked()), this, SLOT(CancelFieldSelect()));
  connect(_vectorReduction, SIGNAL(textChanged(const QString&)), this, SLOT(vectorSubset(const QString&)));
  connect(_vectorSearch, SIGNAL(clicked()), this, SLOT(search()));

  _vectors->setSelectionMode(QListView::Single);
}

KstFieldSelectI::~KstFieldSelectI() {
  _ds = 0;
}

void KstFieldSelectI::setURL(const QString& url) {
  _url = url;

  fillFields();
}

void KstFieldSelectI::fillFields( ) {
  QStringList fl;
  QString fileType;
  bool complete = false;

  fl = KstDataSource::fieldListForSource(_url, QString::null, &fileType, &complete);

  for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
    QStringList     entries = QStringList::split(QDir::separator(), (*it), FALSE);
    QString         item;
    QListViewItem*  parent = 0L;
    QListViewItem*  parentOld = 0L;

    for (QStringList::ConstIterator itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
      item += (*itEntry);

      if (item.compare(*it) != 0) {
        parent = _fields.find(item);
        if (parent == 0L) {
          if (parentOld) {
            QListViewItem *listItem = new QListViewItem(parentOld, *itEntry);

            parentOld->setOpen(true);
            _fields.insert(item, listItem);
            parentOld = listItem;
          } else {
            QListViewItem *listItem = new QListViewItem(_vectors, *itEntry);

            _fields.insert(item, listItem);
            parentOld = listItem;
          }
        } else {
          parentOld = parent;
        }

        item += QDir::separator();
      } else if (parentOld) {
        QListViewItem *listItem = new QListViewItem(parentOld, *itEntry);

        parentOld->setOpen(true);
        _fields.insert(item, listItem);
      } else {
        QListViewItem *listItem = new QListViewItem(_vectors, *itEntry);

        _fields.insert(item, listItem);
      }
    }
  }

  _vectors->sort();
}


void KstFieldSelectI::vectorSubset(const QString& filter) {
  QRegExp re(filter, true /* case insensitive */, true /* wildcard */);
  QListViewItemIterator it(_vectors);

  _vectors->clearSelection();
  _vectors->setSorting(3, true); // Qt 3.1 compat

  while (it.current()) {
    QListViewItem *i = it.current();
    ++it;
    if (re.exactMatch(i->text(0))) {
      i->setSelected(true);
    }
  }
}


void KstFieldSelectI::search() {
  QString s = _vectorReduction->text();
  if (!s.isEmpty()) {
    if (s[0] != '*') {
      s = "*" + s;
    }
    if (s[s.length()-1] != '*') {
      s += "*";
    }
    _vectorReduction->setText(s);
  }
}


void KstFieldSelectI::OKFieldSelect() {
  QListViewItemIterator it(_vectors);
  QPtrList<QListViewItem> lst;

  _selection.truncate(0);

  while (it.current()) {
    if (it.current()->isSelected()) {
      if (it.current()->childCount() == 0) {
        lst.append(it.current());
      }
    }
    ++it;
  }

  if (lst.count() == 1) {
    QListViewItem *item = lst.getFirst();
    QListViewItem *parent = item->parent();
    _selection = item->text(0);

    while (parent) {
      _selection = parent->text(0) + QDir::separator() + _selection;
      parent = parent->parent();
    }
  }

  if (!_selection.isEmpty()) {
    accept();
  }
}


void KstFieldSelectI::CancelFieldSelect() {
  reject();
}

#include "kstfieldselect_i.moc"
