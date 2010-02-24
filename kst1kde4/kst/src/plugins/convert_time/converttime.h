/***************************************************************************
                          converttime.h
                             -------------------
    begin                : July 2nd 2008
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
#ifndef SHIFT_H
#define SHIFT_H

#include <kstbasicplugin.h>

class ConvertTime : public KstBasicPlugin {
  Q_OBJECT
  public:
    ConvertTime(QObject *parent, const char *name, const QStringList &args);
    ConvertTime(QObject *parent, const QStringList &args);
    virtual ~ConvertTime();

    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;
};

#endif
