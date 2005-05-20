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

/**A vector  with n editable pts
 *@author cbn
 */
class KstAVector : public KstVector {
 public:
  KstAVector(const QDomElement &e);
  KstAVector(int n, const QString& tag);

  virtual void save(QTextStream &ts, const QString& indent = QString::null, bool saveAbsolutePosition = false);

  virtual KstObject::UpdateType update(int update_counter);
};

typedef KstSharedPtr<KstAVector> KstAVectorPtr;
typedef KstObjectList<KstAVectorPtr> KstAVectorList;



#endif
// vim: ts=2 sw=2 et
