/***************************************************************************
                     crossspectrumdialog_i.h  -  Part of KST
                             -------------------
    begin                : 09/14/06
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
#include "crossspectrumdialogwidget.h"
#include "crossspectrumdialog_i.h"

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

// application specific includes
#include <kst.h>
#include <kstdoc.h>
#include <scalarselector.h>
#include <stringselector.h>
#include <vectorselector.h>
#include <kstdefaultnames.h>
#include <kstdataobjectcollection.h>
#include <kstobjectdefaults.h>

const QString& CrossSpectrumDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

CrossSpectrumDialogI::CrossSpectrumDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new CrossSpectrumDialogWidget(_contents);
  setMultiple(false);

  connect(_w->_v1, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_v2, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_fft, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_sample, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(KstApp::inst()->document(), SIGNAL(updateDialogs()), this, SLOT(update()));

  _w->_fft->allowDirectEntry( true );
  _w->_sample->allowDirectEntry( true );
}


CrossSpectrumDialogI::~CrossSpectrumDialogI() {
}


QString CrossSpectrumDialogI::editTitle() {
  return i18n("Edit Cross Power Spectrum");
}


QString CrossSpectrumDialogI::newTitle() {
  return i18n("New Cross Power Spectrum");
}


void CrossSpectrumDialogI::update() {
  _w->_v1->update();
  _w->_v2->update();
  _w->_fft->update();
  _w->_sample->update();
}


bool CrossSpectrumDialogI::newObject() {
  //
  // called upon clicking 'ok' in 'new' mode
  // return false if the specified objects can't be made, otherwise true
  //
  QString tagName = _tagName->text();

  if (tagName != defaultTag && KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  }

  //
  // need to create a new object rather than use the one in KstDataObject pluginList...
  //
  CrossPowerSpectrumPtr cps = kst_cast<CrossPowerSpectrum>(KstDataObject::createPlugin("Cross Power Spectrum"));
  if (!cps) {
    return false;
  }

  KstWriteLocker pl(cps);

  if (tagName == defaultTag) {
    tagName = KST::suggestPluginName("crosspowerspectrum");
  }
  cps->setTagName(KstObjectTag::fromString(tagName));

  if (!editSingleObject(cps) || !cps->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    return false;
  }

  cps->setReal(_w->_real->text());
  cps->setImaginary(_w->_imaginary->text());
  cps->setFrequency(_w->_frequency->text());

  if (!cps || !cps->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the cross power spectrum you entered."));
    return false;
  }

  cps->setDirty();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(cps.data());
  KST::dataObjectList.lock().unlock();
  cps = 0L; // drop the reference
  emit modified();

  return true;
}


bool CrossSpectrumDialogI::editObject() {
  //
  // called upon clicking 'ok' in 'edit' mode
  // return false if the specified objects can't be edited, otherwise true
  //
  CrossPowerSpectrumPtr cps = kst_cast<CrossPowerSpectrum>(_dp);
  if (!cps) {
    return false;
  }

  cps->writeLock();
  if (_tagName->text() != cps->tagName() && KstData::self()->dataTagNameNotUnique(_tagName->text())) {
    _tagName->setFocus();
    cps->unlock();
    return false;
  }

  cps->setTagName(KstObjectTag::fromString(_tagName->text()));

  cps->inputVectors().clear();
  cps->inputScalars().clear();
  cps->inputStrings().clear();

  if (!editSingleObject(cps) || !cps->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    return false;
  }

  cps->setRecursed(false);
  if (cps->recursion()) {
    cps->setRecursed(true);
    cps->unlock();
    KMessageBox::error(this, i18n("There is a recursion resulting from the cross power spectrum you entered."));
    return false;
  }

  cps->unlock();
  cps->setDirty();

  emit modified();
  return true;
}


bool CrossSpectrumDialogI::editSingleObject(CrossPowerSpectrumPtr cps) {
  {
    KstReadLocker(&KST::vectorList.lock());
    KstVectorList::Iterator it;
    it = KST::vectorList.findTag(_w->_v1->selectedVector());
    if (it != KST::vectorList.end()) {
      cps->setV1(*it);
    }

    it = KST::vectorList.findTag(_w->_v2->selectedVector());
    if (it != KST::vectorList.end()) {
      cps->setV2(*it);
    }
  }

  {
    KstWriteLocker(&KST::scalarList.lock());
    KstScalarList::Iterator it2;
    it2 = KST::scalarList.findTag(_w->_fft->selectedScalar());
    if (it2 != KST::scalarList.end()) {
      cps->setFFT(*it2);
    } else {
      //
      // deal with direct entry...
      //
      bool ok;
      double val = _w->_fft->_scalar->currentText().toDouble(&ok);
      if (ok) {
        cps->setFFT(new KstScalar(
              KstObjectTag::fromString(_w->_fft->_scalar->currentText()), 0L,
              val, true, false, false));
      } else {
        //
        // deal with error...
        //
      }
    }

    it2 = KST::scalarList.findTag(_w->_sample->selectedScalar());
    if (it2 != KST::scalarList.end()) {
      cps->setSample(*it2);
    } else {
      //
      // deal with direct entry...
      //
      bool ok;
      double val = _w->_sample->_scalar->currentText().toDouble(&ok);
      if (ok) {
        cps->setSample(new KstScalar(
              KstObjectTag::fromString(_w->_sample->_scalar->currentText()),
              0L, val, true, false, false));
      } else {
        //
        // deal with error...
        //
      }
    }
  }

  return true;
}


void CrossSpectrumDialogI::fillFieldsForEdit() {
  CrossPowerSpectrumPtr cps = kst_cast<CrossPowerSpectrum>(_dp);
  if (!cps) {
    return;
  }

  cps->readLock();

  _tagName->setText(cps->tagName());
  _legendText->setText(defaultTag); //FIXME?

  _w->_v1->setSelection(cps->v1Tag());
  _w->_v2->setSelection(cps->v2Tag());

  _w->_fft->setSelection(cps->fftTag());
  _w->_sample->setSelection(cps->sampleTag());

  _w->_real->setText(cps->realTag());
  _w->_real->setEnabled( false );

  _w->_imaginary->setText(cps->imaginaryTag());
  _w->_imaginary->setEnabled( false );

  _w->_frequency->setText(cps->frequencyTag());
  _w->_frequency->setEnabled(false);

  cps->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void CrossSpectrumDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  _w->_fft->_scalar->setCurrentText(QString::number(KST::objectDefaults.fftLen()));
  _w->_sample->_scalar->setCurrentText(QString::number(KST::objectDefaults.psdFreq()));

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

#include "crossspectrumdialog_i.moc"

