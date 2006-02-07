/***************************************************************************
                                jsproxies.cpp
                             -------------------
    begin                : Feb 10 2004
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

#include "jsproxies.h"

KstJSDataProxy::KstJSDataProxy(JSFactory *factory, QObject *parent, const char *name)
: QObject(parent, name) {
  factory->addType("KstScalar");
  factory->addType("KstVector");
  factory->addType("KstPlugin");
  factory->addType("KstEquationCurve");
  factory->addType("KstPSDCurve");
  factory->addType("KstVCurve");
}


KstJSDataProxy::~KstJSDataProxy() {
}


QStringList KstJSDataProxy::scalars() {
  return KST::scalarList.tagNames();
}


QObject *KstJSDataProxy::scalar(const QString& name) {
  KstScalarList::Iterator it = KST::scalarList.findTag(name);
  if (it == KST::scalarList.end()) {
    return 0L;
  }
  return (*it).data();
}


QStringList KstJSDataProxy::vectors() {
  return KST::vectorList.tagNames();
}


QObject *KstJSDataProxy::vector(const QString& name) {
  KstVectorList::Iterator it = KST::vectorList.findTag(name);
  if (it == KST::vectorList.end()) {
    return 0L;
  }
  return (*it).data();
}


QStringList KstJSDataProxy::dataObjects() {
  return KST::dataObjectList.tagNames();
}


QObject *KstJSDataProxy::dataObject(const QString& name) {
  KstDataObjectList::Iterator it = KST::dataObjectList.findTag(name);
  if (it == KST::dataObjectList.end()) {
    return 0L;
  }
  return (*it).data();
}


#include "jsproxies.moc"

// vim: ts=2 sw=2 et
