/***************************************************************************
                             bind_datasource.cpp
                             -------------------
    begin                : Mar 28 2005
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

#include "bind_datasource.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindDataSource::KstBindDataSource(KJS::ExecState *exec, KstDataSourcePtr s)
: KstBinding("DataSource"), _s(s) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindDataSource::KstBindDataSource(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("DataSource") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "DataSource", o);
  }
}


KstBindDataSource::KstBindDataSource(int id)
: KstBinding("DataSource Method", id) {
}


KstBindDataSource::~KstBindDataSource() {
}


KJS::Object KstBindDataSource::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() < 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  if (args[0].type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Object();
  }

  QString file = args[0].toString(exec).qstring();
  QString type;

  if (args.size() == 2) {
    if (args[1].type() != KJS::StringType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    type = args[1].toString(exec).qstring();
  }

  bool newSource = false;
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findFileName(file);
  KST::dataSourceList.lock().readUnlock();

  if (!ds) {
    ds = KstDataSource::loadSource(file, type);
    newSource = true;
  }

  if (!ds) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Object();
  }

  if (newSource) {
    KST::dataSourceList.lock().writeLock();
    KST::dataSourceList.append(ds);
    KST::dataSourceList.lock().writeUnlock();
  }

  return KJS::Object(new KstBindDataSource(exec, ds));
}


struct DataSourceBindings {
  const char *name;
  KJS::Value (KstBindDataSource::*method)(KJS::ExecState*, const KJS::List&);
};


struct DataSourceProperties {
  const char *name;
  void (KstBindDataSource::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindDataSource::*get)(KJS::ExecState*) const;
};


static DataSourceBindings dataSourceBindings[] = {
  // time stuff ?
  // config widget stuff?
  { "isValidField", &KstBindDataSource::isValidField },
  { "samplesPerFrame", &KstBindDataSource::samplesPerFrame },
  { "frameCount", &KstBindDataSource::frameCount },
  { "fieldList", &KstBindDataSource::fieldList },
  { 0L, 0L }
};


static DataSourceProperties dataSourceProperties[] = {
  { "valid", 0L, &KstBindDataSource::valid },
  { "empty", 0L, &KstBindDataSource::empty },
  { "completeFieldList", 0L, &KstBindDataSource::completeFieldList },
  { "fileName", 0L, &KstBindDataSource::fileName },
  { "fileType", 0L, &KstBindDataSource::fileType },
  { "source", 0L, &KstBindDataSource::source },
  { "metaData", 0L, &KstBindDataSource::metaData },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindDataSource::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; dataSourceProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(dataSourceProperties[i].name)));
  }

  return rc;
}


bool KstBindDataSource::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; dataSourceProperties[i].name; ++i) {
    if (prop == dataSourceProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


KJS::Value KstBindDataSource::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_s) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; dataSourceProperties[i].name; ++i) {
    if (prop == dataSourceProperties[i].name) {
      if (!dataSourceProperties[i].get) {
        break;
      }
      return (this->*dataSourceProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindDataSource::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindDataSource *imp = dynamic_cast<KstBindDataSource*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*dataSourceBindings[id - 1].method)(exec, args);
}


void KstBindDataSource::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; dataSourceBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindDataSource(i + 1));
    obj.put(exec, dataSourceBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindDataSource::fieldList(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  KJS::List rc;
  if (!_s) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }
  _s->readLock();
  QStringList l = _s->fieldList();
  _s->readUnlock();
  for (QStringList::ConstIterator i = l.begin(); i != l.end(); ++i) {
    rc.append(KJS::String(*i));
  }

  return KJS::Object(exec->interpreter()->builtinArray().construct(exec, rc));
}


KJS::Value KstBindDataSource::isValidField(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (!_s) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args[0].type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  _s->writeLock();
  bool rc = _s->isValidField(args[0].toString(exec).qstring());
  _s->writeUnlock();

  return KJS::Boolean(rc);
}


KJS::Value KstBindDataSource::frameCount(KJS::ExecState *exec, const KJS::List& args) {
  if (!_s) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  QString field;

  if (args.size() != 1) {
    if (args[0].type() != KJS::StringType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Undefined();
    }
    field = args[0].toString(exec).qstring();
  }

  _s->writeLock();
  int rc = _s->frameCount(field);
  _s->writeUnlock();

  return KJS::Number(rc);
}


KJS::Value KstBindDataSource::samplesPerFrame(KJS::ExecState *exec, const KJS::List& args) {
  if (!_s) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args[0].type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  _s->writeLock();
  int rc = _s->samplesPerFrame(args[0].toString(exec).qstring());
  _s->writeUnlock();

  return KJS::Number(rc);
}


KJS::Value KstBindDataSource::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::Boolean(_s->isValid());
}


KJS::Value KstBindDataSource::empty(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::Boolean(_s->isEmpty());
}


KJS::Value KstBindDataSource::completeFieldList(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::Boolean(_s->fieldListIsComplete());
}


KJS::Value KstBindDataSource::fileName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::String(_s->fileName());
}


KJS::Value KstBindDataSource::fileType(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::String(_s->fileType());
}


KJS::Value KstBindDataSource::source(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::String(_s->sourceName());
}


KJS::Value KstBindDataSource::metaData(KJS::ExecState *exec) const {
  _s->readLock();
  QMap<QString,QString> data = _s->metaData();
  _s->readUnlock();
  KJS::Object array(exec->interpreter()->builtinArray().construct(exec, 0));
  for (QMap<QString,QString>::ConstIterator i = data.begin(); i != data.end(); ++i) {
    array.put(exec, KJS::Identifier(i.key().latin1()), KJS::String(i.data()));
  }
  return array;
}

// vim: ts=2 sw=2 et
