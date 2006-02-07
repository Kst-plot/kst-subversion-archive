/***************************************************************************
                         kstiface.h  -  Part of KST
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
#ifndef KSTIFACE_H
#define KSTIFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qstringlist.h>
#include <qcolor.h>

// Warning: If you change something here, you could break existing scripts.

class KstIface : virtual public DCOPObject {
	K_DCOP
k_dcop:
	virtual void showDataManager() = 0;

  virtual QStringList scalarList() = 0;
  virtual QStringList vectorList() = 0;
  virtual QStringList objectList() = 0;
  virtual QStringList curveList() = 0;
  virtual QStringList plotList() = 0;
  virtual QStringList pluginList() = 0;
  virtual QStringList filterList() = 0;
  virtual QStringList filterSetList() = 0;
  virtual QStringList filterSetContents(const QString& filter) = 0;


  virtual QStringList inputVectors(const QString& objectName) = 0;
  virtual QStringList inputScalars(const QString& objectName) = 0;
  virtual QStringList outputVectors(const QString& objectName) = 0;
  virtual QStringList outputScalars(const QString& objectName) = 0;

  virtual double scalar(const QString& name) = 0;
  virtual bool setScalar(const QString& name, double value) = 0;

  virtual double vector(const QString& name, int index) = 0;
  virtual bool setVector(const QString& name, int index, double value) = 0;
  virtual bool resizeVector(const QString& name, int newSize) = 0;
  virtual bool clearVector(const QString& name) = 0;
  virtual int vectorSize(const QString& name) = 0;

  // FIXME: might want to remove this.  it generates an orphan vector, but
  //        it's useful for testing purposes
  virtual const QString& generateVector(const QString& name, double from, double to, int points) = 0;

  virtual const QString& generateScalar(const QString& name, double value) = 0;


  virtual bool plotEquation(const QString& xvector, const QString& equation, const QString& plotName, const QColor& color) = 0;
  virtual bool plotEquation(const QString& xvector, const QString& equation, const QString& plotName) = 0;
  virtual bool plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName, const QColor& color) = 0;
  virtual bool plotEquation(double start, double end, int numSamples, const QString& equation, const QString& plotName) = 0;

  virtual bool saveVector(const QString& vector, const QString& filename) = 0;
  virtual bool printImage(const QString& filename) = 0;
  virtual bool printPostScript(const QString& filename) = 0;

  virtual const QString& createPlot(const QString& name) = 0;
  virtual bool deletePlot(const QString& name) = 0;
  virtual QStringList plotContents(const QString& name) = 0;
  virtual bool addCurveToPlot(const QString& plot, const QString& curve) = 0;
  virtual bool removeCurveFromPlot(const QString& plot, const QString& curve) = 0;

  virtual const QString& createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector, const QColor& color) = 0; 
  virtual const QString& createCurve(const QString& name, const QString& xVector, const QString& yVector, const QString& xErrorVector, const QString& yErrorVector) = 0; 

  virtual void reloadVectors() = 0;
  virtual void reloadVector(const QString& vector) = 0;

  virtual const QString& loadVector(const QString& file, const QString& field) = 0;

  virtual const QString& fileName() = 0;
  virtual bool save() = 0;
  virtual bool saveAs(const QString& fileName) = 0;
  virtual void newFile() = 0;
  virtual bool open(const QString& fileName) = 0;

};

#endif
// vim: ts=2 sw=2 et
