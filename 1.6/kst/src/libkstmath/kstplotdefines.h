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

#include <qstringlist.h>
#include <klocale.h>

#ifndef I18N_NOOP2
#define I18N_NOOP2( comment,x ) x
#endif

enum KstAxisInterpretation { 
  AXIS_INTERP_YEAR,
  AXIS_INTERP_CTIME,
  AXIS_INTERP_JD,
  AXIS_INTERP_MJD,
  AXIS_INTERP_RJD,
  AXIS_INTERP_AIT,
  AXIS_INTERP_AIT_NANO };

enum KstAxisDisplay { 
  AXIS_DISPLAY_YEAR,
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
  { I18N_NOOP("Julian Year"), AXIS_INTERP_YEAR },
  { I18N_NOOP("Standard C time"), AXIS_INTERP_CTIME },
  { I18N_NOOP2("Julian Date", "JD"), AXIS_INTERP_JD },
  { I18N_NOOP2("Modified Julian Date", "MJD"), AXIS_INTERP_MJD },
  { I18N_NOOP2("Reduced Julian Date", "RJD"), AXIS_INTERP_RJD },
  { I18N_NOOP2("Temps Atomique International", "TAI"), AXIS_INTERP_AIT },
  { I18N_NOOP2("Temps Atomique International - Nanoseconds", "TAI (ns)"), AXIS_INTERP_AIT_NANO }
};

const AxisDisplay AxisDisplays[] = {
  { I18N_NOOP("Julian Year"), AXIS_DISPLAY_YEAR },
  { I18N_NOOP("YYYY/MM/DD HH:MM:SS.SS"), AXIS_DISPLAY_YYMMDDHHMMSS_SS },
  { I18N_NOOP("DD/MM/YYYY HH:MM:SS.SS"), AXIS_DISPLAY_DDMMYYHHMMSS_SS },
  { I18N_NOOP("<Qt Text Date> HH:MM:SS.SS"), AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS },
  { I18N_NOOP("<Qt Local Date> HH:MM:SS.SS"), AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS },
  { I18N_NOOP2("Julian Date", "JD"), AXIS_DISPLAY_JD },
  { I18N_NOOP2("Modified Julian Date", "MJD"), AXIS_DISPLAY_MJD },
  { I18N_NOOP2("Reduced Julian Date", "RJD"), AXIS_DISPLAY_RJD },
  { I18N_NOOP("<KDE Short Date and Time>"), AXIS_DISPLAY_KDE_SHORTDATE },
  { I18N_NOOP("<KDE Long Date and Time>"), AXIS_DISPLAY_KDE_LONGDATE }
};

const MajorTickSpacing MajorTickSpacings[] = {
  { I18N_NOOP("Coarse"), 2 },
  { I18N_NOOP("Default"), 5 },
  { I18N_NOOP("Fine"), 10 },
  { I18N_NOOP("Very fine"), 15 }
};

class TickParameters {
  public:
    // FIXME: use reasonable defaults
    TickParameters() : org(0.0), tick(0.0), delta(false), labelMinor(false), maxWidth(0.0),
                       maxHeight(0.0), iHi(0), iLo(0), oppMaxWidth(0.0), oppMaxHeight(0.0)
    {}

    double org;
    double tick;
    bool delta;
    bool labelMinor;
    double maxWidth;
    double maxHeight;
    QValueList<TickLabelDescription> labels;
    QValueList<TickLabelDescription> labelsOpposite;
    int iHi;
    int iLo;
    double oppMaxWidth;
    double oppMaxHeight;
};

const unsigned int numAxisInterpretations = sizeof( AxisInterpretations ) / sizeof( AxisInterpretation );
const unsigned int numAxisDisplays = sizeof( AxisDisplays ) / sizeof( AxisDisplay );
const unsigned int numMajorTickSpacings = sizeof( MajorTickSpacings ) / sizeof( MajorTickSpacing );

#endif
