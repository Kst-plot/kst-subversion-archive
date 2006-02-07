/***************************************************************************
                            kstdatacollection.cpp
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

#include "kstdatacollection.h"

  /** The list of data sources (files) */
  KstDataSourceList KST::dataSourceList;

  /** The list of vectors that are being read */
  KstVectorList KST::vectorList;

  /** The list of Scalars which have been generated */
  KstScalarList KST::scalarList;

  /** The list of data objects which are in use */
  KstDataObjectList KST::dataObjectList;

  /** The list of filter sets defined */
  KstFilterSetList KST::filterSetList;

