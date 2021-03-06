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

#include "basicplugindialog.h"

#include "dialogpage.h"

#include "datacollection.h"
#include "dataobjectcollection.h"

namespace Kst {

BasicPluginTab::BasicPluginTab(QWidget *parent)
  : DataTab(parent) {

  setupUi(this);
  setTabTitle(tr("Basic Plugin"));

}


BasicPluginTab::~BasicPluginTab() {
}


BasicPluginDialog::BasicPluginDialog(ObjectPtr dataObject, QWidget *parent)
  : DataDialog(dataObject, parent) {

  if (editMode() == Edit)
    setWindowTitle(tr("Edit Basic Plugin"));
  else
    setWindowTitle(tr("New Basic Plugin"));

  _basicPluginTab = new BasicPluginTab(this);
  addDataTab(_basicPluginTab);

  //FIXME need to do validation to enable/disable ok button...
}


BasicPluginDialog::~BasicPluginDialog() {
}


QString BasicPluginDialog::tagString() const {
  return DataDialog::tagString();
}


ObjectPtr BasicPluginDialog::createNewDataObject() const {
  qDebug() << "createNewDataObject" << endl;
  return 0;
}


ObjectPtr BasicPluginDialog::editExistingDataObject() const {
  qDebug() << "editExistingDataObject" << endl;
  return 0;
}

}

// vim: ts=2 sw=2 et
