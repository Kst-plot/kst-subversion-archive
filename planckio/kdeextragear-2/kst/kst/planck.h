/***************************************************************************
                           planck.h  -  Part of KST
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

#ifndef _KST_PLANCK_H
#define _KST_PLANCK_H

#include <config.h>

#include <ksharedptr.h>

#ifdef KST_HAVE_PLANCK
#include <HL2_PIOLIB/PIOLib.h>
#endif

#include <qsize.h>
#include <qstring.h>
#include <qstringlist.h>

namespace Planck {
    extern bool havePlanck();

class Source : public KShared {
  public:
    Source();
    virtual ~Source();

    virtual bool isValid();

    virtual bool setSource(const QString& src);
    
    virtual QStringList fields(const QString& group) const;

    virtual void reset();

    /**
     *  @returns true if something changed (including the group disappearing)
     */
    virtual bool updateGroup(const QString& group);

    virtual int readObject(const QString& group, const QString& object, double *buf, long start, long end);

    virtual QSize range(const QString& group, const QString& object);

  protected:
    bool _isValid;
    QString _source;
};

}

#endif

// vim: ts=2 sw=2 et
