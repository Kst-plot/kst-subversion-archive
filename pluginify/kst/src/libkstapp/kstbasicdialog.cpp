/***************************************************************************
                     kstbasicdialog.cpp  -  Part of KST
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

// include files for KDE

#include "kstbasicdialog.h"

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"
#include "kstdefaultnames.h"
#include "kstdataobjectcollection.h"

const QString& KstBasicDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

KstBasicDialog::KstBasicDialog(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  setMultiple(false);

  connect(this, SIGNAL(modified()), KstApp::inst()->document(), SLOT(wasModified())); //FIXME this should be in KstDataDialog constructor...
}


KstBasicDialog::~KstBasicDialog() {
}


void KstBasicDialog::update() {
  //called upon showing the dialog either in 'edit' mode or 'new' mode
}


bool KstBasicDialog::newObject() {
  //called upon clicking 'ok' in 'new' mode
  //return false if the specified objects can't be made, otherwise true
  return true;
}


bool KstBasicDialog::editObject() {
  //called upon clicking 'ok' in 'edit' mode
  //return false if the specified objects can't be editted, otherwise true
  return true;
}


bool KstBasicDialog::editSingleObject(KstBasicPluginPtr ptr) {
  Q_UNUSED(ptr)
  return true;
}


void KstBasicDialog::fillFieldsForEdit() {

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


void KstBasicDialog::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

#include "kstbasicdialog.moc"

// vim: ts=2 sw=2 et
