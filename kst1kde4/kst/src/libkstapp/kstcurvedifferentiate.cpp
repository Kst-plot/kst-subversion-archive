/**************************************************************************
        kstcurvedifferentiate.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2005
    copyright            : (C) 2005 by University of British Columbia
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

#include <kiconloader.h>

#include "kst2dplot.h"
#include "kstcurvedifferentiate.h"
#include "kstlinestyle.h"
#include "kstnumbersequence.h"
#include "kstviewwindow.h"

KstCurveDifferentiate::KstCurveDifferentiate(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  Q_UNUSED(name)
  Q_UNUSED(modal)

  setupUi(this);

  availableListBox->clear();
  selectedListBox->clear();
  availableListBox->addItem(tr("Line Color"));  
  availableListBox->addItem(tr("Point Style"));  
  availableListBox->addItem(tr("Line Style"));
  availableListBox->addItem(tr("Line Width"));
  
  connect(Cancel_2, SIGNAL(clicked()), this, SLOT(close()));
  connect(OK_2, SIGNAL(clicked()), this, SLOT(apply()));
  
  // more connections to emulate kactionselector behaviour
  connect(_add, SIGNAL(clicked()), this, SLOT(addButtonClicked()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeButtonClicked()));
  connect(_up, SIGNAL(clicked()), this, SLOT(upButtonClicked()));
  connect(_down, SIGNAL(clicked()), this, SLOT(downButtonClicked()));
  connect(availableListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  connect(selectedListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  
// xxx  _up->setPixmap(BarIcon("up"));
  _up->setEnabled(false);
// xxx  _down->setPixmap(BarIcon("down"));
  _down->setEnabled(false);
// xxx  _add->setPixmap(BarIcon("forward"));
  _add->setEnabled(false);
// xxx  _remove->setPixmap(BarIcon("back"));
  _remove->setEnabled(false);

  maxLineWidth->setMaximum(KSTLINEWIDTH_MAX);
  maxLineWidth->setMinimum(1);
  
  _radioButtonRepeatPlot->setChecked(true);
  _radioButtonApplyAllWindows->setChecked(true);
  
  loadProperties();
  setOptions();
  updateCurveDifferentiate();
}


KstCurveDifferentiate::~KstCurveDifferentiate() {
  
}


void KstCurveDifferentiate::updateCurveDifferentiate() {
  updateButtons();
}


void KstCurveDifferentiate::showCurveDifferentiate() {
  updateCurveDifferentiate();
  show();
  raise();
}


void KstCurveDifferentiate::updateButtons() {
  QListWidgetItem *item;
  bool enabledUp = false;
  bool enabledDown = false;
  int i;

  i = selectedListBox->currentRow();
  item = selectedListBox->item(i);
  if (i >= 0 && item->isSelected()) {
    _remove->setEnabled(true); 
  } else {
    _remove->setEnabled(false);
  }
  
  i = availableListBox->currentRow();
  item = availableListBox->item(i);
  if (i >= 0 && item->isSelected()) {
    _add->setEnabled(true); 
  } else {
    _add->setEnabled(false);
  }

  if (selectedListBox->currentRow() > 0) {
    enabledUp = true;
  }
  _up->setEnabled(enabledUp);

  if (selectedListBox->currentRow() >= 0 &&
      selectedListBox->currentRow() < (int)selectedListBox->count() - 1) {
    enabledDown = true;
  }
  _down->setEnabled(enabledDown);
}


void KstCurveDifferentiate::loadProperties( ) {
  KConfig cfg("kstrc");
  KConfigGroup cfgGroup(&cfg, "curveDifferentiate");
/* xxx
  _lineColorOrder = cfgGroup.readNumEntry("DifferentiateLineColor", -1);
  _pointStyleOrder = cfgGroup.readNumEntry("DifferentiatePointStyle", -1);
  _lineStyleOrder = cfgGroup.readNumEntry("DifferentiateLineStyle", -1);
  _lineWidthOrder = cfgGroup.readNumEntry("DifferentiateLineWidth", -1);
  _maxLineWidth = cfgGroup.readNumEntry("DifferentiateMaxLineWidth", 1);
  _pointDensity = cfgGroup.readNumEntry("DifferentiatePointDensity", 0);
  _repeatAcross = cfgGroup.readNumEntry("DifferentiateRepeatAcross", 0);
  _applyTo = cfgGroup.readNumEntry("DifferentiateApplyTo", 0);
*/
}


void KstCurveDifferentiate::saveProperties( ) {  
  KConfig cfg("kstrc");
  KConfigGroup cfgGroup(&cfg, "curveDifferentiate");

  cfgGroup.writeEntry("DifferentiateLineColor", _lineColorOrder);
  cfgGroup.writeEntry("DifferentiatePointStyle", _pointStyleOrder);
  cfgGroup.writeEntry("DifferentiateLineStyle", _lineStyleOrder);
  cfgGroup.writeEntry("DifferentiateLineWidth", _lineWidthOrder);
  cfgGroup.writeEntry("DifferentiateMaxLineWidth", _maxLineWidth);
  cfgGroup.writeEntry("DifferentiatePointDensity", _pointDensity);
  cfgGroup.writeEntry("DifferentiateRepeatAcross", _repeatAcross);
  cfgGroup.writeEntry("DifferentiateApplyTo", _applyTo);

  cfgGroup.sync();
}  


void KstCurveDifferentiate::setOptions( ) {  
  QRadioButton* button;
  
  availableListBox->clear();
  selectedListBox->clear();
  
  if (_lineColorOrder == -1) {
    availableListBox->addItem(tr("Line Color"));
  } else {
    selectedListBox->insertItem(_lineColorOrder, tr("Line Color"));
  }
  if (_pointStyleOrder == -1) {
    availableListBox->addItem(tr("Point Style"));
  } else {
    selectedListBox->insertItem(_pointStyleOrder, tr("Point Style"));
  }
  if (_lineStyleOrder == -1) {
    availableListBox->addItem(tr("Line Style"));
  } else {
    selectedListBox->insertItem(_lineStyleOrder, tr("Line Style"));
  }
  if (_lineWidthOrder == -1) {
    availableListBox->addItem(tr("Line Width"));
  } else {
    selectedListBox->insertItem(_lineWidthOrder, tr("Line Width"));
  }
  
  maxLineWidth->setValue(_maxLineWidth);
  pointDensity->setCurrentIndex(_pointDensity);

  button = (QRadioButton*)_buttonGroupRepeat->find(_repeatAcross);
  if (button) {
    button->setChecked(true);
  }
  button = (QRadioButton*)_buttonGroupApplyTo->find(_applyTo);
  if (button) {
    button->setChecked(true);
  }
}


void KstCurveDifferentiate::getOptions( ) {
  QList<QListWidgetItem*> items;

  items.append(selectedListBox->findItems(tr("Line Color"), Qt::MatchExactly));
  if (!items.isEmpty()) {
    _lineColorOrder = selectedListBox->row(items.first());
  }
  items.clear();

  items.append(selectedListBox->findItems(tr("Point Style"), Qt::MatchExactly));
  if (!items.isEmpty()) {
    _pointStyleOrder = selectedListBox->row(items.first());
  }
  items.clear();

  items.append(selectedListBox->findItems(tr("Line Style"), Qt::MatchExactly));
  if (!items.isEmpty()) {
    _lineStyleOrder = selectedListBox->row(items.first());
  }
  items.clear();

  items.append(selectedListBox->findItems(tr("Line Width"), Qt::MatchExactly));
  if (!items.isEmpty()) {
    _lineWidthOrder = selectedListBox->row(items.first());
  }
  items.clear();
   
  _maxLineWidth = maxLineWidth->value();
  _pointDensity = pointDensity->currentIndex();
  
// xxx  _repeatAcross = _buttonGroupRepeat->selectedId();
// xxx  _applyTo = _buttonGroupApplyTo->selectedId();
}


void KstCurveDifferentiate::removeButtonClicked() {
  //
  // move from selected to available
  //

  for (int i = 0; i < selectedListBox->count(); i++) {
    if (selectedListBox->item(i)->isSelected()) {
      availableListBox->addItem(selectedListBox->item(i)->text());
      selectedListBox->removeItemWidget(selectedListBox->item(i));
      availableListBox->item((int)availableListBox->count() - 1)->setSelected(true); 
    }  
  }
  updateButtons();
}


void KstCurveDifferentiate::addButtonClicked() {
  //
  // move from available to selected
  //

  for (int i = 0; i < availableListBox->count(); i++) {
    if (availableListBox->item(i)->isSelected()) {
      selectedListBox->addItem(availableListBox->item(i)->text());
      availableListBox->removeItemWidget(availableListBox->item(i));  
      selectedListBox->item((int)selectedListBox->count() - 1)->setSelected(true);
    }  
  }
  updateButtons();
}


void KstCurveDifferentiate::upButtonClicked() {
  //
  // move item up
  //

  int i = selectedListBox->row(selectedListBox->currentItem());

  if (i != -1) {
    QString text = selectedListBox->currentItem()->text();
    selectedListBox->removeItemWidget(selectedListBox->currentItem());
    selectedListBox->insertItem(i-1, text);
    selectedListBox->item(i-1)->setSelected(true);

    updateButtons();
  }
}


void KstCurveDifferentiate::downButtonClicked() {
  //
  // move item down
  //

  int i = selectedListBox->row(selectedListBox->currentItem());

  if (i != -1) {
    QString text = selectedListBox->currentItem()->text();
    selectedListBox->removeItemWidget(selectedListBox->currentItem());
    selectedListBox->insertItem(i+1, text);
    selectedListBox->item(i+1)->setSelected(true);

    updateButtons();
  }
}


void KstCurveDifferentiate::cycleWindow(KstViewWindow *window) {
  KstTopLevelViewPtr tlv(kst_cast<KstTopLevelView>(window->view()));
  if (tlv) {
    Kst2DPlotList plotList;
    Kst2DPlotList::Iterator it;

// xxx    plotList = tlv->findChildrenType<Kst2DPlot>(false);
    for (it = plotList.begin(); it != plotList.end(); ++it ) {
      if (_repeatAcross == 0) {
        _seqVect[0].reset();
      }
      
      KstVCurveList vcurves;
      KstVCurveList::Iterator i;

// xxx      vcurves = kstObjectSubList<KstBaseCurve,KstVCurve>((*it)->Curves);
      for (i = vcurves.begin(); i != vcurves.end(); ++i) {
        if (_lineColorOrder > -1) {
          (*i)->setColor(KstColorSequence::entry(_lineColorSeq.current()));
        }
        if (_pointStyleOrder > -1) {
          (*i)->setPointStyle(_pointStyleSeq.current());
          (*i)->setHasPoints(true);
          (*i)->setPointDensity(_pointDensity);
        }
        if (_lineStyleOrder > -1) {
          (*i)->setLineStyle(_lineStyleSeq.current());
        }
        if (_lineWidthOrder > -1) {
          (*i)->setLineWidth(_lineWidthSeq.current());
        }

        _seqVect[0].next();
      }
    }
  }
}


void KstCurveDifferentiate::apply() {
  KstApp *app = KstApp::inst();
  KstViewWindow *window;
  int maxSequences = 0;
  
  getOptions();
  saveProperties();
  
  _seqVect.clear();
  _seqVect.resize(4);
  
  if (_lineColorOrder > -1) {
    _lineColorSeq.setRange(0, KstColorSequence::count());
    _seqVect[_lineColorOrder] = _lineColorSeq;
    maxSequences++;
  }
  if (_pointStyleOrder > -1) {
    _pointStyleSeq.setRange(0, KSTPOINT_MAXTYPE - 1);
    _seqVect[_pointStyleOrder] = _pointStyleSeq;
    maxSequences++;
  }
  if (_lineStyleOrder > -1) {
    _lineStyleSeq.setRange(0, KSTLINESTYLE_MAXTYPE - 1);
    _seqVect[_lineStyleOrder] = _lineStyleSeq;
    maxSequences++;
  }
  if (_lineWidthOrder > -1) {
    _lineWidthSeq.setRange(1, _maxLineWidth);
    _seqVect[_lineWidthOrder] = _lineWidthSeq;
    maxSequences++;
  }
  
  if (maxSequences > 0) {
    int i;
    
    _seqVect.resize(maxSequences);
    for (i = 0; i < maxSequences-1; i++) {
      _seqVect[i].hookToNextSequence(&(_seqVect[i+1]));
    }
    _seqVect[maxSequences-1].hookToNextSequence(0L);
        
    if (_applyTo == 0) {
      window = dynamic_cast<KstViewWindow*>(app->activeSubWindow());
      
      if (window) {
        cycleWindow(window);
      }
    } else {
      QList<QMdiSubWindow*> windows;
      QList<QMdiSubWindow*>::const_iterator i;
    
      windows = app->subWindowList( QMdiArea::CreationOrder );
    
      for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
        KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);

        if (_repeatAcross == 1) {
          _seqVect[0].reset();
        }

        if (viewWindow) {
          if (!viewWindow->view()->children().isEmpty()) {
            cycleWindow(viewWindow);
          }
        }
      }
    }
  }

  close();
}

#include "kstcurvedifferentiate.moc"

