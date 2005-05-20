/***************************************************************************
                          kstdatacollection-gui.cpp
                             -------------------
    begin                : July 15, 2003
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

// include files for Qt
#include <qfile.h>
#include <qglobal.h>

// include files for KDE
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>

// application specific includes
#include "kst.h"
#include "kstdatacollection.h"

bool KST::dataTagNameNotUnique(const QString &tag, bool warn, void *p) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      if (warn) {
        KMessageBox::sorry(static_cast<QWidget*>(p), i18n("Empty tag names are not allowed."));
      }
      return true;
  }

  /* verify that the tag name is not used by a data object */
  KST::dataObjectList.lock().readLock();
  if (KST::dataObjectList.findTag(tag) != KST::dataObjectList.end()) {
    KST::dataObjectList.lock().readUnlock();
      if (warn) {
        KMessageBox::sorry(static_cast<QWidget*>(p), i18n("%1: this name is already in use. Change it to a unique name.").arg(tag));
      }
      return true;
  }
  KST::dataObjectList.lock().readUnlock();

  return false;
}

bool KST::vectorTagNameNotUnique(const QString &tag, bool warn, void *p) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      if (warn) {
        KMessageBox::sorry(static_cast<QWidget*>(p), i18n("Empty tag names are not allowed."));
      }
      return true;
  }

  /* verify that the tag name is not used by a data object */
  KST::vectorList.lock().readLock();
  KST::scalarList.lock().readLock();
  if (KST::vectorList.findTag(tag) != KST::vectorList.end() ||
      KST::scalarList.findTag(tag) != KST::scalarList.end()) {
    KST::scalarList.lock().readUnlock();
    KST::vectorList.lock().readUnlock();
      if (warn) {
        KMessageBox::sorry(static_cast<QWidget*>(p), i18n("%1: this name is already in use. Change it to a unique name.").arg(tag));
      }
      return true;
  }

  KST::scalarList.lock().readUnlock();
  KST::vectorList.lock().readUnlock();
  return false;
}


int KST::vectorToFile(KstVectorPtr v, QFile *f) {
  KstApp *app = KstApp::inst();
#define BSIZE 128
  char buf[BSIZE];

  v->readLock();

  int vSize = v->length();
  double *value = v->value();
  register int modval;
  QString saving = i18n("Saving vector %1").arg(v->tagName());

  modval = QMAX(vSize/100, 100);

  QString ltxt = "; " + v->tagName() + '\n';
  f->writeBlock(ltxt.ascii(), ltxt.length());
  ltxt.fill('-');
  ltxt[0] = ';';
  ltxt[1] = ' ';
  ltxt[ltxt.length() - 1] = '\n';
  f->writeBlock(ltxt.ascii(), ltxt.length());

  app->slotUpdateProgress(vSize, 0, QString::null);

  for (int i = 0; i < vSize; i++) {
    int l = snprintf(buf, BSIZE, "%.15g\n", value[i]);
    f->writeBlock(buf, l);
    if (i % modval == 0) {
      app->slotUpdateProgress(vSize, i, saving);
    }
  }

  v->readUnlock();

  app->slotUpdateProgress(0, 0, QString::null);

#undef BSIZE
  return 0;
}


int KST::vectorsToFile(const KstVectorList& vl, QFile *f, bool interpolate) {
  KstApp *app = KstApp::inst();
  int maxlen = -1;

  if (interpolate) { // code duplication is faster
    maxlen = 0;
    for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
      (*v)->readLock();
      maxlen = KMAX(maxlen, (*v)->length());
    }
  } else {
    for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
      (*v)->readLock();
      if (maxlen == -1) {
        maxlen = (*v)->length();
      } else {
        maxlen = KMIN(maxlen, (*v)->length());
      }
    }
  }

  QString saving = i18n("Saving vectors...");
  register int modval = QMAX(maxlen/100, 100);
  app->slotUpdateProgress(maxlen, 0, QString::null);

  bool first = true;
  QString ltxt = ";";
  for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
    ltxt += ' ';
    ltxt += (*v)->tagName();
  }
  ltxt += '\n';

  f->writeBlock(ltxt.ascii(), ltxt.length());
  ltxt.fill('-');
  ltxt[0] = ';';
  ltxt[1] = ' ';
  ltxt[ltxt.length() - 1] = '\n';
  f->writeBlock(ltxt.ascii(), ltxt.length());
#if QT_VERSION >= 0x030200
  ltxt.reserve(vl.count()*17);
#endif
  for (int line = 0; line < maxlen; ++line) {
    ltxt.truncate(0);
    first = true;
    for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
      if (!first) {
        ltxt += ' ';
      } else {
        first = false;
      }
      double val;
      if (interpolate) {  // might be faster to put this outside the for loops
        val = (*v)->interpolate(line, maxlen);
      } else {
        val = (*v)->value()[line];
      } 
      ltxt += QString::number(val, 'g', 15);
    }
    ltxt += "\n";
    f->writeBlock(ltxt.ascii(), ltxt.length());
    if (line % modval == 0) {
      app->slotUpdateProgress(maxlen, line, saving);
    }
  }

  for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
    (*v)->readUnlock();
  }

  app->slotUpdateProgress(0, 0, QString::null);

  return 0;
}


void KST::removeCurveFromPlots(KstBaseCurve *c) {
  Kst2DPlotList pl = Kst2DPlot::globalPlotList();
  for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
    (*i)->Curves.remove(c);
  }
}


// vim: ts=2 sw=2 et
