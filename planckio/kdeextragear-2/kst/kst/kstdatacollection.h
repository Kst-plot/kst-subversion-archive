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

#include <qfile.h>
#include "kstfile.h"
#include "kstvector.h"
#include "kstscalar.h"
#include "kstdataobject.h"

namespace KST {
  /** The list of files which are being looked at. */
  extern KstFileList fileList;

  /** The list of vectors that are being read */
  extern KstVectorList vectorList;

  /** The list of Scalars which have been generated */
  extern KstScalarList scalarList;

  /** The list of data objects which are in use */
  extern KstDataObjectList dataObjectList;

  /** check that a tag has not been used by any other tags */
  extern bool dataTagNameNotUnique(const QString &tag, bool warn = true);
  extern bool vectorTagNameNotUnique(const QString &tag, bool warn = true);

  /** Save a vector to a file */
  extern int vectorToFile(KstVectorPtr v, QFile *f);
}

#endif
