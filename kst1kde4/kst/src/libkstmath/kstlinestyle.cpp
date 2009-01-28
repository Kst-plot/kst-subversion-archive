/***************************************************************************
                 kstlinestyle.cpp: defines line styles for kst
                             -------------------
    begin                : Apr 22 2004
    copyright            : (C) 2001 The University of British Columbia
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

#include "kstlinestyle.h"

Qt::PenStyle KstLineStyle[] = { 
  Qt::SolidLine, 
  Qt::DashLine, 
  Qt::DotLine, 
  Qt::DashDotLine, 
  Qt::DashDotDotLine
};

const unsigned int KSTLINESTYLE_MAXTYPE = sizeof(KstLineStyle) / sizeof (Qt::PenStyle);

