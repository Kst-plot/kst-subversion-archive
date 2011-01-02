/***************************************************************************
                              dialoglauncher.h
                             -------------------
    begin                : Nov. 24, 2004
    copyright            : (C) 2004 The University of Toronto
    email                : netterfield@astro.utoronto.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOGLAUNCHER_H
#define DIALOGLAUNCHER_H

#include <QObject>

#include "object.h"
#include "vector.h"
#include "matrix.h"
#include "plotiteminterface.h"

#include "kstmath_export.h"

namespace Kst {

class KSTMATH_EXPORT DialogLauncher : public QObject {
  Q_OBJECT
  protected:
    static DialogLauncher *_self;
    static void cleanup();
    DialogLauncher();
    virtual ~DialogLauncher();

  public:
    static void replaceSelf(DialogLauncher *newInstance);
    static DialogLauncher *self();

  public Q_SLOTS:
    //primitives
    virtual void showVectorDialog(QString &vectorname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false) = 0;

    virtual void showMatrixDialog(QString &matrixName, ObjectPtr objectPtr = ObjectPtr(), bool modal = false) = 0;

    virtual void showScalarDialog(QString &scalarname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false) = 0;

    virtual void showStringDialog(QString &scalarname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false) = 0;

    //standard objects
    virtual void showCurveDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr()) = 0;
    virtual void showMultiCurveDialog(QList<ObjectPtr> curves) = 0;

    virtual void showImageDialog(ObjectPtr objectPtr = ObjectPtr(), MatrixPtr matrix = MatrixPtr()) = 0;
    virtual void showMultiImageDialog(QList<ObjectPtr> images) = 0;

    //standard data objects
    virtual void showEquationDialog(ObjectPtr objectPtr = ObjectPtr()) = 0;

    virtual void showHistogramDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr()) = 0;

    virtual void showPowerSpectrumDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr()) = 0;

    virtual void showCSDDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr()) = 0;

    virtual void showEventMonitorDialog(ObjectPtr objectPtr = ObjectPtr()) = 0;

    //plugins
    virtual void showBasicPluginDialog(QString pluginName, ObjectPtr objectPtr = ObjectPtr(), VectorPtr vectorX = VectorPtr(), VectorPtr vectorY = VectorPtr(), PlotItemInterface *plotItem = 0) = 0;

    //show appropriate dialog
    virtual void showObjectDialog(ObjectPtr objectPtr = ObjectPtr()) = 0;

    virtual void showMultiObjectDialog(QList<ObjectPtr> names) = 0;

};

}

#endif

// vim: ts=2 sw=2 et
