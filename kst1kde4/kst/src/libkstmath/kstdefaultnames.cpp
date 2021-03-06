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

#include <stdio.h>

#include <QRegExp>

#include "kstdataobjectcollection.h"
#include "kstdatacollection.h"
#include "kstdefaultnames.h"
#include "ksthistogram.h"
#include "kstcplugin.h"
#include "kstpsd.h"
#include "kstvcurve.h"

QString KST::suggestPlotName() {
  static int last = 0;

  QString tag;

  do {
    tag = QString("P%1").arg(++last);
  } while (KstData::self()->viewObjectNameNotUnique(tag));

  return tag;
}


QString suggestDataObjectName(const QString& field, 
                              const QString &A, bool add_c) {
  QString name;
  int i=1;
  QString cleanedField = KstObjectTag::cleanTag(field);

  if (add_c) {
    name = QString("%1-%2").arg(cleanedField).arg(A);
  } else {
    name = cleanedField;
  }

  while (KST::dataObjectList.findTag(name) != KST::dataObjectList.end()) {
    name = QString("%1-%2%3").arg(cleanedField).arg(A).arg(++i);
  }

  return name;
}


/* takes a vector or plugin name of the form V2-GYRO1 and returns a unique */
/* curve name of the form GYRO1, or GYRO1-N if there are already N curves  */
/* of that name.  If add_c is true, add a -C to the end, even if not */
/* adding it would appear unique.  This is important because in many */
/* dialogs, a curve is created from an object before the object has */
/* been put on the list */
QString KST::suggestCurveName( KstObjectTag vector_name, bool add_c ) {
  return suggestDataObjectName(vector_name.displayString(), 
                      QObject::tr("Minimal abbreviation for 'Curve'", "C"), 
                      add_c);
}


QString KST::suggestPSDName( KstObjectTag vector_name ) {
  return suggestDataObjectName(vector_name.tag(), 
                      QObject::tr("Minimal abbreviation for 'Power spectrum'", "P"),
                      true);
}


QString KST::suggestCSDName( KstObjectTag vector_name ) {
  return suggestDataObjectName(vector_name.tag(),
                               QObject::tr("Minimal abbreviation for 'Spectrogram'", "S"),
                               true);
}


QString KST::suggestHistogramName( KstObjectTag vector_name ) {
  return suggestDataObjectName(vector_name.tag(), 
                      QObject::tr("Minimal abbreviation for 'Histogram'", "H"),
                      true);
}

QString KST::suggestVectorViewName( KstObjectTag vector_name ) {
  return suggestDataObjectName(vector_name.tag(), 
                      QObject::tr("Minimal abbreviation for 'VectorView'", "V"),
                      true);
}


QString KST::suggestEQName(const QString& name_in) {
  return suggestDataObjectName(name_in, 
                      QObject::tr("Minimal abbreviation for 'Equation'", "E"),
                      false);
}


QString KST::suggestPluginName(const QString& pname, KstObjectTag vname) {
  QString tag;

  if (!vname.isValid()) {
    tag = pname;
  } else {
    tag = vname.tag() + "-" + pname;
  }
  return suggestDataObjectName(tag, 
                      QObject::tr("Minimal abbreviation for 'pluGin'", "G"),
                      false);
}


QString KST::suggestImageName(KstObjectTag matrix_name) {
  return suggestDataObjectName(matrix_name.tag(), 
                      QObject::tr("Minimal abbreviation for 'Image'", "I"),
                      true);
}

