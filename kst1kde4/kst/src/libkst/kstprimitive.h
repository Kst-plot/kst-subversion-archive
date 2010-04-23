/***************************************************************************
                               kstprimitive.h
                             -------------------
    begin                : Tue Jun 20 2006
    copyright            : Copyright (C) 2006, The University of Toronto
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

#ifndef KSTPRIMITIVE_H
#define KSTPRIMITIVE_H

#include <qpointer.h>

#include "kst_export.h"
#include "kstobject.h"

class KST_EXPORT KstPrimitive : public KstObject {
  public:
    KstPrimitive(KstObject* provider = 0L);
    KstPrimitive(const KstPrimitive& primitive);
    ~KstPrimitive();

  public:
    inline KstObjectPtr provider() const { return KstObjectPtr(_provider); }

    /** Update the primitive via the provider and/or internalUpdate().
        Return true if there was new data. */
    UpdateType update(int update_counter = -1);

  protected:
    virtual KstObject::UpdateType internalUpdate(KstObject::UpdateType providerRC);

    /** Possibly null.  Be careful, this is non-standard usage of a KShared.
     * The purpose of this is to trigger hierarchical updates properly.
     */
    QPointer<KstObject> _provider;
};

typedef QExplicitlySharedDataPointer<KstPrimitive> KstPrimitivePtr;
typedef KstObjectList<KstPrimitivePtr> KstPrimitiveList;
typedef KstObjectMap<KstPrimitivePtr> KstPrimitiveMap;

#endif
