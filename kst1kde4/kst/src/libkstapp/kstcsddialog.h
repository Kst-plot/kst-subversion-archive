/***************************************************************************
                       kstcsddialog_i.h  -  Part of KST
                             -------------------
    begin                :
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

#ifndef KSTCSDDIALOGI_H
#define KSTCSDDIALOGI_H

#include "kstdatadialog.h"
#include "kstcsd.h"
#include "kstimage.h"
#include "kst_export.h"
#include "ui_csddialogwidget.h"

class CSDDialogWidget;

class KstCsdDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstCsdDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );
    virtual ~KstCsdDialog();
    KST_EXPORT static KstCsdDialog *globalInstance();

  protected:
    QString editTitle() { return tr("Edit Spectrogram"); }
    QString newTitle() { return tr("New Spectrogram"); }

  public slots:
    void update();
    void updateWindow();
    bool newObject();
    bool editObject();
    void populateEditMultiple();
    void setVector(const QString& name);

  private:
    KstImagePtr createImage(KstCSDPtr csd);
    static QPointer<KstCsdDialog> _inst;

    // the following are for the multiple edit mode
    bool _vectorDirty : 1;
    bool _apodizeDirty : 1;
    bool _apodizeFxnDirty : 1;
    bool _gaussianSigmaDirty : 1;
    bool _removeMeanDirty : 1;
    bool _interleavedDirty : 1;
    bool _vectorUnitsDirty : 1;
    bool _fFTLenDirty : 1;
    bool _sampRateDirty : 1;
    bool _rateUnitsDirty : 1;
    bool _outputDirty : 1;
    bool _windowSizeDirty : 1;
    bool _interpolateHolesDirty : 1;
    bool editSingleObject(KstCSDPtr csPtr);

  private slots:
    void setApodizeDirty();
    void setRemoveMeanDirty();
    void setInterleavedDirty();
    void setInterpolateHolesDirty();

  private:
    static const QString& defaultTag;
    void fillFieldsForEdit();
    void fillFieldsForNew();
    void cleanup();
    Ui::CSDDialogWidget *_w;
};

#endif
