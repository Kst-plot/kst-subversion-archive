/***************************************************************************
                    ksteventmonitor.cpp  -  Part of KST
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

#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QTextEdit>

#include "editmultiplewidget.h"
#include "kst.h"
#include "kstdataobjectcollection.h"
#include "ksteventmonitor.h"
#include "ksteventmonitorentry.h"
#include "scalarselector.h"
#include "vectorselector.h"


QPointer<KstEventMonitor> KstEventMonitor::_inst;

KstEventMonitor* KstEventMonitor::globalInstance() {
  if (!_inst) {
    _inst = new KstEventMonitor(KstApp::inst());
  }

  return _inst;
}


KstEventMonitor::KstEventMonitor(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : KstDataDialog(parent) {
  _w = new Ui::EventMonitorWidget();
  _w->setupUi(this);

  setMultiple(true);
  connect(_w->_vectorSelectorEq, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_scalarSelectorEq, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_vectorSelectorEq, SIGNAL(selectionChangedLabel(const QString&)), _w->lineEditEquation, SLOT(insert(const QString&)));
  connect(_w->_vectorSelectorEq, SIGNAL(selectionChangedLabel(const QString&)), _w->lineEditEquation, SLOT(setFocus()));
  connect(_w->_scalarSelectorEq, SIGNAL(selectionChangedLabel(const QString&)), _w->lineEditEquation, SLOT(insert(const QString&)));
  connect(_w->_scalarSelectorEq, SIGNAL(selectionChangedLabel(const QString&)), _w->lineEditEquation, SLOT(setFocus()));
  connect(_w->_pushButtonELOGConfigure, SIGNAL(clicked()), KstApp::inst(), SLOT(EventELOGConfigure()));

  // more multiple edit mode
  connect(_w->checkBoxDebug, SIGNAL(clicked()), this, SLOT(setcheckBoxDebugDirty()));
  connect(_w->checkBoxEMailNotify, SIGNAL(clicked()), this, SLOT(setcheckBoxEMailNotifyDirty()));
  connect(_w->checkBoxELOGNotify, SIGNAL(clicked()), this, SLOT(setcheckBoxELOGNotifyDirty()));
  connect(_w->_useScript, SIGNAL(clicked()), this, SLOT(setScriptDirty()));
  connect(_w->_script, SIGNAL(textChanged()), this, SLOT(setScriptDirty()));

  // for apply button
  connect(_w->lineEditEquation, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->lineEditDescription, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->checkBoxDebug, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->radioButtonLogNotice, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->radioButtonLogWarning, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->radioButtonLogError, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->checkBoxEMailNotify, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->lineEditEMailRecipients, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->checkBoxELOGNotify, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_pushButtonELOGConfigure, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_useScript, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_script, SIGNAL(textChanged()), this, SLOT(wasModifiedApply()));

  setFixedHeight(height());
}


KstEventMonitor::~KstEventMonitor() {
}


void KstEventMonitor::fillFieldsForEdit() {
  EventMonitorEntryPtr ep;

  ep = kst_cast<EventMonitorEntry>(_dp);
  if (ep) {
    ep->readLock();
    _tagName->setText(ep->tagName());
  
    _w->lineEditEquation->setText(ep->event());
    _w->lineEditDescription->setText(ep->description());
    _w->checkBoxDebug->setChecked(ep->logKstDebug());
    _w->checkBoxEMailNotify->setChecked(ep->logEMail());
    _w->checkBoxELOGNotify->setChecked(ep->logELOG());
    _w->lineEditEMailRecipients->setText(ep->eMailRecipients());
    _w->_useScript->setEnabled(!ep->scriptCode().isEmpty());
    _w->_script->setText(ep->scriptCode());
  
    switch (ep->level()) {
      case KstDebug::Notice:
        _w->radioButtonLogNotice->setChecked(true);
        break;
      case KstDebug::Warning:
        _w->radioButtonLogWarning->setChecked(true);
        break;
      case KstDebug::Error:
        _w->radioButtonLogError->setChecked(true);
        break;
      default:
        _w->radioButtonLogWarning->setChecked(true);
        break;
    }
  
    ep->unlock();
    adjustSize();
    resize(minimumSizeHint());
    setFixedHeight(height());
  }
}


void KstEventMonitor::fillFieldsForNew() {
  KstEventMonitorEntryList events;
  QString new_label;

  events = kstObjectSubList<KstDataObject, EventMonitorEntry>(KST::dataObjectList);
  new_label = QString("E%1-").arg(events.count() + 1) + "<New_Event>";
  _tagName->setText(new_label);

  _w->radioButtonLogWarning->setChecked(true);
  _w->lineEditEquation->setText(QString::null);
  _w->lineEditDescription->setText(QString::null);
  _w->checkBoxDebug->setChecked(true);
  _w->checkBoxEMailNotify->setChecked(false);
  _w->checkBoxELOGNotify->setChecked(false);
  _w->lineEditEMailRecipients->setText(QString::null);
  _w->_useScript->setChecked(false);
  _w->_script->setText(QString::null);
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEventMonitor::update() {
  _w->_vectorSelectorEq->update();
  _w->_scalarSelectorEq->update();
}


void KstEventMonitor::fillEvent(EventMonitorEntryPtr& event) {
  event->setEvent(_w->lineEditEquation->text());
  event->setDescription(_w->lineEditDescription->text());
  event->setLogKstDebug(_w->checkBoxDebug->isChecked());
  event->setLogEMail(_w->checkBoxEMailNotify->isChecked());
  event->setLogELOG(_w->checkBoxELOGNotify->isChecked());
  event->setEMailRecipients(_w->lineEditEMailRecipients->text());
  event->setScriptCode(_w->_useScript->isChecked() ? _w->_script->toPlainText() : QString::null);

  if (_w->radioButtonLogNotice->isChecked()) {
    event->setLevel(KstDebug::Notice);
  } else if (_w->radioButtonLogWarning->isChecked()) {
    event->setLevel(KstDebug::Warning);
  } else if (_w->radioButtonLogError->isChecked()) {
    event->setLevel(KstDebug::Error);
  }

  event->reparse();
}


void KstEventMonitor::enableELOG() {
  _w->checkBoxELOGNotify->setEnabled(true);
  _w->_pushButtonELOGConfigure->setEnabled(true);
}


void KstEventMonitor::disableELOG() {
  _w->checkBoxELOGNotify->setEnabled(false);
  _w->_pushButtonELOGConfigure->setEnabled(false);
}


bool KstEventMonitor::newObject() {
  QString tag_name = _tagName->text();
  tag_name.replace("<New_Event>", _w->lineEditEquation->text());
  tag_name.replace(KstObjectTag::tagSeparator, KstObjectTag::tagSeparatorReplacement);

  // verify that the event name is unique
  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  EventMonitorEntryPtr event;

  event = new EventMonitorEntry(tag_name);
  event->writeLock();
  fillEvent(event);

  if (!event->isValid()) {
    event->unlock();
    event = 0L;

    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is a syntax error in the equation you entered."));
    return false;
  }

  event->unlock();

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(event);
  KST::dataObjectList.lock().unlock();

  event = 0L; // drop the reference before we update

// xxx  emit modified();

  return true;
}


bool KstEventMonitor::editSingleObject(EventMonitorEntryPtr emPtr) {
  emPtr->writeLock();

  if (_lineEditEquationDirty) {
    emPtr->setEvent(_w->lineEditEquation->text());
  }

  if (_lineEditDescriptionDirty) {
    emPtr->setDescription(_w->lineEditDescription->text());
  }

  if (_checkBoxDebugDirty) {
    if (!(_w->radioButtonLogNotice->isChecked() ||
          _w->radioButtonLogWarning->isChecked() ||
          _w->radioButtonLogError->isChecked()) && _w->checkBoxDebug->isChecked()) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select a Debug Log type."));
      emPtr->unlock();

      return false;
    }
    emPtr->setLogKstDebug(_w->checkBoxDebug->isChecked());
  }

  if (_checkBoxEMailNotifyDirty) {
    emPtr->setLogEMail(_w->checkBoxEMailNotify->isChecked());
  }

  if (_checkBoxELOGNotifyDirty) {
    emPtr->setLogELOG(_w->checkBoxELOGNotify->isChecked());
  }

  if (_lineEditEMailRecipientsDirty) {
    emPtr->setEMailRecipients(_w->lineEditEMailRecipients->text());
  }

  if (_scriptDirty) {
    if (_w->_useScript->isChecked()) {
      emPtr->setScriptCode(_w->_script->toPlainText());
    } else {
      emPtr->setScriptCode(QString::null);
    }
  }

  if (_w->radioButtonLogNotice->isChecked()) {
    emPtr->setLevel(KstDebug::Notice);
  } else if (_w->radioButtonLogWarning->isChecked()) {
    emPtr->setLevel(KstDebug::Warning);
  } else if (_w->radioButtonLogError->isChecked()) {
    emPtr->setLevel(KstDebug::Error);
  }

  emPtr->reparse();
  emPtr->unlock();

  return true;
}


bool KstEventMonitor::editObject() {
  KstEventMonitorEntryList emList;

  emList = kstObjectSubList<KstDataObject,EventMonitorEntry>(KST::dataObjectList);

  //
  // if editing multiple objects, edit each one...
  //

  if (_editMultipleMode) { 
    //
    // if text fields are empty, treat as non-dirty...
    //

    _lineEditEquationDirty = !_w->lineEditEquation->text().isEmpty();
    _lineEditDescriptionDirty = !_w->lineEditDescription->text().isEmpty();
    _lineEditEMailRecipientsDirty = !_w->lineEditEMailRecipients->text().isEmpty();

    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstEventMonitorEntryList::Iterator emIter;
        EventMonitorEntryPtr emPtr;

        emIter = emList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (emIter == emList.end()) {
          return false;
        }

        emPtr = *emIter;

        if (!editSingleObject(emPtr)) {
          return false;
        }

        didEdit = true;
      }
    }
    if (!didEdit) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select one or more objects to edit."));
      return false;
    }
  } else {
    EventMonitorEntryPtr ep;
    QString tag_name = _tagName->text();

    ep = kst_cast<EventMonitorEntry>(_dp);
    if (!ep || (tag_name != ep->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }

    ep->writeLock();
    ep->setTagName(tag_name);
    ep->unlock();

    // then edit the object
    _lineEditEquationDirty = true;
    _lineEditDescriptionDirty = true;
    _checkBoxDebugDirty = true;
    _radioButtonLogNoticeDirty = true;
    _radioButtonLogWarningDirty = true;
    _radioButtonLogErrorDirty = true;
    _checkBoxEMailNotifyDirty = true;
    _lineEditEMailRecipientsDirty = true;
    _checkBoxELOGNotifyDirty = true;
    _scriptDirty = true;
    if (!editSingleObject(ep)) {
      return false;
    }
  }

// xxx  emit modified();

  return true;
}


void KstEventMonitor::populateEditMultiple() {
  KstEventMonitorEntryList emlist;

  emlist = kstObjectSubList<KstDataObject,EventMonitorEntry>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertItems(0, emlist.tagNames());

  //
  // also intermediate state for multiple edit...
  //

  _w->lineEditEquation->setText("");
  _w->lineEditDescription->setText("");

  _w->checkBoxDebug->setTristate(true);
  _w->checkBoxDebug->setChecked(Qt::PartiallyChecked);
  _w->radioButtonLogNotice->setChecked(Qt::Unchecked);
  _w->radioButtonLogWarning->setChecked(Qt::Unchecked);
  _w->radioButtonLogError->setChecked(Qt::Unchecked);

  _w->checkBoxEMailNotify->setTristate(true);
  _w->checkBoxEMailNotify->setChecked(Qt::PartiallyChecked);
  _w->lineEditEMailRecipients->setText("");

  _w->checkBoxELOGNotify->setTristate(true);
  _w->checkBoxELOGNotify->setChecked(Qt::PartiallyChecked);

  _tagName->setText("");
  _tagName->setEnabled(false);

  _w->lineEditEMailRecipients->setEnabled(true); 
  _w->radioButtonLogNotice->setEnabled(true);
  _w->radioButtonLogWarning->setEnabled(true);
  _w->radioButtonLogError->setEnabled(true);

  _w->_useScript->setTristate(true);
  _w->_useScript->setChecked(Qt::PartiallyChecked);
  _w->_script->setEnabled(false);
  _w->_script->setText("");

  // and clean all the fields
  _lineEditEquationDirty = false;
  _lineEditDescriptionDirty = false;
  _checkBoxDebugDirty = false;
  _radioButtonLogNoticeDirty = false;
  _radioButtonLogWarningDirty = false;
  _radioButtonLogErrorDirty = false;
  _checkBoxEMailNotifyDirty = false;
  _lineEditEMailRecipientsDirty = false;
  _checkBoxELOGNotifyDirty = false;
  _scriptDirty = false;
}


void KstEventMonitor::setScriptDirty() {
  _w->_useScript->setTristate(false);
  _scriptDirty = true; 
}


void KstEventMonitor::setcheckBoxDebugDirty() {
  _w->checkBoxDebug->setTristate(false);
  _checkBoxDebugDirty = true; 
}


void KstEventMonitor::setcheckBoxEMailNotifyDirty() {
  _w->checkBoxEMailNotify->setTristate(false);
  _checkBoxEMailNotifyDirty = true;
}


void KstEventMonitor::setcheckBoxELOGNotifyDirty() {
  _w->checkBoxELOGNotify->setTristate(false);
  _checkBoxELOGNotifyDirty = true;
}

#include "ksteventmonitor.moc"

