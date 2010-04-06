/***************************************************************************
                    kstvectorsavedialog.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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

#include <QFile>
#include <QFileDialog>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>

#include "kstvectorsavedialog.h"
#include "vectorselector.h"

KstVectorSaveDialog::KstVectorSaveDialog(QWidget* parent, const char* name,
                                         bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  connect(pushButton2, SIGNAL(clicked()), this, SLOT(hide()));
  connect(_saveButton, SIGNAL(clicked()), this, SLOT(save()));
  connect(_vectorList, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  connect(_vectorList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(save()));

  init();
}


KstVectorSaveDialog::~KstVectorSaveDialog() {
}


void KstVectorSaveDialog::init() {
  KstVectorList::const_iterator i;
  
  _vectorList->clear();

  KST::vectorList.lock().readLock();
  for (i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    (*i)->readLock();
    _vectorList->addItem((*i)->tag().displayString());
    (*i)->unlock();
  }
  KST::vectorList.lock().unlock();

  _saveButton->setEnabled(false);
}


void KstVectorSaveDialog::save() {
  KstVectorList toSave;
  int i;

  KST::vectorList.lock().readLock();
  for (i = 0; i < _vectorList->count(); ++i) {
    if (_vectorList->item(i)->isSelected()) {
      KstVectorPtr v;

      v = *KST::vectorList.findTag(_vectorList->item(i)->text());
      if (v) {
        toSave += v;
      }
    }
  }
  KST::vectorList.lock().unlock();

  QString str = QFileDialog::getSaveFileName(this, tr("Save Vector As"), QString::null, QString::null);
  if (!str.isEmpty()) {
    bool interpolate = true;

    switch (_multiOptions->currentIndex()) {
      case 0:
        interpolate = false;

      case 1:
        {
          QFile file(str);

          if (KstData::self()->vectorsToFile(toSave, &file, interpolate) != 0) {
            QMessageBox::warning(this, tr("Kst"), tr("Error saving vector to %1.").arg(str));
            return;
          }
          file.close();
/* xxx
          if (KIO::NetAccess::exists(url, false, this)) {
            int rc = QMessageBox::warning(this, tr("Kst"), tr("File %1 exists.  Overwrite?").arg(url.prettyURL()), tr("Kst"), QMessageBox::Yes | QMessageBox::No);
            if (rc == QMessageBox::No) {
              return;
            }
          }
*/
        }
      break;

      case 2:
        {
          KstVectorList::iterator it;
          unsigned n = 0;
 
          for (it = toSave.begin(); it != toSave.end(); ++it) {
            QFile file;

            if (toSave.count() > 1) {
              file.setFileName(str + QString(".%1").arg(++n));
            } else {
              file.setFileName(str);
            }

            if (KstData::self()->vectorToFile(*it, &file) != 0) {
              QMessageBox::warning(this, tr("Kst"), tr("Error saving vector to %1.").arg(str));
              return;
            }
            file.close();
/* xxx
            if (KIO::NetAccess::exists(url2, false, this)) {
              int rc = QMessageBox::warning(this, tr("Kst"), tr("File %1 exists.  Overwrite?").arg(url2.prettyURL()), tr("Kst"));

              if (rc == QMessageBox::No) {
                continue;
              }
            }

            KIO::NetAccess::file_copy(KUrl(tf.name()), url2, -1, true, false, this);
*/
          }
        }
      break;

      default:
        QMessageBox::warning(this, tr("Kst"), tr("Internal error. Please report."));
        break;
    }
  }
}


void KstVectorSaveDialog::show() {
  init();
  QDialog::show();
}


void KstVectorSaveDialog::selectionChanged() {
  int cnt = 0;
  int i;

  for (i = 0; i < _vectorList->count(); ++i) {
    if (_vectorList->item(i)->isSelected()) {
      if (++cnt > 1) {
        break;
      }
    }
  }
  _multiOptions->setEnabled(cnt > 1);
  _saveButton->setEnabled(cnt > 0);
}

#include "kstvectorsavedialog.moc"

