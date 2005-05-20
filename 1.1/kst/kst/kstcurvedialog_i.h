/***************************************************************************
                       kstcurvedialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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

#ifndef KSTCURVEDIALOGI_H
#define KSTCURVEDIALOGI_H

#include "curvedialog.h"
#include "kstvcurve.h"

class KstCurveDialogI : public KstCurveDialog {
  Q_OBJECT
public:
  KstCurveDialogI(QWidget* parent = 0, const char* name = 0,
                   bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstCurveDialogI();
public slots:

  void update();
  void updateWindow();

  bool new_I();
  bool edit_I();

  static KstCurveDialogI *globalInstance();

private:
  static QGuardedPtr<KstCurveDialogI> _inst;
  KstVCurvePtr _getPtr(const QString &tagin);

  bool _newDialog;
  KstVCurvePtr DP;

/***********************************/
/** defined in dataobjectdialog.h **/
public slots:
  void show_Edit(const QString &field);
  void show_New();
  void OK();
  void Init();
  void close();
  void reject();
  void toggledXErrorSame(bool on);
  void toggledYErrorSame(bool on);
private:
  static const QString& defaultTag;
  void _fillFieldsForEdit();
  void _fillFieldsForNew();

};

#endif
