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

KstBindDataSource::KstBindDataSource(KJS::ExecState *exec, KstDataSourcePtr s)
: KstBindObject(exec, s.data(), "DataSource") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindDataSource::KstBindDataSource(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindObject(exec, globalObject, "DataSource") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindDataSource::KstBindDataSource(int id)
: KstBindObject(id, "DataSource Method") {
}


KstBindDataSource::~KstBindDataSource() {
}


KJS::Object KstBindDataSource::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() < 1) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  }

  QString file = args[0].toString(exec).qstring();
  QString type;

  if (args.size() == 2) {
    if (args[1].type() != KJS::StringType) {
      return createTypeError(exec, 1);
    }
    type = args[1].toString(exec).qstring();
  }

  bool newSource = false;
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findFileName(file);
  KST::dataSourceList.lock().unlock();

  if (!ds) {
    ds = KstDataSource::loadSource(file, type);
    newSource = true;
  }

  if (!ds) {
    QString error = QString("Failed to create data source from '%1'").arg(file);
    return createGeneralError(exec, error);
  }

  if (newSource) {
    KST::dataSourceList.lock().writeLock();
    KST::dataSourceList.append(ds);
    KST::dataSourceList.lock().unlock();
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
  { "isValidField", &KstBindDataSource::isValidField },
  { "samplesPerFrame", &KstBindDataSource::samplesPerFrame },
  { "frameCount", &KstBindDataSource::frameCount },
  { "fieldList", &KstBindDataSource::fieldList },
  { "configuration", &KstBindDataSource::configuration },
  { "setConfiguration", &KstBindDataSource::setConfiguration },
  { "matrixList", &KstBindDataSource::matrixList },
  { "reset", &KstBindDataSource::reset },
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
  KJS::ReferenceList rc = KstBindObject::propList(exec, recursive);

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

  return KstBindObject::hasProperty(exec, propertyName);
}


KJS::Value KstBindDataSource::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindObject::get(exec, propertyName);
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

  return KstBindObject::get(exec, propertyName);
}


KJS::Value KstBindDataSource::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindObject::methodCount();
  if (id > start) {
    KstBindDataSource *imp = dynamic_cast<KstBindDataSource*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*dataSourceBindings[id - start - 1].method)(exec, args);
  }

  return KstBindObject::call(exec, self, args);
}


void KstBindDataSource::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindObject::methodCount();
  for (int i = 0; dataSourceBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindDataSource(i + start + 1));
    obj.put(exec, dataSourceBindings[i].name, o, KJS::Function);
  }
}


#define makeSource(X) dynamic_cast<KstDataSource*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindDataSource::fieldList(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  KJS::List rc;
  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }
  s->readLock();
  QStringList l = s->fieldList();
  s->unlock();
  for (QStringList::ConstIterator i = l.begin(); i != l.end(); ++i) {
    rc.append(KJS::String(*i));
  }

  return KJS::Object(exec->interpreter()->builtinArray().construct(exec, rc));
}


KJS::Value KstBindDataSource::matrixList(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  KJS::List rc;
  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }
  s->readLock();
  QStringList l = s->matrixList();
  s->unlock();
  for (QStringList::ConstIterator i = l.begin(); i != l.end(); ++i) {
    rc.append(KJS::String(*i));
  }

  return KJS::Object(exec->interpreter()->builtinArray().construct(exec, rc));
}


KJS::Value KstBindDataSource::reset(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  s->readLock();
  s->reset();
  s->unlock();

  return KJS::Undefined();
}


KJS::Value KstBindDataSource::isValidField(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  s->writeLock();
  bool rc = s->isValidField(args[0].toString(exec).qstring());
  s->unlock();

  return KJS::Boolean(rc);
}


KJS::Value KstBindDataSource::frameCount(KJS::ExecState *exec, const KJS::List& args) {
  QString field;

  if (args.size() == 1) {
    if (args[0].type() != KJS::StringType) {
      return createTypeError(exec, 0);
    }
    field = args[0].toString(exec).qstring();
  } else if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  s->writeLock();
  int rc = s->frameCount(field);
  s->unlock();

  return KJS::Number(rc);
}


KJS::Value KstBindDataSource::samplesPerFrame(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  s->writeLock();
  int rc = s->samplesPerFrame(args[0].toString(exec).qstring());
  s->unlock();

  return KJS::Number(rc);
}


KJS::Value KstBindDataSource::configuration(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  QString result;

  result = s->configuration(args[0].toString(exec).qstring());

  return KJS::String(result);
}


KJS::Value KstBindDataSource::setConfiguration(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 2) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  } else if (args[1].type() != KJS::StringType) {
    return createTypeError(exec, 1);
  }

  KstDataSourcePtr s = makeSource(_d);
  if (!s) {
    return createInternalError(exec);
  }

  if (s->setConfiguration(args[0].toString(exec).qstring(), args[1].toString(exec).qstring())) {
    return KJS::Boolean(true);
  } else {
    return KJS::Boolean(false);
  }

  return KJS::Undefined();
}


KJS::Value KstBindDataSource::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::Boolean(s->isValid());
  }

  return KJS::Boolean(false);
}


KJS::Value KstBindDataSource::empty(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::Boolean(s->isEmpty());
  }
  return KJS::Boolean(false);
}


KJS::Value KstBindDataSource::completeFieldList(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::Boolean(s->fieldListIsComplete());
  }
  return KJS::Boolean(false);
}


KJS::Value KstBindDataSource::fileName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::String(s->fileName());
  }
  return KJS::String();
}


KJS::Value KstBindDataSource::fileType(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::String(s->fileType());
  }
  return KJS::String();
}


KJS::Value KstBindDataSource::source(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    KstReadLocker rl(s);
    return KJS::String(s->sourceName());
  }
  return KJS::String();
}


KJS::Value KstBindDataSource::metaData(KJS::ExecState *exec) const {
  KJS::Object array(exec->interpreter()->builtinArray().construct(exec, 0));
  KstDataSourcePtr s = makeSource(_d);
  if (s) {
    s->readLock();
    QDict<KstString> data = s->metaData();
    s->unlock();
    for (QDictIterator<KstString> i(data); i.current(); ++i) {
      array.put(exec, KJS::Identifier(i.currentKey().latin1()), KJS::String(i.current() ? i.current()->value() : QString::null));
    }
  }
  return array;
}

#undef makeSource
