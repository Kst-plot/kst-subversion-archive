/***************************************************************************
                        plancktoi.cpp  -  Part of KST
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

#define TOIDEBUG
#include "plancktoi.h"

using namespace Planck;

class Planck::TOIGroup {
  friend class TOI;
  protected:
    TOIGroup();
    ~TOIGroup();

    bool open(const QString& groupURL);
    void close();

#ifdef KST_HAVE_PLANCK
    PIOSTRING *flagNameList;
    PIOINT     flagNameSize;

    PIOSTRING *objectType;
    PIOSTRING *objectNameList;
    PIOINT     objectListSize;

    PIOLONG *firstIndex;
    PIOLONG *lastIndex;

    PIOGroup *_group;
#endif
    bool _valid;
};




TOI::TOI() : Source() {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOI created " << (void*)this << endl;
#endif
}


TOI::~TOI() {
  reset();
#ifdef TOIDEBUG
  kdDebug() << "<<<<<<<<<  TOI deleted " << (void*)this << endl;
#endif
}


void TOI::reset() {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOI reset" << endl;
#endif
  for (QMap<QString,TOIGroup*>::Iterator i = _groupInfo.begin(); i != _groupInfo.end(); ++i) {
    delete i.data();
  }
  _groupInfo.clear();
  Source::reset();
}


bool TOI::setSource(const QString& src) {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOI set source - " << src << endl;
#endif
  reset();
  _source = src;
  if (src.startsWith("group:")) {
    QStringList parsed = QStringList::split(':', src);
    if (parsed.size() == 3) {
       return _isValid = updateGroup(parsed[2]);
    }
  }

  return false;
}


bool TOI::updateGroup(const QString& group) {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOI update group - " << group << endl;
#endif
#ifdef KST_HAVE_PLANCK
  TOIGroup *tg;
  if (_groupInfo.contains(group)) {
    tg = _groupInfo[group];
  } else {
    tg  = new TOIGroup;
  }

  if (tg->open(_source) && tg->_valid) {
    _groupInfo[group] = tg;
  } else {
    delete tg;
    _groupInfo.remove(group);
  }

  return true;
#else
  Q_UNUSED(group)
#endif
  return false;
}


QStringList TOI::fields(const QString& group) const {
  QStringList rc;
#ifdef KST_HAVE_PLANCK
  if (_groupInfo.contains(group) && _isValid) {
    TOIGroup *tg = _groupInfo[group];
    assert(tg);
    for (PIOINT i = 0; i < tg->objectListSize; ++i) {
      rc += tg->objectNameList[i];
    }
  }
#else
  Q_UNUSED(group)
#endif
  return rc;
}


QSize TOI::range(const QString& group, const QString& object) {
#ifdef KST_HAVE_PLANCK
  if (isValid()) {
    if (!_groupInfo.contains(group)) {
      return QSize(0, 0);
    }

    const TOIGroup *g = _groupInfo[group];
    assert(g);

    int i;
    for (i = 0; i < g->objectListSize; ++i) {
      if (object == g->objectNameList[i]) {
        break;
      }
    }

    if (i == g->objectListSize) {
      return QSize(0, 0);
    }

    return QSize(g->firstIndex[i], g->lastIndex[i]);
  }
#else
  Q_UNUSED(group)
  Q_UNUSED(object)
#endif
return QSize(0, 0);
}


int TOI::readObject(const QString& group, const QString& object, double *buf, long start, long end) {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOI read object - " << group << ":" << object << " from " << start << " to " << end << endl;
#endif
#ifdef KST_HAVE_PLANCK
  if (isValid()) {
    if (!_groupInfo.contains(group)) {
      return 0;
    }

    const TOIGroup *g = _groupInfo[group];
    assert(g);

    int i;
    for (i = 0; i < g->objectListSize; ++i) {
      if (object == g->objectNameList[i]) {
        break;
      }
    }

    if (i == g->objectListSize) {
      return 0;
    }

    QString range;

    if (start < 0) {
      // Leave blank
    } else {
      range = QString("Begin=%1\nEnd=%2").arg(start).arg(end);
    }

    void *data = 0;
    int n = PIOReadTOIObject(&data, g->objectNameList[i], const_cast<char*>("PIODOUBLE"), const_cast<char*>(range.latin1()), g->_group);
#ifdef TOIDEBUG
    kdDebug() << "READ " << n << " doubles." << endl;
#endif
    if (n < 0) { // error
      // FIXME - might have to reset() here
      abort();
      n = 0;
      return n;
    }
#ifdef TOIDEBUG
    kdDebug() << "Read " << n << " samples of data." << endl;
#endif
    assert(buf);
    if (data && n > 0) {
#ifdef TOIDEBUG
      kdDebug() << "this = " << (void*)this << endl;
      kdDebug() << "buf = " << (void*)buf << endl;
      kdDebug() << "data = " << (void*)data << endl;
#endif
      memcpy(buf, data, n*sizeof(double)); // argh this is so expensive
      // free(data); The manual is wrong.  Don't do this
    }
    return n;
  }
#else
  Q_UNUSED(group)
  Q_UNUSED(object)
  Q_UNUSED(buf)
  Q_UNUSED(start)
  Q_UNUSED(end)
#endif
  return 0;
}


/*
 *  We may want to consider storing with a Qt datatype instead so we can
 *  index these things and get far better performance.
 */

TOIGroup::TOIGroup() {
#ifdef KST_HAVE_PLANCK
  flagNameList = 0L;
  flagNameSize = 0;
  objectType = 0L;
  objectNameList = 0L;
  objectListSize = 0;
  firstIndex = 0L;
  lastIndex = 0L;

  _group = 0L;
#endif
  _valid = false;
}


TOIGroup::~TOIGroup() {
  close();
}


void TOIGroup::close() {
#ifdef KST_HAVE_PLANCK
  if (_valid) {
    PIOFreeInfoTOIGrp(flagNameList, objectType, objectNameList, firstIndex, lastIndex);
    flagNameList = 0L;
    objectNameList = 0L;
    objectNameList = 0L;
    firstIndex = 0L;
    lastIndex = 0L;

    PIOCloseTOIGrp(_group);
    _group = 0L;
  }
#endif
  _valid = false;
}


bool TOIGroup::open(const QString& groupURL) {
#ifdef TOIDEBUG
  kdDebug() << ">>>>>>>>>  TOIGroup open - " << groupURL << endl;
#endif
#ifdef KST_HAVE_PLANCK

  close();

  // API bug
  _group = PIOOpenTOIGrp(const_cast<char*>(groupURL.latin1()), const_cast<char*>("r"));

  if (_group) {
#ifdef TOIDEBUG
    kdDebug() << "               -> opened." << endl;
#endif
    PIOSTRING roiGroup;
    PIOErr e = PIOInfoTOIGrp(&flagNameList, &flagNameSize, &objectType, &objectNameList, &firstIndex, &lastIndex, &objectListSize, roiGroup, _group);
    if (e == 0) {
      _valid = true;
#ifdef TOIDEBUG
      kdDebug() << "               -> info acquired." << endl;
      for (int i = 0; i < objectListSize; ++i) {
        kdDebug() << "                  -> object: " << objectNameList[i] << endl;
        kdDebug() << "                    -> first index: " << (long) firstIndex[i] << endl;
        kdDebug() << "                    -> last index: " << (long) lastIndex[i] << endl;
      }
      kdDebug() << "                  -> objects: " << objectListSize << endl;
#endif
    } else {
      PIOCloseTOIGrp(_group);
      _group = 0L;
    }
  }

  return _valid;
#else
  Q_UNUSED(groupURL)
  return false;
#endif
}


// vim: ts=2 sw=2 et
