/***************************************************************************
                   crosspowerspectrum.h
                             -------------------
    begin                : 09/08/06
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
#ifndef CROSSPOWERSPECTRUM_H
#define CROSSPOWERSPECTRUM_H

#include <kstdataobject.h>

class CrossPowerSpectrum : public KstDataObject {
  Q_OBJECT
public:

    CrossPowerSpectrum(QObject *parent, const char *name, const QStringList &args);
    virtual ~CrossPowerSpectrum();

    virtual KstObject::UpdateType update(int);
    virtual QString propertyString() const;
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&);

protected slots:
  virtual void _showDialog();
};

#endif
