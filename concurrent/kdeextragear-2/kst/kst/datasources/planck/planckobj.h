/***************************************************************************
                         planckobj.h  -  Part of KST
                             -------------------
    begin                : Mon Oct 06 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef _KST_PLANCKOBJ_H
#define _KST_PLANCKOBJ_H


#include "planckdata.h"
#include <qmap.h>


namespace Planck {

class ObjectGroup;

class Object : public Source {
  public:
    Object();
    virtual ~Object();

    virtual bool setGroup(const QString& group);
 
    virtual QStringList fields() const;

    virtual void reset();

    virtual bool updated() const;

    virtual int readObject(const QString& object, double *buf, long start, long end);

    // FIXME: QSize is a bogus class to use here
    virtual QSize range(const QString& object) const;

  private:
    // We lazy load the groups, hence mutable
    mutable QMap<QString, ObjectGroup*> _groupInfo;
    ObjectGroup *findGroup(const QString& group) const;
};

}

#endif

// vim: ts=2 sw=2 et
