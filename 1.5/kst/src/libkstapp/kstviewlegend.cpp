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
#include "kstdataobjectcollection.h"
#include "kstlegenddefaults.h"
#include "kst.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewobjectfactory.h"
#include "labelrenderer.h"
#include "viewlegendwidget.h"
#include "plotlistbox.h"
#include "kstviewwindow.h"

#include <kdatastream.h>
#include <kglobal.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>

#include <qradiobutton.h>
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
  _editTitle = i18n("Edit Legend");
  _fontName = KST::legendDefaults.font();
  _vertical = KST::legendDefaults.vertical();
  _legendMargin = KST::legendDefaults.margin();
  _trackContents = KST::legendDefaults.trackContents();

  _fontSize = -1;
  setFontSize(KST::legendDefaults.fontSize());
  setForegroundColor(KST::legendDefaults.fontColor());
  setBorderColor(KST::legendDefaults.foregroundColor());
  setBackgroundColor(KST::legendDefaults.backgroundColor());
  setBorderWidth(KST::legendDefaults.border());
  setTransparent(KST::legendDefaults.transparent());

  _fallThroughTransparency = false;
  _container = false;
  _rotation = 0;
  _isResizable = false;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  _parsedTitle = 0L;

  reparseTitle();
  computeTextSize();
  setDirty(false);
}


KstViewLegend::KstViewLegend(const QDomElement& e)
: KstBorderedViewObject(e) {
  _editTitle = i18n("Edit Legend");
  _fontName = KST::legendDefaults.font();
  _vertical = KST::legendDefaults.vertical();
  _legendMargin = KST::legendDefaults.margin();
  _trackContents = KST::legendDefaults.trackContents();

  _fontSize = -1;
  setFontSize(KST::legendDefaults.fontSize());
  setForegroundColor(KST::legendDefaults.fontColor());
  setBorderColor(KST::legendDefaults.foregroundColor());
  setBackgroundColor(KST::legendDefaults.backgroundColor());
  setBorderWidth(KST::legendDefaults.border());
  setTransparent(KST::legendDefaults.transparent());

  _fallThroughTransparency = false;
  _type = "Legend";
  _rotation = 0.0;
  _container = false;
  _isResizable = false;
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;
  _parsedTitle = 0L;

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
  if (!_title.isEmpty()) {
    reparseTitle();
  }

  KstBaseCurveList l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  KstBaseCurveList::ConstIterator end = l.end();
  for (QStringList::ConstIterator str = ctaglist.begin(); str != ctaglist.end(); ++str) {
    KstBaseCurveList::ConstIterator it = l.findTag(*str);
    if (it != end) {
      addCurve(*it);
    }
  }
  
  // Padding was incorrectly saved as 5 in Kst <= 1.2.0 so we need to strip it
  // out here since this object is buggy.  Remove this once padding is properly
  // supported?  Maybe, but geometries are then wrong.
  if (e.ownerDocument().documentElement().attribute("version") == "1.2") {
    setPadding(0);
  }
}


KstViewLegend::KstViewLegend(const KstViewLegend& legend)
: KstBorderedViewObject(legend) {
  _editTitle = i18n("Edit Legend");
  _type = "Legend";
  _layoutActions &= ~(MoveTo | Copy | CopyTo);
  _standardActions |= Delete | Edit;

  _fallThroughTransparency = legend._fallThroughTransparency;
  _container = legend._container;
  _rotation = legend._rotation;
  _fontName = legend._fontName;
  _fontSize = legend._fontSize;
  _vertical = legend._vertical;
  _isResizable = legend._isResizable;
  _absFontSize = legend._absFontSize;
  _legendMargin = legend._legendMargin;
  _title = legend._title;
  _parsedTitle = 0L;
  _trackContents = legend._trackContents;
  _curves = QDeepCopy<KstBaseCurveList>(legend._curves);

  reparseTitle();
  computeTextSize();
}


KstViewLegend::~KstViewLegend() {
  delete _parsedTitle;
  _parsedTitle = 0L;
}


KstViewObject* KstViewLegend::copyObjectQuietly(KstViewObject &parent, const QString& name) const { 
  Q_UNUSED(name)

  KstViewLegend* viewLegend = new KstViewLegend(*this);
  parent.appendChild(viewLegend, true);

  return viewLegend;
}


KstViewObject* KstViewLegend::copyObjectQuietly() const { 
  KstViewLegend* viewLegend = new KstViewLegend(*this);

  return viewLegend;
}


void KstViewLegend::resize(const QSize& size) {
  KstBorderedViewObject::resize(size);
  if (!_parsedTitle) {
    reparseTitle();
  }
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
  reparseTitle();
  ts << indent << "<" << type() << ">" << endl;
  KstBorderedViewObject::save(ts, indent + "  ");
  
  for (KstBaseCurveList::ConstIterator j = _curves.begin(); j != _curves.end(); ++j) {
    (*j)->readLock();
    ts << indent+"  " << "<curvetag>" << QStyleSheet::escape((*j)->tagName()) << "</curvetag>" << endl;
    (*j)->unlock();
  }

  ts << indent << "</" << type() << ">" << endl;
}


void KstViewLegend::drawToBuffer() {
  KstPainter p;
  QPen pen;

  setDirty(false);

  _backBuffer.buffer().resize(contentsRect().size());
  _backBuffer.buffer().fill(backgroundColor());

  p.begin(&_backBuffer.buffer());
  pen.setColor(foregroundColor());
  p.setPen(pen);
  drawToPainter(p);
  p.end();
}


void KstViewLegend::drawToPainter(KstPainter& p) {
  RenderContext rc(_fontName, _absFontSize, &p);

  if (_vertical) {
    unsigned i = 0;

    if (!_title.isEmpty()) {
      p.save();
      p.translate(_legendMargin*_ascent/10, _legendMargin*_ascent/10);
      rc.x = 0;
      rc.y = _ascent;
      rc.xStart = rc.x;
      if (_parsedTitle->chunk) {
        _parsedTitle->chunk->attributes.color = foregroundColor();
      }
      renderLabel(rc, _parsedTitle->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
      i = 1;
      p.restore();
    }
    for (KstBaseCurveList::Iterator it = _curves.begin(); it != _curves.end(); ++it) {
      p.save();
      if ((*it)->parsedLegendTag()) {
        p.translate(_legendMargin*_ascent/10, _legendMargin*_ascent/10 + i * (rc.fontHeight() + _ascent / 4));
        QRect symbolBound(QPoint(0,0), QSize(16*_ascent/4, rc.fontHeight()));

        (*it)->paintLegendSymbol(&p, symbolBound);
        p.translate(9*_ascent/2, 0);
        rc.x = 0;
        rc.y = _ascent;
        rc.xStart = rc.x;
        if ((*it)->parsedLegendTag()->chunk) {
          (*it)->parsedLegendTag()->chunk->attributes.color = foregroundColor();
        }
        renderLabel(rc, (*it)->parsedLegendTag()->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
      }
      p.restore();
      ++i;
    }
  } else {
    p.save();
    p.translate(_legendMargin*_ascent/10, _legendMargin*_ascent/10);
    if (!_title.isEmpty()) {
      rc.x = 0;
      rc.y = _ascent;
      rc.xStart = rc.x;
      if (_parsedTitle->chunk) {
        _parsedTitle->chunk->attributes.color = foregroundColor();
      }
      renderLabel(rc, _parsedTitle->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
      p.translate(_titleWidth + _ascent,0);
    }

    for (KstBaseCurveList::Iterator it = _curves.begin(); it != _curves.end(); ++it) {
      if ((*it)->parsedLegendTag()) {
        QRect symbolBound(QPoint(0,0),
                          QSize(16*_ascent/4, rc.fontHeight()));
        (*it)->paintLegendSymbol(&p, symbolBound);
        p.translate(9*_ascent/2, 0);
        rc.x = 0;
        rc.y = _ascent;
        rc.xStart = rc.x;
        if ((*it)->parsedLegendTag()->chunk) {
          (*it)->parsedLegendTag()->chunk->attributes.color = foregroundColor();
        }
        renderLabel(rc, (*it)->parsedLegendTag()->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
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
      renderLabel(rc, (*it)->parsedLegendTag()->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
      if (_vertical) {
        if (rc.xMax > _textWidth) {
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
    if (_curves.count() > 0) {
      _textHeight = _curves.count()*rc.fontHeight() + (_curves.count()-1)*_ascent/4;
    } else {
      _textHeight = _ascent/4;
    }
  } else {
    if (_curves.count() > 0) {
      _textWidth += _curves.count()*_ascent;
    } else {
      _textWidth += _ascent;
    }
  }

  // determine title size
  if (!_title.isEmpty()) {
    if (!_parsedTitle) {
      reparseTitle();
    }
    renderLabel(rc, _parsedTitle->chunk, _vectorsUsed, _scalarsUsed, _stringsUsed);
    _titleWidth = rc.xMax;
    _titleHeight = rc.fontHeight();
  } else {
    _titleWidth = _titleHeight = 0;
  }
}


void KstViewLegend::updateSelf() {
  bool wasDirty(dirty());
  KstBorderedViewObject::updateSelf();
  if (wasDirty) {
    adjustSizeForText(_parent->contentsRect());
    drawToBuffer();
  }
}


void KstViewLegend::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() == KstPainter::P_PRINT || p.type() == KstPainter::P_EXPORT) {
    QRect cr(contentsRectForPainter(p));
    cr.setSize(sizeForText(_parent->geometry()));
    setContentsRectForPainter(p, cr);
    KstBorderedViewObject::paintSelf(p, bounds);

    p.translate(cr.left(), cr.top());
    if (!_transparent) {
      p.fillRect(0, 0, cr.width(), cr.height(), _backgroundColor);
    }
    drawToPainter(p);
  } else {
    if (p.makingMask()) {
      KstBorderedViewObject::paintSelf(p, bounds);
      p.setRasterOp(Qt::SetROP);
      const QRect cr(contentsRect());
      // slow but preserves antialiasing...
      QBitmap bm = _backBuffer.buffer().createHeuristicMask(false);
      bm.setMask(bm);
      p.drawPixmap(cr.left(), cr.top(), bm, 0, 0, cr.width(), cr.height());
    } else {
      const QRegion clip(clipRegion());
      KstBorderedViewObject::paintSelf(p, bounds);
      p.setClipRegion(bounds & clip);
      _backBuffer.paintInto(p, contentsRect());
    }
  }
  p.restore();
}


QRegion KstViewLegend::clipRegion() {
  if (_clipMask.isNull()) {
    if (_transparent) {
      const QRect cr(contentsRect());
      // slow but preserves antialiasing...
      QBitmap bm = _backBuffer.buffer().createHeuristicMask(false);

      _clipMask = QRegion(bm);
      _clipMask.translate(cr.topLeft().x(), cr.topLeft().y());

      QBitmap bm1(_geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1, true);
      if (!bm1.isNull()) {
        KstPainter p;

        p.setMakingMask(true);
        p.begin(&bm1);
        p.setViewXForm(true);
        KstBorderedViewObject::paintSelf(p, QRegion());
        paint(p, QRegion());
        p.flush();
        p.end();

        _clipMask |= QRegion(bm1);
      }
    } else {
      _clipMask = KstBorderedViewObject::clipRegion();
    }
  }

  return _clipMask;
}


void KstViewLegend::setFontSize(int size) {
  if (_fontSize != size) {
    _absFontSize = size + KstSettings::globalSettings()->plotFontSize;
    if (_absFontSize < KstSettings::globalSettings()->plotFontMinSize) {
      _absFontSize = KstSettings::globalSettings()->plotFontMinSize;
    }

    _fontSize = size;
    setDirty();
  }
}


int KstViewLegend::fontSize() const {
  return _fontSize;
}


void KstViewLegend::adjustSizeForText(const QRect& w) {
  QRect cr(contentsRect());
  cr.setSize(sizeForText(w));
  setContentsRect(cr);
}


QSize KstViewLegend::sizeForText(const QRect& w) {
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

  computeTextSize();

  int width, height;
  if (_vertical) {
    width = kMax(_textWidth + 9*_ascent/2, _titleWidth);
    height = _textHeight;
    if (_titleHeight > 0) {
      height += _titleHeight;
    }
  } else {
    height = kMax(_textHeight, _titleHeight);

    if (_titleWidth > 0) {
      width = _titleWidth + _textWidth + 9*_ascent*_curves.count()/2;
    } else {
      width = _textWidth + 9*_ascent*_curves.count()/2 - _ascent;
    }
  }

  QSize sz(width, height);
  
  sz += QSize(2 * _legendMargin * _ascent / 10, 2 * _legendMargin * _ascent / 10);

  if (_parent) {
    QRect r(position(), sz);
    sz = r.intersect(_parent->geometry()).size();
  }
    
  return sz;
}

void KstViewLegend::modifiedLegendEntry() {
  setDirty();
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
  Q_UNUSED(str)
}


/* FIXME: not written for legends yet. */
void KstViewLegend::readBinary(QDataStream& str) {
  Q_UNUSED(str)
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
    connect(incurve, SIGNAL(modifiedLegendEntry()), this, SLOT(modifiedLegendEntry()));
    setDirty();
  }
}


void KstViewLegend::removeCurve(KstBaseCurvePtr incurve) {
  if (_curves.contains(incurve)) {
    _curves.remove(incurve);
    disconnect(incurve,SIGNAL(modifiedLegendEntry()), this, SLOT(modifiedLegendEntry()));
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

void KstViewLegend::reparseTitle() {
  delete _parsedTitle;
  _parsedTitle = Label::parse(_title, true, false);
  collectObjects(_parsedTitle, _vectorsUsed, _scalarsUsed, _stringsUsed);
  _title = labelText(_title, _parsedTitle, _vectorsUsed, _scalarsUsed, _stringsUsed);
  setDirty();
}


bool KstViewLegend::fillConfigWidget(QWidget *w, bool isNew) const {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  KstBaseCurveList allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  if (isNew) {
    widget->_fontSize->setValue(KST::legendDefaults.fontSize());
    widget->_fontColor->setColor(KST::legendDefaults.fontColor());
    widget->_font->setCurrentFont(KST::legendDefaults.font());
    widget->_boxColors->setForeground(KST::legendDefaults.foregroundColor());
    widget->_boxColors->setBackground(KST::legendDefaults.backgroundColor());
    widget->_vertical->setChecked(KST::legendDefaults.vertical());
    widget->_transparent->setChecked(KST::legendDefaults.transparent());
    widget->_margin->setValue(KST::legendDefaults.margin());
    widget->_border->setValue(KST::legendDefaults.border());
    widget->TrackContents->setChecked(KST::legendDefaults.trackContents());

    widget->_title->setText("");

    for (KstBaseCurveList::ConstIterator it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
      widget->AvailableCurveList->insertItem((*it)->tagName());
      (*it)->unlock();
    }
  } else { // fill legend properties into widget
    widget->TrackContents->setChecked(trackContents());
    widget->_title->setText(title());
    widget->_fontSize->setValue(fontSize());
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
      (*it)->unlock();
    }
    for (KstBaseCurveList::ConstIterator it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
      if (_curves.find(*it) == _curves.end()) {
        widget->AvailableCurveList->insertItem((*it)->tagName());
      }
      (*it)->unlock();
    }
  }
  return false;
}


bool KstViewLegend::readConfigWidget(QWidget *w, bool editMultipleMode) {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  if (!editMultipleMode) {
    // apply the curve list, but only to this legend!
    KstBaseCurveList allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
    _curves.clear();
    for (unsigned i = 0; i < widget->DisplayedCurveList->count(); i++) {
      KstBaseCurveList::Iterator it = allCurves.findTag(widget->DisplayedCurveList->text(i));
      if (it != allCurves.end()) {
        _curves.append(*it);
      }
    }
  }

  if (!editMultipleMode || widget->_fontSize->value() != widget->_fontSize->minValue()) {
    setFontSize(widget->_fontSize->value());
    KST::legendDefaults.setFontSize(widget->_fontSize->value());
  }

  if (!editMultipleMode || widget->_fontColor->color() != QColor()) {
    setForegroundColor(widget->_fontColor->color());
    KST::legendDefaults.setFontColor(widget->_fontColor->color());
  }

  if (!editMultipleMode || widget->_font->currentText().compare(QString(" ")) != 0) {
    setFontName(widget->_font->currentFont());
    KST::legendDefaults.setFont(widget->_font->currentFont());
  }

  if (!editMultipleMode || widget->_title->text().compare(QString(" ")) != 0) {
    setTitle(widget->_title->text());
  }

  if (!editMultipleMode || widget->_transparent->state() != QButton::NoChange) {
    setTransparent(widget->_transparent->isChecked());
    KST::legendDefaults.setTransparent(widget->_transparent->isChecked());
  }

  if (!editMultipleMode || widget->_border->value() != widget->_border->minValue()) {
    setBorderWidth(widget->_border->value());
    KST::legendDefaults.setBorder(widget->_border->value());
  }

  if (!editMultipleMode || widget->_changedFgColor) {
    setBorderColor(widget->_boxColors->foreground());
    KST::legendDefaults.setForegroundColor(widget->_boxColors->foreground());
  }

  if (!editMultipleMode || widget->_changedBgColor) {
    setBackgroundColor(widget->_boxColors->background());
    KST::legendDefaults.setBackgroundColor(widget->_boxColors->background());
  }

  if (!editMultipleMode || widget->_margin->value() != widget->_margin->minValue()) {
    setLegendMargin(widget->_margin->value());
    KST::legendDefaults.setMargin(widget->_margin->value());
  }

  if (!editMultipleMode || widget->_vertical->state() != QButton::NoChange) {
    setVertical(widget->_vertical->isChecked());
    KST::legendDefaults.setVertical(widget->_vertical->isChecked());
  }

  if (!editMultipleMode || widget->TrackContents->state() != QButton::NoChange) {
    setTrackContents(widget->TrackContents->isChecked());
    KST::legendDefaults.setTrackContents(widget->TrackContents->isChecked());
  }

  setDirty();
  return true;
}


void KstViewLegend::connectConfigWidget(QWidget *parent, QWidget *w) const {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return;
  }

  connect(widget, SIGNAL(changed()), parent, SLOT(modified()));
  connect(widget->AvailableCurveList, SIGNAL(changed()), parent, SLOT(modified()));
  connect(widget->DisplayedCurveList, SIGNAL(changed()), parent, SLOT(modified()));
  connect(widget->_title, SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->TrackContents, SIGNAL(pressed()), parent, SLOT(modified()));
  connect(widget->_font, SIGNAL(activated(int)), parent, SLOT(modified()));
  connect(widget->_fontSize, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
  connect(widget->_fontSize->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_vertical, SIGNAL(pressed()), parent, SLOT(modified()));
  connect(widget->_fontColor, SIGNAL(changed(const QColor&)), parent, SLOT(modified()));  
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


QWidget *KstViewLegend::configWidget(QWidget *parent) {
  return new ViewLegendWidget(parent, "custom");
}


void KstViewLegend::populateEditMultiple(QWidget *w) {
  ViewLegendWidget *widget = dynamic_cast<ViewLegendWidget*>(w);
  if (!widget) {
    return;
  }

  widget->AvailableCurveList->setEnabled(false);
  widget->DisplayedCurveList->setEnabled(false);
  widget->_remove->setEnabled(false);
  widget->_add->setEnabled(false);
  widget->_up->setEnabled(false);
  widget->_down->setEnabled(false);

  widget->_title->setText(QString(" "));

  widget->_font->insertItem(QString(" "));
  widget->_font->setCurrentItem(widget->_font->count()-1);

  widget->_fontSize->setMinValue(widget->_fontSize->minValue() - 1);
  widget->_fontSize->setSpecialValueText(QString(" "));
  widget->_fontSize->setValue(widget->_fontSize->minValue());

  widget->_vertical->setTristate();
  widget->_vertical->setNoChange();

  widget->_fontColor->setColor(QColor());

  widget->_transparent->setTristate();
  widget->_transparent->setNoChange();

  widget->_boxColors->setForeground(QColor());
  widget->_boxColors->setBackground(QColor());

  widget->_margin->setMinValue(widget->_margin->minValue() - 1);
  widget->_margin->setSpecialValueText(QString(" "));
  widget->_margin->setValue(widget->_margin->minValue());

  widget->_border->setMinValue(widget->_border->minValue() - 1);
  widget->_border->setSpecialValueText(QString(" "));
  widget->_border->setValue(widget->_border->minValue());

  widget->_changedFgColor = false;
  widget->_changedBgColor = false;
}


bool KstViewLegend::supportsDefaults() {
  return false;
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

const QString& KstViewLegend::title() const {
  return _title;
}

void KstViewLegend::setTitle(const QString& title_in) {
  if (_title != title_in) {
    _title = title_in;
    _scalarsUsed.clear();
    _stringsUsed.clear();
    _vectorsUsed.clear();
    reparseTitle(); // calls setDirty()
  }
}

bool KstViewLegend::trackContents() const {
  return _trackContents;
}

void KstViewLegend::setTrackContents( bool track) {
  _trackContents = track;
}

KstBaseCurveList& KstViewLegend::curves() {
  return _curves;
}

KstViewLegendList KstViewLegend::globalLegendList() {
  KstViewLegendList rc;
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  if (it) {
    while (it->currentItem()) {
      KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (view) {
        KstViewLegendList sub = view->view()->findChildrenType<KstViewLegend>(true);
        rc += sub;
      }
      it->next();
    }
    app->deleteIterator(it);
  }
  return rc;
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
