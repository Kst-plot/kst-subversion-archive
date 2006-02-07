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

#include "kstextension.h"
#include "kst.h"

KstExtension::KstExtension(QObject *parent, const char *name, const QStringList&) : QObject(parent, name) {
}


KstExtension::~KstExtension() {
  emit unregister();
}


void KstExtension::load(QDomElement& e) {
  Q_UNUSED(e)
}


void KstExtension::save(QTextStream& ts) {
  Q_UNUSED(ts)
}


void KstExtension::clear() {
}


KstApp* KstExtension::app() const {
  return static_cast<KstApp*>(parent());
}

#include "kstextension.moc"
// vim: ts=2 sw=2 et
