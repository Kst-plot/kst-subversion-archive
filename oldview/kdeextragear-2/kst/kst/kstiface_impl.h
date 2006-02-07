/***************************************************************************
                      kstiface_impl.h  -  Part of KST
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
#ifndef KSTIFACE_IMPL_H
#define KSTIFACE_IMPL_H

#include "kstiface.h"

class KstDoc;
class KstApp;

// Keep in sync with kstiface.h

class KstIfaceImpl : virtual public KstIface {
public:
  KstIfaceImpl(KstDoc *doc, KstApp *app);
  virtual ~KstIfaceImpl();

	virtual void showDataManager();

	virtual QStringList scalarList();
	virtual QStringList vectorList();
	virtual QStringList objectList();
	virtual QStringList curveList();
	virtual QStringList plotList();
  virtual QStringList pluginList();
  virtual QStringList filterList();
  virtual QStringList filterSetList();
  virtual QStringList filterSetContents(const QString& filter);

	virtual bool plotEquation(const QString& xvector, const QString& equation, const QString& plotName, const QColor& color);
  virtual bool plotEquation(const QString& xvector, const QString& equation, const QString& plotName);
  virtual bool plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName, const QColor& color);
  virtual bool plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName);

  virtual const QString& generateVector(const QString& name, double from, double to, int points);
  virtual const QString& generateScalar(const QString& name, double value);

  virtual bool saveVector(const QString& vector, const QString& filename);
  virtual QStringList inputVectors(const QString& objectName);
  virtual QStringList inputScalars(const QString& objectName);
  virtual QStringList outputVectors(const QString& objectName);
  virtual QStringList outputScalars(const QString& objectName);
  virtual double scalar(const QString& name);
  virtual bool setScalar(const QString& name, double value);
  virtual double vector(const QString& name, int index);
  virtual bool setVector(const QString& name, int index, double value);
  virtual bool resizeVector(const QString& name, int newSize);
  virtual bool clearVector(const QString& name);
  virtual int vectorSize(const QString& name);

  virtual bool printImage(const QString& filename);
  virtual bool printPostScript(const QString& filename);

  virtual const QString& createPlot(const QString& name);
  virtual bool deletePlot(const QString& name);
  virtual QStringList plotContents(const QString& name);
  virtual bool addCurveToPlot(const QString& plot, const QString& curve);
  virtual bool removeCurveFromPlot(const QString& plot, const QString& curve);

  virtual const QString& createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector);
  virtual const QString& createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector, const QColor& color);

  virtual void reloadVectors();
  virtual void reloadVector(const QString& vector);

  virtual const QString& loadVector(const QString& file, const QString& field);

  virtual const QString& fileName();
  virtual bool save();
  virtual bool saveAs(const QString& fileName);
  virtual void newFile();
  virtual bool open(const QString& fileName);


private:
  KstDoc *_doc;
  KstApp *_app;
};

#endif
// vim: ts=2 sw=2 et
