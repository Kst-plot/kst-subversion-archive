/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "curveplacementwidget.h"

CurvePlacementWidget::CurvePlacementWidget(QWidget *parent) : QWidget(parent) 
{
  setupUi(this);
}

CurvePlacementWidget::~CurvePlacementWidget() 
{
}

bool CurvePlacementWidget::existingPlot()
{
  return _inPlot->isChecked();
}

bool CurvePlacementWidget::newPlot()
{
  return _newPlot->isChecked();
}

void CurvePlacementWidget::setExistingPlot( bool existingPlot )
{
  _inPlot->setChecked(existingPlot);
}

void CurvePlacementWidget::setNewPlot( bool newPlot )
{
  _newPlot->setChecked(newPlot);
}

QString CurvePlacementWidget::plotName()
{
  return _plotList->currentText();
}

int CurvePlacementWidget::columns()
{
  return _plotColumns->value();
}

void CurvePlacementWidget::setCols( int c )
{
  _plotColumns->setValue(c);
}

void CurvePlacementWidget::setCurrentPlot( const QString &plotName )
{
  int index;

  index = _plotList->findText(plotName, Qt::MatchExactly | Qt::MatchCaseSensitive);
  if (index != -1) {
    _plotList->setCurrentIndex(index);
  }
}

void CurvePlacementWidget::newWindow()
{
  KstData::self()->newWindow(this);
  update();
}

void CurvePlacementWidget::update()
{
  QStringList::ConstIterator i;
  QStringList windows;
  QString plotName;
  int index;

  _plotWindow->clear();
  windows = KstData::self()->windowList();
  for (i = windows.begin(); i != windows.end(); ++i) {
    _plotWindow->insertItem(0, *i);
  }

  plotName = KstData::self()->currentWindow();
  if (!plotName.isEmpty()) {
    index = _plotList->findText(plotName, Qt::MatchExactly | Qt::MatchCaseSensitive);
    if (index != -1) {
      _plotList->setCurrentIndex(index);
    }
  }

  updatePlotList();
  updateEnabled();
  updateGrid();
}

void CurvePlacementWidget::updatePlotList()
{
  QStringList::ConstIterator i;
  QStringList plots;
  QString old;
  int index;

  if (_plotList->count()) {
    old = _plotList->currentText();
  }

  plots = KstData::self()->plotList(_plotWindow->currentText());

  _plotList->clear();
  for (i = plots.begin(); i != plots.end(); ++i) {
    _plotList->insertItem(0, *i);
  }

  index = _plotList->findText(old, Qt::MatchExactly | Qt::MatchCaseSensitive);
  if (index != -1) {
    _plotList->setCurrentIndex(index);
  }
}

void CurvePlacementWidget::updateEnabled()
{
  _plotWindow->setEnabled(_plotWindow->count() > 0);

  _inPlot->setEnabled(_plotList->count() > 0 );

  _plotList->setEnabled(_inPlot->isChecked());
  _reGrid->setEnabled(_newPlot->isChecked());
  _plotColumns->setEnabled(_newPlot->isChecked() && _reGrid->isChecked());
}

void CurvePlacementWidget::updateGrid()
{
  int cols = KstData::self()->columns(_plotWindow->currentText());

  _reGrid->setChecked(cols > -1);
  if (cols > -1) {
    _plotColumns->setValue(cols);
  }
}

bool CurvePlacementWidget::reGrid()
{
  return _reGrid->isChecked();
}

