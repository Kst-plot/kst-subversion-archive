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

#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstdoc.h"
#include "kstplugin.h"
#include "kstplugindialog_i.h"
#include "plugincollection.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <stdlib.h>


static void countScalarsAndVectors(const QValueList<Plugin::Data::IOValue>& table, unsigned& scalars, unsigned& vectors) {
  scalars = 0;
  vectors = 0;

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    switch ((*it)._type) {
      case Plugin::Data::IOValue::FloatType:
        scalars++;
        break;
      case Plugin::Data::IOValue::TableType:
        if ((*it)._subType == Plugin::Data::IOValue::FloatSubType) {
          vectors++;
        }
        break;
      default:
        break;
    }
  }
}


KstPlugin::KstPlugin() : KstDataObject() {
  commonConstructor();
}


KstPlugin::KstPlugin(QDomElement &e) : KstDataObject(e) {
  QString pluginName;

  commonConstructor();

  QDomNode n = e.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "name") {
        pluginName = e.text();
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "iscalar") {
        _inputScalarLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "ovector") {
        KstVectorPtr v = new KstVector(e.text());
        _outputVectors.insert(e.attribute("name"), v);
        KST::addVectorToList(v);
      } else if (e.tagName() == "oscalar") {
        _outputScalars.insert(e.attribute("name"), new KstScalar(e.text()));
      }
    }
    n = n.nextSibling();
  }

  _plugin = PluginCollection::self()->plugin(pluginName);

  if (!_plugin.data()) {
    KstDebug::self()->log(i18n("Unable to load plugin %1 for \"%2\".").arg(pluginName).arg(tagName()), KstDebug::Warning);
  } else {
    countScalarsAndVectors(_plugin->data()._inputs, _inScalarCnt, _inArrayCnt);

    const QValueList<Plugin::Data::IOValue>& otable = _plugin->data()._outputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                           it != otable.end();
                                                                         ++it) {
      // FIXME: i18n?
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        _outArrayCnt++;
        if (!_outputVectors.contains((*it)._name)) {
          KstVectorPtr v = new KstVector(tagName() + " vector - " + (*it)._name);
          _outputVectors.insert((*it)._name, v);
          KST::addVectorToList(v);
        }
      } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
        _outScalarCnt++;
        if (!_outputScalars.contains((*it)._name)) {
          KstScalar *s = new KstScalar(tagName() + " scalar - " + (*it)._name);
          _outputScalars.insert((*it)._name, s);
        }
      }
    }
  }
}


void KstPlugin::commonConstructor() {
  _inScalarCnt = 0;
  _outScalarCnt = 0;
  _inArrayCnt = 0;
  _outArrayCnt = 0;
  _typeString = i18n("Plugin");
  _plugin = 0L;
  _localData = 0L;
  //kdDebug() << "Creating KSTPlugin: " << long(this) << endl;
}


KstPlugin::~KstPlugin() {
  //kdDebug() << "Destroying KSTPlugin: " << long(this) << endl;
}


KstObject::UpdateType KstPlugin::update(int update_counter) {
  if (!isValid()) {
    return NO_CHANGE;
  }

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  int *inArrayLens = 0L, *outArrayLens = 0L;
  double *inScalars = 0L, *outScalars = 0L;
  double **inVectors = 0L, **outVectors = 0L;

  if (_inArrayCnt > 0) {
    inArrayLens = new int[_inArrayCnt];
    inVectors = new double*[_inArrayCnt];
  }

  if (_outArrayCnt > 0) {
    outArrayLens = new int[_outArrayCnt];
    outVectors = new double*[_outArrayCnt];
  }

  if (_inScalarCnt > 0) {
    inScalars = new double[_inScalarCnt];
  }

  if (_outScalarCnt > 0) {
    outScalars = new double[_outScalarCnt];
  }

#define CLEANUP() do {\
  delete[] inVectors;\
  delete[] outVectors;\
  delete[] outArrayLens;\
  delete[] inArrayLens;\
  delete[] outScalars;\
  delete[] inScalars;\
  } while(0)


  const QValueList<Plugin::Data::IOValue>& itable = _plugin->data()._inputs;
  const QValueList<Plugin::Data::IOValue>& otable = _plugin->data()._outputs;
  int itcnt = 0, vitcnt = 0;

  // Populate the input scalars and vectors
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin();
                                                         it != itable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!_inputVectors.contains((*it)._name)) {
        KstDebug::self()->log(i18n("Input vector [%1] for plugin %2 not found.  Unable to continue.").arg((*it)._name).arg(tagName()), KstDebug::Error);
        CLEANUP();
        return NO_CHANGE;
      }
      inVectors[vitcnt] = _inputVectors[(*it)._name]->value();
      inArrayLens[vitcnt++] = _inputVectors[(*it)._name]->sampleCount();
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      inScalars[itcnt++] = _inputScalars[(*it)._name]->value();
    }
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
        return NO_CHANGE;
      }
      outVectors[vitcnt] = _outputVectors[(*it)._name]->value();
      outArrayLens[vitcnt++] = _outputVectors[(*it)._name]->length();
    }
  }

  int rc;
  if (_plugin->data()._localdata) {
    rc = _plugin->call(inVectors,  inArrayLens,  inScalars,
                       outVectors, outArrayLens, outScalars, &_localData);
  } else {
    rc = _plugin->call(inVectors,  inArrayLens,  inScalars,
                       outVectors, outArrayLens, outScalars);
  }

  if (rc != 0) {
    // Error
  }

  itcnt = 0;
  vitcnt = 0;
  // Read back the output vectors and scalars
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                         it != otable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      vectorRealloced(_outputVectors[(*it)._name], outVectors[vitcnt], outArrayLens[vitcnt]);
      vitcnt++;
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      _outputScalars[(*it)._name]->setValue(outScalars[itcnt++]);
    }
  }

  CLEANUP();
#undef CLEANUP
  return UPDATE;
}


void KstPlugin::save(QTextStream &ts) {
  if (_plugin.data() != 0L) {
    ts << " <plugin>" << endl;
    ts << "   <tag>" << tagName() << "</tag>" << endl;
    ts << "   <name>" << _plugin->data()._name << "</name>" << endl;
    for (KstVectorMap::Iterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
      ts << "   <ivector name=\"" << i.key() << "\">"
         << i.data()->tagName()
         << "</ivector>" << endl;
    }
    for (KstScalarMap::Iterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
      ts << "   <iscalar name=\"" << i.key() << "\">"
         << i.data()->tagName()
         << "</iscalar>" << endl;
    }
    for (KstVectorMap::Iterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
      ts << "   <ovector name=\"" << i.key() << "\">"
         << i.data()->tagName()
         << "</ovector>" << endl;
    }
    for (KstScalarMap::Iterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
      ts << "   <oscalar name=\"" << i.key() << "\">"
         << i.data()->tagName()
         << "</oscalar>" << endl;
    }
    ts << " </plugin>" << endl;
  }
}


bool KstPlugin::slaveVectorsUsed() const {
  return true;
}


bool KstPlugin::isValid() const {
  return _inputVectors.count() == _inArrayCnt &&
         _inputScalars.count() == _inScalarCnt &&
         _plugin.data() != 0L;
}


QString KstPlugin::propertyString() const {
  if (!isValid()) {
    return i18n("Invalid plugin.");
  }
  return plugin()->data()._name;
}


bool KstPlugin::setPlugin(KstSharedPtr<Plugin> plugin) {
  if (plugin == _plugin) {
    return true;
  }

  if (!plugin.data()) {
    _inputVectors.clear();
    _inputScalars.clear();
    _outputVectors.clear();
    _outputScalars.clear();
    _plugin = 0L;
    return true;
  }

  countScalarsAndVectors(plugin->data()._inputs, _inScalarCnt, _inArrayCnt);

  if (_inputVectors.count() != _inArrayCnt ||
      _inputScalars.count() != _inScalarCnt) {
    _plugin = 0L;
    return false;
  }

  _outScalarCnt = 0;
  _outArrayCnt = 0;
  _outputVectors.clear();
  _outputScalars.clear();

  const QValueList<Plugin::Data::IOValue>& otable = plugin->data()._outputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin();
                                                         it != otable.end();
                                                                        ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      KstVectorPtr v = new KstVector;
      KST::addVectorToList(v);
      _outputVectors.insert((*it)._name, v);
      _outArrayCnt++;
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      _outputScalars.insert((*it)._name, new KstScalar);
      _outScalarCnt++;
    }
  }

  _plugin = plugin;
  return true;
}


KstSharedPtr<Plugin> KstPlugin::plugin() const {
  return _plugin;
}


void KstPlugin::_showDialog() {
  KstPluginDialogI::globalInstance()->show_I(tagName());
}

// vim: ts=2 sw=2 et
