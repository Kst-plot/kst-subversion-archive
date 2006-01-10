/***************************************************************************
                               kstpainter.cpp
                             -------------------
    begin                : Nov 25, 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "kstpainter.h"

KstPainter::KstPainter(PaintType t)
: QPainter(), _type(t), _drawInlineUI(false), _makingMask(false) {
}


KstPainter::~KstPainter() {
}


void KstPainter::setType(PaintType t) {
  _type = t;
}


KstPainter::PaintType KstPainter::type() const {
  return _type;
}


bool KstPainter::drawInlineUI() const {
  return _drawInlineUI;
}


void KstPainter::setDrawInlineUI(bool draw) {
  _drawInlineUI = draw;
}


bool KstPainter::makingMask() const {
  return _makingMask;
}


void KstPainter::setMakingMask(bool making) {
  _makingMask = making;
}

// vim: ts=2 sw=2 et
