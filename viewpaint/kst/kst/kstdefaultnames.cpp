/***************************************************************************
                            kstdefaultnames.cpp
                             -------------------
    begin                : July 31, 2004
    copyright            : (C) 2003 C. Barth Netterfield
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

#include "kstdatacollection.h"
#include "kstdefaultnames.h"
#include "ksthistogram.h"
#include "kstplugin.h"
#include "kstpsd.h"
#include "kstvcurve.h"

#include <qregexp.h>
#include <klocale.h>
#include <stdio.h>

QString KST::suggestPlotName() {
  static int last = 0;
  return QString("P%1").arg(++last);
}

// takes a field name and returns a unique tag name, which will be
// the field if that is unique, or field-N if there are already N
// vectors of that name
QString KST::suggestVectorName(const QString& field) {
  QString name;
  int i=0;

  name = field;
  while (KST::vectorTagNameNotUnique(name, false)) {
    name = QString("%1-%2").arg(field).arg(++i);
  }

  return name;
}

QString suggestDataObjectName(const QString& vector_name, 
                              const QString &A, bool add_c) {
  QString name, field;
  int i=1;

  field = vector_name;


  if (add_c) {
    name = QString("%1-%2").arg(field).arg(A);
  } else {
    name = field;
  }

  while (KST::dataObjectList.findTag(name) != KST::dataObjectList.end()) {
    name = QString("%1-%2%3").arg(field).arg(A).arg(++i);
  }

  return name;
}

/* takes a vector or plugin name of the form V2-GYRO1 and returns a unique */
/* curve name of the form GYRO1, or GYRO1-N if there are already N curves  */
/* of that name.  If add_c is true, add a -C to the end, even if not */
/* adding it would appear unique.  This is important because in many */
/* dialogs, a curve is created from an object before the object has */
/* been put on the list */
QString KST::suggestCurveName( const QString& vector_name, bool add_c ) {
  return suggestDataObjectName(vector_name, 
                      i18n("Minimal abbreviation for 'Curve'", "C"), 
                      add_c);
}

QString KST::suggestPSDName( const QString& vector_name ) {
  return suggestDataObjectName(vector_name, 
                      i18n("Minimal abbreviation for 'Power spectrum'", "P"),
                      true);
}

QString KST::suggestCSDName( const QString& vector_name ) {
  return suggestDataObjectName(vector_name,
                               i18n("Minimal abbreviation for 'Cumulative Spectral Decay'", "S"),
                               true);
}

QString KST::suggestHistogramName( const QString& vector_name ) {
  return suggestDataObjectName(vector_name, 
                      i18n("Minimal abbreviation for 'Histogram'", "H"),
                      true);                                                                      
}

QString KST::suggestEQName( const QString& name_in) {
  return suggestDataObjectName(name_in, 
                      i18n("Minimal abbreviation for 'Equation'", "E"),
                      false);
}

QString KST::suggestPluginName(const QString& pname, const QString &vname) {
  QString tag;
  
  if (vname.isEmpty()) {
    tag = pname;
  } else {
    tag = vname + "-" + pname;
  }
  return suggestDataObjectName(tag, 
                      i18n("Minimal abbreviation for 'pluGin'", "G"),
                      false);
}

QString KST::suggestMatrixName(const QString& vector_name) {
  
  QString name, field;
  int i=1;

  field = vector_name;


  name = field;

  while (KST::matrixList.findTag(name) != KST::matrixList.end()) {
    name = QString("%1-%2").arg(field).arg(++i);
  }

  return name;
}

QString KST::suggestImageName(const QString& matrix_name) {
  return suggestDataObjectName(matrix_name, 
                      i18n("Minimal abbreviation for 'Image'", "I"),
                      true);
}
// vim: ts=2 sw=2 et
