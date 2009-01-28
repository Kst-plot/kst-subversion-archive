/***************************************************************************
                          trim.h
                             -------------------
    begin                : 06/16/08
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
#ifndef TRIM_H
#define TRIM_H

#include <kstbasicplugin.h>

class Trim : public KstBasicPlugin {
  Q_OBJECT
public:

  Trim(QObject *parent, const char *name, const QStringList &args);
  virtual ~Trim();

  //algorithm
  virtual bool algorithm();

  virtual QStringList inputVectorList() const;
  virtual QStringList inputScalarList() const;
  virtual QStringList inputStringList() const;
  virtual QStringList outputVectorList() const;
  virtual QStringList outputScalarList() const;
  virtual QStringList outputStringList() const;
};

#endif
