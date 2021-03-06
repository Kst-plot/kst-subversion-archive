/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "datasourceselector.h"

#include "datasourceselectordialog.h"

#include <QStyle>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDirModel>
#include <QCompleter>
#include <QPointer>

#include <QDebug>

namespace Kst {

DataSourceSelector::DataSourceSelector(QWidget *parent)
  : QWidget(parent), _mode(QFileDialog::ExistingFile) {
  setup();
}


DataSourceSelector::~DataSourceSelector() {
}


void DataSourceSelector::setup() {

  _fileEdit = new QLineEdit(this);
  _fileButton = new QToolButton(this);

  QHBoxLayout * layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(_fileEdit);
  layout->addWidget(_fileButton);
  setLayout(layout);

  int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
  _fileButton->setIcon(QPixmap(":kst_changefile.png"));
  _fileButton->setFixedSize(size + 8, size + 8);

  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  connect (_fileEdit, SIGNAL(textChanged(const QString &)), this, SIGNAL(changed(const QString &)));
  connect (_fileButton, SIGNAL(clicked()), this, SLOT(chooseFile()));

  QDirModel *dirModel = new QDirModel(this);
  dirModel->setFilter(QDir::AllEntries);

  QCompleter *completer = new QCompleter(this);
  completer->setModel(dirModel); 

  _fileEdit->setCompleter(completer);
}


QString DataSourceSelector::file() const {
  return _file;
}


void DataSourceSelector::setFile(const QString &file) {
  _file = file;
  //FIXME grrr QLineEdit doc *lies* to me... the textEdited signal is being triggered!!
  _fileEdit->blockSignals(true);
  _fileEdit->setText(_file);
  _fileEdit->blockSignals(false);
  emit changed(file);
}


void DataSourceSelector::chooseFile() {
  QString file;
  QPointer<DataSourceSelectorDialog> dialog = new DataSourceSelectorDialog( _file, this );

  if (dialog->exec() == QDialog::Accepted) {
    file = dialog->selectedDataSource();

    if (!file.isEmpty()) {
      setFile(file);
    }
  }

  delete dialog;
}

}

// vim: ts=2 sw=2 et
