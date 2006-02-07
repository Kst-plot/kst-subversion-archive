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

#include "kstdatacollection.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <kprogress.h>
#include <kapplication.h>

bool KST::dataTagNameNotUnique(const QString &tag, bool warn) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      if (warn)
        KMessageBox::sorry(0L, i18n("Empty tag names are not allowed."));
      return true;
  }

  /* verify that the tag name is not used by a data object */
  if (KST::dataObjectList.findTag(tag) != KST::dataObjectList.end()) {
      if (warn)
        KMessageBox::sorry(0L, i18n("%1: this name is already in use. Change it to a unique name.").arg(tag));
      return true;
  }

  return false;
}

bool KST::vectorTagNameNotUnique(const QString &tag, bool warn) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      if (warn)
        KMessageBox::sorry(0L, i18n("Empty tag names are not allowed."));
      return true;
  }

  /* verify that the tag name is not used by a data object */
  if (KST::vectorList.findTag(tag) != KST::vectorList.end() ||
      KST::scalarList.findTag(tag) != KST::scalarList.end()) {
      if (warn)
        KMessageBox::sorry(0L, i18n("%1: this name is already in use. Change it to a unique name.").arg(tag));
      return true;
  }

  return false;
}


int KST::vectorToFile(KstVectorPtr v, QFile *f) {
int rc = 0;
#define BSIZE 128
char buf[BSIZE];
int _size = v->length();
double *_v = v->value();
register int modval;
KProgressDialog *kpd = new KProgressDialog(0L, "vector save", i18n("Saving Vector"), i18n("Saving vector %1...").arg(v->tagName()));

  kpd->setAllowCancel(false);

  kpd->progressBar()->setTotalSteps(_size);
  kpd->show();

  modval = QMAX(_size/100, 100);

  for (int i = 0; i < _size; i++) {
    int l = snprintf(buf, BSIZE, "%d %g\n", i, _v[i]);
    f->writeBlock(buf, l);
    if (i % 100 == 0) {
      kpd->progressBar()->setProgress(i);
      kapp->processEvents();
    }
  }
  kpd->progressBar()->setProgress(_size);

delete kpd;

#undef BSIZE
return rc;
}


