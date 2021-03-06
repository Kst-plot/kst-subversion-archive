/***************************************************************************
                      kstnumbersequence.h  -  Part of KST
                             -------------------
    begin                : Mon Jul 07 2003
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

#ifndef KSTNUMBERSEQUENCE_H
#define KSTNUMBERSEQUENCE_H

#include <QtGlobal>

#include "kst_export.h"

class KST_EXPORT KstNumberSequence {

  public:
    KstNumberSequence();
    KstNumberSequence(int min, int max);
    ~KstNumberSequence();

    int next();
    void reset();

    int current();
    void setRange(int min, int max);

    // numberSequence will be advanced when this sequence rolls over
    // set to NULL to have no next number sequence
    void hookToNextSequence(KstNumberSequence* numberSequence);

  private:
    int _curr;
    int _min;
    int _max;
    KstNumberSequence* _nextSequence;
};

#endif
