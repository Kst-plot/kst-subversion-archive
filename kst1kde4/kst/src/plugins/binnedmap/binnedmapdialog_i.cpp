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

#include <QCheckBox>
#include <QComboBox>
#include <QDomElement>
#include <QDomNode>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QToolTip>
#include <QWhatsThis>

#include <kcolorbutton.h>

#include "binnedmapdialog_i.h"

#include <kst.h>
#include <kstdoc.h>
#include <scalarselector.h>
#include <stringselector.h>
#include <vectorselector.h>
#include <kstdefaultnames.h>
#include <kstdataobjectcollection.h>
#include <kstobjectdefaults.h>

const QString& BinnedMapDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

BinnedMapDialogI::BinnedMapDialogI(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : KstDataDialog(parent) {
  _w = new Ui_BinnedMapDialogWidget();
  _w->setupUi(this);

  setMultiple(false);

  connect(_w->_X, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_Y, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_Z, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(this, SIGNAL(modified()), _w->_X, SLOT(update()));
  connect(this, SIGNAL(modified()), _w->_Y, SLOT(update()));
  connect(this, SIGNAL(modified()), _w->_Z, SLOT(update()));
  connect(_w->_AutoBin, SIGNAL(clicked()), this, SLOT(fillAutoRange()));
  connect(KstApp::inst()->document(), SIGNAL(updateDialogs()), this, SLOT(update()));
}


BinnedMapDialogI::~BinnedMapDialogI() {
}


QString BinnedMapDialogI::editTitle() {
  return QObject::tr("Edit Binned Map");
}


QString BinnedMapDialogI::newTitle() {
  return QObject::tr("New Binned Map");
}


void BinnedMapDialogI::update() {
  _w->_X->update();
  _w->_Y->update();
  _w->_Z->update();
}


bool BinnedMapDialogI::newObject() {
  //
  // called upon clicking 'ok' in 'new' mode
  // return false if the specified objects can't be made, otherwise true
  //

  QString tagName = _tagName->text();
  BinnedMapPtr map;
  bool rc = false;

  if (tagName != defaultTag && KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();

    return false;
  }

  //
  // need to create a new object rather than 
  //  use the one in KstDataObject pluginList...
  //

  map = kst_cast<BinnedMap>(KstDataObject::createPlugin("Binned Map"));
  if (map) {
    KstWriteLocker pl(map.data());
  
    if (tagName == defaultTag) {
      tagName = KST::suggestPluginName("binnedmap");
    }
    map->setTag(KstObjectTag::fromString(tagName));
  
    //
    // save the vectors and scalars...
    //

    if (!editSingleObject(map) || !map->isValid()) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the values you entered."));
      return false;
    }
  
    map->setMap(_w->_binnedMap->text());
    map->setHitsMap(_w->_hitsMap->text());
  
    if (!map || !map->isValid()) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the binned map you entered."));
      return false;
    }
  
    map->setXMin(_w->_Xmin->text().toDouble());
    map->setXMax(_w->_Xmax->text().toDouble());
    map->setYMin(_w->_Ymin->text().toDouble());
    map->setYMax(_w->_Ymax->text().toDouble());
  
    map->setNX(_w->_Xn->value());
    map->setNY(_w->_Yn->value());
    map->setAutoBin(_w->_realTimeAutoBin->isChecked());
  
    map->setDirty();
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(map);
    KST::dataObjectList.lock().unlock();
  
    rc = true;

    map = 0L; // drop the reference
  
// xxx    emit modified();
  }

  return rc;
}


bool BinnedMapDialogI::editObject() {
  //
  // called upon clicking 'ok' in 'edit' mode
  // return false if the specified objects can't be editted, otherwise true
  //

  BinnedMapPtr map;
  bool rc = false;

  map = kst_cast<BinnedMap>(_dp);
  if (map) {
    map->writeLock();

    if (_tagName->text() != map->tagName() && 
        KstData::self()->dataTagNameNotUnique(_tagName->text())) {
      _tagName->setFocus();
      map->unlock();
    } else {
      map->setTag(KstObjectTag::fromString(_tagName->text()));
      map->inputVectors().clear();
      map->unlock();
    
      //
      // save the vectors and scalars...
      //
    
      if (editSingleObject(map) && map->isValid()) {
        rc = true;

        map->setDirty();    

        emit modified();
      } else {
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the values you entered."));
      }
    }
  }

  return rc;
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

    map->setXMin(_w->_Xmin->text().toDouble());
    map->setXMax(_w->_Xmax->text().toDouble());
    map->setYMin(_w->_Ymin->text().toDouble());
    map->setYMax(_w->_Ymax->text().toDouble());

    map->setNX(_w->_Xn->value());
    map->setNY(_w->_Yn->value());
    map->setAutoBin(_w->_realTimeAutoBin->isChecked());
  }

  return true;
}


void BinnedMapDialogI::fillFieldsForEdit() {
  BinnedMapPtr map;

  map = kst_cast<BinnedMap>(_dp);
  if (map) {
    map->readLock();
  
    _tagName->setText(map->tagName());
    _legendText->setText(defaultTag);
  
    _w->_X->setSelection(map->xTag());
    _w->_Y->setSelection(map->yTag());
    _w->_Z->setSelection(map->zTag());
  
    _w->_binnedMap->setText(map->mapTag());
    _w->_hitsMap->setText(map->hitsMapTag());
  
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
}


void BinnedMapDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void BinnedMapDialogI::fillAutoRange() {
  KstVectorPtr vx;
  KstVectorPtr vy;
  double minx;
  double miny;
  double maxx;
  double maxy;
  int nx;
  int ny;

  vx = *KST::vectorList.findTag(_w->_X->selectedVector());
  vy = *KST::vectorList.findTag(_w->_Y->selectedVector());
  if (vx && vy) {
    BinnedMap::autoSize(vx, vy, &nx, &minx, &maxx, &ny, &miny, &maxy);

    _w->_Xmin->setText(QString::number(minx));
    _w->_Xmax->setText(QString::number(maxx));
    _w->_Ymin->setText(QString::number(miny));
    _w->_Ymax->setText(QString::number(maxy));
    _w->_Xn->setValue(nx);
    _w->_Yn->setValue(ny);
  }
}

#include "binnedmapdialog_i.moc"
