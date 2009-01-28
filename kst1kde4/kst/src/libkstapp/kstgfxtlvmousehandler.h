/***************************************************************************
                  kstgfxtlvmousehandler.h  -  Part of KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 The University of British Columbia
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

#ifndef KSTGFXTLVMOUSEHANDLER_H
#define KSTGFXTLVMOUSEHANDLER_H

#include "kstgfxmousehandler.h"

class KstGfxTLVMouseHandler : public KstGfxMouseHandler {
  public:
    KstGfxTLVMouseHandler();
    ~KstGfxTLVMouseHandler();

    void applyDefaults(KstTopLevelViewPtr view);
    void pressMove(KstTopLevelViewPtr view, const QPoint& pos, bool shift, const QRect& geom);
    void releasePress(KstTopLevelViewPtr view, const QPoint& pos, bool shift);
};

#endif
