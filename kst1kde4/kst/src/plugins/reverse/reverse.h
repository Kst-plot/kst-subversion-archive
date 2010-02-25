/***************************************************************************
                   reverse.h
                             -------------------
    begin                : 01/13/10
    copyright            : (C) 2010 The University of British Columbia
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
#ifndef REVERSE_H
#define REVERSE_H

#include <kstbasicplugin.h>

class Reverse : public KstBasicPlugin {
  Q_OBJECT
public:

  Reverse(QObject *parent, const char *name, const QStringList &args);
  Reverse(QObject *parent, const QStringList &args);
  virtual ~Reverse();

  virtual bool algorithm();

  virtual QStringList inputVectorList() const;
  virtual QStringList inputScalarList() const;
  virtual QStringList inputStringList() const;
  virtual QStringList outputVectorList() const;
  virtual QStringList outputScalarList() const;
  virtual QStringList outputStringList() const;
};

#endif
