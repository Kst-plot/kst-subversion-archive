/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

#include "kst_export.h"

/*
FIXME!! This class needs to be rewritten to give completion...
*/

namespace Kst {

class  KST_EXPORT ComboBox : public QComboBox {
  Q_OBJECT
  public:
    ComboBox(QWidget *parent=0);
    ComboBox(bool editable, QWidget *parent=0);
    virtual ~ComboBox();

    void setEditable(bool editable);

  private:
    void setupLineEdit();

  private:
    bool _editable;
};

}

#endif

// vim: ts=2 sw=2 et
