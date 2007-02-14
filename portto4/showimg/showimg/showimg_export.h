/***************************************************************************
                ShowImg import/export macros
    begin                : 2004-11-30
    copyright            : (C) 2004-2005 by Jaroslaw Staniek
    email                : js@iidea.pl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   as published by the Free Software Foundation; either version 2        *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 ***************************************************************************/

#ifndef SHOWIMG_EXP_H
#define SHOWIMG_EXP_H

#ifdef KDEMACROS_USABLE
#include <kdemacros.h>

#ifndef KDE_IMPORT
#define KDE_IMPORT KDE_EXPORT
#endif

#ifdef MAKE_SHOWIMGCORE_LIB
# define SHOWIMGCORE_EXPORT KDE_EXPORT
#else
# define SHOWIMGCORE_EXPORT KDE_IMPORT
#endif

#else  /* KDEMACROS_USABLE */

#define SHOWIMGCORE_EXPORT 

#endif /* KDEMACROS_USABLE */

#endif /* SHOWIMG_EXP_H */

