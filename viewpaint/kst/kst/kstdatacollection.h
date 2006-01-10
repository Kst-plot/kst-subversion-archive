/***************************************************************************
                             kstdatacollection.h
                             -------------------
    begin                : June 12, 2003
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

#ifndef KSTDATACOLLECTION_H
#define KSTDATACOLLECTION_H

#include "kstdataobject.h"
#include "kstdatasource.h"
#include "kststring.h"
#include "kst_export.h"

class QFile;
class KstBaseCurve;

namespace KST {
  /** The list of data sources (files) */
  KST_EXPORT extern KstDataSourceList dataSourceList;

  /** The list of vectors that are being read */
  KST_EXPORT extern KstVectorList vectorList;

  /** The list of Scalars which have been generated */
  KST_EXPORT extern KstScalarList scalarList;

  /** The list of Strings */
  KST_EXPORT extern KstStringList stringList;
  
  /** The list of matrices that are being read */
  KST_EXPORT extern KstMatrixList matrixList;

  /** The list of data objects which are in use */
  KST_EXPORT extern KstDataObjectList dataObjectList;

  /** check that a tag has not been used by any other tags */
  KST_EXPORT extern bool tagNameNotUnique(const QString& tag, bool warn = true, void *parent = 0L);
  KST_EXPORT extern bool dataTagNameNotUnique(const QString& tag, bool warn = true, void *parent = 0L);
  KST_EXPORT extern bool vectorTagNameNotUnique(const QString& tag, bool warn = true, void *parent = 0L);
  KST_EXPORT extern bool vectorTagNameNotUniqueInternal(const QString& tag);
  KST_EXPORT extern bool matrixTagNameNotUnique(const QString& tag, bool warn = true, void *parent = 0L);
  KST_EXPORT extern bool matrixTagNameNotUniqueInternal(const QString& tag);

  KST_EXPORT extern void removeCurveFromPlots(KstBaseCurve *c); // no sharedptr here

  /** Save a vector to a file */
  KST_EXPORT extern int vectorToFile(KstVectorPtr v, QFile *f);
  KST_EXPORT extern int vectorsToFile(const KstVectorList& l, QFile *f, bool interpolate);

  KST_EXPORT extern void addMatrixToList(KstMatrixPtr m);
  KST_EXPORT extern void addVectorToList(KstVectorPtr v);
  KST_EXPORT extern void addDataObjectToList(KstDataObjectPtr d);

  /** Bad choice for location - maybe move it later */
  KST_EXPORT void *malloc(size_t size);
  KST_EXPORT void *realloc(void *ptr, size_t size);

}

#endif
// vim: ts=2 sw=2 et
