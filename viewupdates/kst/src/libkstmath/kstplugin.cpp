/***************************************************************************
                                kstplugin.cpp
                             -------------------
    begin                : May 15 2003
    copyright            : (C) 2003 The University of Toronto
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
#include "ksdebug.h"
#include <klocale.h>

// application specific includes
#include "dialoglauncher.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstplugin.h"
#include "plugincollection.h"


KstPlugin::KstPlugin()
: KstDataObject() {
  _inStringCnt = 0;
  _outStringCnt = 0;
  commonConstructor();
}


KstPlugin::KstPlugin(const QDomElement& e)
: KstDataObject(e) {
  QString pluginName;

  _inStringCnt = 0;
  _outStringCnt = 0;
  commonConstructor();

  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "name") {
        pluginName = e.text();
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "iscalar") {
        _inputScalarLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "istring") {
        _inputStringLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "ovector") {
        KstVectorPtr v;
        if (e.attribute("scalarList", "0").toInt()) {
          v = new KstVector(e.text(), 0, true);
        } else {
          v = new KstVector(e.text(), 0, false);
        }
        v->setProvider(this);
        _outputVectors.insert(e.attribute("name"), v);
        KST::addVectorToList(v);
      } else if (e.tagName() == "oscalar") {
        KstScalarPtr sp = new KstScalar(e.text());
        sp->writeLock();
        sp->setProvider(this);
        sp->writeUnlock();
        _outputScalars.insert(e.attribute("name"), sp);
      } else if (e.tagName() == "ostring") {
        KstStringPtr sp = new KstString(e.text());
        sp->writeLock();
        sp->setProvider(this);
        sp->writeUnlock();
        _outputStrings.insert(e.attribute("name"), sp);
      }
    }
    n = n.nextSibling();
  }

  _plugin = PluginCollection::self()->plugin(pluginName);

  if (!_plugin.data()) {
    KstDebug::self()->log(i18n("Unable to load plugin %1 for \"%2\".").arg(pluginName).arg(tagName()), KstDebug::Warning);
  } else {
    Plugin::countScalarsVectorsAndStrings(_plugin->data()._inputs, _inScalarCnt, _inArrayCnt, _inStringCnt, _inPid);

    const QValueList<Plugin::Data::IOValue>& otable = _plugin->data()._outputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                           it != otable.end();
                                                                         ++it) {
      // FIXME: i18n?
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        _outArrayCnt++;
        if (!_outputVectors.contains((*it)._name)) {
          KstVectorPtr v;

          if ((*it)._subType == Plugin::Data::IOValue::FloatNonVectorSubType) {
            v = new KstVector(tagName() + " vector - " + (*it)._name, 0, true);
          } else {
            v = new KstVector(tagName() + " vector - " + (*it)._name, 0, false);
          }
          v->setProvider(this);
          _outputVectors.insert((*it)._name, v);
          KST::addVectorToList(v);
        }
      } else if ((*it)._type == Plugin::Data::IOValue::MatrixType) {
          abort(); // FIXME:
#if 0
        _outArrayCnt += 2;
        if (!_outputMatrices.contains((*it)._name)) {
          KstMatrixPtr m;

          if ((*it)._subType == Plugin::Data::IOValue::FloatNonVectorSubType) {
            m = new KstMatrix(tagName() + " matrix - " + (*it)._name, 0, true);
          } else {
            m = new KstMatrix(tagName() + " matrix - " + (*it)._name, 0, false);
          }
          m->setProvider(this);
          _outputMatrices.insert((*it)._name, m);
          KST::addVectorToList(v);
        }
#endif
      } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
        _outScalarCnt++;
        if (!_outputScalars.contains((*it)._name)) {
          KstScalarPtr s = new KstScalar(tagName() + " scalar - " + (*it)._name);
          s->writeLock();
          s->setProvider(this);
          s->writeUnlock();
          _outputScalars.insert((*it)._name, s);
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        _outStringCnt++;
        if (!_outputStrings.contains((*it)._name)) {
          KstStringPtr s = new KstString(tagName() + " string - " + (*it)._name);
          s->writeLock();
          s->setProvider(this);
          s->writeUnlock();
          _outputStrings.insert((*it)._name, s);
        }
      }
    }
    allocateParameters();
  }
}


void KstPlugin::commonConstructor() {
  _inArrayLens = 0L;
  _outArrayLens = 0L;
  _inScalars = 0L;
  _outScalars = 0L;
  _inVectors = 0L;
  _outVectors = 0L;
  _inStrings = 0L;
  _outStrings = 0L;

  _inScalarCnt = 0;
  _outScalarCnt = 0;
  _inArrayCnt = 0;
  _outArrayCnt = 0;
  _typeString = i18n("Plugin");
  _type = "Plugin";
  _plugin = 0L;
  _localData = 0L;
  //kstdDebug() << "Creating KSTPlugin: " << long(this) << endl;
}


KstPlugin::~KstPlugin() {
  freeParameters();
  if (_localData) {
    if (!_plugin || !_plugin->freeLocalData(&_localData)) {
      free(_localData);
    }
    _localData = 0L;
  }
  //kstdDebug() << "Destroying KSTPlugin: " << long(this) << endl;
}



void KstPlugin::allocateParameters() {
  if (_inArrayCnt > 0) {
    _inArrayLens = new int[_inArrayCnt];
    _inVectors = new double*[_inArrayCnt];
  }

  if (_outArrayCnt > 0) {
    _outArrayLens = new int[_outArrayCnt];
    _outVectors = new double*[_outArrayCnt];
  }

  if (_inScalarCnt > 0) {
    _inScalars = new double[_inScalarCnt];
  }

  if (_outScalarCnt > 0) {
    _outScalars = new double[_outScalarCnt];
  }

  if (_inStringCnt > 0) {
    _inStrings = new char*[_inStringCnt];
    memset(_inStrings, 0, sizeof(char*)*_inStringCnt);
  }

  if (_outStringCnt > 0) {
    _outStrings = new char*[_outStringCnt];
    memset(_outStrings, 0, sizeof(char*)*_outStringCnt);
  }
}


void KstPlugin::freeParameters() {
  delete[] _inVectors;
  _inVectors = 0L;
  delete[] _outVectors;
  _outVectors = 0L;
  delete[] _outArrayLens;
  _outArrayLens = 0L;
  delete[] _inArrayLens;
  _inArrayLens = 0L;
  delete[] _outScalars;
  _outScalars = 0L;
  delete[] _inScalars;
  _inScalars = 0L;
  if (_outStrings) {
    for (unsigned i = 0; i < _outStringCnt; ++i) {
      if (_outStrings[i]) {
        free(_outStrings[i]);
        _outStrings[i] = 0L;
      }
    }
  }
  delete[] _outStrings;
  _outStrings = 0L;
  if (_inStrings) {
    for (unsigned i = 0; i < _inStringCnt; ++i) {
      if (_inStrings[i]) {
        free(_inStrings[i]);
        _inStrings[i] = 0L;
      }
    }
  }
  delete[] _inStrings;
  _inStrings = 0L;
}


KstObject::UpdateType KstPlugin::update(int update_counter) {
  if (!isValid()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

#define CLEANUP() do {\
  for (unsigned i = 0; i < _outStringCnt; ++i) { \
    if (_outStrings[i]) { \
      free(_outStrings[i]); \
      _outStrings[i] = 0L; \
    } \
  } \
  for (unsigned i = 0; i < _inStringCnt; ++i) { \
    if (_inStrings[i]) { \
      free(_inStrings[i]); \
      _inStrings[i] = 0L; \
    } \
  } \
  } while(0)


  const QValueList<Plugin::Data::IOValue>& itable = _plugin->data()._inputs;
  const QValueList<Plugin::Data::IOValue>& otable = _plugin->data()._outputs;
  int itcnt = 0, vitcnt = 0, sitcnt = 0;
  bool doUpdate = force;

  // Populate the input scalars and vectors
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin();
                                                         it != itable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!_inputVectors.contains((*it)._name)) {
        KstDebug::self()->log(i18n("Input vector [%1] for plugin %2 not found.  Unable to continue.").arg((*it)._name).arg(tagName()), KstDebug::Error);
        CLEANUP();
        return setLastUpdateResult(NO_CHANGE);
      }
      KstVectorPtr iv = _inputVectors[(*it)._name];
      doUpdate = (UPDATE == iv->update(update_counter)) || doUpdate;
      _inVectors[vitcnt] = iv->value();
      _inArrayLens[vitcnt++] = iv->length();
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      KstScalarPtr is = _inputScalars[(*it)._name];
      doUpdate = (UPDATE == is->update(update_counter)) || doUpdate;
      _inScalars[itcnt++] = is->value();
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      KstStringPtr is = _inputStrings[(*it)._name];
      doUpdate = (UPDATE == is->update(update_counter)) || doUpdate;
      // Maybe we should use UTF-8 instead?
      _inStrings[sitcnt++] = strdup(is->value().latin1());
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      _inScalars[itcnt++] = getpid();
    }
  }

  if (!doUpdate) {
    CLEANUP();
    return setLastUpdateResult(NO_CHANGE);
  }

  vitcnt = 0;
  // Populate the output vectors
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                         it != otable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!_outputVectors.contains((*it)._name)) {
        KstDebug::self()->log(i18n("Output vector [%1] for plugin %2 not found.  Unable to continue.").arg((*it)._name).arg(tagName()), KstDebug::Error);
        CLEANUP();
        return setLastUpdateResult(NO_CHANGE);
      }
      _outVectors[vitcnt] = _outputVectors[(*it)._name]->value();
      _outArrayLens[vitcnt++] = _outputVectors[(*it)._name]->length();
    }
  }

  if (_outStringCnt > 0) {
    memset(_outStrings, 0, _outStringCnt*sizeof(char *));
  }

  int rc;
  if (_inStringCnt > 0 || _outStringCnt > 0) {
    if (_plugin->data()._localdata) {
      rc = _plugin->call(_inVectors, _inArrayLens, _inScalars,
          _outVectors, _outArrayLens, _outScalars,
          const_cast<const char**>(_inStrings), _outStrings, &_localData);
    } else {
      rc = _plugin->call(_inVectors, _inArrayLens, _inScalars,
          _outVectors, _outArrayLens, _outScalars,
          const_cast<const char**>(_inStrings), _outStrings);
    }
  } else {
    if (_plugin->data()._localdata) {
      rc = _plugin->call(_inVectors, _inArrayLens, _inScalars,
          _outVectors, _outArrayLens, _outScalars, &_localData);
    } else {
      rc = _plugin->call(_inVectors, _inArrayLens, _inScalars,
          _outVectors, _outArrayLens, _outScalars);
    }
  }

  if (rc == 0) {
    itcnt = 0;
    vitcnt = 0;
    sitcnt = 0;
    setLastUpdateResult(UPDATE); // make sure that provider callbacks work
    // Read back the output vectors and scalars
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
        it != otable.end();
        ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        KstVectorPtr vp = _outputVectors[(*it)._name];
        vectorRealloced(vp, _outVectors[vitcnt], _outArrayLens[vitcnt]);
        vp->setDirty();
        // Inefficient, but do we have any other choice?  We don't really know
        // from the plugin how much of this vector is "new" or "shifted"
        vp->setNewAndShift(vp->length(), vp->numShift());
        vp->update(update_counter);
        vitcnt++;
      } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
        KstScalarPtr sp = _outputScalars[(*it)._name];
        sp->setValue(_outScalars[itcnt++]);
        sp->update(update_counter);
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        KstStringPtr sp = _outputStrings[(*it)._name];
        sp->setValue(_outStrings[sitcnt++]);
        sp->update(update_counter);
      }
    }

    // if we have a fit plugin then create the necessary scalars from the parameter vector
    createFitScalars();
    _lastError = QString::null;
  } else if (rc > 0) {
    if (_lastError.isEmpty()) {
      const char *err = _plugin->errorCode(rc);
      if (err && *err) {
        _lastError = err;
        KstDebug::self()->log(i18n("Plugin %1 produced error: %2.").arg(tagName()).arg(_lastError), KstDebug::Error);
      } else {
        _lastError = QString::null;
      }
    }
  } else {
    bool doSend = _lastError.isEmpty() ? true : false;

    switch (rc) {
      case -1:
        _lastError = i18n("Generic Error");
        break;
      case -2:
        _lastError = i18n("Input Error");
        break;
      case -3:
        _lastError = i18n("Memory Error");
        break;
      default:
        _lastError = i18n("Unknown Error");
        break;
    }

    if (doSend) {
      KstDebug::self()->log(i18n("Plugin %2 produced error: %1.").arg(_lastError).arg(tagName()), KstDebug::Error);
    }
  }

  CLEANUP();
#undef CLEANUP
  return setLastUpdateResult(UPDATE);
}

QString KstPlugin::label(int precision) const {
  QString label;
  
  label = i18n("%1: %2").arg(plugin()->data()._readableName).arg(tagName());
  if ((outputVectors())["Parameters"]) {
    QString strParamName;
    QString strValue;
    int length = (outputVectors())["Parameters"]->length();
    int i = 0;
    
    for (strParamName = plugin()->parameterName(0); 
        !strParamName.isEmpty() && i < length; 
        strParamName = plugin()->parameterName(++i)) {
      KstScalarPtr scalar = outputScalars()[strParamName];
      if (scalar) {
        strValue = QString::number(scalar->value(), 'g', precision);
        label += i18n("\n%1: %2").arg(strParamName).arg(strValue);
      }
    }
  }

  return label;
}


void KstPlugin::save(QTextStream &ts, const QString& indent) {
  if (!_plugin) {
    return;
  }

  QString l2 = indent + "  ";
  ts << indent << "<plugin>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<name>" << QStyleSheet::escape(_plugin->data()._name) << "</name>" << endl;
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


bool KstPlugin::slaveVectorsUsed() const {
  return true;
}


bool KstPlugin::isValid() const {
  return _inputVectors.count() == _inArrayCnt &&
         _inputScalars.count() == _inScalarCnt - _inPid &&
         _inputStrings.count() == _inStringCnt &&
         _plugin.data() != 0L;
}


QString KstPlugin::propertyString() const {
  if (!isValid()) {
    return i18n("Invalid plugin.");
  }
  return plugin()->data()._name;
}


bool KstPlugin::setPlugin(KstSharedPtr<Plugin> plugin) {
  // Assumes that this is called with a write lock in place on this object
  if (plugin == _plugin) {
    return true;
  }

  freeParameters();

  if (_localData) {
    if (!_plugin || !_plugin->freeLocalData(&_localData)) {
      free(_localData);
    }
    _localData = 0L;
  }

  if (!plugin) {
    _inputVectors.clear();
    _inputScalars.clear();
    _inputStrings.clear();
    _outputVectors.clear();
    _outputScalars.clear();
    _outputStrings.clear();
    _plugin = 0L;
    return true;
  }

  Plugin::countScalarsVectorsAndStrings(plugin->data()._inputs, _inScalarCnt, _inArrayCnt, _inStringCnt, _inPid);

  if (_inputVectors.count() != _inArrayCnt ||
      _inputScalars.count() != _inScalarCnt - _inPid ||
      _inputStrings.count() != _inStringCnt) {
    _plugin = 0L;
    return false;
  }

  _outScalarCnt = 0;
  _outArrayCnt = 0;
  _outStringCnt = 0;
  _outputVectors.clear();
  _outputScalars.clear();
  _outputStrings.clear();

  const QValueList<Plugin::Data::IOValue>& otable = plugin->data()._outputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                         it != otable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      KstVectorPtr v;

      if ((*it)._subType == Plugin::Data::IOValue::FloatNonVectorSubType) {
        v = new KstVector(QString::null, 0, true);
      } else {
        v = new KstVector;
      }
      v->writeLock();
      v->setProvider(this);
      _outputVectors.insert((*it)._name, v);
      _outArrayCnt++;
      KST::addVectorToList(v);
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      KstScalarPtr s = new KstScalar;
      s->writeLock();
      s->setProvider(this);
      _outputScalars.insert((*it)._name, s);
      _outScalarCnt++;
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      KstStringPtr s = new KstString;
      s->writeLock();
      s->setProvider(this);
      _outputStrings.insert((*it)._name, s);
      _outStringCnt++;
    }
  }

  allocateParameters();
  _plugin = plugin;
  return true;
}


KstSharedPtr<Plugin> KstPlugin::plugin() const {
  return _plugin;
}


void KstPlugin::_showDialog() {
  KstDialogs::self()->showPluginDialog(tagName());
}


const KstCurveHintList* KstPlugin::curveHints() const {
  _curveHints->clear();
  if (_plugin) {
    for (QValueList<Plugin::Data::CurveHint>::ConstIterator i = _plugin->data()._hints.begin(); i != _plugin->data()._hints.end(); ++i) {
      KstVectorPtr xv = _outputVectors[(*i).x];
      KstVectorPtr yv = _outputVectors[(*i).y];
      if (xv && yv) {
        _curveHints->append(new KstCurveHint((*i).name, xv->tagName(), yv->tagName()));
      }
    }
  }

  return KstDataObject::curveHints();
}


QString KstPlugin::lastError() const {
  return _lastError;
}


// FIXME: KstPlugin should not know about fit scalars!!
void KstPlugin::createFitScalars() {
  if (_plugin->data()._isFit && _outputVectors.contains("Parameters")) {
    KstVectorPtr vectorParam = _outputVectors["Parameters"];
    if (vectorParam) {
      QString paramName;
      int i = 0;
      int length = vectorParam->length();

      for (paramName = _plugin->parameterName(i); 
          !paramName.isEmpty() && i < length; 
           paramName = _plugin->parameterName(++i)) {
        double scalarValue = vectorParam->value(i);
        if (!_outputScalars.contains(paramName)) {
          QString scalarName = i18n("%1-%2").arg(tagName()).arg(paramName);
          KstScalarPtr s = new KstScalar(scalarName, scalarValue);
          s->writeLock();
          s->setProvider(this);
          s->writeUnlock();
          _outputScalars.insert(paramName, s);
        } else {
          _outputScalars[paramName]->setValue(scalarValue);
        }
      }
    }
  }
}


KstDataObjectPtr KstPlugin::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  KstPluginPtr plugin = new KstPlugin;

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
    KstVectorPtr v = new KstVector;
    v->writeLock();
    plugin->outputVectors().insert(iter.key(), v);
    v->setTagName(iter.data()->tagName() + "'");
    v->setProvider(plugin.data());
    KST::addVectorToList(v);
    v->writeUnlock();
  }
  for (KstScalarMap::ConstIterator iter = outputScalars().begin(); iter != outputScalars().end(); ++iter) {
    KstScalarPtr s = new KstScalar;
    s->writeLock();
    plugin->outputScalars().insert(iter.key(), s);
    s->setTagName(iter.data()->tagName() + "'");
    s->setProvider(plugin.data());
    s->writeUnlock();
  }
  for (KstStringMap::ConstIterator iter = outputStrings().begin(); iter != outputStrings().end(); ++iter) {
    KstStringPtr s = new KstString;
    s->writeLock();
    plugin->outputStrings().insert(iter.key(), s);
    s->setTagName(iter.data()->tagName() + "'");
    s->setProvider(plugin.data());
    s->writeUnlock();  
  }
  
  // set the same plugin
  plugin->setPlugin(_plugin);
  plugin->setTagName(tagName() + "'");
  duplicatedMap.insert(this, KstDataObjectPtr(plugin));  
  return KstDataObjectPtr(plugin);
}

// vim: ts=2 sw=2 et
