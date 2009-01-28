/***************************************************************************
                      bind_csdcollection.cpp
                             -------------------
    begin                : Nov 28 2007
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

#include "bind_csdcollection.h"
#include "bind_csd.h"

#include <kst.h>
#include <kstdataobjectcollection.h>

#include <kdebug.h>

KstBindCSDCollection::KstBindCSDCollection(KJS::ExecState *exec)
: KstBindCollection(exec, "SpectrogramCollection", true) {
  _csds = kstObjectSubList<KstDataObject,KstCSD>(KST::dataObjectList).tagNames();
}


KstBindCSDCollection::~KstBindCSDCollection() {
}


KJS::Value KstBindCSDCollection::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return KJS::Number(_csds.count());
}


QStringList KstBindCSDCollection::collection(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  return _csds;
}


KJS::Value KstBindCSDCollection::extract(KJS::ExecState *exec, const KJS::Identifier& item) const {
  KstCSDList pl = kstObjectSubList<KstDataObject,KstCSD>(KST::dataObjectList);
  KstCSDPtr p = *pl.findTag(item.qstring());

  if (p) {
    return KJS::Object(new KstBindCSD(exec, p));
  }
  return KJS::Undefined();
}


KJS::Value KstBindCSDCollection::extract(KJS::ExecState *exec, unsigned item) const {
  KstCSDList pl = kstObjectSubList<KstDataObject,KstCSD>(KST::dataObjectList);
  KstCSDPtr p;

  if (item < pl.count()) {
    p = pl[item];
  }
  if (p) {
    return KJS::Object(new KstBindCSD(exec, p));
  }
  return KJS::Undefined();
}
