/***************************************************************************
                              kstextension.cpp
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

#include <kmainwindow.h>

#include "kstextension.h"

KstExtension::KstExtension(QObject *parent, const char *name, const QStringList&) : QObject(parent) {
  setObjectName(QString(name));
}


KstExtension::~KstExtension() {
  emit unregister();
}


void KstExtension::load(const QDomElement& e) {
  Q_UNUSED(e)
}


void KstExtension::save(QTextStream& ts, const QString& indent) {
  Q_UNUSED(ts)
  Q_UNUSED(indent)
}


void KstExtension::clear() {
}


KMainWindow* KstExtension::app() const {
  return static_cast<KMainWindow*>(parent());
}


void KstExtension::processArguments(const QString& args) {
  Q_UNUSED(args)
}

#include "kstextension.moc"
