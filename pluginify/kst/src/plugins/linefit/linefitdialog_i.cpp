/***************************************************************************
                     kstlinefitdialog_i.h  -  Part of KST
                             -------------------
    begin                : 09/12/06
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

#include <assert.h>

#include "linefitdialogwidget.h"

// include files for Qt
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "linefitdialog_i.h"

// application specific includes
#include <kst.h>
#include <scalarselector.h>
#include <stringselector.h>
#include <vectorselector.h>

const QString& LineFitDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<LineFitDialogI> LineFitDialogI::_inst;

LineFitDialogI::LineFitDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new LineFitDialogWidget(_contents);
  setMultiple(false);
}

LineFitDialogI::~LineFitDialogI() {
}

#include <kdebug.h>
void LineFitDialogI::update()
{
  //called upon showing the dialog either in 'edit' mode or 'new' mode
}

bool LineFitDialogI::newObject()
{
  //called upon clicking 'ok' in 'new' mode
  //return false if the specified objects can't be made, otherwise true
  return false;
}

bool LineFitDialogI::editObject()
{
  //called upon clicking 'ok' in 'edit' mode
  //return false if the specified objects can't be editted, otherwise true

  LineFitPtr lf = kst_cast<LineFit>(_dp);
  if (!lf) {
    return false;
  }

  lf->writeLock();
  if (_tagName->text() != lf->tagName() && KstData::self()->dataTagNameNotUnique(_tagName->text())) {
    _tagName->setFocus();
    lf->unlock();
    return false;
  }

  lf->setTagName(_tagName->text());

  // Must unlock before clear()
  for (KstVectorMap::Iterator i = lf->inputVectors().begin(); i != lf->inputVectors().end(); ++i) {
    (*i)->unlock();
  }
  for (KstScalarMap::Iterator i = lf->inputScalars().begin(); i != lf->inputScalars().end(); ++i) {
    (*i)->unlock();
  }
  for (KstStringMap::Iterator i = lf->inputStrings().begin(); i != lf->inputStrings().end(); ++i) {
    (*i)->unlock();
  }
  lf->inputVectors().clear();
  lf->inputScalars().clear();
  lf->inputStrings().clear();

  // Save the vectors and scalars
  if (!saveInputs(lf)) {
    KMessageBox::sorry(this, i18n("There is an error in the input you entered."));
    lf->unlock();
    return false;
  }

  if (!saveOutputs(lf)) {
    KMessageBox::sorry(this, i18n("There is an error in the output you entered."));
    lf->unlock();
    return false;
  }

  if (!lf->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    lf->unlock();
    return false;
  }
  lf->setDirty();
  lf->unlock();

  emit modified();
  return true;
}

bool LineFitDialogI::saveInputs(LineFitPtr lf)
{
  Q_UNUSED(lf);
  //implement me
  return false;
}

bool LineFitDialogI::saveOutputs(LineFitPtr lf)
{
  Q_UNUSED(lf);
  //implement me
  return false;
}

void LineFitDialogI::fillFieldsForEdit() {
  LineFitPtr lf = kst_cast<LineFit>(_dp);
  if (!lf) {
    return;
  }

  lf->readLock();

  _tagName->setText(lf->tagName());
  _legendText->setText(defaultTag); //FIXME?

  _w->_xArray->setSelection( lf->xArrayTag() );
  _w->_yArray->setSelection( lf->yArrayTag() );

  _w->_xInterpolated->setText( lf->xInterpolatedTag() );
  _w->_yInterpolated->setText( lf->yInterpolatedTag() );

  _w->_a->setText( lf->aTag() );
  _w->_b->setText( lf->bTag() );
  _w->_chi2->setText( lf->chi2Tag() );

  lf->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void LineFitDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

#include "linefitdialog_i.moc"

// vim: ts=2 sw=2 et
