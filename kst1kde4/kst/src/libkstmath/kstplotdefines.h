/***************************************************************************
                              plotdefines.h
                             ---------------
    begin                : Oct 18, 2004
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef PLOTDEFINES_H
#define PLOTDEFINES_H

#define MARKER_LABEL_PRECISION        15

#include <QStringList>
// xxx #include <klocale.h>

#ifndef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP( comment,x ) x
#endif

#define JD1900                  2415020.5
#define JD1970                  2440587.5
#define JD_RJD                  2400000.0
#define JD_MJD                  2400000.5

enum KstAxisInterpretation { 
  AXIS_INTERP_YEAR = 0,
  AXIS_INTERP_CTIME,
  AXIS_INTERP_JD,
  AXIS_INTERP_MJD,
  AXIS_INTERP_RJD,
  AXIS_INTERP_AIT,
  AXIS_INTERP_AIT_NANO,
  AXIS_INTERP_AIT_PLANCK_IDEF };

enum KstAxisDisplay { 
  AXIS_DISPLAY_YEAR = 0,
  AXIS_DISPLAY_YYMMDDHHMMSS_SS,
  AXIS_DISPLAY_DDMMYYHHMMSS_SS,
  AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS,
  AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS,
  AXIS_DISPLAY_JD,
  AXIS_DISPLAY_MJD,
  AXIS_DISPLAY_RJD,
  AXIS_DISPLAY_KDE_SHORTDATE,
  AXIS_DISPLAY_KDE_LONGDATE };

struct AxisInterpretation {
  const char *label;
  KstAxisInterpretation type;
};

struct AxisDisplay {
  const char *label;
  KstAxisDisplay type;
};

struct MajorTickSpacing {
  const char *label;
  int majorTickDensity;
};

struct TickLabelDescription {
  QString label;
  double position;
  bool minorTick;
};

const AxisInterpretation AxisInterpretations[] = {
  { QT_TR_NOOP("Julian Year"), AXIS_INTERP_YEAR },
  { QT_TR_NOOP("Standard C time"), AXIS_INTERP_CTIME },
  { QT_TRANSLATE_NOOP("Julian Date", "JD"), AXIS_INTERP_JD },
  { QT_TRANSLATE_NOOP("Modified Julian Date", "MJD"), AXIS_INTERP_MJD },
  { QT_TRANSLATE_NOOP("Reduced Julian Date", "RJD"), AXIS_INTERP_RJD },
  { QT_TRANSLATE_NOOP("Temps Atomique International", "TAI"), AXIS_INTERP_AIT },
  { QT_TRANSLATE_NOOP("Temps Atomique International - Nanoseconds", "TAI (ns)"), AXIS_INTERP_AIT_NANO },
  { QT_TRANSLATE_NOOP("Temps Atomique International - 2^-16 seconds", "TAI (2^-16 seconds)"), AXIS_INTERP_AIT_PLANCK_IDEF }
};

const AxisDisplay AxisDisplays[] = {
  { QT_TR_NOOP("Julian Year"), AXIS_DISPLAY_YEAR },
  { QT_TR_NOOP("YYYY/MM/DD HH:MM:SS.SS"), AXIS_DISPLAY_YYMMDDHHMMSS_SS },
  { QT_TR_NOOP("DD/MM/YYYY HH:MM:SS.SS"), AXIS_DISPLAY_DDMMYYHHMMSS_SS },
  { QT_TR_NOOP("<Qt Text Date> HH:MM:SS.SS"), AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS },
  { QT_TR_NOOP("<Qt Local Date> HH:MM:SS.SS"), AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS },
  { QT_TRANSLATE_NOOP("Julian Date", "JD"), AXIS_DISPLAY_JD },
  { QT_TRANSLATE_NOOP("Modified Julian Date", "MJD"), AXIS_DISPLAY_MJD },
  { QT_TRANSLATE_NOOP("Reduced Julian Date", "RJD"), AXIS_DISPLAY_RJD },
  { QT_TR_NOOP("<KDE Short Date and Time>"), AXIS_DISPLAY_KDE_SHORTDATE },
  { QT_TR_NOOP("<KDE Long Date and Time>"), AXIS_DISPLAY_KDE_LONGDATE }
};

const MajorTickSpacing MajorTickSpacings[] = {
  { QT_TR_NOOP("None"), 0 },
  { QT_TR_NOOP("Coarse"), 2 },
  { QT_TR_NOOP("Default"), 5 },
  { QT_TR_NOOP("Fine"), 10 },
  { QT_TR_NOOP("Very fine"), 15 }
};

class TickParameters {
  public:
    TickParameters() : org(0.0), tick(0.0), delta(false), labelMinor(false), maxWidth(0.0),
                       maxHeight(0.0), iHi(0), iLo(0), oppMaxWidth(0.0), oppMaxHeight(0.0)
    {}

    double org;
    double tick;
    bool delta;
    bool label;
    bool labelMinor;
    double maxWidth;
    double maxHeight;
    QList<TickLabelDescription> labels;
    QList<TickLabelDescription> labelsOpposite;
    int iHi;
    int iLo;
    double oppMaxWidth;
    double oppMaxHeight;
};

const unsigned int numAxisInterpretations = sizeof( AxisInterpretations ) / sizeof( AxisInterpretation );
const unsigned int numAxisDisplays = sizeof( AxisDisplays ) / sizeof( AxisDisplay );
const unsigned int numMajorTickSpacings = sizeof( MajorTickSpacings ) / sizeof( MajorTickSpacing );

#endif
