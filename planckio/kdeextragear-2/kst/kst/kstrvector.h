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

#include "kstvector.h"
#include <qstring.h>
#include <qdom.h>

#include "kstfile.h"

/**A class for handling data vectors for kst.
 *@author cbn
 */

class KstRVector : public KstVector {
public:
  /** Create an RVECTOR */
  KstRVector(KstFilePtr file, const QString &field, const QString &tag,
             int f0, int n,
             int skip, bool in_doSkip,
             bool in_doAve);

  KstRVector(QDomElement &e,
             const QString &o_file="|",
             int o_n = -2, int o_f = -2,
             int o_s = -1, bool o_ave = false);

  virtual ~KstRVector();

  /** change the properties of a kstrvector */
  void change(KstFilePtr file, const QString &field, const QString &tag,
              int f0, int n, int skip,
              bool in_doSkip, bool in_doAve);

  void changeFile(KstFilePtr file);

  void changeFrames(int f0, int n, int skip,
                    bool in_doSkip, bool in_doAve);

  /** Return frames held in Vector */
  int numFrames() const;

  /** Return the requested number of frames in the vector */
  int reqNumFrames() const;

  /** Return Starting Frame of Vector */
  int startFrame() const;

  /** Return the requested starting frame of the Vector */
  int reqStartFrame() const;

  /** Return frames to skip per read */
  bool doSkip() const;
  bool doAve() const;
  int skip() const;

  /** Update the vector.  Return true if there was new data. */
  virtual UpdateType update(int update_counter = -1);

  /** Returns intrinsic samples per frame */
  int samplesPerFrame() const;

  /** Save vector information */
  virtual void save(QTextStream &ts);

  /** return the name of the file if a RVector - otherwise return "" */
  QString getFilename() const;

  /** return the field name if an RVector, other wise return "" */
  QString getField() const;

  /** return a sensible label for this vector */
  virtual QString label() const;
  virtual QString fileLabel() const;

  /** return the length of the file */
  int getFileLength() const;

  /** return whether the vector is suppose to read to end of file */
  bool readToEOF() const;

  /** read whether the vector is suppose to count back from end of file */
  bool countFromEOF() const;


  /** return true if it has a valid file and field, or false otherwise */
  bool isValid() const;

private: // Private attributes

  bool _dirty;

  /** Common contructor for an RVector */
  void  commonRVConstructor(KstFilePtr file,
			    const QString &field,
                            int f0, int n,
			    int skip, bool in_doSkip,
                            bool in_doAve);

  /** Samples Per Frame */
  int SPF;

  /** current number of frames */
  int NF;

  /** current starting frame */
  int F0;

  /** frames to skip per read */
  bool DoSkip;
  bool DoAve;
  int Skip;

  /** max number of frames */
  int ReqNF;

  /** Requested Starting Frame */
  int ReqF0;

  /** file to read for rvectors */
  KstFilePtr File;

  /** For rvector, the field.  For fvector, the equation. */
  QString Field;

  /** Number of Samples allocated to the vector */
  int _numSamples;

  int N_AveReadBuf;
  double *AveReadBuf;

  void reset();

  void checkIntegrity();
};

typedef KSharedPtr<KstRVector> KstRVectorPtr;
typedef KstObjectList<KstRVectorPtr> KstRVectorList;

#endif
