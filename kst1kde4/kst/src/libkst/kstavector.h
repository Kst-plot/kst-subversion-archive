/***************************************************************************
                          kstavector.h - a vector with editable points.
                             -------------------
    begin                : april, 2005
    copyright            : (C) 2005 by cbn
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
#ifndef KSTAVECTOR_H
#define KSTAVECTOR_H

#include "kstvector.h"
#include "kst_export.h"

class KstAVector : public KstVector {
 public:
  KST_EXPORT KstAVector(const QDomElement &e);
  KST_EXPORT KstAVector(int n, KstObjectTag tag);

  void save(QTextStream &ts, const QString& indent = QString::null, bool saveAbsolutePosition = false);
  KstObject::UpdateType update(int update_counter);
  void setSaveData(bool save);
};

typedef QExplicitlySharedDataPointer<KstAVector> KstAVectorPtr;
typedef KstObjectList<KstAVectorPtr> KstAVectorList;

#endif
