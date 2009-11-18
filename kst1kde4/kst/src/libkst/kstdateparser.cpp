/***************************************************************************
                              kstdateparser.cpp
                             -------------------
    begin                : Nov 07, 2005
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

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <QStringList>

#include "kstdateparser.h"

namespace KST {

KDateTime millisecondsToExtDateTime(double ms) {
  KDateTime edt;
  edt.setTime_t(0);
  if (ms > 0.0) {
    double milli = fmod(ms, 1000.0);
    ms = (ms - milli) / 1000.0;
    assert(ms < 60*365*24*60*60); // we can't handle big dates yet
    // this will have to change when we do
    edt.setTime_t(int(ms));
    QTime t = edt.time();
    t.setHMS(t.hour(), t.minute(), t.second(), int(milli));
    edt.setTime(t);
  } if (ms < 0.0) {
    abort(); // unhandled at this point
  }
  return edt;
}


double extDateTimeToMilliseconds(const KDateTime& edt) {
  double rc = 0.0;
  if (edt.isNull()) {
    return rc;
  }

  int year = edt.date().year();
  if (year > 1969 && year < 2030) { // fix later
    rc = 1000.0 * edt.toTime_t() + edt.time().msec();
  } else {
    // Manually construct rc
    abort();
  }
  return rc;
}


double DateTimeUTCToTime_t(const KDateTime& edt) {
  time_t timeUTC;
  time_t timeLocal;
  tm brokenDown;
  tm tmUTC;

  brokenDown.tm_sec = edt.time().second();
  brokenDown.tm_min = edt.time().minute();
  brokenDown.tm_hour = edt.time().hour();
  brokenDown.tm_mday = edt.date().day();
  brokenDown.tm_mon = edt.date().month() - 1;
  brokenDown.tm_year = edt.date().year() - 1900;
  brokenDown.tm_isdst = 0;
  timeLocal = mktime( &brokenDown );

  tmUTC = *gmtime( &timeLocal );
  tmUTC.tm_isdst = 0;
  timeUTC = mktime( &tmUTC );
  timeUTC -= 2 * ( timeUTC - timeLocal );

  int secsSince1Jan1970UTC = (int)timeUTC;
  if ( secsSince1Jan1970UTC < -1 ) {
    secsSince1Jan1970UTC = -1;
  }

  return (uint)secsSince1Jan1970UTC;
}


KDateTime parsePlanckDate(const QString& dateString, bool applyOffset) {
  QStringList secondSplit = dateString.split('.');
  if (secondSplit.isEmpty() || secondSplit.count() > 2) {
    return KDateTime();
  }

  int seconds = 0;
  int offset = 0;

  if (secondSplit.count() > 1) {
    seconds = secondSplit[1].toUInt();
  }

  QStringList mainSplit = secondSplit[0].split(':');
  KDateTime edt = KDateTime::currentUtcDateTime();

  if (applyOffset) {
    offset = KDateTime::currentUtcDateTime().toTime_t() - edt.toTime_t();
  }

  QDate d = edt.date();
  QTime t = edt.time();
  int i = 0;
  
  switch (mainSplit.count()) {
    default:
      return KDateTime();
    case 5:
      {
        int years = mainSplit[i++].toInt();
        if (years < 100) {
          if (years < 0) {
            years = 1970 - years;
          } else {
            years += 2000;
          }
        }
        d.setYMD(years, d.month(), d.day());
      }
    case 4:
      {
        unsigned month = mainSplit[i++].toUInt();
        d.setYMD(d.year(), month, d.day());
      }
    case 3:
      {
        unsigned day = mainSplit[i++].toInt();
        d.setYMD(d.year(), d.month(), day);
      }
      edt.setDate(d);
    case 2:
      {
        unsigned hour = mainSplit[i++].toInt();
        t.setHMS(hour, t.minute(), t.second());
      }
    case 1:
      {
        unsigned minute = mainSplit[i].toInt();
        t.setHMS(t.hour(), minute, t.second());
      }
    case 0:
      t.setHMS(t.hour(), t.minute(), seconds);
      edt.setTime(t);
      break;
  }
  return edt.addSecs(-offset);
}

}
