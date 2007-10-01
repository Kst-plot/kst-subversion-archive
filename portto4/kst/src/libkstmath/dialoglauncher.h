/***************************************************************************
                              dialoglauncher.h
                             -------------------
    begin                : Nov. 24, 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef DIALOGLAUNCHER_H
#define DIALOGLAUNCHER_H

#include <QObject>

#include "kstobject.h"

#include "kst_export.h"

namespace Kst {

class KST_EXPORT DialogLauncher : public QObject {
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
    virtual void showVectorDialog(KstObjectPtr objectPtr = 0);

    virtual void showMatrixDialog(KstObjectPtr objectPtr = 0);

    virtual void showScalarDialog(KstObjectPtr objectPtr = 0);

    virtual void showStringDialog(KstObjectPtr objectPtr = 0);

    //standard objects
    virtual void showCurveDialog(KstObjectPtr objectPtr = 0);

    virtual void showImageDialog(KstObjectPtr objectPtr = 0);

    //standard data objects
    virtual void showEquationDialog(KstObjectPtr objectPtr = 0);

    virtual void showHistogramDialog(KstObjectPtr objectPtr = 0);

    virtual void showPSDDialog(KstObjectPtr objectPtr = 0);

    virtual void showCSDDialog(KstObjectPtr objectPtr = 0);

    //plugins
    virtual void showBasicPluginDialog(KstObjectPtr objectPtr = 0);
};

}

#endif

// vim: ts=2 sw=2 et
