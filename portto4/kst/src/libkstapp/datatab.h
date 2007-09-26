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

#ifndef DATATAB_H
#define DATATAB_H

#include "dialogtab.h"

#include "kst_export.h"

namespace Kst {

class DataDialog;

class KST_EXPORT DataTab : public DialogTab
{
  Q_OBJECT
  public:
    DataTab(QWidget *parent);
    virtual ~DataTab();

    DataDialog *dataDialog() const;
};

}

#endif

// vim: ts=2 sw=2 et