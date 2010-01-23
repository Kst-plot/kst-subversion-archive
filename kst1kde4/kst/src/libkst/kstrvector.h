/***************************************************************************
                          kstvector.h  -  description
                             -------------------
    begin                : Fri Sep 22 2000
    copyright            : (C) 2000 by cbn
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

#ifndef KSTRVECTOR_H
#define KSTRVECTOR_H

#include <QExplicitlySharedDataPointer>

#include "kstvector.h"
#include "kstdatasource.h"
#include "kst_export.h"

namespace Equation {
  class Node;
}


class KST_EXPORT KstRVector : public KstVector {
public:
  /** Create an RVECTOR */
  KstRVector(KstDataSourcePtr file, const QString &field,
             KstObjectTag tag,
             KstFrameSize f0, KstFrameSize n,
             int skip, bool in_doSkip,
             bool in_doAve);

  KstRVector(const QDomElement &e,
             const QString &o_file="|",
             KstFrameSize o_n = -2, KstFrameSize o_f = -2,
             int o_s = -1, bool o_ave = false);

  virtual ~KstRVector();

  void reset(); // must be called with a lock
  void resetFile(KstDataSourcePtr in_file);

  /** change the properties of a kstrvector */
  void change(KstDataSourcePtr file, const QString &field,
              KstObjectTag tag,
              KstFrameSize f0, KstFrameSize n, int skip,
              bool in_doSkip, bool in_doAve);

  void changeFile(KstDataSourcePtr file);

  void changeFrames(KstFrameSize f0, KstFrameSize n, int skip,
                    bool in_doSkip, bool in_doAve);

  /** Return frames held in Vector */
  int numFrames() const;

  /** Return the requested number of frames in the vector */
  int reqNumFrames() const;

  /** Return Starting Frame of Vector */
  KstFrameSize startFrame() const;

  /** Return the requested starting frame of the Vector */
  KstFrameSize reqStartFrame() const;

  /** Return frames to skip per read */
  bool doSkip() const;
  bool doAve() const;
  int skip() const;

  /** Update the vector.  Return true if there was new data. */
  virtual UpdateType update(int update_counter = -1);

  /** Reload the contents of the vector */
  void reload();

  /** Returns intrinsic samples per frame */
  int samplesPerFrame() const;

  /** Save vector information */
  virtual void save(QTextStream &ts, const QString& indent = QString::null, bool saveAbsolutePosition = false);

  /** return the name of the file if a RVector - otherwise return "" */
  QString filename() const;

  /** return the field name if an RVector, other wise return "" */
  const QString& field() const;

  /** return a sensible label for this vector */
  virtual QString label() const;
  virtual QString fileLabel() const { return filename(); }

  /** return the length of the file */
  KstFrameSize fileLength() const;

  /** return whether the vector is suppose to read to end of file */
  bool readToEOF() const;

  /** read whether the vector is suppose to count back from end of file */
  bool countFromEOF() const;

  /** return true if it has a valid file and field, or false otherwise */
  bool isValid() const;

  /** the data source */
  KstDataSourcePtr dataSource() const;

  /** Read from end */
  void setFromEnd();

  // make a copy
  QExplicitlySharedDataPointer<KstRVector> makeDuplicate() const;

private:
  KstObject::UpdateType doUpdate(bool force = false);
  void checkIntegrity(); // must be called with a lock
  bool _dirty; // different from the KstObject dirty flag

  /** Common contructor for an RVector */
  void commonRVConstructor(KstDataSourcePtr file,
      const QString &field,
      KstFrameSize f0, KstFrameSize n,
      int skip, bool in_doSkip,
      bool in_doAve);

  int _samplesPerFrame;
  KstFrameSize _numberOfFrames;
  KstFrameSize _startingFrame;
  bool _doSkip;
  bool _doAve;
  int _skip;

  /** max number of frames */
  KstFrameSize _reqNumberFrames;

  /** Requested Starting Frame */
  KstFrameSize _reqStartingFrame;

  /** file to read for rvectors */
  KstDataSourcePtr _file;

  /** For rvector, the field.  For fvector, the equation. */
  QString _field;

  /** Number of Samples allocated to the vector */
  KstFrameSize _numSamples;

  int _nAveReadBuf;
  double *_aveReadBuf;
  bool _dontUseSkipAccel;
};

typedef QExplicitlySharedDataPointer<KstRVector> KstRVectorPtr;
typedef KstObjectList<KstRVectorPtr> KstRVectorList;

#endif
