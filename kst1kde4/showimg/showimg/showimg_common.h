/***************************************************************************
                         showimg_common.h  -  description
                             -------------------
    begin                : Sat Sep 23 2006
    copyright            : (C) 2001-2005 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

/*****************************************************************************
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the Free Software             *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301 *
 *   USA.                                                                    *
 *                                                                           *
 *   For license exceptions see LICENSE.EXC file, attached with the source   *
 *   code.                                                                   *
 *                                                                           *
 *****************************************************************************/

#ifndef SHOWIMG_COMMON_H
#define SHOWIMG_COMMON_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "
#define MYDEBUG       kdDebug(0)  << k_funcinfo << " " << __LINE__ << ": " 
#define MYWARNING     kdWarning(0)<< k_funcinfo << " "
#define MYBACKTRACE   MYDEBUG     << kdBacktrace() << " "

#endif /* SHOWIMG_COMMON_H */
