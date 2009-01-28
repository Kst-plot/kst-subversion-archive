/***************************************************************************
                            combine.h
                             -------------------
    begin                : 05/13/06
    copyright            : (C) 2008 The University of British Columbia
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
#ifndef COMBINE_H
#define COMBINE_H

#include <kstbasicplugin.h>

class Combine : public KstBasicPlugin {
  Q_OBJECT
  public:
    Combine(QObject *parent, const char *name, const QStringList &args);
    virtual ~Combine();

    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;
};

#endif
