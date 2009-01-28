/***************************************************************************
                       jsiface_impl.cpp  -  Part of KST
                             -------------------
    begin                : Tue Feb 08 2005
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

#include <config.h>
#include <stdio.h>
#ifdef KST_HAVE_READLINE
#include <readline/history.h>
#endif

#include "jsiface_impl.h"
#include <kjsembed/jsbinding.h>

JSIfaceImpl::JSIfaceImpl(KJSEmbed::KJSEmbedPart *part)
: DCOPObject("KstScript"), _jsPart(part) {
#ifdef KST_HAVE_READLINE
  using_history();
#endif
}


JSIfaceImpl::~JSIfaceImpl() {
#ifdef KST_HAVE_READLINE
  clear_history();
#endif
}


QString JSIfaceImpl::evaluate(const QString& script) {
  KJS::Completion res;
  QString result;

#ifdef KST_HAVE_READLINE
  add_history(script.latin1());
#endif

  if (_jsPart->execute(res, script, KJS::Null())) {
    if (!res.isValueCompletion()) {
      return QString::null;
    }
    result = res.value().toString(_jsPart->globalExec()).qstring();
  } else {
    KJS::UString s = res.value().toString(_jsPart->globalExec());

    if (s.isEmpty()) {
      result = i18n("Unknown error.");
    }
    result = i18n("Error: %1").arg(s.qstring());
  }

  if (!_output.isEmpty()) {
    result += _output;
    _output = QString();
  }

  return result;
}


QString JSIfaceImpl::evaluateFile(const QString& filename) {
  if (_jsPart->runFile(filename)) {
    // FIXME
#if 0
    if (!_scripts.contains(filename)) {
      _scripts.append(filename);
    }
#endif
    KJS::Completion c = _jsPart->completion();
    if (!c.isValueCompletion()) {
      return QString::null;
    }
    return c.value().toString(_jsPart->globalExec()).qstring();
  } else {
    KJS::Completion c = _jsPart->completion();
    if (c.isNull()) {
      return i18n("Unknown error running script.");
    }
    return i18n("Error: %1").arg(c.toString(_jsPart->globalExec()).qstring());
  }
}


bool JSIfaceImpl::writeHistory(const QString& filename) {
  bool retVal = false;

#ifdef KST_HAVE_READLINE
  if( write_history(filename.latin1()) == 0) {
    retVal = true;
  }
#endif

  return retVal;
}


void JSIfaceImpl::clearHistory( ) {
#ifdef KST_HAVE_READLINE
  clear_history();
#endif
}


bool JSIfaceImpl::addToOutput(const QString& output) {
  _output += QString("\n%1").arg(output);

  return true;
}
