/***************************************************************************
                   stdin.h  -  data source plugin for stdin
                             -------------------
    begin                : Fri Oct 31 2003
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

#ifndef STDINSRC_H
#define STDINSRC_H

#include <kstdatasource.h>

class KTempFile;

class KstStdinSource : public KstDataSource {
public:
  KstStdinSource();

  virtual ~KstStdinSource();

  virtual KstObject::UpdateType update(int = -1);

  virtual int readField(double *v, const QString &field, int s, int n);

  virtual bool isValidField(const QString &field) const;

  virtual int samplesPerFrame(const QString &field);

  virtual int frameCount() const;

  virtual QString fileType() const;

  virtual void save(QTextStream &ts);

  virtual bool isValid() const;

private:
  KstDataSourcePtr _src;
  KTempFile *_file;
};


#endif
// vim: ts=2 sw=2 et
