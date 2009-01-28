/***************************************************************************
                              bind_debuglog.cpp
                             -------------------
    begin                : Apr 07 2005
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

#include "bind_debuglog.h"
#include "bind_debuglogentry.h"

#include <kstdebug.h>

#include <kdebug.h>

KstBindDebugLog::KstBindDebugLog(KJS::ExecState *exec)
: KstBinding("DebugLog", false) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindDebugLog::KstBindDebugLog(int id)
: KstBinding("DebugLog Method", id) {
}


KstBindDebugLog::~KstBindDebugLog() {
}


KJS::Object KstBindDebugLog::construct(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  return KJS::Object(new KstBindDebugLog(exec));
}


struct DebugLogBindings {
  const char *name;
  KJS::Value (KstBindDebugLog::*method)(KJS::ExecState*, const KJS::List&);
};


struct DebugLogProperties {
  const char *name;
  void (KstBindDebugLog::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindDebugLog::*get)(KJS::ExecState*) const;
};


static DebugLogBindings debugLogBindings[] = {
  { 0L, 0L }
};


static DebugLogProperties debugLogProperties[] = {
  { "length", 0L, &KstBindDebugLog::length },
  { "text", 0L, &KstBindDebugLog::text },
  { "lengthNotices", 0L, &KstBindDebugLog::lengthNotices },
  { "textNotices", 0L, &KstBindDebugLog::textNotices },
  { "lengthWarnings", 0L, &KstBindDebugLog::lengthWarnings },
  { "textWarnings", 0L, &KstBindDebugLog::textWarnings },
  { "lengthErrors", 0L, &KstBindDebugLog::lengthErrors },
  { "textErrors", 0L, &KstBindDebugLog::textErrors },
  { "lengthDebugs", 0L, &KstBindDebugLog::lengthDebugs },
  { "textDebugs", 0L, &KstBindDebugLog::textDebugs },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindDebugLog::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; debugLogProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(debugLogProperties[i].name)));
  }

  return rc;
}


bool KstBindDebugLog::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; debugLogProperties[i].name; ++i) {
    if (prop == debugLogProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


KJS::Value KstBindDebugLog::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindDebugLog *imp = dynamic_cast<KstBindDebugLog*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*debugLogBindings[id - 1].method)(exec, args);
}


void KstBindDebugLog::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  QString prop = propertyName.qstring();

  for (int i = 0; debugLogProperties[i].name; ++i) {
    if (prop == debugLogProperties[i].name) {
      if (!debugLogProperties[i].set) {
        break;
      }
      (this->*debugLogProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindDebugLog::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; debugLogProperties[i].name; ++i) {
    if (prop == debugLogProperties[i].name) {
      if (!debugLogProperties[i].get) {
        break;
      }
      return (this->*debugLogProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindDebugLog::getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const {
  if (propertyName < unsigned(KstDebug::self()->logLength())) {
    return KJS::Object(new KstBindDebugLogEntry(exec, KstDebug::self()->message(propertyName)));
  }

  return createGeneralError(exec, i18n("Index is out of range."));
}


void KstBindDebugLog::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; debugLogBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindDebugLog(i + 1));
    obj.put(exec, debugLogBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindDebugLog::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Number(KstDebug::self()->logLength());
}


KJS::Value KstBindDebugLog::text(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QString log;
  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    QString lev;
    switch ((*i).level) {
      case KstDebug::Notice:
        lev = i18n("notice", "N");
        break;
      case KstDebug::Warning:
        lev = i18n("warning", "W");
        break;
      case KstDebug::Error:
        lev = i18n("error", "E");
        break;
      case KstDebug::Debug:
        lev = i18n("debug", "D");
        break;
      default:
        lev = " ";
    }
    log += i18n("date loglevel logtext", "%1 %2 %3\n").arg(KGlobal::locale()->formatDateTime((*i).date)).arg(lev).arg((*i).msg);
  }

  return KJS::String(log);
}


KJS::Value KstBindDebugLog::lengthNotices(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();
  int num = 0;

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Notice) {
      num++;
    }
  }

  return KJS::Number(num);
}

KJS::Value KstBindDebugLog::textNotices(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QString log;
  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Notice) {
      log += i18n("date logtext", "%1 %2\n").arg(KGlobal::locale()->formatDateTime((*i).date)).arg((*i).msg);
    }
  }

  return KJS::String(log);
}


KJS::Value KstBindDebugLog::lengthWarnings(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();
  int num = 0;

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Warning) {
      num++;
    }
  }

  return KJS::Number(num);
}


KJS::Value KstBindDebugLog::textWarnings(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QString log;
  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Warning) {
      log += i18n("date logtext", "%1 %2\n").arg(KGlobal::locale()->formatDateTime((*i).date)).arg((*i).msg);
    }
  }

  return KJS::String(log);
}


KJS::Value KstBindDebugLog::lengthErrors(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();
  int num = 0;

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Error) {
      num++;
    }
  }

  return KJS::Number(num);
}


KJS::Value KstBindDebugLog::textErrors(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QString log;
  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Error) {
      log += i18n("date logtext", "%1 %2\n").arg(KGlobal::locale()->formatDateTime((*i).date)).arg((*i).msg);
    }
  }

  return KJS::String(log);
}


KJS::Value KstBindDebugLog::lengthDebugs(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();
  int num = 0;

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Debug) {
      num++;
    }
  }

  return KJS::Number(num);
}


KJS::Value KstBindDebugLog::textDebugs(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QString log;
  QValueList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  for (QValueList<KstDebug::LogMessage>::ConstIterator i = msgs.begin(); i != msgs.end(); ++i) {
    if ((*i).level == KstDebug::Debug) {
      log += i18n("date logtext", "%1 %2\n").arg(KGlobal::locale()->formatDateTime((*i).date)).arg((*i).msg);
    }
  }

  return KJS::String(log);
}
