/***************************************************************************
                              kstviewlabel.cpp
                             ------------------
    begin                : Apr 10 2004
    copyright            : (C) 2000 by cbn
                           (C) 2004 by The University of Toronto
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

#include <kdatastream.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include "kstviewlabeldialog_i.h"
#include "kstviewlabel.h"
#include <qptrstack.h>

KstViewLabel::KstViewLabel(const QString& txt, KstVLJustifyType justify,
                           float rotation)
: KstBorderedViewObject("KstViewLabel") {
  _txt = txt;
  _interpret = true;
  _rotation = rotation;
  _justify = justify;
  _fontName = "helvetica"; // FIXME
  _symbolFontName = "Symbol";
  _fontSize = 20;
  _layoutActions |= Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename | Edit;
  _standardActions |= Delete | Edit;
  _chunk = 0L;
  _editDialog = 0L;
  setBackgroundColor("white"); // FIXME
  reparse();
}


KstViewLabel::~KstViewLabel() {
  delete _editDialog;
  _editDialog = 0L;
}


void KstViewLabel::setText(const QString& text) {
  if (_txt != text) {
    _txt = text;
    reparse();
  }
}


const QString& KstViewLabel::text() const {
  return _txt;
}


void KstViewLabel::setRotation(float rotation) {
  _rotation = rotation;  // no sense in float == float check
  drawToBuffer(_chunk);
}


void KstViewLabel::setJustification(KstVLJustifyType justify) {
  _justify = justify;
  drawToBuffer(_chunk);
}


void KstViewLabel::resize(const QSize& size) {
  KstBorderedViewObject::resize(size);
  drawToBuffer(_chunk);
}


int KstViewLabel::ascent() const {
  return _ascent;
}


void KstViewLabel::setFontName(const QString& fontName) {
  _fontName = fontName;
  drawToBuffer(_chunk);
}


const QString& KstViewLabel::fontName() const {
  return _fontName;
}


void KstViewLabel::setInterpreted(bool interpreted) {
  if (_interpret != interpreted) {
    _interpret = interpreted;
    reparse();
  }
}


void KstViewLabel::save(QTextStream &ts, const QString& indent) {
}


void KstViewLabel::setDoScalarReplacement(bool replace) {
  if (replace != _replace) {
    _replace = replace;
    drawToBuffer(_chunk);
  }
}


struct FontChange {
  int size;
  int y;
};


void KstViewLabel::drawToBuffer(LabelChunk *fi) {
  computeTextSize(fi); // hmm this is inefficient

  _backBuffer.buffer().resize(size());
  _backBuffer.buffer().fill(backgroundColor());
  QPainter p(&_backBuffer.buffer());
  QPen pen;
  pen.setColor(foregroundColor());
  p.setPen(pen);
  p.rotate(_rotation);
  //double rotationRadians = 3.1415926535897932333796 * _rotation / 180.0;

  // FIXME: text direction support
  int x, y;
  switch (KST_JUSTIFY_H(_justify)) {
    case KST_JUSTIFY_H_RIGHT:
        x = size().width() - _textWidth;
      break;
    case KST_JUSTIFY_H_CENTER:
        x = (size().width() - _textWidth) / 2;
      break;
    case KST_JUSTIFY_H_NONE:
    case KST_JUSTIFY_H_LEFT:
    default:
        x = 0;
      break;
  }

  int descent = _textHeight - 1 - _ascent;
  switch (KST_JUSTIFY_V(_justify)) {
    case KST_JUSTIFY_V_BOTTOM:
        y = size().height() - descent;
      break;
    case KST_JUSTIFY_V_CENTER:
        y = (size().height() + _textHeight) / 2 - descent;
      break;
    case KST_JUSTIFY_V_NONE:
    case KST_JUSTIFY_V_TOP:
    default:
        y = _textHeight - descent;
      break;
  }

  QPtrStack<FontChange> fstack;

  bool prevSym = false;
  QFont fnt(_fontName, _fontSize);
  p.setFont(fnt);
  int size = _fontSize;

  while (fi) {
    if (fi->voffset == None) {
      if (!fstack.isEmpty()) {
        FontChange *fc = fstack.pop();
        QFont f = p.font();
        f.setPointSize(fc->size);
        p.setFont(f);
        size = fc->size;
        y = fc->y;
        delete fc;
      }
    } else {
      FontChange *fc = new FontChange;
      fc->size = size;
      fc->y = y;
      fstack.push(fc);
      if (fi->voffset == Up) {
        y -= int(0.4 * p.fontMetrics().height());
      } else { // Down
        y += int(0.4 * p.fontMetrics().height());
      }
      if (size > 5) {
        size -= 1;
      }
      QFont f = p.font();
      f.setPointSize(size);
      p.setFont(f);
    }

    if (prevSym != fi->symbol) {
      prevSym = fi->symbol;
      QFont f = p.font();
      f.setFamily(fi->symbol ? _symbolFontName : _fontName);
      p.setFont(f);
    }
    p.drawText(x, y, fi->text);
    x += p.fontMetrics().width(fi->text);
    fi = fi->next;
  }
}


void KstViewLabel::computeTextSize(LabelChunk *fi) {
  QPtrStack<FontChange> fstack;
  bool prevSym = false;
  QFont fnt(_fontName, _fontSize);
  int size = _fontSize, y = 0, asc = 0, des = 0;
  _textWidth = 0;
  _textHeight = 0;

  // FIXME: code duplication is undesired here
  while (fi) {
    if (fi->voffset == None) {
      if (!fstack.isEmpty()) {
        FontChange *fc = fstack.pop();
        fnt.setPointSize(fc->size);
        size = fc->size;
        y = fc->y;
        delete fc;
      }
    } else {
      FontChange *fc = new FontChange;
      fc->size = size;
      fc->y = y;
      fstack.push(fc);
      if (fi->voffset == Up) {
        y -= int(0.4 * QFontMetrics(fnt).height());
      } else { // Down
        y += int(0.4 * QFontMetrics(fnt).height());
      }
      if (size > 5) {
        size -= 1;
      }
      fnt.setPointSize(size);
    }

    if (prevSym != fi->symbol) {
      prevSym = fi->symbol;
      fnt.setFamily(fi->symbol ? _symbolFontName : _fontName);
    }

    QFontMetrics fm(fnt);

    _textWidth += fm.width(fi->text);
    asc = QMAX(asc, - y + fm.ascent());
    if (- y - fm.descent() < 0) {
      des = QMAX(des, fm.descent() + y);
    }

    fi = fi->next;
  }

  _ascent = asc;
  _textHeight = 1 + asc + des;

  kdDebug() << "Computed label width=" << _textWidth << " height=" << _textHeight << " (asc,des)=" << asc << "," << des << endl;
}


void KstViewLabel::paint(KstPaintType type, QPainter& p) {
  KstBorderedViewObject::paint(type, p);
  _backBuffer.paintInto(p, geometry());
}


void KstViewLabel::adjustSizeForText() {
  QSize sz(_textWidth, _textHeight);
  if (_parent) {
    QRect r(position(), sz);
    resize(r.intersect(_parent->geometry()).size());
  } else {
    resize(sz);
  }
}


void KstViewLabel::edit() {
  // Hmm, we shouldn't hold widgets  FIXME
  if (!_editDialog) {
    _editDialog = new KstViewLabelDialogI;
  }
  _editDialog->showI(this);
}

bool KstViewLabel::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
  menu->insertItem(i18n("&Adjust Size"), this, SLOT(adjustSizeForText()));
  return true;
}


KstViewObjectPtr create_KstViewLabel() {
  return KstViewObjectPtr(new KstViewLabel(QString::null));
}


KstViewObjectFactoryMethod KstViewLabel::factory() const {
  return &create_KstViewLabel;
}

void KstViewLabel::writeBinary(QDataStream& str) {
  KstBorderedViewObject::writeBinary(str);
  str << _rotation << _txt << _symbolFontName << _fontName
      << _replace << _interpret << _justify
      ;
}


void KstViewLabel::readBinary(QDataStream& str) {
  KstBorderedViewObject::readBinary(str);
  bool b;
  str >> _rotation >> _txt >> _symbolFontName >> _fontName
      >> b;
  _replace = b;
  str >> b;
  _interpret = b;
  str >> b;
  _justify = b;
  reparse(); // FIXME: this should go away and updateFromAspect should be
             //        overridden.  this hack fails when dragging a label
             //        from a smaller parent to a larger one
}


#include "kstviewlabel.moc"
// vim: ts=2 sw=2 et
