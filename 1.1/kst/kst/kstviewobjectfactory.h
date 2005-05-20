/***************************************************************************
                   kstviewobjectfactory.h: factory for view objects
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

#ifndef KSTVIEWOBJECTFACTORY_H
#define KSTVIEWOBJECTFACTORY_H

#include "kstviewobject.h"

#include <kstaticdeleter.h>

class KstViewObjectFactory {
  friend class KStaticDeleter<KstViewObjectFactory>;
  public:
    static KstViewObjectFactory *self();

    KstViewObjectPtr createA(const QString& type);

    QString typeOf(KstViewObjectPtr ptr);

    void registerType(KstViewObjectPtr ptr, KstViewObjectPtr (*factory)());

  protected:
    KstViewObjectFactory();
    ~KstViewObjectFactory();

    static KstViewObjectFactory *_self;

  private:
    QMap<QString, KstViewObjectPtr (*)()> _registry;
};


#endif
// vim: ts=2 sw=2 et
