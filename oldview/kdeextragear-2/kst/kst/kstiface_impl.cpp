/***************************************************************************
                      kstiface_impl.cpp  -  Part of KST
                             -------------------
    begin                : Thu Jun 19 2003
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

#include "kstiface_impl.h"
#include "kstdoc.h"
#include "kst.h"
#include "kstvector.h"
#include "kstscalar.h"
#include "kstplot.h"
#include "kstequationcurve.h"
#include "kstdatacollection.h"
#include "kstcolorsequence.h"
#include "kstvcurve.h"
#include "kstview.h"
#include "plugincollection.h"

#include <kprinter.h>
#include <assert.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>


KstIfaceImpl::KstIfaceImpl(KstDoc *doc, KstApp *app)
: DCOPObject("KstIface"), _doc(doc), _app(app) {
  assert(doc);
}


KstIfaceImpl::~KstIfaceImpl() {
}


void KstIfaceImpl::showDataManager() {
  _app->showDataManager();
}


QStringList KstIfaceImpl::scalarList() {
  KstReadLocker ml(&KST::scalarList.lock());
  return KST::scalarList.tagNames();
}


QStringList KstIfaceImpl::vectorList() {
  KstReadLocker ml(&KST::vectorList.lock());
  return KST::vectorList.tagNames();
}


QStringList KstIfaceImpl::objectList() {
  KstReadLocker ml(&KST::dataObjectList.lock());
  return KST::dataObjectList.tagNames();
}


QStringList KstIfaceImpl::curveList() {
  QStringList rc;

  KstBaseCurveList bcl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);

  for (KstBaseCurveList::Iterator it = bcl.begin(); it != bcl.end(); ++it) {
    (*it)->readLock();
    rc += (*it)->tagName();
    (*it)->readUnlock();
  }

  return rc;
}


QStringList KstIfaceImpl::plotList() {
  return KST::plotList.tagNames();
}


QStringList KstIfaceImpl::pluginList() {
  QStringList rc;
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
  QMap<QString,Plugin::Data>::ConstIterator it;

  for (it = pluginList.begin(); it != pluginList.end(); ++it) {
    if (it.data()._filter == false) {
      rc += it.data()._name;
    }
  }

  return rc;
}


QStringList KstIfaceImpl::filterList() {
  QStringList rc;
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
  QMap<QString,Plugin::Data>::ConstIterator it;

  for (it = pluginList.begin(); it != pluginList.end(); ++it) {
    if (it.data()._filter) {
      rc += it.data()._name;
    }
  }

  return rc;
}


QStringList KstIfaceImpl::filterSetList() {
  QStringList rc;
  KstReadLocker ml(&KST::filterSetList.lock());

  for (KstFilterSetList::Iterator it = KST::filterSetList.begin(); it != KST::filterSetList.end(); ++it) {
    (*it)->lock().readLock();
    rc += (*it)->name();
    (*it)->lock().readUnlock();
  }

  return rc;
}


QStringList KstIfaceImpl::filterSetContents(const QString& filter) {
  QStringList rc;
  KstReadLocker ml(&KST::filterSetList.lock());

  KstFilterSetPtr fs = KST::filterSetList.find(filter);
  if (fs) {
    for (KstFilterSet::Iterator it = fs->begin(); it != fs->end(); ++it) {
      (*it)->readLock();
      rc += (*it)->tagName();
      (*it)->readUnlock();
    }
  }

  return rc;
}


bool KstIfaceImpl::plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName, const QColor& color) {
  KstPlot *plot = 0;
  QString etag, ptag;

  if (equation.isEmpty()) {
    return false;
  }

  etag = "Eq-" + QString(equation).replace(QRegExp("[\\[\\]\\s]"), "_");
  ptag = "P-" + plotName;

  if (!plotName.isEmpty()) {
    plot = KST::plotList.FindKstPlot(plotName);
  }

  while (KST::dataTagNameNotUnique(etag, false)) {
    etag += '\'';
  }

  KstEquationCurvePtr eq;

  eq = new KstEquationCurve(etag, equation, start, end, numSamples,
                            color.isValid() ? color : QColor("darkBlue"));

  if (!eq->isValid()) {
    return false;
  }

  if (!plot) {
    plot = new KstPlot(ptag);
    KST::plotList.append(plot);
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(KstDataObjectPtr(eq));
  KST::dataObjectList.lock().writeUnlock();

  plot->Curves.append(KstBaseCurvePtr(eq));

  KST::plotList.arrangePlots(KST::plotList.getPlotCols());

  _doc->forceUpdate();
  _doc->setModified();

  return true;
}


bool KstIfaceImpl::plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName) {
  return plotEquation(start, end, numSamples, equation, plotName, KstColorSequence::next());
}


bool KstIfaceImpl::plotEquation(const QString& xvector, const QString& equation, const QString& plotName) {
  return plotEquation(xvector, equation, plotName, KstColorSequence::next());
}

bool KstIfaceImpl::plotEquation(const QString& xvector, const QString& equation, const QString& plotName, const QColor& color) {
  KstVectorPtr v;
  KstPlot *plot = 0;
  QString etag, ptag;
  KST::vectorList.lock().readLock();
  KstVectorList::Iterator it = KST::vectorList.findTag(xvector);
  KST::vectorList.lock().readUnlock();

  if (equation.isEmpty() || it == KST::vectorList.end()) {
    return false;
  }

  v = *it;

  etag = "Eq-" + QString(equation).replace(QRegExp("[\\[\\]\\s]"), "_");
  ptag = "P-" + plotName;

  if (!plotName.isEmpty()) {
    plot = KST::plotList.FindKstPlot(plotName);
  }

  while (KST::dataTagNameNotUnique(etag, false)) {
    etag += '\'';
  }

  KstEquationCurvePtr eq;

  eq = new KstEquationCurve(etag, equation, v, true,
                            color.isValid() ? color : QColor("darkBlue"));

  if (!eq->isValid()) {
    return false;
  }

  if (!plot) {
    plot = new KstPlot(ptag);
    KST::plotList.append(plot);
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(KstDataObjectPtr(eq));
  KST::dataObjectList.lock().writeUnlock();

  plot->Curves.append(KstBaseCurvePtr(eq));

  KST::plotList.arrangePlots(KST::plotList.getPlotCols());

  _doc->forceUpdate();
  _doc->setModified();

  return true;
}


const QString& KstIfaceImpl::generateScalar(const QString& name, double value) {
  KstScalarPtr s = new KstScalar(name, value);
  KstReadLocker rl(s);
  s->setOrphan(true);
  return s->tagName();
}


const QString& KstIfaceImpl::generateVector(const QString& name, double from, double to, int points) {
  KstVectorPtr v = KstVector::generateVector(from, to, points, name);
  KstReadLocker rl(v);
  return v->tagName();
}


bool KstIfaceImpl::saveVector(const QString& vector, const QString& filename) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(vector);

  if (it == KST::vectorList.end() || filename.isEmpty()) {
    return false;
  }

  QFile f(filename);
  if (!f.open(IO_WriteOnly)) {
    return false;
  }

  return 0 == KST::vectorToFile(*it, &f);
}


QStringList KstIfaceImpl::inputVectors(const QString& objectName) {
  KstReadLocker ml(&KST::dataObjectList.lock());
  KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
  QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    (*oi)->readLock();
    rc = (*oi)->inputVectors().tagNames();
    (*oi)->readUnlock();
  }

  return rc;
}


QStringList KstIfaceImpl::inputScalars(const QString& objectName) {
  KstReadLocker ml(&KST::dataObjectList.lock());
  KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
  QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    (*oi)->readLock();
    rc = (*oi)->inputScalars().tagNames();
    (*oi)->readUnlock();
  }

  return rc;
}


QStringList KstIfaceImpl::outputVectors(const QString& objectName) {
  KstReadLocker ml(&KST::dataObjectList.lock());
  KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
  QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    (*oi)->readLock();
    rc = (*oi)->outputVectors().tagNames();
    (*oi)->readUnlock();
  }

  return rc;
}


QStringList KstIfaceImpl::outputScalars(const QString& objectName) {
  KstReadLocker ml(&KST::dataObjectList.lock());
  KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
  QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    (*oi)->readLock();
    rc = (*oi)->outputScalars().tagNames();
    (*oi)->readUnlock();
  }

  return rc;
}


double KstIfaceImpl::scalar(const QString& name) {
  KstReadLocker ml(&KST::scalarList.lock());
  KstScalarList::Iterator it = KST::scalarList.findTag(name);

  if (it == KST::scalarList.end()) {
    return 0.0;
  }

  KstReadLocker l2(*it);

  return (*it)->value();
}


bool KstIfaceImpl::setScalar(const QString& name, double value) {
  KstReadLocker ml(&KST::scalarList.lock());
  KstScalarList::Iterator it = KST::scalarList.findTag(name);

  if (it == KST::scalarList.end()) {
    return false;
  }

  KstWriteLocker l2(*it);
  *(*it) = value;
  return true;
}


int KstIfaceImpl::vectorSize(const QString& name) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(name);
  int rc = 0;

  if (it != KST::vectorList.end()) {
    (*it)->readLock();
    rc = (*it)->length();
    (*it)->readUnlock();
  }

  return rc;
}


double KstIfaceImpl::vector(const QString& name, int index) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(name);
  double rc = 0.0;

  if (it != KST::vectorList.end() && index >= 0) {
    (*it)->readLock();
    if (index < (*it)->length()) {
      rc = (*it)->value(index);
    }
    (*it)->readUnlock();
  }

  return rc;
}


bool KstIfaceImpl::setVector(const QString& name, int index, double value) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end() && index >= 0) {
    (*it)->writeLock();
    if (index < (*it)->length()) {
      (*it)->value()[index] = value;
      (*it)->writeUnlock();
      return true;
    }
    (*it)->writeUnlock();
  }

  return false;
}


bool KstIfaceImpl::resizeVector(const QString& name, int newSize) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end()) {
    (*it)->writeLock();
    (*it)->resize(newSize);
    bool rc = (*it)->length() == newSize;
    (*it)->writeUnlock();
    return rc;
  }

  return false;
}


bool KstIfaceImpl::clearVector(const QString& name) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end()) {
    (*it)->writeLock();
    (*it)->zero();
    (*it)->writeUnlock();
    return true;
  }

  return false;
}


const QString& KstIfaceImpl::createPlot(const QString& name) {
  QString n = name;

  if (KST::plotList.FindKstPlot(n)) {
    n = KST::plotList.generatePlotName();
  }

  KstPlot *p = new KstPlot(n);
  KST::plotList.append(p);
  _doc->forceUpdate();
  _doc->setModified();
  KST::plotList.arrangePlots(KST::plotList.getPlotCols());

  return p->tagName();
}


bool KstIfaceImpl::deletePlot(const QString& name) {
  KstPlot *p = KST::plotList.FindKstPlot(name);

  if (p) {
    KST::plotList.remove(p);
    //delete p; autodeletes
    _doc->forceUpdate();
    _doc->setModified();
    return true;
  }

  return false;
}


QStringList KstIfaceImpl::plotContents(const QString& name) {
  KstPlot *p = KST::plotList.FindKstPlot(name);

  if (!p) {
    return QStringList();
  }

  return p->Curves.tagNames();
}


bool KstIfaceImpl::addCurveToPlot(const QString& plot, const QString& curve) {
  KstPlot *p = KST::plotList.FindKstPlot(plot);
  KstBaseCurveList bcl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  KstBaseCurveList::Iterator ci = bcl.findTag(curve);

  if (p && ci != bcl.end()) {
    if (!p->Curves.contains(*ci)) {
      p->addCurve(*ci);
    }
    _doc->forceUpdate();
    _doc->setModified();
    return true;
  }

  return false;
}


bool KstIfaceImpl::removeCurveFromPlot(const QString& plot, const QString& curve) {
  KstPlot *p = KST::plotList.FindKstPlot(plot);
  KstBaseCurveList bcl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  KstBaseCurveList::Iterator ci = bcl.findTag(curve);

  if (p && ci != bcl.end()) {
    (*ci)->readLock();
    p->Curves.remove(*ci);
    (*ci)->readUnlock();
    _doc->forceUpdate();
    _doc->setModified();
    return true;
  }

  return false;
}


const QString& KstIfaceImpl::createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector) {
  return createCurve(name, xVector, yVector, xErrorVector, yErrorVector, KstColorSequence::next());
}


const QString& KstIfaceImpl::createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector, const QColor& color) {
  QString n = name;
  KST::vectorList.lock().readLock();
  KstVectorPtr vx = *KST::vectorList.findTag(xVector);
  KstVectorPtr vy = *KST::vectorList.findTag(yVector);
  KstVectorPtr ex = *KST::vectorList.findTag(xErrorVector);
  KstVectorPtr ey = *KST::vectorList.findTag(yErrorVector);
  KST::vectorList.lock().readUnlock();

  KST::dataObjectList.lock().writeLock();
  while (KST::dataObjectList.findTag(n) != KST::dataObjectList.end()) {
    n += "'";
  }

  KstVCurvePtr c = new KstVCurve(n, vx, vy, ex, ey, color);
  KST::dataObjectList.append(KstDataObjectPtr(c));
  KST::dataObjectList.lock().writeUnlock();
  _doc->forceUpdate();
  _doc->setModified();

  return c->tagName();
}


bool KstIfaceImpl::printImage(const QString& url) {
  if (url.isEmpty()) {
    return false;
  }

  KstView *view = _app->viewObject();
  view->printToGraphicsFile(url, view->width(), view->height());
  return true; // FIXME: eventually return an error code
}


bool KstIfaceImpl::printPostScript(const QString& url) {
  if (url.isEmpty()) {
    return false;
  }

  KstView *view = _app->viewObject();
  KPrinter printer;
  printer.setPageSize(KPrinter::Letter);
  printer.setOrientation(KPrinter::Landscape);
  printer.setOutputToFile(true);
  printer.setOutputFileName(url);
  view->print(&printer);
  return true; // FIXME: eventually return an error code
}


void KstIfaceImpl::reloadVectors() {
  _app->reload();
}


void KstIfaceImpl::reloadVector(const QString& vector) {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator v = KST::vectorList.findTag(vector);
  if (v != KST::vectorList.end()) {
    (*v)->writeLock();
    KstRVector *r = dynamic_cast<KstRVector*>((*v).data());
    if (r) {
      r->reload();
    }
    (*v)->writeUnlock();
  }
}


const QString& KstIfaceImpl::loadVector(const QString& file, const QString& field) {
  KstDataSourcePtr src;
  /* generate or find the kstfile */
  KST::dataSourceList.lock().writeLock();
  KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(file);

  if (it == KST::dataSourceList.end()) {
    src = KstDataSource::loadSource(file);
    if (!src || !src->isValid()) {
      KST::dataSourceList.lock().writeUnlock();
      return QString::null;
    }
    if (src->frameCount() < 1) {
      KST::dataSourceList.lock().writeUnlock();
      return QString::null;
    }
    KST::dataSourceList.append(src);
  } else {
    src = *it;
  }
  src->writeLock();
  KST::dataSourceList.lock().writeUnlock();

  KST::vectorList.lock().readLock();
  QString vname = "V" + QString::number(KST::vectorList.count() + 1);

  while (KST::vectorTagNameNotUnique(vname, false)) {
    vname = "V" + QString::number(KST::vectorList.count() + 1);
  }
  KST::vectorList.lock().readUnlock();

  KstVectorPtr p = new KstRVector(src, field, vname, 0, -1, 0, false, false);
  KST::addVectorToList(p);

  src->writeUnlock();

  if (p) {
    _doc->forceUpdate();
    _doc->setModified();
    return p->tagName();
  }

  return QString::null;
}


const QString& KstIfaceImpl::fileName() {
  return _doc->getAbsFilePath();
}


bool KstIfaceImpl::save() {
  if (_doc->getTitle() != "Untitled") {
    return _doc->saveDocument(_doc->getAbsFilePath());
  }
  return false;
}


bool KstIfaceImpl::saveAs(const QString& fileName) {
  bool rc = _doc->saveDocument(fileName);
  if (rc) {
    QFileInfo saveAsInfo(fileName);
    _doc->setTitle(saveAsInfo.fileName());
    _doc->setAbsFilePath(saveAsInfo.absFilePath());

    _app->setCaption(kapp->caption() + ": " + _doc->getTitle());
  }
  return rc;
}


void KstIfaceImpl::newFile() {
  _doc->newDocument();
}


bool KstIfaceImpl::open(const QString& fileName) {
  return _app->openDocumentFile(fileName);
}



// vim: ts=2 sw=2 et
