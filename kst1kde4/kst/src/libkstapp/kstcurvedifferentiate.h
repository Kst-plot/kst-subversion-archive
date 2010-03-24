/**************************************************************************
        kstcurvedifferentiate.h - source file: inherits designer dialog
                             -------------------
    begin                :  2005
    copyright            : (C) 2005 The University of British Columbia
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

#ifndef KSTCURVEDIFFERENTIATEI_H
#define KSTCURVEDIFFERENTIATEI_H

#include <QVector>

#include "kstnumbersequence.h"
#include "kst_export.h"
#include "ui_curvedifferentiate.h"

class KstNumberSequence;
class KstViewWindow;

class KstCurveDifferentiate : public QDialog, public Ui::curveDifferentiate {
  Q_OBJECT
  public:
    KST_EXPORT KstCurveDifferentiate(QWidget* parent = 0,
                         const char* name = 0, bool modal = TRUE, Qt::WindowFlags fl = 0 );
    virtual ~KstCurveDifferentiate();

    void setOptions();
    void getOptions();

  public slots:
    void updateCurveDifferentiate() KST_EXPORT;
    void showCurveDifferentiate() KST_EXPORT;

  private slots:
    void updateButtons();
    void addButtonClicked();
    void removeButtonClicked();
    void upButtonClicked();
    void downButtonClicked();
    void apply();

  private:
    void saveProperties();
    void loadProperties();
    void cycleWindow(KstViewWindow *window);

    KstNumberSequence _lineColorSeq;
    KstNumberSequence _lineStyleSeq;
    KstNumberSequence _pointStyleSeq;
    KstNumberSequence _lineWidthSeq;
    QVector<KstNumberSequence> _seqVect;
    int _lineColorOrder;
    int _pointStyleOrder;
    int _lineStyleOrder;
    int _lineWidthOrder;
    int _maxLineWidth;
    int _pointDensity;
    int _repeatAcross;
    int _applyTo;
};

#endif
