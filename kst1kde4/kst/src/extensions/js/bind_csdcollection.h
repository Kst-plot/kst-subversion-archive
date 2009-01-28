/***************************************************************************
                       bind_csdcollection.h
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

#ifndef BIND_CSDCOLLECTION_H
#define BIND_CSDCOLLECTION_H

#include "bind_collection.h"

#include <kstcsd.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class SpectrogramCollection
   @inherits Collection
   @description An array of Spectrograms.
*/
class KstBindCSDCollection : public KstBindCollection {
  public:
    KstBindCSDCollection(KJS::ExecState *exec);
    ~KstBindCSDCollection();

    virtual KJS::Value length(KJS::ExecState *exec) const;

    virtual QStringList collection(KJS::ExecState *exec) const;
    virtual KJS::Value extract(KJS::ExecState *exec, const KJS::Identifier& item) const;
    virtual KJS::Value extract(KJS::ExecState *exec, unsigned item) const;

  protected:
    QStringList _csds;
};

#endif

