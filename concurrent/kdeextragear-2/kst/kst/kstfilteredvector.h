/***************************************************************************
                             kstfilteredvector.h
                             -------------------
    begin                : Mon Dec 15 2003
    copyright            : (C) 2003 by The University of Toronto
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

#ifndef KSTFILTEREDVECTOR_H
#define KSTFILTEREDVECTOR_H

#include "kstfilter.h"
#include "kstvector.h"

class KstFilteredVector : public KstVector {
public:
  KstFilteredVector(KstVectorPtr parent, KstFilterSetPtr filterSet);
  KstFilteredVector(QDomElement &e);
  virtual ~KstFilteredVector();

  /** Update the vector.  Return true if there was new data. */
  virtual UpdateType update(int update_counter = -1);

  virtual void save(QTextStream& ts);

protected:
  KstVectorPtr _parent;
  KstFilterSetPtr _filterSet;
};

typedef KstSharedPtr<KstFilteredVector> KstFilteredVectorPtr;
typedef KstObjectList<KstFilteredVectorPtr> KstFilteredVectorList;

#endif

// vim: ts=2 sw=2 et
