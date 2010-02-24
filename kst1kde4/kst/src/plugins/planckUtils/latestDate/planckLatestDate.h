/***************************************************************************
                   planckLatestDate.h
                             -------------------
    begin                : Sept 24, 2009
    copyright            : (C) 2009 The University of British Columbia
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

#ifndef LATESTDATE_H
#define LATESTDATE_H

#include <kstbasicplugin.h>

class PlanckLatestDate : public KstBasicPlugin {
  Q_OBJECT
  public:
    PlanckLatestDate(QObject *parent, const char *name, const QStringList &args);
    virtual ~PlanckLatestDate();

    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;

  private:
};

#endif
