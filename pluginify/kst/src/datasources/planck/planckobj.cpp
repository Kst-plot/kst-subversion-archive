/***************************************************************************
                        planckobj.cpp  -  Part of KST
                             -------------------
    begin                : Mon Oct 06 2003
    copyright            : (C) 2003 The University of Toronto
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

#include <kdebug.h>
#include <math.h>

//#define PIOLIBDEBUG
#include "planckobj.h"
#include <HL2_PIOLIB/PIOErr.h>

#ifdef NAN
double NOPOINT = NAN;
#else
double NOPOINT = 0.0/0.0; // NaN
#endif

using namespace Planck;


class Planck::ObjectGroup {
  friend class Object;
  public:
    ObjectGroup();
    ~ObjectGroup();

    bool open(const QString& groupURL);
    void close();

    bool updateObjectList();

    PIOSTRING *objTypes;
    PIOSTRING *objNames;
    PIOINT     objectListSize;

    PIOLONG *firstIndex;
    PIOLONG *lastIndex;

    PIOGroup *_group;
    bool _valid;
};


Planck::TimeType Object::typeOfTime() {
  PIOParseData MyData, MyDataFlag;
  PIOObject *obj = 0L;
  char range[128];
  Planck::ObjectGroup *g = findGroup(_group);
  assert(g);
#ifdef PIOLONG_FMT
  snprintf(range, 127, "Begin=" PIOLONG_FMT "\nEnd=" PIOLONG_FMT, *g->firstIndex, *g->firstIndex);
#else
  snprintf(range, 127, "Begin=%lld\nEnd=%lld", *g->firstIndex, *g->firstIndex);
#endif

  long n = PIORead_1(const_cast<char*>("TIMES_OF_SAMPLES"), const_cast<char*>("Written"), const_cast<char*>("PIODOUBLE"), range, &g->_group, &obj, &MyData, &MyDataFlag);

  if (n != 1) {
    n = PIORead_1(const_cast<char*>("TIMESEC"), const_cast<char*>("Written"), const_cast<char*>("PIODOUBLE"), range, &g->_group, &obj, &MyData, &MyDataFlag);
    if (n != 1) {
      return Planck::Unknown;
    } else {
      return Planck::TimeSec;
    }
  }
  return Planck::TimesOfSamples;
}


double Object::fetchTimePoint(PIOLONG idx) {
  PIOParseData MyData, MyDataFlag;
  PIOObject *obj = 0L;
  char range[128];
  long n;
#ifdef PIOLONG_FMT
  snprintf(range, 127, "Begin=" PIOLONG_FMT "\nEnd=" PIOLONG_FMT, idx, idx);
#else
  snprintf(range, 127, "Begin=%lld\nEnd=%lld", idx, idx);
#endif
  double buf[1];
  Planck::ObjectGroup *g = findGroup(_group);
  assert(g);
  static PIOFLAG Mask[1];

  switch (_tt) {
    case Planck::TimeSec:
      n = PIORead_1(const_cast<char*>("TIMESEC"), const_cast<char*>("Written"), const_cast<char*>("PIODOUBLE"), range, &g->_group, &obj, &MyData, &MyDataFlag);
      PIORead_2(buf, 0L, Mask, const_cast<char*>("TIMESEC"), const_cast<char*>("PIODOUBLE"), range, const_cast<char*>("Written"), g->_group, &obj, &MyData, &MyDataFlag, PIOLONG(n));
      break;
    case Planck::TimesOfSamples:
      n = PIORead_1(const_cast<char*>("TIMES_OF_SAMPLES"), const_cast<char*>("Written"), const_cast<char*>("PIODOUBLE"), range, &g->_group, &obj, &MyData, &MyDataFlag);
      PIORead_2(buf, 0L, Mask, const_cast<char*>("TIMES_OF_SAMPLES"), const_cast<char*>("PIODOUBLE"), range, const_cast<char*>("Written"), g->_group, &obj, &MyData, &MyDataFlag, PIOLONG(n));
      break;
    default:
      return NOPOINT;
  }

  return buf[0];
}


Object::Object() : Source() {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  Object created " << (void*)this << endl;
#endif
  _tt = Undetermined;
}


Object::~Object() {
  reset();
#ifdef PIOLIBDEBUG
  kdDebug() << "<<<<<<<<<  Object deleted " << (void*)this << endl;
#endif
}


bool Object::updated() const {
  if (_isValid) {
    ObjectGroup *tg = findGroup(_group);
    if (tg) {
      bool rc = PIOUPDATE == PIOCheckUpdateGrp(tg->_group);
      if (rc) {
        tg->updateObjectList();
      }
      return rc;
    }
  }
  return false;
}


void Object::reset() {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  Object reset" << endl;
#endif
  for (QMap<QString,ObjectGroup*>::Iterator i = _groupInfo.begin(); i != _groupInfo.end(); ++i) {
    delete i.data();
  }
  _groupInfo.clear();
  Source::reset();
}


bool Object::setGroup(const QString& group) {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  Object set group - " << group << endl;
#endif
  // FIXME: verify the source
  if (group != _group) {
    reset();
    _group = group;
  }
  _isValid = true;
  return true;
}


QStringList Object::fields() const {
  QStringList rc;
  if (_isValid) {
    const ObjectGroup *tg = findGroup(_group);
    if (tg) {
      for (PIOINT i = 0; i < tg->objectListSize; ++i) {
        rc += tg->objNames[i];
      }
    }
  }
  return rc;
}


QSize Object::range(const QString& object) const {
  if (isValid()) {
    const ObjectGroup *g = findGroup(_group);
    if (!g) {
      return QSize(0, 0);
    }

    int i;
    for (i = 0; i < g->objectListSize; ++i) {
      if (object == g->objNames[i]) {
        break;
      }
    }

    if (i == g->objectListSize) {
#ifdef PIOLIBDEBUG
      kdDebug() << "Error finding object " << object << endl;
#endif
      return QSize(0, 0);
    }

    return QSize(g->firstIndex[i], g->lastIndex[i]);
  }
return QSize(0, 0);
}


int Object::readObject(const QString& object, double *buf, long start, long end) {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  Object read object - " << _group << ":" << object << " from " << start << " to " << end << endl;
#endif
  if (isValid()) {
    ObjectGroup *g = findGroup(_group);
    if (!g) {
      return 0;
    }

    int i;
    for (i = 0; i < g->objectListSize; ++i) {
      if (object == g->objNames[i]) {
        break;
      }
    }

    if (i == g->objectListSize) {
      return 0;
    }

    QString range("");

    if (start < 0 || end < start) {
      // Leave blank
    } else {
      range = QString("Begin=%1\nEnd=%2").arg(start).arg(end);
    }

    PIOObject *MyObject = 0L;
    PIOParseData MyData;
     /* Specific data description needed to check that sample were written */
    PIOParseData MyDataFlag;

    PIOSTRING ObjName;
    sprintf(ObjName, "%s/%s", g->_group->HTMLName, g->objNames[i]);

    /* open the group only for reading this object */
    PIOGroup *MyGroup=NULL;

    long n = PIORead_1(ObjName, 
        const_cast<char*>("Written"),
        const_cast<char*>("PIODOUBLE"),
        const_cast<char*>(range.latin1()),
        &MyGroup, &MyObject, &MyData, &MyDataFlag);

#ifdef PIOLIBDEBUG
    kdDebug() << "READ " << n << " doubles." << endl;
#endif
    if (n < 0) { // error
      // FIXME - might have to reset() here
      abort();
      n = 0;
      return n;
    }

    {
      /* table to store the sample validity */
      PIOFLAG *Mask = (PIOFLAG*)_PIOMALLOC(n);

      PIORead_2(buf, 0L, Mask,
          ObjName, 
          const_cast<char*>("PIODOUBLE"),
          const_cast<char*>(range.latin1()),
          const_cast<char*>("Written"), 
          MyGroup, 
          &MyObject, 
          &MyData, 
          &MyDataFlag,
          PIOLONG(n));

      /* the group is close no need to deletelink */
      //PIODeleteLink(buf, g->_group);

      for (i = 0; i < n; i++) {
        if (Mask[i] == 0) {
          buf[i] = NOPOINT;
        }
      }

      _PIOFREE(Mask);
    }
#ifdef PIOLIBDEBUG
    kdDebug() << "Read " << n << " samples of data.  Range = [" << range << "]" << endl;
    if (n > 0) {
      kdDebug() << "this = " << (void*)this << endl;
      kdDebug() << "buf = " << (void*)buf << endl;
    }
#endif
    return int(n);
  }
  return 0;
}


ObjectGroup *Object::findGroup(const QString& group) const {
  if (_groupInfo.contains(group)) {
    return _groupInfo[group];
  }

  ObjectGroup *grp = new ObjectGroup;
  if (!grp->open(group)) {
    delete grp;
    grp = 0L;
  } else {
    _groupInfo.insert(group, grp);
  }

#ifdef PIOLIBDEBUG
  if (!grp) {
    kdDebug() << "Error finding group " << group << endl;
  }
#endif
  return grp;
}


double Object::timeForSample(int sample) {
  if (_tt == Undetermined) {
    _tt = typeOfTime();
  }

  return fetchTimePoint(sample);
}


int Object::sampleForTimeRelative(double ms, bool *ok) {
  if (_tt == Undetermined) {
    _tt = typeOfTime();
  }

  Planck::ObjectGroup *g = findGroup(_group);
  if (!g) {
    if (ok) {
      *ok = false;
    }
    return -1;
  }
  double left = fetchTimePoint(*g->firstIndex);
  if (left != left) {
    if (ok) {
      *ok = false;
    }
    return -1;
  }
  return sampleForTime(floor(left + 0.5) + ms / 1000, ok);
}


int Object::sampleForTime(double ms, bool *ok) {
  PIOLONG location = -1;
  bool found = false;

  if (_tt == Undetermined) {
    _tt = typeOfTime();
  }

  Planck::ObjectGroup *g = findGroup(_group);
  if (!g || _tt == Unknown) {
    if (ok) {
      *ok = false;
    }
    return -1;
  }

  PIOLONG FirstIndex = *g->firstIndex;
  PIOLONG LastIndex = *g->lastIndex;
  double left = fetchTimePoint(FirstIndex);
  double right = fetchTimePoint(LastIndex);
  double sch = ms;

  if (sch >= left && sch <= right) {
    if (sch == left) {
      found = true;
      location = FirstIndex;
    } else if (sch == right) {
      found = true;
      location = LastIndex;
    } else {
      while (FirstIndex < LastIndex) {
        PIOLONG point = (LastIndex - FirstIndex) / 2 + FirstIndex;
        if (LastIndex - FirstIndex < 3) {
          double left = fetchTimePoint(FirstIndex);
          double right = fetchTimePoint(LastIndex);
          double mid = fetchTimePoint(point);
          found = true;
          double ld = fabs(left - sch);
          double rd = fabs(right - sch);
          double md = fabs(mid - sch);
          if (ld <= rd && ld <= md) {
            location = FirstIndex;
          } else if (md <= ld && md <= rd) {
            location = point;
          } else if (rd <= ld && rd <= md) {
            location = LastIndex;
          } else {
            location = -1;
          }
          break;
        }

        double pt = fetchTimePoint(point);
        if (pt > sch) {
          LastIndex = point - 1;
        } else if (pt < sch) {
          FirstIndex = point + 1;
        } else if (pt == sch) {
          found = true;
          location = point;
          break;
        }
        if (LastIndex == FirstIndex) {
          found = true;
          location = FirstIndex;
        }
      }
    }
  }

  if (ok) {
    *ok = found;
  }

  return location;
}


/*
 *  We may want to consider storing with a Qt datatype instead so we can
 *  index these things and get far better performance.
 */

ObjectGroup::ObjectGroup() {
  objNames = 0L;
  objTypes = 0L;
  firstIndex = 0L;
  lastIndex = 0L;
  objectListSize = 0;

  _group = 0L;
  _valid = false;
}


ObjectGroup::~ObjectGroup() {
  close();
}


void ObjectGroup::close() {
  //kdDebug() << "Close group" << endl;
  if (_valid) {
    _PIOFREE(objNames);
    objNames = 0L;
    _PIOFREE(objTypes);
    objTypes = 0L;

    objectListSize = 0;
    _valid = false;
  }

  delete[] firstIndex;
  firstIndex = 0L;
  delete[] lastIndex;
  lastIndex = 0L;

  if (_group) {
    PIOCloseVoidGrp(&_group);
    _group = 0L;
  }
}


bool ObjectGroup::updateObjectList() {
  if (objNames) {
    _PIOFREE(objNames);
    objNames = 0L;
    _PIOFREE(objTypes);
    objTypes = 0L;
  }

  _valid = false;

  PIOErr e = PIOGetObjectList(&objNames, &objTypes, _group);
  if (e > 0) {
    _valid = true;
    delete[] firstIndex;
    delete[] lastIndex;
    firstIndex = new PIOLONG[e];
    lastIndex = new PIOLONG[e];
    objectListSize = e;

#ifdef PIOLIBDEBUG
    kdDebug() << "               -> info acquired." << endl;
    kdDebug() << "                  -> objects: " << e << endl;
#endif  
    /* there are two solution:
       - get directly only the group maximal size,
       - get the object size (it depend how KST manage index after. */
    PIOLONG LastIdx, FirstIdx;
    
    PIOGetGrpSize(&FirstIdx, &LastIdx, _group);

    for (int i = 0; i < e; ++i) { 
      firstIndex[i] = FirstIdx;
      lastIndex[i] = LastIdx;
    }
  }
  return _valid;
}


bool ObjectGroup::open(const QString& groupURL) {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  ObjectGroup open - " << groupURL << endl;
#endif

  close();

  // API bug
  _group = PIOOpenVoidGrp(const_cast<char*>(groupURL.latin1()), const_cast<char*>("r"));

  if (_group) {
#ifdef PIOLIBDEBUG
    kdDebug() << "               -> opened." << endl;
#endif
    if (!updateObjectList()) {
      PIOCloseVoidGrp(&_group);
      objectListSize = 0;
      _group = 0L;
    }
  }

  return _valid;
}

// vim: ts=2 sw=2 et
