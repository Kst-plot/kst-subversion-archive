/***************************************************************************
                              kstviewlegend.cpp
                             ------------------
    begin                : September 2005
    copyright            : (C) 2005 by cbn
                           (C) 2005 by The University of Toronto
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

#include "kstviewlegend.h"

#include "enodes.h"
#include "ksdebug.h"
#include "kst2dplot.h"
#include "kstdatacollection.h"
#include "kst.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewobjectfactory.h"
#include "labelrenderer.h"
#include "viewlegendwidget.h"
#include "plotlistbox.h"

#include <kdatastream.h>
#include <kglobal.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qdeepcopy.h>
#include <qmetaobject.h>
#include <qptrstack.h>
#include <qstylesheet.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <stdlib.h>

#define MIN_FONT_SIZE 5

KstViewLegend::KstViewLegend()
: KstBorderedViewObject("Legend") {
  _container = false;
  _rotation = 0;
  _vertical = true;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = 0;
  setForegroundColor(KstSettings::globalSettings()->foregroundColor);
  setBorderColor(KstSettings::globalSettings()->foregroundColor);
  setBackgroundColor(KstSettings::globalSettings()->backgroundColor);
  setBorderWidth(2);
  _legendMargin = 5;
  _absFontSize = _fontSize+KstSettings::globalSettings()->plotFontSize;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  computeTextSize();
  setDirty(false);
}


KstViewLegend::KstViewLegend(const QDomElement& e)
: KstBorderedViewObject(e) {

  // some defaults and invariants
  _container = false;
  _type = "Legend";
  _rotation = 0.0;
  _fontName = KstApp::inst()->defaultFont();
  _fontSize = 0;
  _vertical = true;
  _legendMargin = 5;
  _absFontSize = _fontSize+KstSettings::globalSettings()->plotFontSize;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  QStringList ctaglist;

  // read the properties
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement();
    if (!el.isNull()) {
      if (metaObject()->findProperty(el.tagName().latin1(), true) > -1) {
        setProperty(el.tagName().latin1(), QVariant(el.text()));
      } else if (el.tagName() == "curvetag") {
        ctaglist.append(el.text()); 
      }
    }
    n = n.nextSibling();
  }

  KstBaseCurveList l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  KstBaseCurveList::ConstIterator end = l.end();
  for (QStringList::ConstIterator str = ctaglist.begin(); str != ctaglist.end(); ++str) {
    KstBaseCurveList::ConstIterator it = l.findTag(*str);
    if (it != end) {
      addCurve(*it);
    }
  }
}


KstViewLegend::~KstViewLegend() {
}


void KstViewLegend::resize(const QSize& size) {
  KstBorderedViewObject::resize(size);
  drawToBuffer();
}


int KstViewLegend::ascent() const {
  return _ascent;
}


void KstViewLegend::setFontName(const QString& fontName) {
  if (_fontName != fontName) {
    setDirty();
    _fontName = fontName;
  }
}


const QString& KstViewLegend::fontName() const {
  return _fontName;
}


void KstViewLegend::save(QTextStream &ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstBorderedViewObject::save(ts, indent + "  ");
  
  for (KstBaseCurveList::ConstIterator j = _curves.begin(); j != _curves.end(); ++j) {
    (*j)->readLock();
    ts << indent+"  " << "<curvetag>" << QStyleSheet::escape((*j)->tagName()) << "</curvetag>" << endl;
    (*j)->readUnlock();
  }

  ts << indent << "</" << type() << ">" << endl;
}


void KstViewLegend::drawToBuffer() {
  setDirty(false);

  _backBuffer.buffer().resize(size());
  _backBuffer.buffer().fill(backgroundColor());
  QPainter p(&_backBuffer.buffer());
  QPen pen;
  pen.setColor(foregroundColor());
  p.setPen(pen);
  drawToPainter(p);
}


void KstViewLegend::drawToPainter(QPainter& p) {
  RenderContext rc(_fontName, _absFontSize, &p);

  if (_vertical) {
    unsigned i = 0;
    for (KstBaseCurveList::Iterator it = _curves.begin(); it != _curves.end(); ++it) {
      p.save();
      if ((*it)->parsedLegendTag()) {
        p.translate(borderWidth() + _legendMargin*_ascent/10, borderWidth() + _legendMargin*_ascent/10 + (i)*(rc.fontHeight()+_ascent/4));
        QRect symbolBound(QPoint(0,0),
                          QSize(16*_ascent/4, rc.fontHeight()));

        (*it)->paintLegendSymbol(&p, symbolBound);
        p.translate(9*_ascent/2, 0);
        rc.x = 0;
        rc.y = _ascent;
        rc.xStart = rc.x;
        renderLabel(rc, (*it)->parsedLegendTag()->chunk);
      }
      p.restore();
      ++i;
    }
  } else {
    p.save();
    p.translate(borderWidth() + _legendMargin*_ascent/10, borderWidth() + _legendMargin*_ascent/10);
    for (KstBaseCurveList::Iterator it = _curves.begin(); it != _curves.end(); ++it) {
      if ((*it)->parsedLegendTag()) {
        QRect symbolBound(QPoint(0,0),
                          QSize(16*_ascent/4, rc.fontHeight()));
        (*it)->paintLegendSymbol(&p, symbolBound);
        p.translate(9*_ascent/2, 0);
        rc.x = 0;//(*it)->legendLabelSize().width() / 2;
        rc.y = _ascent;
        rc.xStart = rc.x;
        renderLabel(rc, (*it)->parsedLegendTag()->chunk);
        p.translate((*it)->legendLabelSize().width() + _ascent,0);
      }
    }
    p.restore();
  }

  QApplication::syncX();
}


void KstViewLegend::computeTextSize() {
  _textWidth = 0;
  _ascent = 0;
  _textHeight = 0;

  for (KstBaseCurveList::Iterator it = _curves.begin(); it != _curves.end(); it++) {
    if ((*it)->parsedLegendTag()) {
      RenderContext rc(_fontName, _absFontSize, 0L);
      renderLabel(rc, (*it)->parsedLegendTag()->chunk);
      if (_vertical) {
        if (rc.xMax>_textWidth) {
          _textWidth = rc.xMax;
        }
      } else {
        if (rc.fontHeight() > _textHeight) {
          _textHeight = rc.fontHeight();
        }
        _textWidth += rc.xMax;
      }
      (*it)->setLegendLabelSize(QSize(rc.xMax, rc.fontHeight()));
    } else {
      (*it)->setLegendLabelSize(QSize(0,0));
    }
  }
  RenderContext rc(_fontName, _absFontSize, 0L);
  _ascent = rc.fontAscent();
  if (_vertical) {
    _textHeight = _curves.count()*rc.fontHeight() + (_curves.count()-1)*_ascent/4;
  } else {
    _textWidth += _curves.count()*_ascent;
  }
}


void KstViewLegend::updateSelf() {
  if (dirty()) {
    adjustSizeForText(contentsRect());
    drawToBuffer();
  }
  KstBorderedViewObject::updateSelf();
}


void KstViewLegend::paintSelf(KstPainter& p, const QRegion& bounds) {
  const QRect cr(contentsRect());
  if (p.type() == KstPainter::P_PRINT) {
    p.save();
    KstBorderedViewObject::paint(p, bounds);
    adjustSizeForText(p.window());
    p.setViewport(contentsRect());
    p.setWindow(0, 0, cr.width(), cr.height());
    drawToPainter(p);
    p.restore();
    adjustSizeForText(cr);
  } else {
    if (p.makingMask()) {
      p.setRasterOp(Qt::SetROP);
    } else {
      const QRegion clip(clipRegion());
      KstBorderedViewObject::paintSelf(p, bounds - _myClipMask);
      p.setClipRegion(bounds & clip);
    }

    _backBuffer.paintInto(p, cr);
  }
}


void KstViewLegend::invalidateClipRegion() {
  KstBorderedViewObject::invalidateClipRegion();
  _myClipMask = QRegion();
}


QRegion KstViewLegend::clipRegion() {
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


void KstViewLegend::setFontSize(int size) {
  if (_fontSize != size) {
    _absFontSize = size + KstSettings::globalSettings()->plotFontSize;
    if (_absFontSize < MIN_FONT_SIZE) {
      _absFontSize = MIN_FONT_SIZE;
    }

    _fontSize = size;
    setDirty();
  }
}


int KstViewLegend::fontSize() const {
  return _fontSize;
}


void KstViewLegend::adjustSizeForText(QRect w) {
  double x_s, y_s, s;
  int width, height;

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

  computeTextSize();

  if (_vertical) {
    width = _textWidth + 9*_ascent/2;
  } else {
    width = _textWidth + 9*_ascent*_curves.count()/2 - _ascent;
  }
  height = _textHeight;

  QSize sz(width, height);

  if (_parent) {
    QRect r(position(), sz);
    sz = r.intersect(_parent->geometry()).size();
  }

  resize(sz + QSize((borderWidth()+_legendMargin*_ascent/10)*2, (borderWidth()+_legendMargin*_ascent/10)*2));
}


bool KstViewLegend::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  return KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
}


KstViewObjectPtr create_KstViewLegend() {
  return KstViewObjectPtr(new KstViewLegend());
}


KstViewObjectFactoryMethod KstViewLegend::factory() const {
  return &create_KstViewLegend;
}


/* FIXME: not written for legends yet. */
void KstViewLegend::writeBinary(QDataStream& str) {
}


/* FIXME: not written for legends yet. */
void KstViewLegend::readBinary(QDataStream& str) {
}


bool KstViewLegend::transparent() const {
  return KstViewObject::transparent();
}


void KstViewLegend::setTransparent(bool transparent) {
  KstViewObject::setTransparent(transparent);
}


void KstViewLegend::addCurve(KstBaseCurvePtr incurve) {
  if (!_curves.contains(incurve)) {
    _curves.append(incurve);
    setDirty();
  }
}


void KstViewLegend::removeCurve(KstBaseCurvePtr incurve) {
  if (_curves.contains(incurve)) {
    _curves.remove(incurve);
    setDirty();
  }
}


void KstViewLegend::clear() {
  if (!_curves.isEmpty()) {
    _curves.clear();
    setDirty();
  }
}


void KstViewLegend::setCurveList(Kst2DPlotPtr pl) {
  _curves = QDeepCopy<KstBaseCurveList>(pl->Curves);
  setDirty();
}


/** fill the custom widget with current properties */
bool KstViewLegend::fillConfigWidget(QWidget *w, bool isNew) const {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  KstBaseCurveList allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  if (isNew) {
    widget->_fontSize->setValue(0);
    widget->_fontColor->setColor(KstSettings::globalSettings()->foregroundColor);
    widget->_font->setCurrentFont(KstApp::inst()->defaultFont());
    widget->_margin->setValue(5);
    widget->_boxColors->setForeground(KstSettings::globalSettings()->foregroundColor);
    widget->_boxColors->setBackground(KstSettings::globalSettings()->backgroundColor);
    widget->_vertical->setChecked(true);
    widget->_transparent->setChecked(false);
    widget->_border->setValue(2);
    
    for (KstBaseCurveList::ConstIterator it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
      widget->AvailableCurveList->insertItem((*it)->tagName());
      (*it)->readUnlock();
    }

  } else { // fill legend properties into widget
    widget->_fontSize->setValue(int(fontSize()));
    widget->_fontColor->setColor(foregroundColor());
    widget->_font->setCurrentFont(fontName());
    widget->_transparent->setChecked(transparent());
    widget->_border->setValue(borderWidth());
    widget->_boxColors->setForeground(borderColor());
    widget->_boxColors->setBackground(backgroundColor());
    widget->_margin->setValue(_legendMargin);
    widget->_vertical->setChecked(vertical());
    for (KstBaseCurveList::ConstIterator it = _curves.begin(); it != _curves.end(); ++it) {
      (*it)->readLock();
      widget->DisplayedCurveList->insertItem((*it)->tagName());
      (*it)->readUnlock();
    }
    for (KstBaseCurveList::ConstIterator it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
      if (_curves.find(*it) == _curves.end()) {
        widget->AvailableCurveList->insertItem((*it)->tagName());
      }
      (*it)->readUnlock();
    }
  }
  return false;
}


/** apply properties in the custom config widget to this */
bool KstViewLegend::readConfigWidget(QWidget *w) {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  setFontSize(widget->_fontSize->value());
  setForegroundColor(widget->_fontColor->color());
  setFontName(widget->_font->currentFont());

  setTransparent(widget->_transparent->isChecked());
  setBorderWidth(widget->_border->value());
  setBorderColor(widget->_boxColors->foreground());
  setBackgroundColor(widget->_boxColors->background());
  setLegendMargin(widget->_margin->value());
  setVertical(widget->_vertical->isChecked());

  KstBaseCurveList allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  _curves.clear();
  for (unsigned i = 0; i < widget->DisplayedCurveList->count(); i++) {
    KstBaseCurveList::Iterator it = allCurves.findTag(widget->DisplayedCurveList->text(i));
    if (it != allCurves.end()) {
      _curves.append(*it);
    }
  }

  setDirty();
  return true;
}


QWidget *KstViewLegend::configWidget() {
  return new ViewLegendWidget;
}


bool KstViewLegend::vertical() const {
  return _vertical;
}


void KstViewLegend::setVertical(bool vertical) {
  if (_vertical != vertical) {
    _vertical = vertical;
    setDirty();
  }
}


void KstViewLegend::setLegendMargin(int margin) {
  int mm = kMax(0, margin);
  if (_legendMargin != mm) {
    _legendMargin = mm;
    setDirty();
  }
}


int KstViewLegend::legendMargin() const {
  return _legendMargin;
}


KstBaseCurveList& KstViewLegend::curves() {
  return _curves;
}


namespace {
KstViewObject *create_KstViewLegend() {
  return new KstViewLegend;
}


KstGfxMouseHandler *handler_KstViewLegend() {
  return 0L;
}

KST_REGISTER_VIEW_OBJECT(Legend, create_KstViewLegend, handler_KstViewLegend)
}


#include "kstviewlegend.moc"
// vim: ts=2 sw=2 et
