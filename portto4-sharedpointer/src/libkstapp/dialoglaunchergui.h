/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOGLAUNCHERGUI_H
#define DIALOGLAUNCHERGUI_H

#include "dialoglauncher.h"

#include "kst_export.h"

namespace Kst {

class DialogLauncherGui : public DialogLauncher {
  public:
    DialogLauncherGui();
    virtual ~DialogLauncherGui();

    //primitives
    virtual void showVectorDialog(QString &vectorname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false);

    virtual void showMatrixDialog(QString &matrixName, ObjectPtr objectPtr = ObjectPtr(), bool modal = false);

    virtual void showScalarDialog(QString &scalarname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false);

    virtual void showStringDialog(QString &stringname, ObjectPtr objectPtr = ObjectPtr(), bool modal = false);

    //standard objects
    virtual void showCurveDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr());
    virtual void showMultiCurveDialog(QList<ObjectPtr> curves);

    virtual void showImageDialog(ObjectPtr objectPtr = ObjectPtr(), MatrixPtr matrix = MatrixPtr());
    virtual void showMultiImageDialog(QList<ObjectPtr> images);

    //standard data objects
    virtual void showEquationDialog(ObjectPtr objectPtr = ObjectPtr());

    virtual void showHistogramDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr());

    virtual void showPowerSpectrumDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr());

    virtual void showCSDDialog(ObjectPtr objectPtr = ObjectPtr(), VectorPtr vector = VectorPtr());

    virtual void showEventMonitorDialog(ObjectPtr objectPtr = ObjectPtr());

    //plugins
    virtual void showBasicPluginDialog(QString pluginName, ObjectPtr objectPtr = ObjectPtr(), VectorPtr vectorX = VectorPtr(), VectorPtr vectorY = VectorPtr(), PlotItemInterface *plotItem = 0 );

    //show appropriate dialog
    virtual void showObjectDialog(ObjectPtr objectPtr = ObjectPtr());

    virtual void showMultiObjectDialog(QList<ObjectPtr> objects);

};

}

#endif

// vim: ts=2 sw=2 et
