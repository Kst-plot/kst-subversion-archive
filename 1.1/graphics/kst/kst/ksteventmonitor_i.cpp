/***************************************************************************
                    ksteventmonitor_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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

// include files for Qt
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>

// include files for KDE
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// application specific includes
#include "kst.h"
#include "ksteventmonitor_i.h"
#include "scalarselector.h"
#include "vectorselector.h"

#define DIALOGTYPE KstEventMonitorI
#define DTYPE "Event Monitor"
#include "dataobjectdialog.h"

QGuardedPtr<KstEventMonitorI> KstEventMonitorI::_inst;

KstEventMonitorI* KstEventMonitorI::globalInstance() {
  if (!_inst) {
    _inst = new KstEventMonitorI(KstApp::inst());
  }

  return _inst;
}


KstEventMonitorI::KstEventMonitorI(QWidget* parent,
                                   const char* name, bool modal, WFlags fl)
: EventMonitor(parent, name, modal, fl) {
  Init();

  connect(_vectorSelectorEq, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_scalarSelectorEq, SIGNAL(newScalarCreated()),
          this, SIGNAL(modified()));
  connect(_vectorSelectorEq, SIGNAL(selectionChangedLabel(const QString&)),
          lineEditEquation, SLOT(insert(const QString&)));
  connect(_scalarSelectorEq, SIGNAL(selectionChangedLabel(const QString&)),
          lineEditEquation, SLOT(insert(const QString&)));
  connect(_pushButtonELOGConfigure, SIGNAL(clicked()),
          KstApp::inst(), SLOT(EventELOGConfigure()));

  setFixedHeight(height());
}


KstEventMonitorI::~KstEventMonitorI() {
  DP = 0L;
}


EventMonitorEntryPtr KstEventMonitorI::_getPtr(const QString &tagin) {
  KstEventMonitorEntryList c = kstObjectSubList<KstDataObject, EventMonitorEntry>(KST::dataObjectList);
  return *c.findTag(tagin);
}


void KstEventMonitorI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }
  DP->readLock();
  _tagName->setText(DP->tagName());

  lineEditEquation->setText(DP->event());
  lineEditDescription->setText(DP->description());
  checkBoxDebug->setChecked(DP->logKstDebug());
  checkBoxEMailNotify->setChecked(DP->logEMail());
  checkBoxELOGNotify->setChecked(DP->logELOG());
  lineEditEMailRecipients->setText(DP->eMailRecipients());

  switch (DP->level()) {
    case KstDebug::Notice:
      radioButtonLogNotice->setChecked(true);
      break;
    case KstDebug::Warning:
      radioButtonLogWarning->setChecked(true);
      break;
    case KstDebug::Error:
      radioButtonLogError->setChecked(true);
      break;
    default:
      radioButtonLogWarning->setChecked(true);
      break;
  }

  DP->readUnlock();
}


void KstEventMonitorI::_fillFieldsForNew() {
  KstEventMonitorEntryList events = kstObjectSubList<KstDataObject, EventMonitorEntry>(KST::dataObjectList);

  QString new_label = QString("E%1-").arg(events.count() + 1) + "<New_Event>";
  _tagName->setText(new_label);

  radioButtonLogWarning->setChecked(true);
  lineEditEquation->setText(QString::null);
  lineEditDescription->setText(QString::null);
  checkBoxDebug->setChecked(true);
  checkBoxEMailNotify->setChecked(false);
  checkBoxELOGNotify->setChecked(false);
  lineEditEMailRecipients->setText(QString::null);
}


void KstEventMonitorI::update() {
  _vectorSelectorEq->update();
  _scalarSelectorEq->update();
}


void KstEventMonitorI::fillEvent(EventMonitorEntryPtr& event) {
  event->setEvent(lineEditEquation->text());
  event->setDescription(lineEditDescription->text());
  event->setLogKstDebug(checkBoxDebug->isChecked());
  event->setLogEMail(checkBoxEMailNotify->isChecked());
  event->setLogELOG(checkBoxELOGNotify->isChecked());
  event->setEMailRecipients(lineEditEMailRecipients->text());

  if (radioButtonLogNotice->isChecked()) {
    event->setLevel(KstDebug::Notice);
  } else if (radioButtonLogWarning->isChecked()) {
    event->setLevel(KstDebug::Warning);
  } else if (radioButtonLogError->isChecked()) {
    event->setLevel(KstDebug::Error);
  }

  event->reparse();
}


void KstEventMonitorI::enableELOG() {
  checkBoxELOGNotify->setEnabled(true);
  _pushButtonELOGConfigure->setEnabled(true);
}


void KstEventMonitorI::disableELOG() {
  checkBoxELOGNotify->setEnabled(false);
  _pushButtonELOGConfigure->setEnabled(false);
}


bool KstEventMonitorI::new_I() {
  QString tag_name = _tagName->text();
  tag_name.replace("<New_Event>", lineEditEquation->text());

  // verify that the event name is unique
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  EventMonitorEntryPtr event = new EventMonitorEntry(tag_name);
  fillEvent(event);

  if (!event->isValid()) {
    event = 0L;

    KMessageBox::sorry(this, i18n("There is a syntax error in the equation "
          "you entered. Please fix it."));
    return false;
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(event.data());
  KST::dataObjectList.lock().writeUnlock();

  event = 0L; // drop the reference before we update
  emit modified();
  return true;
}


bool KstEventMonitorI::edit_I() {
  // verify that the tag name is unique
  QString tag_name = _tagName->text();
  DP->writeLock();
  if (tag_name != DP->tagName() && KST::dataTagNameNotUnique(tag_name)) {
    DP->writeUnlock();
    _tagName->setFocus();
    return false;
  }
  DP->setTagName(_tagName->text());

  fillEvent(DP);
  DP->writeUnlock();
  emit modified();

  return true;
}

#include "ksteventmonitor_i.moc"
// vim: ts=2 sw=2 et
