/***************************************************************************
                dirfile.h  -  data source plugin for dirfiles
                             -------------------
    begin                : Tue Oct 21 2003
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

#ifndef DIRFILE_H
#define DIRFILE_H

#include <kstdatasource.h>


class DirFileSource : public KstDataSource {
public:
  DirFileSource(const QString& filename, const QString& type);

  virtual ~DirFileSource();

  bool init();

  virtual KstObject::UpdateType update(int = -1);

  virtual int readField(double *v, const QString &field, int s, int n);

  virtual bool isValidField(const QString &field) const;

  virtual int samplesPerFrame(const QString &field);

  virtual int frameCount() const;

  virtual QString fileType() const;

  virtual void save(QTextStream &ts);

private:
  int _frameCount;
};


#endif
// vim: ts=2 sw=2 et
