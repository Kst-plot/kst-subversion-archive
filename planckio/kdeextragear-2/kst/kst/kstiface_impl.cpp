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
#include <kprinter.h>
#include <assert.h>


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
  return KST::scalarList.tagNames();
}


QStringList KstIfaceImpl::vectorList() {
  return KST::vectorList.tagNames();
}


QStringList KstIfaceImpl::objectList() {
  return KST::dataObjectList.tagNames();
}


QStringList KstIfaceImpl::curveList() {
QStringList rc;

  KstBaseCurveList bcl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);

  for (KstBaseCurveList::Iterator it = bcl.begin(); it != bcl.end(); ++it) {
    rc += (*it)->tagName();
  }

return rc;
}


QStringList KstIfaceImpl::plotList() {
return KST::plotList.tagNames();
}


bool KstIfaceImpl::plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName, const QColor& color) {
  KstPlot *plot = 0;
  QString etag, ptag;

  if (equation.isEmpty()) {
    return false;
  }

  // FIXME: generate etag, ptag properly - work out semantics for finding existing plot
  etag = "Eq-" + plotName;
  ptag = "P-" + plotName;

  if (!plotName.isEmpty()) {
    plot = KST::plotList.FindKstPlot(plotName);
  }

  if (!plot) {
    plot = new KstPlot(ptag);
    KST::plotList.append(plot);
  }

  KstEquationCurvePtr eq;

  eq = new KstEquationCurve(etag, equation, start, end, numSamples,
                            color.isValid() ? color : QColor("darkBlue"));

  KST::dataObjectList.append(KstDataObjectPtr(eq));

  plot->Curves.append(KstBaseCurvePtr(eq));

  KST::plotList.arrangePlots(KST::plotList.getPlotCols());

  _doc->update();
  _doc->setModified();
  _doc->updateDialogs();

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
  KstVectorList::Iterator it = KST::vectorList.findTag(xvector);
  QString etag, ptag;

  if (equation.isEmpty() || it == KST::vectorList.end()) {
    return false;
  }

  v = *it;

  // FIXME: generate etag, ptag properly - work out semantics for finding existing plot
  etag = "Eq-" + plotName;
  ptag = "P-" + plotName;

  if (!plotName.isEmpty()) {
    plot = KST::plotList.FindKstPlot(plotName);
  }

  if (!plot) {
    plot = new KstPlot(ptag);
    KST::plotList.append(plot);
  }

  KstEquationCurvePtr eq;

  eq = new KstEquationCurve(etag, equation, v, true,
                            color.isValid() ? color : QColor("darkBlue"));

  KST::dataObjectList.append(KstDataObjectPtr(eq));

  plot->Curves.append(KstBaseCurvePtr(eq));

  KST::plotList.arrangePlots(KST::plotList.getPlotCols());

  _doc->update();
  _doc->setModified();
  _doc->updateDialogs();

  return true;
}


const QString& KstIfaceImpl::generateScalar(const QString& name, double value) {
KstScalarPtr s = new KstScalar(name, value);
return s->tagName();
}


const QString& KstIfaceImpl::generateVector(const QString& name, double from, double to, int points) {
KstVectorPtr v = KstVector::generateVector(from, to, points, name);
return v->tagName();
}


bool KstIfaceImpl::saveVector(const QString& vector, const QString& filename) {
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
KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    rc = (*oi)->inputVectors().tagNames();
  }

  return rc;
}


QStringList KstIfaceImpl::inputScalars(const QString& objectName) {
KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    rc = (*oi)->inputScalars().tagNames();
  }

  return rc;
}


QStringList KstIfaceImpl::outputVectors(const QString& objectName) {
KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    rc = (*oi)->outputVectors().tagNames();
  }

  return rc;
}


QStringList KstIfaceImpl::outputScalars(const QString& objectName) {
KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(objectName);
QStringList rc;

  if (oi != KST::dataObjectList.end()) {
    rc = (*oi)->outputScalars().tagNames();
  }

  return rc;
}


double KstIfaceImpl::scalar(const QString& name) {
KstScalarList::Iterator it = KST::scalarList.findTag(name);

  if (it == KST::scalarList.end()) {
    return 0.0;
  }

  return (*it)->value();
}


bool KstIfaceImpl::setScalar(const QString& name, double value) {
KstScalarList::Iterator it = KST::scalarList.findTag(name);

  if (it == KST::scalarList.end()) {
    return false;
  }

  *(*it) = value;
  return true;
}


int KstIfaceImpl::vectorSize(const QString& name) {
KstVectorList::Iterator it = KST::vectorList.findTag(name);
int rc = 0;

  if (it != KST::vectorList.end()) {
    rc = (*it)->length();
  }

return rc;
}


double KstIfaceImpl::vector(const QString& name, int index) {
KstVectorList::Iterator it = KST::vectorList.findTag(name);
double rc = 0.0;

  if (it != KST::vectorList.end() && rc >= 0 && rc < (*it)->length()) {
    rc = (*it)->value(index);
  }

return rc;
}


bool KstIfaceImpl::setVector(const QString& name, int index, double value) {
KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end() && index >= 0 && index < (*it)->length()) {
    (*it)->value()[index] = value;
    return true;
  }

return false;
}


bool KstIfaceImpl::resizeVector(const QString& name, int newSize) {
KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end()) {
    (*it)->resize(newSize);
    return (*it)->length() == newSize;
  }

return false;
}


bool KstIfaceImpl::clearVector(const QString& name) {
KstVectorList::Iterator it = KST::vectorList.findTag(name);

  if (it != KST::vectorList.end()) {
    (*it)->zero();
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
  _doc->update();
  _doc->setModified();
  _doc->updateDialogs();

return p->tagName();
}


bool KstIfaceImpl::deletePlot(const QString& name) {
KstPlot *p = KST::plotList.FindKstPlot(name);

  if (p) {
    KST::plotList.remove(p);
    //delete p; autodelets
    _doc->update();
    _doc->setModified();
    _doc->updateDialogs();
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
    _doc->update();
    _doc->setModified();
    _doc->updateDialogs();
    return true;
  }

return false;
}


bool KstIfaceImpl::removeCurveFromPlot(const QString& plot, const QString& curve) {
KstPlot *p = KST::plotList.FindKstPlot(plot);
KstBaseCurveList bcl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
KstBaseCurveList::Iterator ci = bcl.findTag(curve);

  if (p && ci != bcl.end()) {
    p->Curves.remove(*ci);
    _doc->update();
    _doc->setModified();
    _doc->updateDialogs();
    return true;
  }

return false;
}


const QString& KstIfaceImpl::createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector) {
  return createCurve(name, xVector, yVector, xErrorVector, yErrorVector, KstColorSequence::next());
}


const QString& KstIfaceImpl::createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector, const QColor& color) {
QString n = name;
KstVectorList::Iterator vx = KST::vectorList.findTag(xVector);
KstVectorList::Iterator vy = KST::vectorList.findTag(yVector);
KstVectorList::Iterator ex = KST::vectorList.findTag(xErrorVector);
KstVectorList::Iterator ey = KST::vectorList.findTag(yErrorVector);

  while (KST::dataObjectList.findTag(n) != KST::dataObjectList.end()) {
    n += "'";
  }

  KstVCurvePtr c = new KstVCurve(n, *vx, *vy, *ex, *ey, color);
  KST::dataObjectList.append(KstDataObjectPtr(c));
  _doc->update();
  _doc->setModified();
  _doc->updateDialogs();

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

// vim: ts=2 sw=2 et
