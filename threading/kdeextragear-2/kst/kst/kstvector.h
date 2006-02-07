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

#ifndef KSTVECTOR_H
#define KSTVECTOR_H

#include <qstring.h>
#include <qdom.h>
#include <qdict.h>

#include "kstobject.h"
#include "kstscalar.h"

#include <math.h>

#define IS_POINT(x) (!isnan(x))
#define NOT_POINT(x) (isnan(x))

namespace KST {
  // Do not compare against this, only assign it and use the helpers above.
  extern double NOPOINT;
}

/**A class for handling data vectors for kst.
 *@author cbn
 */

class KstScalar;
class KstVector;
typedef KstSharedPtr<KstVector> KstVectorPtr;

class KstVector : public KstObject {
public:
  /**
   * Vectors automatically add themselves to the global vector collection
   */
  KstVector(const QString& name = QString::null, int size = 0);

  virtual ~KstVector();

  virtual int length() const;

  /** Return V[i], interpolated/decimated to have ns_i total samples */
  double interpolate(int i, int ns_i);

  /** Return V[i] uninterpolated */
  double value(int i);

  /** Return a pointer to the raw vector */
  double *const value() const;

  /** Return Minimum value in Vector */
  double min() const;

  /** Return max value in Vector */
  double max() const;

  /** Return SpikeInsensitive max value in vector **/
  double ns_max() const { return _ns_max;}

  /** Return SpikeInsensitive min value in vector **/
  double ns_min() const { return _ns_min;}

  /** Return Mean value in Vector */
  double mean() const;

  /** Return Least Positive value in Vector */
  double minPos() const;

  /** Return number of samples in the vector */
  int sampleCount() const;

  /** Number of new samples in the vector since last newSync */
  int numNew() const;

  /** Number of samples  shifted since last newSync */
  int numShift() const;

  bool isRising() const { return _is_rising;}

  /** reset New Samples and Shifted samples */
  void newSync();

  /** Save vector information */
  virtual void save(QTextStream &ts);

  /** Update the vector.  Return true if there was new data. */
  virtual UpdateType update(int update_counter = -1);

  /** return a sensible label for this vector */
  virtual QString label() const;

  /** return a sensible top label.... */
  virtual QString fileLabel() const;

  virtual void resize(int sz, bool reinit = true);

  virtual void SetNewAndShift(int inNew, int inShift);

  /** Clear out the vector */
  void zero();

  virtual void setTagName(const QString& newTag);

  /** Generate a new vector [x0..x1] with n total points */
  static KstVectorPtr generateVector(double x0, double x1, int n, const QString& tag);

protected: // Protected attributes
  /** current number of samples */
  int _size;

  /** number of valid points */
  int _nsum;

  /** variables for SpikeInsensitiveAutoscale **/
  double _ns_max;
  double _ns_min;

  /** Where the vector is held */
  double *_v;

  /** number of samples shifted since last newSync */
  int NumShifted;

  /** number of new samples since last newSync */
  int NumNew;

  /** How many Curves are using this file */
  int NumUsed;

  /** Statistics Scalars */
  QDict<KstScalar> _scalars;

  /** is the vector monotonically rising */
  bool _is_rising;

  /** Scalar Maintenance methods */
  void CreateScalars();
  void RenameScalars();
  void UpdateScalars();

protected:
  friend class KstDataObject;
  virtual double* realloced(double *memptr, int newSize);
};

typedef KstObjectList<KstVectorPtr> KstVectorList;
typedef KstObjectMap<KstVectorPtr> KstVectorMap;

#endif
