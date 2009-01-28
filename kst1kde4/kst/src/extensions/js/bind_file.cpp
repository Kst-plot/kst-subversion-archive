/***************************************************************************
                              bind_file.cpp
                             -------------------
    begin                : Oct 18 2007
    copyright            : (C) 2007 The University of British Columbia
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

#include "bind_file.h"

#include <qfile.h>

#include <kdebug.h>

KstBindFile::KstBindFile(KJS::ExecState *exec, QFile *f)
: KstBinding("File"), _f(f) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindFile::KstBindFile(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("File") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "File", o);
  } else {
    _f = new QFile;
  }
}


KstBindFile::KstBindFile(int id)
: KstBinding("File Method", id) {
}


KstBindFile::~KstBindFile() {
}


KJS::Object KstBindFile::construct(KJS::ExecState *exec, const KJS::List& args) {
  QString name;
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  if (args.size() == 1) {
    if (args[0].type() != KJS::StringType) {
      return createTypeError(exec, 0);
    }
    name = args[0].toString(exec).qstring();
  }

  QFile *f = new QFile(name);

  return KJS::Object(new KstBindFile(exec, f));
}


struct FileBindings {
  const char *name;
  KJS::Value (KstBindFile::*method)(KJS::ExecState*, const KJS::List&);
};


struct FileProperties {
  const char *name;
  void (KstBindFile::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindFile::*get)(KJS::ExecState*) const;
};


static FileBindings fileBindings[] = {
  { "open", &KstBindFile::open },
  { "close", &KstBindFile::close },
  { "readLine", &KstBindFile::readLine },
  { 0L, 0L }
};


static FileProperties fileProperties[] = {
  { "name", 0L, &KstBindFile::name },
  { "size", 0L, &KstBindFile::size },
  { "exists", 0L, &KstBindFile::exists },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindFile::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; fileProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(fileProperties[i].name)));
  }

  return rc;
}


bool KstBindFile::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; fileProperties[i].name; ++i) {
    if (prop == fileProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindFile::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_f) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; fileProperties[i].name; ++i) {
    if (prop == fileProperties[i].name) {
      if (!fileProperties[i].set) {
        break;
      }
      (this->*fileProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindFile::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_f) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; fileProperties[i].name; ++i) {
    if (prop == fileProperties[i].name) {
      if (!fileProperties[i].get) {
        break;
      }
      return (this->*fileProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindFile::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindFile *imp = dynamic_cast<KstBindFile*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*fileBindings[id - 1].method)(exec, args);
}


void KstBindFile::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; fileBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindFile(i + 1));
    obj.put(exec, fileBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindFile::open(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)

  if (!_f) {
    return createInternalError(exec);
  }
  if (!_f->open(IO_ReadOnly)) {
    return createGeneralError(exec, i18n("Failed to open file."));
  }

  return KJS::Undefined();
}


KJS::Value KstBindFile::close(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  if (!_f) {
    return createInternalError(exec);
  }
  _f->close();
  return KJS::Undefined();
}


KJS::Value KstBindFile::readLine(KJS::ExecState *exec, const KJS::List& args) {
  unsigned maxLength = 0;
  QString str;

  if (args.size() == 0) {
    maxLength = 500;
  } else if (args.size() == 1) {
    if (args[0].type() != KJS::NumberType || !args[0].toUInt32(maxLength)) {
      return createTypeError(exec, 0);
    }
  } else {
    return createSyntaxError(exec);
  }

  if (!_f) {
    return createInternalError(exec);
  }

  if (_f->readLine(str, maxLength) == -1) {
    return createGeneralError(exec, i18n("Failed to read line from file."));
  }

  return KJS::String(str);
}


KJS::Value KstBindFile::size(KJS::ExecState *exec) const {
  if (!_f) {
    return createInternalError(exec);
  }
  return KJS::Number(_f->size());
}


KJS::Value KstBindFile::name(KJS::ExecState *exec) const {
  if (!_f) {
    return createInternalError(exec);
  }
  return KJS::String(_f->name());
}


KJS::Value KstBindFile::exists(KJS::ExecState *exec) const {
  if (!_f) {
    return createInternalError(exec);
  }
  return KJS::Boolean(_f->exists());
}
