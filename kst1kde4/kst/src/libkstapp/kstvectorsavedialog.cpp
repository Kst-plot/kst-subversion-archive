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

#include <qlayout.h>
#include <qlistbox.h>
#include <QMessageBox>
#include <qpushbutton.h>
#include <qtable.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>

#include "kstvectorsavedialog_i.h"
#include "vectorselector.h"

KstVectorSaveDialog::KstVectorSaveDialog(QWidget* parent,
                                             const char* name,
                                             bool modal,
                                             Qt::WindowFlags fl)
: QDialog(parent, name, modal, fl) {
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
  KstVectorList::ConstIterator i;
  
  _vectorList->clear();
  KST::vectorList.lock().readLock();
  for (i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    (*i)->readLock();
    _vectorList->insertItem((*i)->tag().displayString());
    (*i)->unlock();
  }
  KST::vectorList.lock().unlock();
  _saveButton->setEnabled(false);
}


void KstVectorSaveDialog::save() {
  KstVectorList toSave;

  KST::vectorList.lock().readLock();
  for (QListBoxItem *i = _vectorList->firstItem(); i; i = i->next()) {
    if (i->isSelected()) {
      KstVectorPtr v = *KST::vectorList.findTag(i->text());
      if (v) {
        toSave += v;
      }
    }
  }
  KST::vectorList.lock().unlock();

  KUrl url = KFileDialog::getSaveURL(QString::null, QString::null, this, i18n("Save Vector As"));
  if (!url.isEmpty()) {
    bool interpolate = true;
    switch (_multiOptions->currentItem()) {
      case 0:
        interpolate = false;

      case 1:
        {
          KTempFile tf(locateLocal("tmp", "kstvectors"), "txt");

          tf.setAutoDelete(true);
          if (0 != KstData::self()->vectorsToFile(toSave, tf.file(), interpolate)) {
            QMessageBox::warning(this, i18n("Kst"), i18n("Error saving vector to %1.").arg(url.prettyURL()), i18n("Kst"));
            return;
          }
          tf.sync();
          tf.close();

          if (KIO::NetAccess::exists(url, false, this)) {
            int rc = QMessageBox::warning(this, i18n("Kst"), i18n("File %1 exists.  Overwrite?").arg(url.prettyURL()), i18n("Kst"));
            if (rc == QMessageBox::No) {
              return;
            }
          }
          KIO::NetAccess::file_copy(KUrl(tf.name()), url, -1, true, false, this);
        }
      break;

      case 2:
        {
          unsigned n = 0;
          for (KstVectorList::Iterator i = toSave.begin(); i != toSave.end(); ++i) {
            Kurl url2 = url;

            if (toSave.count() > 1) {
              url2.setFileName(url.fileName() + QString(".%1").arg(++n));
            } else {
              url2.setFileName(url.fileName());
            }
            KTempFile tf(locateLocal("tmp", "kstvectors"), "txt");
            tf.setAutoDelete(true);
            if (0 != KstData::self()->vectorToFile(*i, tf.file())) {
              QMessageBox::warning(this, i18n("Kst"), i18n("Error saving vector to %1.").arg(url2.prettyURL()), i18n("Kst"));
              return;
            }
            tf.sync();
            tf.close();

            if (KIO::NetAccess::exists(url2, false, this)) {
              int rc = QMessageBox::warning(this, i18n("Kst"), i18n("File %1 exists.  Overwrite?").arg(url2.prettyURL()), i18n("Kst"));
              if (rc == QMessageBox::No) {
                continue;
              }
            }

            KIO::NetAccess::file_copy(KUrl(tf.name()), url2, -1, true, false, this);
          }
        }
      break;

      default:
        QMessageBox::warning(this, i18n("Kst"), i18n("Internal error.  Please report."), i18n("Kst"));
        break;
    }
  }
}


void KstVectorSaveDialogI::show() {
  init();
  QDialog::show();
}


void KstVectorSaveDialogI::selectionChanged() {
  int cnt = 0;
  // expensive but qlistbox provides nothing better at the moment
  for (QListBoxItem *i = _vectorList->firstItem(); i; i = i->next()) {
    if (i->isSelected()) {
      if (++cnt > 1) {
        break;
      }
    }
  }
  _multiOptions->setEnabled(cnt > 1);
  _saveButton->setEnabled(cnt > 0);
}

#include "kstvectorsavedialog.moc"

