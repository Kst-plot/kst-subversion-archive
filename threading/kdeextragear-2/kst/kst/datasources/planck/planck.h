/***************************************************************************
                 planck.h  -  data source plugin for planck
                             -------------------
    begin                : Wed Oct 22 2003
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

#ifndef PLANCK_H
#define PLANCK_H

#include <kstdatasource.h>
#include "planckdata.h"
#include "plancktoi.h"
#include "pparser.h"


class PlanckSource : public KstDataSource {
  public:
    PlanckSource(const QString& filename, Planck::Database& db);

    virtual ~PlanckSource();

    virtual KstObject::UpdateType update(int = -1);

    virtual int readField(double *v, const QString &field, int s, int n);

    virtual bool isValidField(const QString &field) const;

    virtual int samplesPerFrame(const QString &field);

    virtual int frameCount() const;

    virtual QString fileType() const;

    virtual void save(QTextStream &ts);

  private:
    Planck::Database _db;
    KstSharedPtr<Planck::TOI> _planckTOI;
};


#endif
// vim: ts=2 sw=2 et
