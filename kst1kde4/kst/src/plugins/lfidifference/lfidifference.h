/***************************************************************************
                   syncbin.h
                             -------------------
    begin                : 12/07/06
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
#ifndef SYNCBIN_H
#define SYNCBIN_H

#include <kstbasicplugin.h>

class LFIDifference : public KstBasicPlugin {
  Q_OBJECT
  public:
    LFIDifference(QObject *parent, const char *name, const QStringList &args);
    LFIDifference(QObject *parent, const QStringList &args);
    virtual ~LFIDifference();

    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;
};

#endif
