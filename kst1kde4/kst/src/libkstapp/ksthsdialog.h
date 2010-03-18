/***************************************************************************
                       ksthsdialog.h  -  Part of KST
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

#ifndef KSTHSDIALOGI_H
#define KSTHSDIALOGI_H

#include "kstdatadialog.h"
#include "ksthistogram.h"
#include "kst_export.h"
#include "ui_histogramdialogwidget.h"

class KST_EXPORT KstHsDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstHsDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );
    virtual ~KstHsDialog();
    static KstHsDialog *globalInstance();

  protected:
    QString editTitle() { return tr("Edit Histogram"); }
    QString newTitle() { return tr("New Histogram"); }
 
  public slots:
    void update();
    void updateWindow();
    bool newObject();
    bool editObject();
    void autoBin();
    void populateEditMultiple();
    void setVector(const QString& name);

  private:
    static QPointer<KstHsDialogI> _inst;
    // the following are for the multiple edit mode
    bool _vectorDirty;
    bool _minDirty;
    bool _maxDirty;
    bool _nDirty;
    bool _realTimeAutoBinDirty;
    bool _normIsPercentDirty;
    bool _normIsFractionDirty;
    bool _peakIs1Dirty;
    bool _normIsNumberDirty;
    bool editSingleObject(KstHistogramPtr hsPtr);

  private slots:
    void updateButtons();
    void setVectorDirty() { _vectorDirty = true; }
    void setMinDirty() { _minDirty = true; }
    void setMaxDirty() { _maxDirty = true; }
    void setRealTimeAutoBinDirty();
    void setNormIsPercentDirty() { _normIsPercentDirty = true; }
    void setNormIsFractionDirty() { _normIsFractionDirty = true; }
    void setPeakIs1Dirty() { _peakIs1Dirty = true; }
    void setNormIsNumberDirty() { _normIsNumberDirty = true; }

  private:
    static const QString& defaultTag;
    void fillFieldsForEdit();
    void fillFieldsForNew();
    void cleanup();
    Ui::HistogramDialogWidget *_w;
};

#endif
