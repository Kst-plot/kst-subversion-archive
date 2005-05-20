/***************************************************************************
                           kstviewobjectfactory.cpp
                             -------------------
    begin                : Apr 14, 2004
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

#include "kstviewobjectfactory.h"
#include <kdebug.h>

static KStaticDeleter<KstViewObjectFactory> sdViewObjectFactory;

KstViewObjectFactory *KstViewObjectFactory::self() {
  if (!_self) {
    sdViewObjectFactory.setObject(_self, new KstViewObjectFactory);
  }

  return _self;
}


KstViewObjectFactory::KstViewObjectFactory() {
}


KstViewObjectFactory::~KstViewObjectFactory() {
}


KstViewObjectFactory *KstViewObjectFactory::_self = 0L;


void KstViewObjectFactory::registerType(KstViewObjectPtr ptr, KstViewObjectPtr (*factory)()) {
  if (!_registry.contains(ptr->type())) {
    _registry[ptr->type()] = factory;
  }
}


KstViewObjectPtr KstViewObjectFactory::createA(const QString& type) {
  if (!_registry.contains(type)) {
    return 0L;
  }

  return (_registry[type])();
}


QString KstViewObjectFactory::typeOf(KstViewObjectPtr ptr) {
  return ptr->type();
}


// vim: ts=2 sw=2 et
