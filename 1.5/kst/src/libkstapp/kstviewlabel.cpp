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

#include "kstviewlabel.h"

#include "enodes.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstgfxtextmousehandler.h"
#include "kst.h"
#include "kstcplugin.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewobjectfactory.h"
#include "labelrenderer.h"
#include "viewlabelwidget.h"

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdatastream.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpopupmenu.h>
#include "ksdebug.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qmetaobject.h>
#include <qptrstack.h>
#include <qtextedit.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <stdlib.h>

/* TODO: Rendering testcases */
#define MIN_FONT_SIZE 5

KstViewLabel::KstViewLabel(const QString& txt, KstLJustifyType justify, float rotation)
: KstBorderedViewObject("Label") {
  _fallThroughTransparency = false;
  _container = false;
  _dataPrecision = 8;
  _txt = txt;
  _interpret = true;
  _replace = true;
  _rotation = rotation;
  _justify = justify;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = -1;
  setFontSize(0);
  _standardActions |= Delete | Edit | Rename;
  _parsed = 0L;
  _labelMargin = 0;
  _isResizable = false;
  _editTitle = i18n("Edit Label");
  reparse();
  computeTextSize(_parsed);
}


KstViewLabel::KstViewLabel(const QDomElement& e) 
: KstBorderedViewObject(e) {
  // some defaults and invariants
  _fallThroughTransparency = false;
  _container = false;
  _type = "Label";
  _editTitle = i18n("Edit Label");
  _dataPrecision = 8;
  _interpret = true;
  _replace = true;
  _rotation = 0.0;
  _labelMargin = 0;
  _justify = 0L;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = -1;
  setFontSize(0);
  _standardActions |= Delete | Edit | Rename;
  _parsed = 0L;
  _isResizable = false;
  reparse();

  // read the properties
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); 
    if (!el.isNull()) {
      if (metaObject()->findProperty(el.tagName().latin1(), true) > -1) {
        setProperty(el.tagName().latin1(), QVariant(el.text()));  
      }
    }
    n = n.nextSibling();
  }
}


KstViewLabel::KstViewLabel(const KstViewLabel& label)
: KstBorderedViewObject(label) {
  _editTitle = i18n("Edit Label");

  _dataPrecision = label._dataPrecision;
  _interpret = label._interpret;
  _replace = label._replace;
  _rotation = label._rotation;
  _labelMargin = label._labelMargin;
  _justify = label._justify;
  _fontName = label._fontName;
  _fontSize = label._fontSize;
  _absFontSize = label._absFontSize;
  _txt = label._txt;
  _standardActions |= Delete | Edit | Rename;

  _parsed = 0L;
  reparse();

  // these always have these values
  _type = "Label";
}


KstViewLabel::~KstViewLabel() {
  delete _parsed;
  _parsed = 0L;
}


KstViewObject* KstViewLabel::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  Q_UNUSED(name)

  KstViewLabel* viewLabel = new KstViewLabel(*this);
  parent.appendChild(viewLabel, true);

  return viewLabel;
}


KstViewObject* KstViewLabel::copyObjectQuietly() const {
  KstViewLabel* viewLabel = new KstViewLabel(*this);

  return viewLabel;
}


void KstViewLabel::setupConnections() {
  for (KstScalarMap::iterator i = _scalarsUsed.begin(); i != _scalarsUsed.end(); ++i) {
    disconnect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
    connect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
  } 
  for (KstStringMap::iterator i = _stringsUsed.begin(); i != _stringsUsed.end(); ++i) {
    disconnect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
    connect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
  } 
  for (KstVectorMap::iterator i = _vectorsUsed.begin(); i != _vectorsUsed.end(); ++i) {
    disconnect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
    connect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
  } 
}


void KstViewLabel::reparse() {
  delete _parsed;
  _parsed = Label::parse(_txt, _interpret);
  collectObjects(_parsed, _vectorsUsed, _scalarsUsed, _stringsUsed);
  _txt = labelText(_txt, _parsed, _vectorsUsed, _scalarsUsed, _stringsUsed);
  setDirty();
  setupConnections();
}


void KstViewLabel::setText(const QString& text) {
  if (_txt != text) {
    _txt = text;
    _scalarsUsed.clear();
    _stringsUsed.clear();
    _vectorsUsed.clear();
    reparse(); // calls setDirty()
  }
}


const QString& KstViewLabel::text() const {
  return _txt;
}


void KstViewLabel::setRotation(double rotation) {
  if (_rotation != rotation) {
    setDirty();
    _rotation = rotation;
  }
}


void KstViewLabel::setJustification(KstLJustifyType justify) {
  if (_justify != justify) {
    setDirty();
    _justify = justify;
  }
}

void KstViewLabel::resize(const QSize& size) {
  KstBorderedViewObject::resize(size);
  if (!_parsed) {
    reparse();
  }
}


int KstViewLabel::ascent() const {
  return _ascent;
}


void KstViewLabel::setFontName(const QString& fontName) {
  if (_fontName != fontName) {
    setDirty();
    _fontName = fontName;
  }
}


const QString& KstViewLabel::fontName() const {
  return _fontName;
}


void KstViewLabel::setInterpreted(bool interpreted) {
  if (_interpret != interpreted) {
    _interpret = interpreted;
    reparse(); // calls setDirty();
  }
}


void KstViewLabel::save(QTextStream &ts, const QString& indent) {
  reparse();
  ts << indent << "<" << type() << ">" << endl;
  KstBorderedViewObject::save(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


void KstViewLabel::setDoScalarReplacement(bool replace) {
  if (replace != _replace) {
    setDirty();
    _replace = replace;
  }
}


void KstViewLabel::drawToBuffer(Label::Parsed *lp) {
  _backBuffer.buffer().resize(contentsRect().size());
  _backBuffer.buffer().fill(backgroundColor());
  QPainter p(&_backBuffer.buffer());
  drawToPainter(lp, p);
}


void KstViewLabel::drawToPainter(Label::Parsed *lp, QPainter& p) {
  int hJust = KST_JUSTIFY_H(_justify);
  if (QApplication::reverseLayout()) {
    if (hJust == KST_JUSTIFY_H_NONE) {
      hJust = KST_JUSTIFY_H_RIGHT;
    }
  } else {
    if (hJust == KST_JUSTIFY_H_NONE) {
      hJust = KST_JUSTIFY_H_LEFT;
    }
  }

  RenderContext rc(_fontName, _absFontSize, &p);
  rc.setSubstituteScalars(_replace);
  rc.precision = _dataPrecision;
  rc._cache = &_cache.data;
  _cache.valid = false;
  _cache.data.clear();
  double rotationRadians = M_PI * (int(_rotation) % 360) / 180;
  double absin = fabs(sin(rotationRadians));
  double abcos = fabs(cos(rotationRadians));

  int tx = 0, ty = 0; // translation
  const QRect cr(contentsRect());

  switch (hJust) {
    case KST_JUSTIFY_H_RIGHT:
      rc.x = -_textWidth / 2;
      tx = cr.width() - int(_textWidth * abcos + _textHeight * absin) / 2 - _labelMargin*_ascent/10;
      break;
    case KST_JUSTIFY_H_CENTER:
      rc.x = -_textWidth / 2;
      tx = cr.width() / 2;
      break;
    case KST_JUSTIFY_H_NONE:
      abort(); // should never be able to happen
    case KST_JUSTIFY_H_LEFT:
    default:
      rc.x = -_textWidth / 2;
      tx = int(_textWidth * abcos + _textHeight * absin) / 2  + _labelMargin*_ascent/10;
      break;
  }

  rc.y = _ascent - _textHeight / 2;
  ty = int(_textHeight * abcos + _textWidth * absin) / 2 + _labelMargin*_ascent/10;

  p.translate(tx, ty);
  p.rotate(_rotation);

  rc.pen = foregroundColor();

  rc.xStart = rc.x;
#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  if (lp && lp->chunk) {
    renderLabel(rc, lp->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
    _cache.valid = true;
  }
#ifdef BENCHMARK
  kstdDebug() << "render took: " << t.elapsed() << endl;
  t.start();
#endif
  QApplication::syncX();
#ifdef BENCHMARK
  kstdDebug() << "sync X took: " << t.elapsed() << endl;
#endif
}


void KstViewLabel::computeTextSize(Label::Parsed *lp) {
  if (lp && lp->chunk) {
    RenderContext rc(_fontName, _absFontSize, 0L);
    rc.setSubstituteScalars(_replace);
    rc.precision = _dataPrecision;
#ifdef BENCHMARK
    QTime t;
    t.start();
#endif
    renderLabel(rc, lp->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
#ifdef BENCHMARK
    kstdDebug() << "compute (false render) took: " << t.elapsed() << endl;
#endif
    _textWidth = rc.xMax;
    _ascent = rc.ascent;
    _textHeight = 1 + rc.ascent + rc.descent;
  }
}


KstObject::UpdateType KstViewLabel::update(int counter) {
  if (checkUpdateCounter(counter)) {
    return lastUpdateResult();
  }

  KstObject::UpdateType rc = NO_CHANGE;

  _cache.update();

  if (!_cache.valid) {
    rc = UPDATE;
    setDirty();
  }

  if (rc != UPDATE) {
    rc = KstBorderedViewObject::update(counter);
  } else {
    KstBorderedViewObject::update(counter);
  }

  return setLastUpdateResult(rc);
}


void KstViewLabel::updateSelf() {
  bool wasDirty(dirty());
  KstBorderedViewObject::updateSelf();

  // FIXME: remove this once update() is called often enough.
  if (!wasDirty) {
    _cache.update();

    if (!_cache.valid) {
      wasDirty = true;
    }
  }

  if (wasDirty) {
    adjustSizeForText(_parent->geometry());
    drawToBuffer(_parsed);
  }
}


void KstViewLabel::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() == KstPainter::P_PRINT || p.type() == KstPainter::P_EXPORT) {
    int absFontSizeOld = _absFontSize;

    QRect cr(contentsRectForPainter(p));
    cr.setSize(sizeForText(_parent->geometry()));
    setContentsRectForPainter(p, cr);    
    KstBorderedViewObject::paintSelf(p, bounds);

    p.translate(cr.left(), cr.top());
    if (!_transparent) {
      p.fillRect(0, 0, cr.width(), cr.height(), backgroundColor());
    }
    drawToPainter(_parsed, p);

    _absFontSize = absFontSizeOld;
  } else {
    if (p.makingMask()) {
      p.setRasterOp(Qt::SetROP);
    } else {
      const QRegion clip(clipRegion());
      KstBorderedViewObject::paintSelf(p, bounds - _myClipMask);
      p.setClipRegion(bounds & clip);
    }

    _backBuffer.paintInto(p, contentsRect());
  }
  p.restore();
}


void KstViewLabel::invalidateClipRegion() {
  KstBorderedViewObject::invalidateClipRegion();
  _myClipMask = QRegion();
}


QRegion KstViewLabel::clipRegion() {
  if (!_transparent) {
    return KstBorderedViewObject::clipRegion();
  }

  if (_clipMask.isNull() && _myClipMask.isNull()) {
    const QRect cr(contentsRect());
    QBitmap bm = _backBuffer.buffer().createHeuristicMask(false); // slow but preserves antialiasing...
    _myClipMask = QRegion(bm);
    _myClipMask.translate(cr.topLeft().x(), cr.topLeft().y());

    QBitmap bm1(_geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1, true);
    if (!bm1.isNull()) {
      KstPainter p;
      p.setMakingMask(true);
      p.begin(&bm1);
      p.setViewXForm(true);
      KstBorderedViewObject::paintSelf(p, QRegion());
      p.flush();
      p.end();
      _clipMask = QRegion(bm1);
    }
  }

  return _clipMask | _myClipMask;
}


void KstViewLabel::setFontSize(int size) {
  if (_fontSize != size) {
    _absFontSize = size + KstSettings::globalSettings()->plotFontSize;
    if (_absFontSize < KstSettings::globalSettings()->plotFontMinSize) {
      _absFontSize = KstSettings::globalSettings()->plotFontMinSize;
    }

    _fontSize = size;
    setDirty();
  }
}


int KstViewLabel::fontSize() const {
  return _fontSize;
}


void KstViewLabel::adjustSizeForText(const QRect& w) {
  QRect cr(contentsRect());
  cr.setSize(sizeForText(w));
  setContentsRect(cr);
}


QSize KstViewLabel::sizeForText(const QRect& w) {
  double x_s, y_s;

  x_s = y_s = _fontSize + (double)KstSettings::globalSettings()->plotFontSize;

  int x_pix = w.width();
  int y_pix = w.height();

  if (x_pix < y_pix) {
    x_s *= x_pix/540.0;
    y_s *= y_pix/748.0;
  } else {
    y_s *= y_pix/540.0;
    x_s *= x_pix/748.0;
  }

  _absFontSize = int((x_s + y_s)/2.0);
  if (_absFontSize < KstSettings::globalSettings()->plotFontMinSize) {
    _absFontSize = KstSettings::globalSettings()->plotFontMinSize;
  }

  if (!_parsed) {
    reparse();
  }

  if (_parsed) {
    computeTextSize(_parsed);
  }

  QSize sz(kMax(1, _textWidth), kMax(1, _textHeight));

  if (int(_rotation) != 0 && int(_rotation) != 180) {
    QPointArray pts(4);
    pts[0] = QPoint(0, 0);
    pts[1] = QPoint(0, _textHeight);
    pts[2] = QPoint(_textWidth, _textHeight);
    pts[3] = QPoint(_textWidth, 0);
    double theta = M_PI * (int(_rotation) % 360) / 180;
    double sina = sin(theta);
    double cosa = cos(theta);
    QWMatrix m(cosa, sina, -sina, cosa, 0.0, 0.0);

    pts = m.map(pts);

    if (_parent) {
      QRect r(position(), pts.boundingRect().size());
      r.setSize(r.size() + QSize(2 * _labelMargin * _ascent / 10, 2 * _labelMargin * _ascent / 10));
      sz = r.intersect(_parent->geometry()).size();
    } else {
      sz = pts.boundingRect().size();
      sz += QSize(2 * _labelMargin * _ascent / 10, 2 * _labelMargin * _ascent / 10);
    }
  } else {
    if (_parent) {
      QRect r(position(), sz);
      r.setSize(r.size() + QSize(2 * _labelMargin * _ascent / 10, 2 * _labelMargin * _ascent / 10));
      sz = r.intersect(_parent->geometry()).size();
    }
  }

  return sz;
}


bool KstViewLabel::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
  return true;
}


bool KstViewLabel::interpreted() const {
  return _interpret;
}


bool KstViewLabel::doScalarReplacement() const {
  return _replace;
}


KstViewObjectPtr create_KstViewLabel() {
  return KstViewObjectPtr(new KstViewLabel(QString::null));
}


KstViewObjectFactoryMethod KstViewLabel::factory() const {
  return &create_KstViewLabel;
}


void KstViewLabel::writeBinary(QDataStream& str) {
  KstBorderedViewObject::writeBinary(str);
  str << _rotation << _txt << _fontName << _replace << _interpret << _justify;
}


void KstViewLabel::readBinary(QDataStream& str) {
  KstBorderedViewObject::readBinary(str);
  bool b;
  str >> _rotation >> _txt >> _fontName >> b;
  _replace = b;
  str >> b;
  _interpret = b;
  str >> b;
  _justify = b;
  reparse(); // FIXME: this should go away and updateFromAspect should be
             //        overridden.  this hack fails when dragging a label
             //        from a smaller parent to a larger one
}


double KstViewLabel::rotation() const {
  return _rotation; 
}


bool KstViewLabel::fillConfigWidget(QWidget *w, bool isNew) const {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return false;
  }

  if (!isNew) {
    widget->_text->setText(text());
  }

  widget->_precision->setValue(int(dataPrecision()));
  widget->_rotation->setValue(double(rotation()));
  widget->_fontSize->setValue(int(fontSize()));
  widget->_horizontal->setCurrentItem(horizJustifyWrap());
  widget->_fontColor->setColor(foregroundColor());
  widget->_font->setCurrentFont(fontName());

  widget->_transparent->setChecked(transparent());
  widget->_border->setValue(borderWidth());
  widget->_boxColors->setForeground(borderColor());
  widget->_boxColors->setBackground(backgroundColor());
  widget->_margin->setValue(labelMargin());

  widget->_text->setFocus();

  return true;
}


bool KstViewLabel::readConfigWidget(QWidget *w, bool editMultipleMode) {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return false;
  }

  if (!editMultipleMode || widget->_text->text().compare(QString(" ")) != 0) {
    _txt = widget->_text->text();
  }

  if (!editMultipleMode || widget->_precision->value() != widget->_precision->minValue()) {
    setDataPrecision(widget->_precision->value());
  }

  if (!editMultipleMode || widget->_rotation->value() != widget->_rotation->minValue()) {
    setRotation(widget->_rotation->value());
  }

  if (!editMultipleMode || widget->_fontSize->value() != widget->_fontSize->minValue()) {
    setFontSize(widget->_fontSize->value());
  }

  if (!editMultipleMode || widget->_horizontal->currentText().compare(QString(" ")) != 0) {
    setHorizJustifyWrap(widget->_horizontal->currentItem());
  }

  if (!editMultipleMode || widget->_fontColor->color() != QColor()) {
    setForegroundColor(widget->_fontColor->color());
  }

  if (!editMultipleMode || widget->_font->currentText().compare(QString(" ")) != 0) {
    setFontName(widget->_font->currentFont());
  }

  if (!editMultipleMode || widget->_transparent->state() != QButton::NoChange) {
    setTransparent(widget->_transparent->isChecked());
  }

  if (!editMultipleMode || widget->_border->value() != widget->_border->minValue()) {
    setBorderWidth(widget->_border->value());
  }

  if (!editMultipleMode || widget->_changedFgColor) {
    setBorderColor(widget->_boxColors->foreground());
  }

  if (!editMultipleMode || widget->_changedBgColor) {
    setBackgroundColor(widget->_boxColors->background());
  }

  if (!editMultipleMode || widget->_margin->value() != widget->_margin->minValue()) {
    setLabelMargin(widget->_margin->value());
  }

  reparse(); // calls setDirty()
  return true;
}


void KstViewLabel::connectConfigWidget(QWidget *parent, QWidget *w) const {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return;
  }

  connect(widget->_text, SIGNAL(textChanged()), parent, SLOT(modified()));
  connect(widget->_font, SIGNAL(activated(int)), parent, SLOT(modified()));
  connect(widget->_fontSize, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_fontSize->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_horizontal, SIGNAL(activated(int)), parent, SLOT(modified()));
  connect(widget->_fontColor, SIGNAL(changed(const QColor&)), parent, SLOT(modified()));  
  connect(widget->_precision, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_precision->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_rotation, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_rotation, SIGNAL(valueChanged(double)), parent, SLOT(modified()));
  connect(widget->_rotation->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_transparent, SIGNAL(pressed()), parent, SLOT(modified()));
  connect(widget->_boxColors, SIGNAL(fgChanged(const QColor&)), parent, SLOT(modified()));
  connect(widget->_boxColors, SIGNAL(bgChanged(const QColor&)), parent, SLOT(modified()));
  connect(widget->_boxColors, SIGNAL(fgChanged(const QColor&)), widget, SLOT(changedFgColor()));
  connect(widget->_boxColors, SIGNAL(bgChanged(const QColor&)), widget, SLOT(changedBgColor()));
  connect(widget->_margin, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_margin->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_border, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_border->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
}


void KstViewLabel::populateEditMultiple(QWidget *w) {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return;
  }

  widget->_text->setText(QString(" "));

  widget->_font->insertItem(QString(" "));
  widget->_font->setCurrentItem(widget->_font->count()-1);

  widget->_fontSize->setMinValue(widget->_fontSize->minValue() - 1);
  widget->_fontSize->setSpecialValueText(QString(" "));
  widget->_fontSize->setValue(widget->_fontSize->minValue());

  widget->_horizontal->insertItem(QString(" "));
  widget->_horizontal->setCurrentItem(widget->_horizontal->count()-1);

  widget->_fontColor->setColor(QColor());

  widget->_transparent->setTristate();
  widget->_transparent->setNoChange();

  widget->_boxColors->setForeground(QColor());
  widget->_boxColors->setBackground(QColor());

  widget->_precision->setMinValue(widget->_precision->minValue() - 1);
  widget->_precision->setSpecialValueText(QString(" "));
  widget->_precision->setValue(widget->_precision->minValue());

  widget->_rotation->setMinValue(widget->_rotation->minValue() - 1);
  widget->_rotation->setSpecialValueText(QString(" "));
  widget->_rotation->setValue(widget->_rotation->minValue());

  widget->_margin->setMinValue(widget->_margin->minValue() - 1);
  widget->_margin->setSpecialValueText(QString(" "));
  widget->_margin->setValue(widget->_margin->minValue());

  widget->_border->setMinValue(widget->_border->minValue() - 1);
  widget->_border->setSpecialValueText(QString(" "));
  widget->_border->setValue(widget->_border->minValue());

  widget->_changedFgColor = false;
  widget->_changedBgColor = false;
}


void KstViewLabel::setDataPrecision(int prec) {
  int n;

  if (prec < 0) {
    n = 0;
  } else if (prec > 16) {
    n = 16;
  } else {
    n = prec;
  }

  if (n != _dataPrecision) {
    setDirty();
    _dataPrecision = n;
  }
}


int KstViewLabel::dataPrecision() const {
  return _dataPrecision;
}


void KstViewLabel::setTransparent(bool transparent) {
  KstViewObject::setTransparent(transparent);  
}


bool KstViewLabel::transparent() const {
  return KstViewObject::transparent();  
}


int KstViewLabel::horizJustifyWrap() const {
  Q_UINT8 justify = KST_JUSTIFY_H(justification());
  switch (justify) {
    case KST_JUSTIFY_H_LEFT:
      return 0;
      break;
    case KST_JUSTIFY_H_RIGHT:
      return 1;
      break;
    case KST_JUSTIFY_H_CENTER:
      return 2;
      break;
    default:
      return 0;  
  }
}


void KstViewLabel::setHorizJustifyWrap(int justify) {
  Q_UINT8 justifySet;

  switch (justify) {
    case 0:
      justifySet = KST_JUSTIFY_H_LEFT;
      break;
    case 1:
      justifySet = KST_JUSTIFY_H_RIGHT;
      break;
    case 2:
      justifySet = KST_JUSTIFY_H_CENTER;
      break;
    default:
      justifySet = KST_JUSTIFY_H_LEFT;
  }
  setJustification(SET_KST_JUSTIFY(justifySet, KST_JUSTIFY_V(justification())));
}


void KstViewLabel::setLabelMargin(int margin) {
  int mm = kMax(0, margin);

  if (mm != _labelMargin) {
    _labelMargin = mm;
    setDirty();
  }
}


int KstViewLabel::labelMargin() const {
  return _labelMargin;
}


QWidget *KstViewLabel::configWidget(QWidget *parent) {
  return new ViewLabelWidget(parent, "custom");
}


namespace {
KstViewObject *create_KstViewLabel() {
  return new KstViewLabel;
}


KstGfxMouseHandler *handler_KstViewLabel() {
  return new KstGfxTextMouseHandler;
}

KST_REGISTER_VIEW_OBJECT(Label, create_KstViewLabel, handler_KstViewLabel)
}


void KstViewLabel::DataCache::update() {
  for (QValueVector<DataRef>::ConstIterator i = data.begin(); valid && i != data.end(); ++i) {
    switch ((*i).type) {
      case DataRef::DataRef::DRScalar:
        {
          KST::scalarList.lock().readLock();
          KstScalarPtr p = *KST::scalarList.findTag((*i).name);
          KST::scalarList.lock().unlock();
          if (p) {
            p->readLock();
            if (QVariant(p->value()) != (*i).value) {
              valid = false;
            }
            p->unlock();
          }
        }
        break;

      case DataRef::DRString:
        {
          KST::stringList.lock().readLock();
          KstStringPtr p = *KST::stringList.findTag((*i).name);
          KST::stringList.lock().unlock();
          if (p) {
            p->readLock();
            if (QVariant(p->value()) != (*i).value) {
              valid = false;
            }
            p->unlock();
          }
        }
        break;

      case DataRef::DRExpression:
        {
          bool ok = false;
          const double val = Equation::interpret((*i).name.latin1(), &ok, (*i).name.length());
          if (QVariant(val) != (*i).value) {
            valid = false;
          }
        }
        break;

      case DataRef::DRVector:
        {
          bool ok = false;
          const double idx = Equation::interpret((*i).index.latin1(), &ok, (*i).index.length());
          if (idx != (*i).indexValue) {
            valid = false;
            break;
          }
          KST::vectorList.lock().readLock();
          KstVectorPtr p = *KST::vectorList.findTag((*i).name);
          KST::vectorList.lock().unlock();
          if (p) {
            p->readLock();
            if (QVariant(p->value(int((*i).indexValue))) != (*i).value) {
              valid = false;
            }
            p->unlock();
          }
        }
        break;

      case DataRef::DataRef::DRFit:
        {
          KST::dataObjectList.lock().readLock();
          KstDataObjectList::Iterator oi = KST::dataObjectList.findTag((*i).name);
          KST::dataObjectList.lock().unlock();
          if (oi != KST::dataObjectList.end()) {
            KstCPluginPtr fit = kst_cast<KstCPlugin>(*oi);
            if (fit) {
              fit->readLock(); 
              if (fit->label((int)((*i).indexValue)) != (*i).index) {
                valid = false;
              }
              fit->unlock();
            }
          }
        }
        break;
    }
  }
}

#include "kstviewlabel.moc"

