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

#include "curveappearance.h"

namespace Kst {

CurveAppearance::CurveAppearance(QWidget *parent)
  : QWidget(parent) {
  setupUi(this);
}


CurveAppearance::~CurveAppearance() {
}

}

// vim: ts=2 sw=2 et