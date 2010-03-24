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

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qradiobutton.h>
#include <qregexp.h>

#include <kstandarddirs.h>

#include "kstfieldselect.h"
#include "vectorlistview.h"

KstFieldSelect::KstFieldSelect(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  connect(_OK, SIGNAL(clicked()), this, SLOT(OKFieldSelect()));
  connect(_Cancel, SIGNAL(clicked()), this, SLOT(CancelFieldSelect()));
  connect(_vectorReduction, SIGNAL(textChanged(const QString&)), this, SLOT(vectorSubset(const QString&)));
  connect(_vectorSearch, SIGNAL(clicked()), this, SLOT(search()));

// xxx  _vectors->setSelectionMode(QListView::Single);
}

KstFieldSelect::~KstFieldSelect() {
  _ds = 0;
}

void KstFieldSelect::setURL(const QString& url) {
  _url = url;

  fillFields();
}

void KstFieldSelect::fillFields( ) {
  QStringList fl;
  QString fileType;
  bool complete = false;
/* xxx
  fl = KstDataSource::fieldListForSource(_url, QString::null, &fileType, &complete);

  for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
    QStringList       entries = (*it).split(QDir::separator(), FALSE);
    QString           item;
    QListWidgetItem*  parent = 0L;
    QListWidgetItem*  parentOld = 0L;

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
*/
}


void KstFieldSelect::vectorSubset(const QString& filter) {
/* xxx
  QRegExp re(filter, Qt::CaseInsensitive, QRegExp::Wildcard);
  QListWidgetItem::iterator it(_vectors);

  _vectors->clearSelection();
  _vectors->setSorting(3, true); // Qt 3.1 compat

  while (it.current()) {
    QListWidgetItem *i = *it;

    ++it;
    if (re.exactMatch(i->text(0))) {
      i->setSelected(true);
    }
  }
*/
}


void KstFieldSelect::search() {
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


void KstFieldSelect::OKFieldSelect() {
/* xxx
  QListWidgetItem::iterator it(_vectors);
  QList<QListWidgetItem> lst;

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
    QListWidgetItem *item = lst.getFirst();
    QListWidgetItem *parent = item->parent();

    _selection = item->text();

    while (parent) {
      _selection = parent->text() + QDir::separator() + _selection;
      parent = parent->parent();
    }
  }

  if (!_selection.isEmpty()) {
    accept();
  }
*/
}


void KstFieldSelect::CancelFieldSelect() {
  reject();
}

#include "kstfieldselect.moc"
