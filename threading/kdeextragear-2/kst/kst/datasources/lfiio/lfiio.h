/***************************************************************************
                  lfiio.h  -  data source plugin template
                             -------------------
    begin                : Fri Oct 17 2003
    copyright            : (C) 2003 The University of British Columbia
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

#ifndef LFIIO_H
#define LFIIO_H

#include <kstdatasource.h>


class LFIIOSource : public KstDataSource {
public:
  LFIIOSource(const QString& filename, const QString& type);

  virtual ~LFIIOSource();
  virtual bool initFile();
  virtual KstObject::UpdateType update(int = -1);
  virtual int readField(double *v, const QString &field, int s, int n);
  virtual bool isValidField(const QString &field) const;
  virtual int samplesPerFrame(const QString &field);
  virtual int frameCount() const;
  virtual QString fileType() const;
  virtual void save(QTextStream &ts);

private:
  bool        getColNumber( const QString& field, int* piColNumber ) const;

  QStringList _strListColNames;
  int         _numFrames;
  int         _numCols;
};


#endif
// vim: ts=2 sw=2 et
