/***************************************************************************
                         plancktoi.h  -  Part of KST
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

#ifndef _KST_PLANCKTOI_H
#define _KST_PLANCKTOI_H


#include "planck.h"
#include <qmap.h>


namespace Planck {

class TOIGroup;

class TOI : public Source {
  public:
    TOI();
    virtual ~TOI();

    virtual bool setSource(const QString& src);
 
    virtual QStringList fields(const QString& group) const;

    virtual void reset();

    virtual bool updateGroup(const QString& group);

    virtual int readObject(const QString& group, const QString& object, double *buf, long start, long end);

    // FIXME: QSize is a bogus class to use here
    virtual QSize range(const QString& group, const QString& object);

  private:
    QMap<QString, TOIGroup*> _groupInfo;
};

}

#endif

// vim: ts=2 sw=2 et
