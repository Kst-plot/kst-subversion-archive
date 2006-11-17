/***************************************************************************
                   linear_unweighted.h
                             -------------------
    begin                : 11/15/06
    copyright            : (C) 2006 The University of Toronto
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
#ifndef LINEAR_UNWEIGHTED_H
#define LINEAR_UNWEIGHTED_H

#include <kstbasicplugin.h>

class LinearUnweighted : public KstBasicPlugin {
  Q_OBJECT
  public:
    LinearUnweighted(QObject *parent, const char *name, const QStringList &args);
    virtual ~LinearUnweighted();

    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;

  protected:
    virtual QString parameterName(int index) const;
};

#endif
