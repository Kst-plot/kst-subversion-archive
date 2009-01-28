/***************************************************************************
                         bind_pluginiocollection.cpp
                             -------------------
    begin                : Apr 18 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "bind_pluginiocollection.h"
#include "bind_pluginio.h"

#include <plugin.h>

#include <qdeepcopy.h>
#include <qstringlist.h>

#include <kdebug.h>

KstBindPluginIOCollection::KstBindPluginIOCollection(KJS::ExecState *exec, const QValueList<Plugin::Data::IOValue>& data, bool input)
: KstBindCollection(exec, input ? "PluginInputCollection" : "PluginOutputCollection", true), _d(data), _input(input) {
  _cstyle = true;
}


KstBindPluginIOCollection::KstBindPluginIOCollection(KJS::ExecState *exec, const QStringList& vectors, const QStringList& scalars, const QStringList& strings, bool input)
: KstBindCollection(exec, input ? "PluginInputCollection" : "PluginOutputCollection", true), _input(input) {
  _vectors = QDeepCopy<QStringList>( vectors );
  _scalars = QDeepCopy<QStringList>( scalars );
  _strings = QDeepCopy<QStringList>( strings );
  _cstyle = false;
}


KstBindPluginIOCollection::~KstBindPluginIOCollection() {
}


KJS::Value KstBindPluginIOCollection::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  if (_cstyle) {
    return KJS::Number(_d.count());
  } else {
    return KJS::Number(_vectors.size() + _scalars.size() + _strings.size());
  }
}


QStringList KstBindPluginIOCollection::collection(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QStringList rc;

  if (_cstyle) {
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = _d.begin(); it != _d.end(); ++it) {
      rc << (*it)._name;
    }
  } else {
    rc += _vectors;
    rc += _scalars;
    rc += _strings;
  }

  return rc;
}


KJS::Value KstBindPluginIOCollection::extract(KJS::ExecState *exec, const KJS::Identifier& item) const {
  QString i = item.qstring();

  if (_cstyle) {
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = _d.begin(); it != _d.end(); ++it) {
      if ((*it)._name == i) {
        return KJS::Object(new KstBindPluginIO(exec, *it, _input));
      }
    }
  } else {
    for (QStringList::ConstIterator it = _vectors.begin(); it != _vectors.end(); ++it) {
      if ((*it) == i) {
        Plugin::Data::IOValue value;

        value._name = *it;
        value._type = Plugin::Data::IOValue::TableType;
        value._subType = Plugin::Data::IOValue::FloatSubType;
        value._description = "";
        value._default = "";
        value._optional = false;

        return KJS::Object(new KstBindPluginIO(exec, value, _input));
      }
    }
    for (QStringList::ConstIterator it = _scalars.begin(); it != _scalars.end(); ++it) {
      if ((*it) == i) {
        Plugin::Data::IOValue value;

        value._name = *it;
        value._type = Plugin::Data::IOValue::FloatType;
        value._subType = Plugin::Data::IOValue::UnknownSubType;
        value._description = "";
        value._default = "";
        value._optional = false;

        return KJS::Object(new KstBindPluginIO(exec, value, _input));
      }
    }
    for (QStringList::ConstIterator it = _strings.begin(); it != _strings.end(); ++it) {
      if ((*it) == i) {
        Plugin::Data::IOValue value;

        value._name = *it;
        value._type = Plugin::Data::IOValue::StringType;
        value._subType = Plugin::Data::IOValue::UnknownSubType;
        value._description = "";
        value._default = "";
        value._optional = false;

        return KJS::Object(new KstBindPluginIO(exec, value, _input));
      }
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindPluginIOCollection::extract(KJS::ExecState *exec, unsigned item) const {
  if (_cstyle) {
    if (item < _d.count()) {
      return KJS::Object(new KstBindPluginIO(exec, _d[item], _input));
    }
  } else {
    if (item < _vectors.size()) {
      Plugin::Data::IOValue value;

      value._name = _vectors[item];
      value._type = Plugin::Data::IOValue::TableType;
      value._subType = Plugin::Data::IOValue::FloatSubType;
      value._description = "";
      value._default = "";
      value._optional = false;

      return KJS::Object(new KstBindPluginIO(exec, value, _input));
    } else if (item < _vectors.size()  + _scalars.size()) {
      Plugin::Data::IOValue value;

      value._name = _scalars[item - _vectors.size()];
      value._type = Plugin::Data::IOValue::FloatType;
      value._subType = Plugin::Data::IOValue::UnknownSubType;
      value._description = "";
      value._default = "";
      value._optional = false;

      return KJS::Object(new KstBindPluginIO(exec, value, _input));
    } else if (item < _vectors.size()  + _scalars.size() + _strings.size()) {
      Plugin::Data::IOValue value;

      value._name = _strings[item - _vectors.size() - _scalars.size()];
      value._type = Plugin::Data::IOValue::StringType;
      value._subType = Plugin::Data::IOValue::UnknownSubType;
      value._description = "";
      value._default = "";
      value._optional = false;

      return KJS::Object(new KstBindPluginIO(exec, value, _input));
    }
  }

  return KJS::Undefined();
}

