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
  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "iscalar") {
        _inputScalarLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "istring") {
        _inputStringLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "ovector") {
        KstVectorPtr v;
        if (e.attribute("scalarList", "0").toInt()) {
          v = new KstVector(e.text(), 0, this, true);
        } else {
          v = new KstVector(e.text(), 0, this, false);
        }
        _outputVectors.insert(e.attribute("name"), v);
        KST::addVectorToList(v);
      } else if (e.tagName() == "oscalar") {
        KstScalarPtr sp = new KstScalar(e.text(), this);
        _outputScalars.insert(e.attribute("name"), sp);
      } else if (e.tagName() == "ostring") {
        KstStringPtr sp = new KstString(e.text(), this);
        _outputStrings.insert(e.attribute("name"), sp);
      }
    }
    n = n.nextSibling();
  }
}


void KstBasicPlugin::save(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  //The plugin name _must_ be the same as the entry in the .desktop file
  ts << indent << "<plugin name=\"" << propertyString() << "\">" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstVectorMap::Iterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    ts << l2 << "<ivector name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ivector>" << endl;
  }
  for (KstScalarMap::Iterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    ts << l2 << "<iscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</iscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    ts << l2 << "<istring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</istring>" << endl;
  }
  for (KstVectorMap::Iterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    ts << l2 << "<ovector name=\"" << QStyleSheet::escape(i.key());
    if (i.data()->isScalarList()) {
      ts << "\" scalarList=\"1";
    }
    ts << "\">" << QStyleSheet::escape(i.data()->tagName())
        << "</ovector>" << endl;
  }
  for (KstScalarMap::Iterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    ts << l2 << "<oscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</oscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    ts << l2 << "<ostring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ostring>" << endl;
  }
  ts << indent << "</plugin>" << endl;
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
