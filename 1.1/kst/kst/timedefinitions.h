/***************************************************************************
                             timdefinitions.h
                             -------------------
    begin                : Jan 20, 2005
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

#ifndef TIMEDEFINITIONS_H
#define TIMEDEFINITIONS_H

// Don't include this anywhere except datarangewidget.ui

#include <klocale.h>

struct KstTimeDef {
  const char *name;
  const char *description;
  long factor;
};

namespace KST {
KstTimeDef timeDefinitions[] = {
  { I18N_NOOP("frames"), "frames", 1 },
  { I18N_NOOP("s"), "seconds", 1 },
  { I18N_NOOP("m"), "minutes", 60 },
  { I18N_NOOP("h"), "hours", 3600 },
  { I18N_NOOP("days"), "days", 86400 },
  { I18N_NOOP("weeks"), "weeks", 604800 },
  { I18N_NOOP("months"), "months", 2628000 }, // 1/12 of a year
  { I18N_NOOP("years"), "years", 31536000 },  // 365 days
  { 0, 0, 0 }
};
}

#endif
// vim: ts=2 sw=2 et
