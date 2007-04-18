/***************************************************************************
                      binnedmapdialog_i.h  -  Part of KST
                             -------------------
    begin                : 09/14/06
    copyright            : (C) 2006 The University of Toronto
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

#ifndef BINNEDMAPDIALOGI_H
#define BINNEDMAPDIALOGI_H

#include "binnedmap.h"

#include "kstdatadialog.h"
#include "kst_export.h"

class BinnedMapDialogWidget;

class KST_EXPORT BinnedMapDialogI : public KstDataDialog {
  Q_OBJECT
  public:
    BinnedMapDialogI(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0);
    virtual ~BinnedMapDialogI();

  public slots:
    virtual void update();
    virtual bool newObject();
    virtual bool editObject();
    void fillAutoRange();

  protected:
    virtual void fillFieldsForEdit();
    virtual void fillFieldsForNew();

  private:
    bool editSingleObject(BinnedMapPtr cps);
    static const QString& defaultTag;
    BinnedMapDialogWidget *_w;
};

#endif
// vim: ts=2 sw=2 et
