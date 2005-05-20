/***************************************************************************
                  kstplotimagedrag.h: class for drag objects
                             -------------------
    begin                : Mar 17, 2005
    copyright            : (C) 2005 The University of Toronto
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

#ifndef KSTPLOTIMAGEDRAG_H
#define KSTPLOTIMAGEDRAG_H

#include "kstdrag.h"
#include "kstviewobject.h"

class KstPlotImageDrag : public KstDrag {
  public:
    KstPlotImageDrag(QWidget *dragSource);
    virtual ~KstPlotImageDrag();

    virtual const char *format(int i = 0) const;
    void setPlots(const KstViewObjectList& l);
    virtual QByteArray encodedData(const char *mimeType) const;
    virtual bool provides(const char *mimeType) const;

  private:
    mutable KstViewObjectList _plots; // hmm, not sure if this is good
    QStringList _mimeTypes;
};

#endif
// vim: ts=2 sw=2 et
