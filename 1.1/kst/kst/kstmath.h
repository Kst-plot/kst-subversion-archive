/***************************************************************************
                     kstmath.h  -  Math portability tools
                             -------------------
    begin                : Oct 20, 2004
    copyright            : (C) 2004 by The University of Toronto
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

// NOTE: only include this from .cpp files

#ifndef KSTMATH_H
#define KSTMATH_H

#include <math.h>

#ifdef __sun
#include <ieeefp.h>
#endif

/*
** For systems without NAN, this is a NAN in IEEE double format.
** Code lifted from kde screensavers.
*/
#if !defined(NAN)
static inline double nan__()
{
  static const unsigned int one = 1;
  static const bool BigEndian = (*((unsigned char *) &one) == 0);

  static const unsigned char be_nan_bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
  static const unsigned char le_nan_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };

  return *( ( const double * )( BigEndian ? be_nan_bytes : le_nan_bytes ) );
}
#define NAN (::nan__())
#endif

/*
** Both Solaris and FreeBSD-current do weird things with the
** isnan() defined in math.h - in particular on FreeBSD it
** gets #undeffed by the C++ math routines later. Use the
** std:: version in those cases.
*/
#ifdef isnan
#define KST_ISNAN(a)	isnan(a)
#elif defined(__APPLE__)
#define KST_ISNAN(a)	(a == NAN ? 1 : 0)
#else
  // HUH? Ok let's get rid of the silliness here sometime.
#define KST_ISNAN(a)	isnan(a)
#endif

#endif
// vim: ts=2 sw=2 et


