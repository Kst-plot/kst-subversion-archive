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
#include "dialoglauncher.h"
#include "kstdatacollection.h"

KstBasicPlugin::KstBasicPlugin()
: KstDataObject() {
}


KstBasicPlugin::KstBasicPlugin(const QDomElement& e)
: KstDataObject(e) {
}


KstBasicPlugin::~KstBasicPlugin() {
}


KstDataObjectPtr KstBasicPlugin::makeDuplicate(KstDataObjectDataObjectMap &map) {
  KstBasicPluginPtr plugin = kst_cast<KstBasicPlugin>(KstDataObject::createPlugin(propertyString()));

  // use same inputs
  for (KstVectorMap::ConstIterator iter = _inputVectors.begin(); iter != _inputVectors.end(); ++iter) {
    plugin->inputVectors().insert(iter.key(), iter.data());
  }
  for (KstScalarMap::ConstIterator iter = _inputScalars.begin(); iter != _inputScalars.end(); ++iter) {
    plugin->inputScalars().insert(iter.key(), iter.data());
  }
  for (KstStringMap::ConstIterator iter = _inputStrings.begin(); iter != _inputStrings.end(); ++iter) {
    plugin->inputStrings().insert(iter.key(), iter.data());
  }

  // create new outputs
  for (KstVectorMap::ConstIterator iter = outputVectors().begin(); iter != outputVectors().end(); ++iter) {
    KstVectorPtr v = new KstVector(iter.data()->tagName() + "'", 0, plugin.data());
    plugin->outputVectors().insert(iter.key(), v);
    KST::addVectorToList(v);
  }
  for (KstScalarMap::ConstIterator iter = outputScalars().begin(); iter != outputScalars().end(); ++iter) {
    KstScalarPtr s = new KstScalar(iter.data()->tagName() + "'", plugin.data());
    plugin->outputScalars().insert(iter.key(), s);
  }
  for (KstStringMap::ConstIterator iter = outputStrings().begin(); iter != outputStrings().end(); ++iter) {
    KstStringPtr s = new KstString(iter.data()->tagName() + "'", plugin.data());
    plugin->outputStrings().insert(iter.key(), s);
  }

  // set the same plugin
  plugin->setTagName(tagName() + "'");
  map.insert(this, KstDataObjectPtr(plugin));
  return KstDataObjectPtr(plugin);
}

void KstBasicPlugin::showNewDialog() {
  //FIXME shouldn't tagName() == propertyString() ??
  KstDialogs::self()->showBasicPluginDialog(propertyString());
}


void KstBasicPlugin::showEditDialog() {
  KstDialogs::self()->showBasicPluginDialog(tagName(), true);
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
  KstVectorMap::ConstIterator i = _outputVectors.find(vector);
  if (i != _outputVectors.end())
    return *i;
  else
    return 0;
}


KstScalarPtr KstBasicPlugin::outputScalar(const QString& scalar) const {
  KstScalarMap::ConstIterator i = _outputScalars.find(scalar);
  if (i != _outputScalars.end())
    return *i;
  else
    return 0;
}


KstStringPtr KstBasicPlugin::outputString(const QString& string) const {
  KstStringMap::ConstIterator i = _outputStrings.find(string);
  if (i != _outputStrings.end())
    return *i;
  else
    return 0;
}


void KstBasicPlugin::setInputVector(const QString &type, KstVectorPtr ptr) {
  if (ptr) {
    _inputVectors[type] = ptr;
  } else {
    _inputVectors.remove(type);
  }
  setDirty();
}


void KstBasicPlugin::setInputScalar(const QString &type, KstScalarPtr ptr) {
  if (ptr) {
    _inputScalars[type] = ptr;
  } else {
    _inputScalars.remove(type);
  }
  setDirty();
}


void KstBasicPlugin::setInputString(const QString &type, KstStringPtr ptr) {
  if (ptr) {
    _inputStrings[type] = ptr;
  } else {
    _inputStrings.remove(type);
  }
  setDirty();
}


void KstBasicPlugin::setOutputVector(const QString &type, const QString &name) {
  KstVectorPtr v = new KstVector(name, 0, this, false);
  _outputVectors.insert(type, v);
  KST::addVectorToList(v);
}


void KstBasicPlugin::setOutputScalar(const QString &type, const QString &name) {
  KstScalarPtr s = new KstScalar(name, this);
  _outputScalars.insert(type, s);
}


void KstBasicPlugin::setOutputString(const QString &type, const QString &name) {
  KstStringPtr s = new KstString(name, this);
  _outputStrings.insert(type, s);
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
  bool depUpdated = updateInput(updateCounter, force);

  //Call the plugins algorithm to operate on the inputs
  //and produce the outputs
  if ( !algorithm() ) {
    KstDebug::self()->log(i18n("There is an error in the %1 algorithm.").arg(propertyString()), KstDebug::Error);
    return lastUpdateResult();
  }

  //Perform update on the outputs
  updateOutput(updateCounter);

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
  QStringList iv = inputVectorList();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI) {
    if (!inputVector(*ivI))
      return false;
  }

  //Now, check the inputScalars...
  QStringList is = inputScalarList();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI) {
    if (!inputScalar(*isI))
      return false;
  }

  //Finally, check the inputStrings...
  QStringList istr = inputStringList();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI) {
    if (!inputString(*istrI))
      return false;
  }
  return true;
}


bool KstBasicPlugin::updateInput(int updateCounter, bool force) const {
  bool depUpdated = force;

  //First, update the inputVectors...
  QStringList iv = inputVectorList();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI) {
    depUpdated =
        UPDATE == inputVector(*ivI)->update(updateCounter) || depUpdated;
  }

  //Now, update the inputScalars...
  QStringList is = inputScalarList();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI) {
    depUpdated =
        UPDATE == inputScalar(*isI)->update(updateCounter) || depUpdated;
  }

  //Finally, update the inputStrings...
  QStringList istr = inputStringList();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI) {
    depUpdated =
        UPDATE == inputString(*istrI)->update(updateCounter) || depUpdated;
  }
  return depUpdated;
}


void KstBasicPlugin::updateOutput(int updateCounter) const {
  //output vectors...
  QStringList ov = outputVectorList();
  QStringList::ConstIterator ovI = ov.begin();
  for (; ovI != ov.end(); ++ovI) {
    if (KstVectorPtr o = outputVector(*ovI)) {
      vectorRealloced(o, o->value(), o->length());
      o->setDirty();
      o->setNewAndShift(o->length(), o->numShift());
      o->update(updateCounter);
    }
  }

  //output scalars...
  QStringList os = outputScalarList();
  QStringList::ConstIterator osI = os.begin();
  for (; osI != os.end(); ++osI) {
    if (KstScalarPtr o = outputScalar(*osI)) {
      o->update(updateCounter);
    }
  }

  //ouput strings...
  QStringList ostr = outputStringList();
  QStringList::ConstIterator ostrI = ostr.begin();
  for (; ostrI != ostr.end(); ++ostrI) {
    if (KstStringPtr o = outputString(*ostrI)) {
      o->update(updateCounter);
    }
  }
}

// vim: ts=2 sw=2 et
