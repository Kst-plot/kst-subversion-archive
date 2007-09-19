/***************************************************************************
                     binnedmapdialog_i.h  -  Part of KST
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

#include "binnedmapdialogwidget.h"

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
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "binnedmapdialog_i.h"

// application specific includes
#include <kst.h>
#include <kstdoc.h>
#include <scalarselector.h>
#include <stringselector.h>
#include <vectorselector.h>
#include <kstdefaultnames.h>
#include <kstdataobjectcollection.h>
#include <kstobjectdefaults.h>

const QString& BinnedMapDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

BinnedMapDialogI::BinnedMapDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new BinnedMapDialogWidget(_contents);
  setMultiple(false);

  connect(_w->_X, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_Y, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_Z, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  //connect(_w->_binnedMap, SIGNAL(newMatrixCreated()), this, SIGNAL(modified()));
  //connect(_w->_hitsMap,   SIGNAL(newMatrixCreated()), this, SIGNAL(modified()));
  connect(this, SIGNAL(modified()), _w->_X, SLOT(update()));
  connect(this, SIGNAL(modified()), _w->_Y, SLOT(update()));
  connect(this, SIGNAL(modified()), _w->_Z, SLOT(update()));
  connect(_w->_AutoBin, SIGNAL(clicked()), this, SLOT(fillAutoRange()));
}


BinnedMapDialogI::~BinnedMapDialogI() {
}


void BinnedMapDialogI::update() {
  //called upon showing the dialog either in 'edit' mode or 'new' mode
}


bool BinnedMapDialogI::newObject() {
  //called upon clicking 'ok' in 'new' mode
  //return false if the specified objects can't be made, otherwise true

  QString tagName = _tagName->text();

  if (tagName != defaultTag && KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  }

  //Need to create a new object rather than use the one in KstDataObject pluginList
  BinnedMapPtr map = kst_cast<BinnedMap>(KstDataObject::createPlugin("Binned Map"));
  Q_ASSERT(map); //should never happen...

  KstWriteLocker pl(map);

  if (tagName == defaultTag) {
    tagName = KST::suggestPluginName("binnedmap");
  }
  map->setTagName(KstObjectTag::fromString(tagName));

  // Save the vectors and scalars
  if (!editSingleObject(map) || !map->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    return false;
  }

  map->setMap(_w->_binnedMap->text());
  map->setHitsMap(_w->_hitsMap->text());

  if (!map || !map->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the binned map you entered."));
    return false;
  }

  //xxxx
  map->setXMin(_w->_Xmin->text().toDouble());
  map->setXMax(_w->_Xmax->text().toDouble());
  map->setYMin(_w->_Ymin->text().toDouble());
  map->setYMax(_w->_Ymax->text().toDouble());

  map->setNX(_w->_Xn->value());
  map->setNY(_w->_Yn->value());
  map->setAutoBin(_w->_realTimeAutoBin->isChecked());

  map->setDirty();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(map.data());
  KST::dataObjectList.lock().unlock();
  map = 0L; // drop the reference
  emit modified();

  return true;
}


bool BinnedMapDialogI::editObject() {
  //called upon clicking 'ok' in 'edit' mode
  //return false if the specified objects can't be editted, otherwise true

  BinnedMapPtr map = kst_cast<BinnedMap>(_dp);
  if (!map) {
    return false;
  }

  map->writeLock();
  if (_tagName->text() != map->tagName() && KstData::self()->dataTagNameNotUnique(_tagName->text())) {
    _tagName->setFocus();
    map->unlock();
    return false;
  }

  map->setTagName(KstObjectTag::fromString(_tagName->text()));

  map->inputVectors().clear();

  map->unlock();

  // Save the vectors and scalars
  if (!editSingleObject(map) || !map->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    return false;
  }

  map->setDirty();

  emit modified();
  return true;
}


bool BinnedMapDialogI::editSingleObject(BinnedMapPtr map) {
  {
    KstReadLocker(&KST::vectorList.lock());
    KstVectorList::Iterator it;

    it = KST::vectorList.findTag(_w->_X->selectedVector());
    if (it != KST::vectorList.end()) {
      map->setX(*it);
    }

    it = KST::vectorList.findTag(_w->_Y->selectedVector());
    if (it != KST::vectorList.end()) {
      map->setY(*it);
    }

    it = KST::vectorList.findTag(_w->_Z->selectedVector());
    if (it != KST::vectorList.end()) {
      map->setZ(*it);
    }
    
    //xxxx
    map->setXMin(_w->_Xmin->text().toDouble());
    map->setXMax(_w->_Xmax->text().toDouble());
    map->setYMin(_w->_Ymin->text().toDouble());
    map->setYMax(_w->_Ymax->text().toDouble());

    map->setNX(_w->_Xn->value());
    map->setNY(_w->_Yn->value());
    map->setAutoBin(_w->_realTimeAutoBin->isChecked());

  }

  // FIXME: set the map properties
  return true;
}


void BinnedMapDialogI::fillFieldsForEdit() {
  BinnedMapPtr map = kst_cast<BinnedMap>(_dp);
  if (!map) {
    return;
  }

  map->readLock();

  _tagName->setText(map->tagName());
  _legendText->setText(defaultTag); //FIXME?

  _w->_X->setSelection(map->xTag());
  _w->_Y->setSelection(map->yTag());
  _w->_Z->setSelection(map->zTag());

  _w->_binnedMap->setText(map->mapTag());
  _w->_hitsMap->setText(map->hitsMapTag());

//   _w->_real->setText(map->realTag());
//   _w->_real->setEnabled( false );
// 
//   _w->_imaginary->setText(map->imaginaryTag());
//   _w->_imaginary->setEnabled( false );
// 
//   _w->_frequency->setText(map->frequencyTag());
//   _w->_frequency->setEnabled(false);

  // xxxxx
  _w->_Xmin->setText(QString::number(map->xMin()));
  _w->_Xmax->setText(QString::number(map->xMax()));
  _w->_Ymin->setText(QString::number(map->yMin()));
  _w->_Ymax->setText(QString::number(map->yMax()));

  _w->_Xn->setValue(map->nX());
  _w->_Yn->setValue(map->nY());
  _w->_realTimeAutoBin->setChecked(map->autoBin());

  map->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void BinnedMapDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

//   _w->_fft->_scalar->setCurrentText(QString::number(KST::objectDefaults.fftLen()));
//   _w->_sample->_scalar->setCurrentText(QString::number(KST::objectDefaults.psdFreq()));

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void BinnedMapDialogI::fillAutoRange() {
  int nx, ny;
  double minx, miny, maxx, maxy;
  KstVectorPtr vx = *KST::vectorList.findTag(_w->_X->selectedVector());
  KstVectorPtr vy = *KST::vectorList.findTag(_w->_Y->selectedVector());
  
  BinnedMap::AutoSize(vx,vy, &nx, &minx, &maxx, &ny, &miny, &maxy);
  
  _w->_Xmin->setText(QString::number(minx));
  _w->_Xmax->setText(QString::number(maxx));
  _w->_Ymin->setText(QString::number(miny));
  _w->_Ymax->setText(QString::number(maxy));
  _w->_Xn->setValue(nx);
  _w->_Yn->setValue(ny);

}

#include "binnedmapdialog_i.moc"

// vim: ts=2 sw=2 et
