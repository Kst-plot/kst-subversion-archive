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
#include "kstgfxtextmousehandler.h"
#include "kst.h"
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
  _container = false;
  setFollowsFlow(true);
  _dataPrecision = 8;
  _autoResize = false; // avoid madness
  _txt = txt;
  _interpret = true;
  _replace = true;
  _rotation = rotation;
  _justify = justify;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = 0;
  _absFontSize = _fontSize+KstSettings::globalSettings()->plotFontSize;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  _parsed = 0L;
  _labelMargin = 0;
  reparse();
  computeTextSize(_parsed);
  _autoResize = true;
}


KstViewLabel::KstViewLabel(const QDomElement& e) 
: KstBorderedViewObject(e) {
  // some defaults and invariants
  _container = false;
  setFollowsFlow(true);
  _type = "Label";
  _dataPrecision = 8;
  _autoResize = false; // avoid madness
  _interpret = true;
  _replace = true;
  _rotation = 0.0;
  _labelMargin = 0;
  _justify = 0L;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = 0;
  _absFontSize = _fontSize+KstSettings::globalSettings()->plotFontSize;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  _parsed = 0L;
  reparse();
  
  // read the properties
  bool in_autoResize = false;
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); 
    if (!el.isNull()) {
      if (metaObject()->findProperty(el.tagName().latin1(), true) > -1) {
        if (el.tagName() == "autoResize") {
          in_autoResize = QVariant(el.text()).toBool();
        } else {
          setProperty(el.tagName().latin1(), QVariant(el.text()));  
        }
      }  
    }
    n = n.nextSibling();
  }

  _autoResize = in_autoResize;
}


KstViewLabel::~KstViewLabel() {
  delete _parsed;
  _parsed = 0L;
}


void KstViewLabel::reparse() {
  delete _parsed;
  _parsed = Label::parse(_txt, _interpret);
  setDirty();
}


void KstViewLabel::setText(const QString& text) {
  if (_txt != text) {
    _txt = text;
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
  setDirty(false);

  _backBuffer.buffer().resize(size());
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
  double rotationRadians = M_PI * (int(_rotation) % 360) / 180;
  double absin = fabs(sin(rotationRadians));
  double abcos = fabs(cos(rotationRadians));

  int tx = 0, ty = 0; // translation

  switch (hJust) {
    case KST_JUSTIFY_H_RIGHT:
      rc.x = -_textWidth / 2;
      tx = size().width() - int(_textWidth * abcos + _textHeight * absin) / 2  - borderWidth() - _labelMargin*_ascent/10;
      break;
    case KST_JUSTIFY_H_CENTER:
      rc.x = -_textWidth / 2;
      tx = size().width() / 2;
      break;
    case KST_JUSTIFY_H_NONE:
      abort(); // should never be able to happen
    case KST_JUSTIFY_H_LEFT:
    default:
      rc.x = -_textWidth / 2;
      tx = int(_textWidth * abcos + _textHeight * absin) / 2  + borderWidth() + _labelMargin*_ascent/10;
      break;
  }

  switch (KST_JUSTIFY_V(_justify)) {
    case KST_JUSTIFY_V_BOTTOM:
      rc.y = _ascent - _textHeight / 2;
      ty = size().height() - int(_textHeight * abcos + _textWidth * absin) / 2  - borderWidth() - _labelMargin*_ascent/10;
      break;
    case KST_JUSTIFY_V_CENTER:
      rc.y = _ascent - _textHeight / 2;
      ty = size().height() / 2;
      break;
    case KST_JUSTIFY_V_NONE:
    case KST_JUSTIFY_V_TOP:
    default:
      rc.y = _ascent - _textHeight / 2;
      ty = int(_textHeight * abcos + _textWidth * absin) / 2  + borderWidth() + _labelMargin*_ascent/10;
      break;
  }

  p.translate(tx, ty);
  p.rotate(_rotation);

  QPen pen;
  pen.setColor(foregroundColor());
  p.setPen(pen);

  rc.xStart = rc.x;
#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  if (lp && lp->chunk) {
    renderLabel(rc, lp->chunk);
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
    renderLabel(rc, lp->chunk);
#ifdef BENCHMARK
    kstdDebug() << "compute (false render) took: " << t.elapsed() << endl;
#endif
    _textWidth = rc.xMax;
    _ascent = rc.ascent;
    _textHeight = 1 + rc.ascent + rc.descent;
  }
}


void KstViewLabel::updateSelf() {
  if (dirty()) {
    if (_autoResize) {
      adjustSizeForText(contentsRect());
    } else {
      computeTextSize(_parsed);
    }
    drawToBuffer(_parsed);
  }
  KstBorderedViewObject::updateSelf();
}


void KstViewLabel::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() == KstPainter::P_PRINT || p.type() == KstPainter::P_EXPORT) {
    KstBorderedViewObject::paintSelf(p, bounds);
    if (_autoResize) {
      adjustSizeForText(p.window());
    } else {
      computeTextSize(_parsed);
    }
    const QRect geom(geometry());
    p.setViewport(geom);
    p.setWindow(0, 0, geom.width(), geom.height());

    if (!_transparent) {
      p.fillRect(p.window(), backgroundColor());
    }

    drawToPainter(_parsed, p);
  } else {
    if (p.makingMask()) {
      p.setRasterOp(Qt::SetROP);
    } else {
      const QRegion clip(clipRegion());
      KstBorderedViewObject::paintSelf(p, bounds - _myClipMask);
      p.setClipRegion(bounds & clip);
    }

    _backBuffer.paintInto(p, geometry());
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

    QBitmap bm1(_geom.bottomRight().x(), _geom.bottomRight().y(), true);
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
    if (_absFontSize < MIN_FONT_SIZE) {
      _absFontSize = MIN_FONT_SIZE;
    }
    
    _fontSize = size;
    setDirty();
  }
}


int KstViewLabel::fontSize() const {
  return _fontSize;
}


void KstViewLabel::adjustSizeForText(QRect w) {
  if (_autoResize) {
    double x_s, y_s, s;
    
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

    s = (x_s + y_s)/2.0;

    if (s < MIN_FONT_SIZE) {
      s = MIN_FONT_SIZE;
    }
    _absFontSize = int(s);
    if (_absFontSize < MIN_FONT_SIZE) {
      _absFontSize = MIN_FONT_SIZE;
    }
  }
 
  if (!_parsed) {
    reparse();
  }

  if (_parsed) {
    computeTextSize(_parsed);
  }
  
  QSize sz(_textWidth, _textHeight);
  if (int(_rotation) != 0 && int(_rotation) != 180) {
    QPointArray pts(4);
    pts[0] = QPoint(0, 0);
    pts[1] = QPoint(0, _textHeight);
    pts[2] = QPoint(_textWidth, 0);
    pts[3] = QPoint(_textWidth, _textHeight);
    double theta = M_PI * (int(_rotation) % 360) / 180;
    double sina = sin(theta);
    double cosa = cos(theta);
    QWMatrix m(cosa, sina, -sina, cosa, 0.0, 0.0);

    pts = m.map(pts);

    if (_parent) {
      QRect r(position(), pts.boundingRect().size());
      sz = r.intersect(_parent->geometry()).size();
    } else {
      sz = pts.boundingRect().size();
    }
  } else {
    if (_parent) {
      QRect r(position(), sz);
      sz = r.intersect(_parent->geometry()).size();
    }
  }

  resize(sz + QSize((borderWidth()+_labelMargin*_ascent/10)*2, (borderWidth()+_labelMargin*_ascent/10)*2));
}


bool KstViewLabel::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
  //menu->insertItem(i18n("&Adjust Size"), this, SLOT(adjustSizeForText()));
  return true;
}


bool KstViewLabel::interpreted() const {
  return _interpret;
}


bool KstViewLabel::doScalarReplacement() const {
  return _replace;
}


void KstViewLabel::setAutoResize(bool on) {
    _autoResize = on;
}


bool KstViewLabel::autoResize() const {
  return _autoResize;
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
  str >> _rotation >> _txt >> _fontName
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


double KstViewLabel::rotation() const { 
  return _rotation; 
}

bool KstViewLabel::fillConfigWidget(QWidget *w, bool isNew) const {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return false;
  }
  
  if (isNew) { // probably a new label: set widget to defaults
    widget->_precision->setValue(8);
    widget->_rotation->setValue(0);
    widget->_fontSize->setValue(0);
    widget->_horizontal->setCurrentItem(2);
    widget->_vertical->setCurrentItem(2);
    widget->_fontColor->setColor(KstSettings::globalSettings()->foregroundColor);
    widget->_font->setCurrentFont(KstApp::inst()->defaultFont());
    widget->_margin->setValue(5);

    widget->_boxColors->setForeground(KstSettings::globalSettings()->foregroundColor);
    widget->_boxColors->setBackground(KstSettings::globalSettings()->backgroundColor);

    if (size().width()*size().height()<25) { // assume a click, and default to just text
      widget->_transparent->setChecked(true);
      widget->_autoResize->setChecked(true);
      widget->_border->setValue(0);
    } else { // someone drew a box, so assume that is what they wanted
      widget->_transparent->setChecked(false);
      widget->_autoResize->setChecked(false);
      widget->_border->setValue(2);
    }

  } else { 
    // replace \n & \t with tabs and newlines for the text edit box
    QString tmpstr = text();
    tmpstr.replace(QString("\\n"), "\n");
    tmpstr.replace(QString("\\t"), "\t");
    widget->_text->setText(tmpstr);

    widget->_precision->setValue(int(dataPrecision()));
    widget->_rotation->setValue(double(rotation()));
    widget->_fontSize->setValue(int(fontSize()));
    widget->_horizontal->setCurrentItem(horizJustifyWrap());
    widget->_vertical->setCurrentItem(vertJustifyWrap());
    widget->_fontColor->setColor(foregroundColor());
    widget->_font->setCurrentFont(fontName());

    widget->_transparent->setChecked(transparent());
    widget->_autoResize->setChecked(autoResize());
    widget->_border->setValue(borderWidth());
    widget->_boxColors->setForeground(borderColor());
    widget->_boxColors->setBackground(backgroundColor());
    widget->_margin->setValue(_labelMargin);
  }
  widget->_text->setFocus();
  return true;
}


bool KstViewLabel::readConfigWidget(QWidget *w) {
  ViewLabelWidget *widget = dynamic_cast<ViewLabelWidget*>(w);
  if (!widget) {
    return false;
  }

  // Replace tabs and newlines in text edit box with \n and \t 
  _txt = widget->_text->text();
  _txt.replace(QString("\n"), "\\n");
  _txt.replace(QString("\t"), "\\t");

  setDataPrecision(widget->_precision->value());
  setRotation(widget->_rotation->value());
  setFontSize(widget->_fontSize->value());
  setHorizJustifyWrap(widget->_horizontal->currentItem());
  setVertJustifyWrap(widget->_vertical->currentItem());
  setForegroundColor(widget->_fontColor->color());
  setFontName(widget->_font->currentFont());

  setTransparent(widget->_transparent->isChecked());
  setAutoResize(widget->_autoResize->isChecked());
  setBorderWidth(widget->_border->value());
  setBorderColor(widget->_boxColors->foreground());
  setBackgroundColor(widget->_boxColors->background());
  setLabelMargin(widget->_margin->value());
  
  reparse(); // calls setDirty()
  return true;
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
    
        
int KstViewLabel::vertJustifyWrap() const {
  Q_UINT8 justify = KST_JUSTIFY_V(justification());
  switch (justify) {
    case KST_JUSTIFY_V_TOP:
      return 0;
      break;
    case KST_JUSTIFY_V_BOTTOM:
      return 1;
      break;
    case KST_JUSTIFY_V_CENTER:
      return 2;
      break;
    default:
      return 0;  
  }
}


void KstViewLabel::setVertJustifyWrap(int justify) {
  Q_UINT8 justifySet;
  
  switch (justify) {
    case 0:
      justifySet = KST_JUSTIFY_V_TOP;
      break;  
    case 1:
      justifySet = KST_JUSTIFY_V_BOTTOM;
      break;
    case 2:
      justifySet = KST_JUSTIFY_V_CENTER;
      break;
    default:
      justifySet = KST_JUSTIFY_V_TOP;
  }
  setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H(justification()), justifySet));
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


QWidget *KstViewLabel::configWidget() {
  return new ViewLabelWidget;
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


#include "kstviewlabel.moc"
// vim: ts=2 sw=2 et
