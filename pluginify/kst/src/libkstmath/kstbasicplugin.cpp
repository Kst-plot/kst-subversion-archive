/***************************************************************************
                                kstbasicplugin.cpp
                             -------------------
    begin                : 09/15/06
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

#include <stdlib.h>
#include <unistd.h>

// include files for Qt
#include <qstylesheet.h>

// include files for KDE
#include <klocale.h>

// application specific includes
#include "kstdebug.h"
#include "kstbasicplugin.h"
#include "kstdatacollection.h"

KstBasicPlugin::KstBasicPlugin()
: KstDataObject() {
}


KstBasicPlugin::KstBasicPlugin(const QDomElement& e)
: KstDataObject(e) {
}


KstBasicPlugin::~KstBasicPlugin() {
}


KstVectorPtr KstBasicPlugin::inputVector(const QString& vector) const {
  KstVectorMap::ConstIterator i = _inputVectors.find(vector);
  if (i != _inputVectors.end())
    return *i;
  else
    return 0;
}


KstScalarPtr KstBasicPlugin::inputScalar(const QString& scalar) const {
  KstScalarMap::ConstIterator i = _inputScalars.find(scalar);
  if (i != _inputScalars.end())
    return *i;
  else
    return 0;
}


KstStringPtr KstBasicPlugin::inputString(const QString& string) const {
  KstStringMap::ConstIterator i = _inputStrings.find(string);
  if (i != _inputStrings.end())
    return *i;
  else
    return 0;
}


KstVectorPtr KstBasicPlugin::outputVector(const QString& vector) const {
  KstVectorMap::ConstIterator i = _inputVectors.find(vector);
  if (i != _inputVectors.end())
    return *i;
  else
    return 0;
}


KstScalarPtr KstBasicPlugin::outputScalar(const QString& scalar) const {
  KstScalarMap::ConstIterator i = _inputScalars.find(scalar);
  if (i != _inputScalars.end())
    return *i;
  else
    return 0;
}


KstStringPtr KstBasicPlugin::outputString(const QString& string) const {
  KstStringMap::ConstIterator i = _inputStrings.find(string);
  if (i != _inputStrings.end())
    return *i;
  else
    return 0;
}


KstObject::UpdateType KstBasicPlugin::update(int updateCounter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  //Make sure we have all the necessary inputs
  if (!inputsExist())
    return setLastUpdateResult(NO_CHANGE);

  //Update the dependent inputs
  bool depUpdated = updateDependentInput(updateCounter, force);

  //Call the plugins algorithm to operate on the inputs
  //to produce the outputs
  algorithm();

  //Perform any necessary operations on the outputs

  return setLastUpdateResult(depUpdated ? UPDATE : NO_CHANGE);
}


void KstBasicPlugin::load(const QDomElement &e) {
  //TODO
  Q_UNUSED(e)
}


void KstBasicPlugin::save(QTextStream& ts, const QString& indent) {
  //TODO
  Q_UNUSED(ts)
  Q_UNUSED(indent)
}


//TODO Could use some templates perhaps...
bool KstBasicPlugin::inputsExist() const {
  //First, check the inputVectors...
  QStringList iv = inputVectors();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI)
  {
    if (!inputVector(*ivI))
      return false;
  }

  //Now, check the inputScalars...
  QStringList is = inputScalars();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI)
  {
    if (!inputScalar(*isI))
      return false;
  }

  //Finally, check the inputStrings...
  QStringList istr = inputStrings();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI)
  {
    if (!inputString(*istrI))
      return false;
  }
  return true;
}


bool KstBasicPlugin::updateDependentInput(int updateCounter, bool force) const {
  bool depUpdated = force;

  //First, update the inputVectors...
  QStringList iv = inputVectors();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI)
  {
    depUpdated =
        UPDATE == inputVector(*ivI)->update(updateCounter) || depUpdated;
  }

  //Now, update the inputScalars...
  QStringList is = inputScalars();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI)
  {
    depUpdated =
        UPDATE == inputScalar(*isI)->update(updateCounter) || depUpdated;
  }

  //Finally, update the inputStrings...
  QStringList istr = inputStrings();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI)
  {
    depUpdated =
        UPDATE == inputString(*istrI)->update(updateCounter) || depUpdated;
  }
  return depUpdated;
}

// vim: ts=2 sw=2 et
