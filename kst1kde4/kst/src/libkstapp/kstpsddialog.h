/***************************************************************************
                       kstpsddialog.h  -  Part of KST
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

#ifndef KSTPSDDIALOGI_H
#define KSTPSDDIALOGI_H

#include "kstdatadialog.h"
#include "kstpsd.h"
#include "kst_export.h"
#include "ui_psddialogwidget.h"

class KstPsdDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstPsdDialog(QWidget* parent = 0, const char* name = 0,
        bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstPsdDialog();
    KST_EXPORT static KstPsdDialog *globalInstance();

  protected:
    QString editTitle() { return tr("Edit Spectrum"); }
    QString newTitle() { return tr("New Spectrum"); }
    bool newObject();
    bool editObject();
 
  public slots:
    void update();
    void updateWindow();
    void populateEditMultiple();
    void setVector(const QString& name);

  private:
    static QPointer<KstPsdDialog> _inst;
    Ui::PSDDialogWidget *_w;

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
    bool _interpolateHolesDirty : 1;
    bool editSingleObject(KstPSDPtr psPtr);

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
};

#endif

