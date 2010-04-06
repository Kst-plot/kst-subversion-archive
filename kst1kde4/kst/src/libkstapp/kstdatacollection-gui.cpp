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

#include <QFile>
#include <QMessageBox>

#include "kst.h"
#include "kst2dplot.h"
#include "kstdatacollection-gui.h"
#include "kstdataobjectcollection.h"
#include "kstviewwindow.h"


KstGuiData::KstGuiData()
: KstData() {
}


KstGuiData::~KstGuiData() {
}


bool KstGuiData::dataTagNameNotUnique(const QString &tag, bool warn, void *p) {
  //
  // verify that the tag name is not empty
  //

  if (tag.trimmed().isEmpty()) {
    if (warn) {
      QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("Empty tag names are not allowed."));
    }
    return true;
  }

  //
  // verify that the tag name is not used by a data object
  //

  KST::dataObjectList.lock().readLock();
  if (KST::dataObjectList.findTag(tag) != KST::dataObjectList.end()) {
    KST::dataObjectList.lock().unlock();
      if (warn) {
        QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("%1: this name is already in use. Change it to a unique name.").arg(tag));
      }

      return true;
  }
  KST::dataObjectList.lock().unlock();

  return false;
}


bool KstGuiData::vectorTagNameNotUnique(const QString &tag, bool warn, void *p) {
  //
  // verify that the tag name is not empty
  //

  if (tag.trimmed().isEmpty()) {
    if (warn) {
      QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("Empty tag names are not allowed."));
    }

    return true;
  }

  //
  // verify that the tag name is not used by a data object
  //

  KstReadLocker vl(&KST::vectorList.lock());
  KstReadLocker sl(&KST::scalarList.lock());

  if (KST::vectorList.tagExists(tag) || KST::scalarList.tagExists(tag)) {
    if (warn) {
      QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("%1: this name is already in use. Change it to a unique name.").arg(tag));
    }

    return true;
  }

  return false;
}


bool KstGuiData::matrixTagNameNotUnique(const QString &tag, bool warn, void *p) {
  //
  // verify that the tag name is not empty
  //

  if (tag.trimmed().isEmpty()) {
    if (warn) {
      QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("Empty tag names are not allowed."));
    }
    return true;
  }

  //
  // verify that the tag name is not used by a data object
  //

  KstReadLocker ml(&KST::matrixList.lock());
  KstReadLocker sl(&KST::scalarList.lock());

  if (KST::matrixList.tagExists(tag) || KST::scalarList.tagExists(tag)) {
    if (warn) {
      QMessageBox::warning(static_cast<QWidget*>(p), QObject::tr("Kst"), QObject::tr("%1: this name is already in use. Change it to a unique name.").arg(tag));
    }
    return true;
  }

  return false;
}


int KstGuiData::vectorToFile(KstVectorPtr v, QFile *f) {
  KstApp *app = KstApp::inst();
  QString saving;
  QString ltxt;
  double *value;
  int BSIZE = 128;
  char buf[BSIZE];
  register int modval;
  int vSize;
  int i;
  int l;

  v->readLock();

  vSize = v->length();
  value = v->value();
  saving = QObject::tr("Saving vector %1").arg(v->tagName());
  modval = qMax(vSize/100, 100);
  ltxt = "; " + v->tagName() + '\n';

  f->write(ltxt.toAscii());

  ltxt.fill('-');
  ltxt[0] = ';';
  ltxt[1] = ' ';
  ltxt[ltxt.length() - 1] = '\n';

  f->write(ltxt.toAscii());

  app->slotUpdateProgress(vSize, 0, QString::null);

  for (i = 0; i < vSize; i++) {
    l = snprintf(buf, BSIZE, "%.15g\n", value[i]);
    f->write(buf, l);
    if (i % modval == 0) {
      app->slotUpdateProgress(vSize, i, saving);
    }
  }

  v->unlock();

  app->slotUpdateProgress(0, 0, QString::null);

  return 0;
}


int KstGuiData::vectorsToFile(const KstVectorList& vl, QFile *f, bool interpolate) {
  KstApp *app = KstApp::inst();
  int maxlen = -1;

  if (interpolate) { // code duplication is faster
    maxlen = 0;
    for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
      (*v)->readLock();
      maxlen = qMax(maxlen, (*v)->length());
    }
  } else {
    for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
      (*v)->readLock();
      if (maxlen == -1) {
        maxlen = (*v)->length();
      } else {
        maxlen = qMin(maxlen, (*v)->length());
      }
    }
  }

  QString saving = QObject::tr("Saving vectors...");
  register int modval = qMax(maxlen/100, 100);
  app->slotUpdateProgress(maxlen, 0, QString::null);

  bool first = true;
  QString ltxt = ";";
  for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
    ltxt += ' ';
    ltxt += (*v)->tagName();
  }
  ltxt += '\n';

  f->write(ltxt.toAscii());
  ltxt.fill('-');
  ltxt[0] = ';';
  ltxt[1] = ' ';
  ltxt[ltxt.length() - 1] = '\n';
  f->write(ltxt.toAscii());
  ltxt.reserve(vl.count()*17);

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

      if (interpolate) {
        val = (*v)->interpolate(line, maxlen);
      } else {
        val = (*v)->value()[line];
      } 
      ltxt += QString::number(val, 'g', 15);
    }
    ltxt += "\n";
    f->write(ltxt.toAscii());
    if (line % modval == 0) {
      app->slotUpdateProgress(maxlen, line, saving);
    }
  }

  for (KstVectorList::ConstIterator v = vl.begin(); v != vl.end(); ++v) {
    (*v)->unlock();
  }

  app->slotUpdateProgress(0, 0, QString::null);

  return 0;
}


void KstGuiData::removeCurveFromPlots(KstBaseCurve *c) {
  Kst2DPlotList pl = Kst2DPlot::globalPlotList();
  Kst2DPlotList::Iterator i;
  
  for (i = pl.begin(); i != pl.end(); ++i) {
    (*i)->removeCurve(KstBaseCurvePtr(c));
  }
}


QStringList KstGuiData::plotList(const QString& strWindow) {
  if (strWindow.isEmpty()) {
    return Kst2DPlot::globalPlotList().tagNames();
  }

  QMdiSubWindow *window = KstApp::inst()->findChild<QMdiSubWindow*>(strWindow);
  QStringList rc;

  if (window) {
    KstViewWindow* viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(window);
    if (viewWindow) {
/* xxx
      Kst2DPlotList plots = viewWindow->view()->findChildrenType<Kst2DPlot>();

      for (Kst2DPlotList::ConstIterator i = plots.begin(); i != plots.end(); ++i) {
        rc << (*i)->tagName();
      }
*/
    }
  }

  return rc;
}


bool KstGuiData::viewObjectNameNotUnique(const QString& tag) {
  KstApp *app = KstApp::inst();
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;
  bool retVal = false;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *view = dynamic_cast<KstViewWindow*>(*i);
    if (view) {
      if (view->view()->findChild(tag, true)) {
        retVal = true;

        break;
      }
    }
  }

  return retVal;
}


int KstGuiData::columns(const QString& strWindow) {
  QMdiSubWindow *window = KstApp::inst()->findChild<QMdiSubWindow*>(strWindow);

  if (window) {
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(window);

    if (viewWindow) {
      KstTopLevelViewPtr view = viewWindow->view();

      if (view && view->onGrid()) {
        return view->columns();
      }
    }
  }

  return -1;
}


void KstGuiData::newWindow(QWidget *dialogParent) {
  KstApp::inst()->slotFileNewWindow(dialogParent);
}


QStringList KstGuiData::windowList() {
  QStringList rc;
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    rc << (*i)->objectName();
  }

  return rc;
}


QString KstGuiData::currentWindow() {
  QMdiSubWindow *window = KstApp::inst()->activeSubWindow();

  return window ? window->windowTitle() : QString::null;
}
