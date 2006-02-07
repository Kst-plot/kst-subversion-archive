/***************************************************************************
                frame.h  -  data source plugin for frames
                             -------------------
    begin                : Wed Oct 22 2003
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

#ifndef FRAME_H
#define FRAME_H

#include <kstdatasource.h>


class FrameSource : public KstDataSource {
public:
  FrameSource(const QString& filename, const QString& type);

  virtual ~FrameSource();

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
  int _bytesPerFrame;
  int _framesPerFile;
  QString _rootFileName;
  long _rootExt;
  int _maxExt;
};


#endif
// vim: ts=2 sw=2 et
