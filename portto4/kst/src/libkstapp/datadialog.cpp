/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "datadialog.h"

#include "datatab.h"
#include "dialogpage.h"

#include "editmultiplewidget.h"

#include "dataobject.h"

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

namespace Kst {

DataDialog::DataDialog(KstObjectPtr dataObject, QWidget *parent)
  : Dialog(parent), _defaultTag("<Auto Name>"), _dataObject(dataObject) {

  if (_dataObject)
    _mode = Edit;
  else
    _mode = New;

  createGui();
}


DataDialog::~DataDialog() {
}


void DataDialog::createGui() {

  buttonBox()->button(QDialogButtonBox::Apply)->setVisible(false);

  connect(this, SIGNAL(ok()), this, SLOT(slotOk()));

  QWidget *extension = extensionWidget();

  QVBoxLayout *extensionLayout = new QVBoxLayout(extension);
  extensionLayout->setContentsMargins(0, -1, 0, -1);

  EditMultipleWidget *editMultipleWidget = new EditMultipleWidget();
  extensionLayout->addWidget(editMultipleWidget);

  extension->setLayout(extensionLayout);

  QWidget *box = topCustomWidget();

  QHBoxLayout *layout = new QHBoxLayout(box);
  layout->setContentsMargins(0, -1, 0, -1);

  QLabel *label = new QLabel(tr("Unique Name:"), box);
  _tagName = new QLineEdit(box);

  QPushButton *button = new QPushButton(tr("Edit Multiple >>"));
  connect(button, SIGNAL(clicked()), this, SLOT(slotEditMultiple()));

  if (_dataObject) {
    setTagName(_dataObject->tagName());
  } else {
    setTagName(tagName());
    button->setVisible(false);
  }

  layout->addWidget(label);
  layout->addWidget(_tagName);
  layout->addWidget(button);

  box->setLayout(layout);
}


QString DataDialog::tagName() const {
  const QString tag = _tagName->text();
  return tag.isEmpty() ? _defaultTag : tag;
}


void DataDialog::setTagName(const QString &tagName) {
  _tagName->setText(tagName);
}


void DataDialog::addDataTab(DataTab *tab) {
  DialogPage *page = new DialogPage(this);
  page->setPageTitle(tab->tabTitle());
  page->addDialogTab(tab);
  addDialogPage(page);
}

void DataDialog::slotOk() {
  KstObjectPtr ptr;
  if (!dataObject())
    ptr = createNewDataObject();
  else
    ptr = editExistingDataObject();
  setDataObject(ptr);
}


void DataDialog::slotEditMultiple() {
  extensionWidget()->setVisible(!extensionWidget()->isVisible());
}


}

// vim: ts=2 sw=2 et
