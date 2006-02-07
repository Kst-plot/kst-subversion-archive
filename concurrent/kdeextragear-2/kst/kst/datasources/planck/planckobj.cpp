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

//#define PIOLIBDEBUG
#include "planckobj.h"
#include <HL2_PIOLIB/PIOErr.h>

using namespace Planck;

class Planck::ObjectGroup {
  friend class Object;
  protected:
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




Object::Object() : Source() {
#ifdef PIOLIBDEBUG
  kdDebug() << ">>>>>>>>>  Object created " << (void*)this << endl;
#endif
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

    QString range;

    if (start < 0 || end < start) {
      // Leave blank
    } else {
      range = QString("Begin=%1\nEnd=%2").arg(start).arg(end);
    }

    PIOObject *MyObject;
    PIOParseData MyData;

    long n = PIORead_1(g->objNames[i], const_cast<char*>(""),
        const_cast<char*>("PIODOUBLE"),
        const_cast<char*>(range.latin1()), &(g->_group),
        &MyObject, &MyData, 0L);
#ifdef PIOLIBDEBUG
    kdDebug() << "READ " << n << " doubles." << endl;
#endif
    if (n < 0) { // error
      // FIXME - might have to reset() here
      abort();
      n = 0;
      return n;
    }

    PIORead_2(buf, 0L, 0L, g->objNames[i], const_cast<char*>("PIODOUBLE"),
        const_cast<char*>(range.latin1()),
        const_cast<char*>(""), g->_group, &MyObject, &MyData, 0L,
        PIOLONG(n));

    PIODeleteLink(buf, g->_group);

    assert(buf);
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
  kdDebug () << "Close group" << endl;
  if (_valid) {
    free(objNames);
    objNames = 0L;
    free(objTypes);
    objTypes = 0L;

    delete[] firstIndex;
    firstIndex = 0L;
    delete[] lastIndex;
    lastIndex = 0L;

    objectListSize = 0;

    PIOCloseVoidGrp(&_group);
    _group = 0L;
  }
  _valid = false;
}


bool ObjectGroup::updateObjectList() {
  if (objNames) {
    free(objNames);
    objNames = 0L;
    free(objTypes);
    objTypes = 0L;
  }

  PIOErr e = PIOGetObjectList(&objNames, &objTypes, _group);
  if (e > 0) {
    _valid = true;
    firstIndex = new PIOLONG[e];
    lastIndex = new PIOLONG[e];
    objectListSize = e;
#ifdef PIOLIBDEBUG
    kdDebug() << "               -> info acquired." << endl;
    kdDebug() << "                  -> objects: " << e << endl;
#endif
    for (int i = 0; i < e; ++i) {
      firstIndex[i] = PIOGetBeginObjectIdx(objNames[i], _group);
      lastIndex[i] = PIOGetEndObjectIdx(objNames[i], _group);
#ifdef PIOLIBDEBUG
      kdDebug() << "                  -> object: " << objNames[i] << endl;
      kdDebug() << "                    -> first index: " << (long) firstIndex[i] << endl;
      kdDebug() << "                    -> last index: " << (long) lastIndex[i] << endl;
#endif
    }
    return true;
  }
  return false;
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
