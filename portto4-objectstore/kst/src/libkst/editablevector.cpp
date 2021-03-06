/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *   copyright : (C) 2005  University of British Columbia                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// use KCodecs::base64Encode() in kmdcodecs.h
// Create QDataStream into a QByteArray
// qCompress the bytearray
#include <QTextStream>

#include "editablevector.h"
#include "debug.h"
#include "kst_i18n.h"

namespace Kst {

const QString EditableVector::staticTypeString = I18N_NOOP("Editable Vector");

EditableVector::EditableVector(ObjectStore *store, const ObjectTag& tag, const QByteArray &data)
    : Vector(store, tag, data) {
  _editable = true;
  _saveable = true;
  _saveData = true;
}


EditableVector::EditableVector(ObjectStore *store, const ObjectTag& tag, int n)
    : Vector(store, tag, n) {
  _editable = true;
  _saveable = true;
  _saveData = true;
}


const QString& EditableVector::typeString() const {
  return staticTypeString;
}


Object::UpdateType EditableVector::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();

  if (Object::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  Object::UpdateType baseRC = Vector::update(update_counter);
  if (force) {
    baseRC = UPDATE;
  }

  return baseRC;
}


void EditableVector::setSaveData(bool save) {
  Q_UNUSED(save)
}

}
// vim: ts=2 sw=2 et
