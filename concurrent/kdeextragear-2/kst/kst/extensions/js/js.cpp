/***************************************************************************
                                   js.cpp
                             -------------------
    begin                : Feb 09 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "js.h"

#include "jsproxies.h"

#include <kst.h>

#include <kaction.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kjsembed/kjsembedpart.h>
#include <kjsembed/jsconsolewidget.h>
#include <kjsembed/jsfactory.h>
#include <kjsembed/jssecuritypolicy.h>
#include <kmessagebox.h>

K_EXPORT_COMPONENT_FACTORY(kstextension_js, KGenericFactory<KstJS>)


KstJS::KstJS(QObject *parent, const char *name, const QStringList& l) : KstExtension(parent, name, l), KXMLGUIClient() {

  KJSEmbed::JSSecurityPolicy::setDefaultPolicy(KJSEmbed::JSSecurityPolicy::CapabilityAll);
  _jsPart = new KJSEmbed::KJSEmbedPart(0L, "javascript", this, "kjsembedpart");
  _jsPart->addObject(app(), "kst");
  _jsPart->addObject(_jsPart->view(), "console");
  KstJSDataProxy *dataProxy = new KstJSDataProxy(_jsPart->factory(), this, "data");
  _jsPart->addObject(dataProxy, "data");

  new KAction(i18n("Show JavaScript Console..."), 0, 0, _jsPart->view(), SLOT(show()), actionCollection(), "js_console_show");
  new KAction(i18n("Load JavaScript..."), 0, 0, this, SLOT(loadScript()), actionCollection(), "js_load");
  setInstance(app()->instance());
  setXMLFile("kstextension_js.rc", true);
  app()->guiFactory()->addClient(this);
}


KstJS::~KstJS() {
  // crash
  //app()->guiFactory()->removeClient(this);
}


void KstJS::loadScript() {
  QString fn = KFileDialog::getOpenFileName("::<kstfiledir>",
      i18n("*.js|JavaScript (*.js)\n*|All Files"),
      app(), i18n("Open Script"));

  if (!fn.isEmpty()) {
    if (_jsPart->runFile(fn)) {
      if (!_scripts.contains(fn)) {
        _scripts.append(fn);
      }
    } else {
      KJS::Completion c = _jsPart->completion();
      QString err = c.toString(_jsPart->globalExec()).qstring();
      KMessageBox::error(app(), i18n("Error running script %1: %2").arg(fn).arg(err));
    }
  }
}


void KstJS::load(QDomElement& e) {
}


void KstJS::save(QTextStream& ts) {
}


#include "js.moc"
// vim: ts=2 sw=2 et
