/***************************************************************************
                      kstcolorsequence.cpp  -  Part of KST
                             -------------------
    begin                : Mon Jul 07 2003
    copyright            : (C) 2003 The University of Toronto
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

#include "kstcolorsequence.h"
#include <kpalette.h>

// Default palette that is used if "Kst Colors" is not found.
static const int colorcnt = 8;
static const char *const colors[colorcnt] = { "red",
                                              "blue",
                                              "green",
                                              "black",
                                              "magenta",
                                              "steelblue",
                                              "#501010",
                                              "#105010"
                                            };

static KStaticDeleter<KstColorSequence> sd;

QColor KstColorSequence::next() {
  bool shift = false;

  if (!_self) {
    _self = sd.setObject(_self, new KstColorSequence);
  }

  if (_self->_ptr >= _self->_count * 2) {
    _self->_ptr = 0;
  } else if (_self->_ptr >= _self->_count) {
    shift = true;
  }

  if (shift) {
    return _self->_pal->color(_self->_ptr++ % _self->_count).dark();
  }

return _self->_pal->color(_self->_ptr++);
}


KstColorSequence::KstColorSequence() : _ptr(0) {
  _pal = new KPalette("Kst Colors");

  if (_pal->nrColors() <= 0) {
    for (int i = 0; i < colorcnt; i++) {
      _pal->addColor(QColor(colors[i]));
    }
  } 

  _count = _pal->nrColors();
}

KstColorSequence::~KstColorSequence() {
  delete _pal;
  _pal = 0L;
}

KstColorSequence* KstColorSequence::_self = 0L;

// vim: ts=2 sw=2 et
