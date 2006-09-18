/***************************************************************************
                     kstbasicdialog_i.cpp  -  Part of KST
                             -------------------
    begin                : 09/15/06
    copyright            : (C) 2006 The University of Toronto
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
#include <qlineedit.h>
#include <qvbox.h>

// include files for KDE

#include "kstbasicdialog_i.h"
#include "basicdialogwidget.h"

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"
#include "kstdefaultnames.h"
#include "kstdataobjectcollection.h"

const QString& KstBasicDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstBasicDialogI> KstBasicDialogI::_inst;

KstBasicDialogI *KstBasicDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstBasicDialogI(KstApp::inst());
  }
  return _inst;
}


KstBasicDialogI::KstBasicDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  setMultiple(false);
  _w = new BasicDialogWidget(_contents);
  connect(this, SIGNAL(modified()), KstApp::inst()->document(), SLOT(wasModified())); //FIXME this should be in KstDataDialog constructor...

  _inputOutputGrid = 0L;
}


KstBasicDialogI::~KstBasicDialogI() {
}


void KstBasicDialogI::update() {
  //called upon showing the dialog either in 'edit' mode or 'new' mode
}


bool KstBasicDialogI::newObject() {
  //called upon clicking 'ok' in 'new' mode
  //return false if the specified objects can't be made, otherwise true
  return true;
}


bool KstBasicDialogI::editObject() {
  //called upon clicking 'ok' in 'edit' mode
  //return false if the specified objects can't be editted, otherwise true
  return true;
}


bool KstBasicDialogI::editSingleObject(KstBasicPluginPtr ptr) {
  Q_UNUSED(ptr)
  return true;
}


void KstBasicDialogI::fillFieldsForEdit() {

  KstBasicPluginPtr ptr = kst_cast<KstBasicPlugin>(_dp);
  if (!ptr) {
    return;
  }

  ptr->readLock();

  _tagName->setText(ptr->tagName());
  _legendText->setText(defaultTag); //FIXME?

  //Update the various widgets...

  ptr->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstBasicDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

#include "kstbasicdialog_i.moc"

// vim: ts=2 sw=2 et
