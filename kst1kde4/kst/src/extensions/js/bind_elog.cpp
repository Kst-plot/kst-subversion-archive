/***************************************************************************
                              bind_elog.cpp
                             -------------------
    begin                : Nov 20 2007
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

#include "bind_elog.h"
#include "elogthreadsubmit.h"

#include <kdebug.h>

KstBindELOG::KstBindELOG(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("ELOG") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "ELOG", o);
  }

  _port = 80;
  _suppressEmailNotification = false;
  _encodedHTML = false;
  _includeCapture = false;
  _includeConfiguration = false;
  _includeDebugInfo = false;
  _captureWidth = 640;
  _captureHeight = 480;
}



KstBindELOG::KstBindELOG(int id)
: KstBinding("ELOG Method", id) {
}


KstBindELOG::~KstBindELOG() {
}


KJS::Object KstBindELOG::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  return KJS::Object(new KstBindELOG(exec));
}


struct ELOGBindings {
  const char *name;
  KJS::Value (KstBindELOG::*method)(KJS::ExecState*, const KJS::List&);
};


struct ELOGProperties {
  const char *name;
  void (KstBindELOG::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindELOG::*get)(KJS::ExecState*) const;
};


static ELOGBindings elogBindings[] = {
  { "submit", &KstBindELOG::submit },
  { "addAttribute", &KstBindELOG::addAttribute },
  { "removeAttribute", &KstBindELOG::removeAttribute },
  { "clearAttributes", &KstBindELOG::clearAttributes },
  { "addAttachment", &KstBindELOG::addAttachment },
  { "clearAttachments", &KstBindELOG::clearAttachments },
  { 0L, 0L }
};


static ELOGProperties elogProperties[] = {
  { "hostname", &KstBindELOG::setHostname , &KstBindELOG::hostname },
  { "port", &KstBindELOG::setPort , &KstBindELOG::port },
  { "logbook", &KstBindELOG::setLogbook , &KstBindELOG::logbook },
  { "username", &KstBindELOG::setUsername , &KstBindELOG::username },
  { "password", &KstBindELOG::setPassword , &KstBindELOG::password },
  { "writePassword", &KstBindELOG::setWritePassword , &KstBindELOG::writePassword },
  { "encodedHTML", &KstBindELOG::setEncodedHTML , &KstBindELOG::encodedHTML },
  { "text", &KstBindELOG::setText , &KstBindELOG::text },
  { "includeCapture", &KstBindELOG::setIncludeCapture , &KstBindELOG::includeCapture },
  { "includeConfiguration", &KstBindELOG::setIncludeConfiguration , &KstBindELOG::includeConfiguration },
  { "includeDebugInfo", &KstBindELOG::setIncludeDebugInfo , &KstBindELOG::includeDebugInfo },
  { "captureWidth", &KstBindELOG::setCaptureWidth , &KstBindELOG::captureWidth },
  { "captureHeight", &KstBindELOG::setCaptureHeight , &KstBindELOG::captureHeight },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindELOG::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; elogProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(elogProperties[i].name)));
  }

  return rc;
}


bool KstBindELOG::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; elogProperties[i].name; ++i) {
    if (prop == elogProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindELOG::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  QString prop = propertyName.qstring();
  for (int i = 0; elogProperties[i].name; ++i) {
    if (prop == elogProperties[i].name) {
      if (!elogProperties[i].set) {
        break;
      }
      (this->*elogProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindELOG::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; elogProperties[i].name; ++i) {
    if (prop == elogProperties[i].name) {
      if (!elogProperties[i].get) {
        break;
      }
      return (this->*elogProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindELOG::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindELOG *imp = dynamic_cast<KstBindELOG*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*elogBindings[id - 1].method)(exec, args);
}


void KstBindELOG::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; elogBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindELOG(i + 1));
    obj.put(exec, elogBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindELOG::submit(KJS::ExecState *exec, const KJS::List& args) {
  ElogThreadSubmit* pThread;

  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  if (_hostname.isEmpty()) {
    return createGeneralError(exec, i18n("No hostname yet defined."));
  }

  pThread = new ElogThreadSubmit(_hostname,
                                 _port,
                                 _includeCapture,
                                 _includeConfiguration,
                                 _includeDebugInfo,
                                 _text,
                                 _username,
                                 _password,
                                 _writePassword,
                                 _logbook,
                                 _attributes,
                                 _attachments,
                                 _encodedHTML,
                                 _suppressEmailNotification,
                                 _captureWidth,
                                 _captureHeight);
  pThread->doTransmit();

  return KJS::Boolean(true);
}


KJS::Value KstBindELOG::addAttachment(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  }

  if (_attachments.count()+1 >= MAX_ATTACHMENTS) {
    return createGeneralError(exec, i18n("Maximum number of attachments has been reached."));
  }

  _attachments.append(args[0].toString(exec).qstring());
  return KJS::Boolean(true);
}


KJS::Value KstBindELOG::clearAttachments(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  _attachments.clear();
  return KJS::Undefined();
}


KJS::Value KstBindELOG::addAttribute(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 2) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  } else if (args[1].type() != KJS::StringType) {
    return createTypeError(exec, 1);
  }

  if (_attributes.count()+1 >= MAX_N_ATTR) {
    return createGeneralError(exec, i18n("Maximum number of attributes has been reached."));
  }

  _attributes.insert(args[0].toString(exec).qstring(), args[1].toString(exec).qstring());
  return KJS::Boolean(true);
}


KJS::Value KstBindELOG::removeAttribute(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  if (args[0].type() != KJS::StringType) {
    return createTypeError(exec, 0);
  }

  _attributes.remove(args[0].toString(exec).qstring());
  return KJS::Boolean(false);
}


KJS::Value KstBindELOG::clearAttributes(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }
  _attributes.clear();
  return KJS::Undefined();
}


KJS::Value KstBindELOG::hostname(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_hostname);
}


void KstBindELOG::setHostname(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _hostname = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::port(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Number(_port);
}


void KstBindELOG::setPort(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    return createPropertyTypeError(exec);
  }
  _port = value.toUInt32(exec);
}


KJS::Value KstBindELOG::logbook(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_logbook);
}


void KstBindELOG::setLogbook(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _logbook = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::username(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_username);
}


void KstBindELOG::setUsername(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _username = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::password(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_password);
}


void KstBindELOG::setPassword(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _password = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::writePassword(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_writePassword);
}


void KstBindELOG::setWritePassword(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _writePassword = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::text(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::String(_text);
}


void KstBindELOG::setText(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  _text = value.toString(exec).qstring();
}


KJS::Value KstBindELOG::suppressEmailNotification(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Boolean(_suppressEmailNotification);
}


void KstBindELOG::setSuppressEmailNotification(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  _suppressEmailNotification = value.toBoolean(exec);
}


KJS::Value KstBindELOG::encodedHTML(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Boolean(_encodedHTML);
}


void KstBindELOG::setEncodedHTML(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  _encodedHTML = value.toBoolean(exec);
}


KJS::Value KstBindELOG::includeCapture(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Boolean(_includeCapture);
}


void KstBindELOG::setIncludeCapture(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  _includeCapture = value.toBoolean(exec);
}


KJS::Value KstBindELOG::captureWidth(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Number(_captureWidth);
}


void KstBindELOG::setCaptureWidth(KJS::ExecState *exec, const KJS::Value& value) {
  int val;

  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }

  val = value.toInt32(exec);
  if (val <= 0 || val > 10000) {
    return createPropertyRangeError(exec);
  }

  _captureWidth = val;
}


KJS::Value KstBindELOG::captureHeight(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Number(_captureHeight);
}


void KstBindELOG::setCaptureHeight(KJS::ExecState *exec, const KJS::Value& value) {
  int val;

  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }

  val = value.toInt32(exec);
  if (val <= 0 || val > 10000) {
    return createPropertyRangeError(exec);
  }

  _captureHeight = val;
}


KJS::Value KstBindELOG::includeConfiguration(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Boolean(_includeConfiguration);
}


void KstBindELOG::setIncludeConfiguration(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  _includeConfiguration = value.toBoolean(exec);
}


KJS::Value KstBindELOG::includeDebugInfo(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Boolean(_includeDebugInfo);
}


void KstBindELOG::setIncludeDebugInfo(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  _includeDebugInfo = value.toBoolean(exec);
}
