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

#include <stdlib.h>

#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QFontComboBox>
#include <QMetaObject>
#include <QStack>
#include <QRadioButton>
#include <QTextDocument>
#include <QSpinBox>

/* xxx #include <kglobal.h>
#include <kcolorbutton.h> */

#include "kstviewlegend.h"

#include "enodes.h"
#include "kst2dplot.h"
#include "kstdatacollection.h"
#include "kstgfxlegendmousehandler.h"
#include "kstdataobjectcollection.h"
#include "kstlegenddefaults.h"
#include "kstviewlegendwidget.h"
#include "kst.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewobjectfactory.h"
#include "labelrenderer.h"
#include "plotlistbox.h"
#include "kstviewwindow.h"

#define MIN_FONT_SIZE 5

KstViewLegend::KstViewLegend()
: KstBorderedViewObject("Legend") {
  _editTitle = QObject::tr("Edit Legend");
  _newTitle = QObject::tr("New Legend");
  _fontName = KST::legendDefaults.font();
  _vertical = KST::legendDefaults.vertical();
  _legendMargin = KST::legendDefaults.margin();
  _trackContents = KST::legendDefaults.trackContents();
  _scaleLineWidth = KST::legendDefaults.scaleLineWidth();

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
  _editTitle = QObject::tr("Edit Legend");
  _newTitle = QObject::tr("New Legend");
  _fontName = KST::legendDefaults.font();
  _vertical = KST::legendDefaults.vertical();
  _legendMargin = KST::legendDefaults.margin();
  _trackContents = KST::legendDefaults.trackContents();
  _scaleLineWidth = KST::legendDefaults.scaleLineWidth();

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
      if (metaObject()->indexOfProperty(el.tagName().toLatin1()) > -1) {
        setProperty(el.tagName().toLatin1(), QVariant(el.text()));
      } else if (el.tagName() == "curvetag") {
        ctaglist.append(el.text()); 
      }
    }
    n = n.nextSibling();
  }
  if (!_title.isEmpty()) {
    reparseTitle();
  }

  KstBaseCurveList l;
  KstBaseCurveList::const_iterator end;
  QStringList::const_iterator str;

// xxx  l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  end = l.end();
  for (str = ctaglist.begin(); str != ctaglist.end(); ++str) {
    KstBaseCurveList::const_iterator it;

    it = l.findTag(*str);
    if (it != end) {
      addCurve(*it);
    }
  }

  //
  // padding was incorrectly saved as 5 in Kst <= 1.2.0 so we need to strip it
  // out here since this object is buggy.  Remove this once padding is properly
  // supported?  Maybe, but geometries are then wrong...
  //

  if (e.ownerDocument().documentElement().attribute("version") == "1.2") {
    setPadding(0);
  }
}


KstViewLegend::KstViewLegend(const KstViewLegend& legend)
: KstBorderedViewObject(legend) {
  _editTitle = QObject::tr("Edit Legend");
  _newTitle = QObject::tr("New Legend");
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
  _scaleLineWidth = legend._scaleLineWidth;
  _title = legend._title;
  _parsedTitle = 0L;
  _trackContents = legend._trackContents;
  _curves = legend._curves;

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
  parent.appendChild(KstViewLegendPtr(viewLegend), true);

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
    ts << indent+"  " << "<curvetag>" << Qt::escape((*j)->tagName()) << "</curvetag>" << endl;
    (*j)->unlock();
  }

  ts << indent << "</" << type() << ">" << endl;
}


void KstViewLegend::drawToBuffer() {
  KstPainter p;
  QPen pen;

  setDirty(false);

// xxx  _backBuffer.buffer().resize(contentsRect().size());
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
      rc.pen = foregroundColor();
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

// xxx        (*it)->paintLegendSymbol(&p, symbolBound, _scaleLineWidth);
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
// xxx        (*it)->paintLegendSymbol(&p, symbolBound, _scaleLineWidth);
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
// xxx      p.setRasterOp(Qt::SetROP);
      const QRect cr(contentsRect());

      //
      // slow but preserves antialiasing...
      //

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
  if (_clipMask.isEmpty()) {
    if (_transparent) {
      const QRect cr(contentsRect());

      //
      // slow but preserves antialiasing...
      //

      QBitmap bm = _backBuffer.buffer().createHeuristicMask(false);

      _clipMask = QRegion(bm);
      _clipMask.translate(cr.topLeft().x(), cr.topLeft().y());

      QBitmap bm1(_geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1);
      if (!bm1.isNull()) {
        KstPainter p;

        p.setMakingMask(true);
        p.begin(&bm1);
// xxx        p.setViewXForm(true);
        KstBorderedViewObject::paintSelf(p, QRegion());
        paint(p, QRegion());
// xxx        p.flush();
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
    width = qMax(_textWidth + 9*_ascent/2, _titleWidth);
    height = _textHeight;
    if (_titleHeight > 0) {
      height += _titleHeight;
    }
  } else {
    height = qMax(_textHeight, _titleHeight);

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


bool KstViewLegend::layoutPopupMenu(QMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
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
// xxx    connect(incurve, SIGNAL(modifiedLegendEntry()), this, SLOT(modifiedLegendEntry()));
    setDirty();
  }
}


void KstViewLegend::removeCurve(KstBaseCurvePtr incurve) {
  if (_curves.contains(incurve)) {
    _curves.removeAll(incurve);
// xxx    disconnect(incurve,SIGNAL(modifiedLegendEntry()), this, SLOT(modifiedLegendEntry()));
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
  _curves = pl->_curves;
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
  KstViewLegendWidget *widget = dynamic_cast<KstViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  KstBaseCurveList allCurves;
  KstBaseCurveList::const_iterator it;

// xxx  allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  if (isNew) {
    widget->_fontSize->setValue(KST::legendDefaults.fontSize());
// xxx    widget->_fontColor->setColor(KST::legendDefaults.fontColor());
    widget->_font->setCurrentFont(KST::legendDefaults.font());
// xxx    widget->_boxColors->setForeground(KST::legendDefaults.foregroundColor());
// xxx    widget->_boxColors->setBackground(KST::legendDefaults.backgroundColor());
    widget->_vertical->setChecked(KST::legendDefaults.vertical());
    widget->_transparent->setChecked(KST::legendDefaults.transparent());
    widget->_margin->setValue(KST::legendDefaults.margin());
    widget->_scaleLineWidth->setValue(KST::legendDefaults.scaleLineWidth());
    widget->_border->setValue(KST::legendDefaults.border());
    widget->TrackContents->setChecked(KST::legendDefaults.trackContents());
    widget->_title->setText("");

    for (it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
      widget->AvailableCurveList->insertItem(0, (*it)->tagName());
      (*it)->unlock();
    }
  } else { 
    //
    // fill legend properties into widget   
    //

    widget->TrackContents->setChecked(trackContents());
    widget->_title->setText(title());
    widget->_fontSize->setValue(fontSize());
// xxx    widget->_fontColor->setColor(foregroundColor());
    widget->_font->setCurrentFont(fontName());
    widget->_transparent->setChecked(transparent());
    widget->_border->setValue(borderWidth());
// xxx    widget->_boxColors->setForeground(borderColor());
// xxx    widget->_boxColors->setBackground(backgroundColor());
    widget->_margin->setValue(_legendMargin);
    widget->_scaleLineWidth->setValue(_scaleLineWidth);
    widget->_vertical->setChecked(vertical());

    for (it = _curves.begin(); it != _curves.end(); ++it) {
      (*it)->readLock();
      widget->DisplayedCurveList->insertItem(0, (*it)->tagName());
      (*it)->unlock();
    }

    for (it = allCurves.begin(); it != allCurves.end(); ++it) {
      (*it)->readLock();
/* xxx
      if (_curves.find(*it) == _curves.end()) {
        widget->AvailableCurveList->insertItem(0, (*it)->tagName());
      }
*/
      (*it)->unlock();
    }
  }

  return false;
}


bool KstViewLegend::readConfigWidget(QWidget *w, bool editMultipleMode) {
  KstViewLegendWidget *widget = dynamic_cast<KstViewLegendWidget*>(w);
  if (!widget) {
    return false;
  }

  if (!editMultipleMode) {
    KstBaseCurveList allCurves;
    int i;

    //
    // apply the curve list, but only to this legend...
    //

// xxx    allCurves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
    _curves.clear();
    for (i = 0; i < widget->DisplayedCurveList->count(); i++) {
      KstBaseCurveList::iterator it;

      it = allCurves.findTag(widget->DisplayedCurveList->item(i)->text());
      if (it != allCurves.end()) {
        _curves.append(*it);
      }
    }
  }

  if (!editMultipleMode || widget->_fontSize->value() != widget->_fontSize->minimum()) {
    setFontSize(widget->_fontSize->value());
    KST::legendDefaults.setFontSize(widget->_fontSize->value());
  }
/* xxx
  if (!editMultipleMode || widget->_fontColor->color() != QColor()) {
    setForegroundColor(widget->_fontColor->color());
    KST::legendDefaults.setFontColor(widget->_fontColor->color());
  }
*/
  if (!editMultipleMode || widget->_font->currentText().compare(QString(" ")) != 0) {
    setFontName(widget->_font->currentFont().family());
    KST::legendDefaults.setFont(widget->_font->currentFont().family());
  }

  if (!editMultipleMode || widget->_title->text().compare(QString(" ")) != 0) {
    setTitle(widget->_title->text());
  }

  if (!editMultipleMode || widget->_transparent->checkState() != Qt::PartiallyChecked) {
    setTransparent(widget->_transparent->isChecked());
    KST::legendDefaults.setTransparent(widget->_transparent->isChecked());
  }

  if (!editMultipleMode || widget->_border->value() != widget->_border->minimum()) {
    setBorderWidth(widget->_border->value());
    KST::legendDefaults.setBorder(widget->_border->value());
  }

  if (!editMultipleMode || widget->_changedFgColor) {
// xxx    setBorderColor(widget->_boxColors->foreground());
// xxx    KST::legendDefaults.setForegroundColor(widget->_boxColors->foreground());
  }

  if (!editMultipleMode || widget->_changedBgColor) {
// xxx    setBackgroundColor(widget->_boxColors->background());
// xxx    KST::legendDefaults.setBackgroundColor(widget->_boxColors->background());
  }

  if (!editMultipleMode || widget->_margin->value() != widget->_margin->minimum()) {
    setLegendMargin(widget->_margin->value());
    KST::legendDefaults.setMargin(widget->_margin->value());
  }

  if (!editMultipleMode || widget->_scaleLineWidth->value() != widget->_scaleLineWidth->minimum()) {
    setScaleLineWidth(widget->_scaleLineWidth->value());
    KST::legendDefaults.setMargin(widget->_scaleLineWidth->value());
  }

  if (!editMultipleMode || widget->_vertical->checkState() != Qt::PartiallyChecked) {
    setVertical(widget->_vertical->isChecked());
    KST::legendDefaults.setVertical(widget->_vertical->isChecked());
  }

  if (!editMultipleMode || widget->TrackContents->checkState() != Qt::PartiallyChecked) {
    setTrackContents(widget->TrackContents->isChecked());
    KST::legendDefaults.setTrackContents(widget->TrackContents->isChecked());
  }

  setDirty();
  return true;
}


void KstViewLegend::connectConfigWidget(QWidget *parent, QWidget *w) const {
  KstViewLegendWidget *widget = dynamic_cast<KstViewLegendWidget*>(w);
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
// xxx  connect(widget->_fontSize->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_vertical, SIGNAL(pressed()), parent, SLOT(modified()));
// xxx  connect(widget->_fontColor, SIGNAL(changed(const QColor&)), parent, SLOT(modified()));  
  connect(widget->_transparent, SIGNAL(pressed()), parent, SLOT(modified()));
/* xxx
  connect(widget->_boxColors, SIGNAL(fgChanged(const QColor&)), parent, SLOT(modified()));
  connect(widget->_boxColors, SIGNAL(bgChanged(const QColor&)), parent, SLOT(modified()));
  connect(widget->_boxColors, SIGNAL(fgChanged(const QColor&)), widget, SLOT(changedFgColor()));
  connect(widget->_boxColors, SIGNAL(bgChanged(const QColor&)), widget, SLOT(changedBgColor()));
*/
  connect(widget->_margin, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
// xxx  connect(widget->_margin->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_scaleLineWidth, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
// xxx  connect(widget->_scaleLineWidth->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
  connect(widget->_border, SIGNAL(valueChanged(int)), parent, SLOT(modified()));
// xxx  connect(widget->_border->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), parent, SLOT(modified()));
}


QWidget *KstViewLegend::configWidget(QWidget *parent) {
  return new KstViewLegendWidget(parent, "custom");
}


void KstViewLegend::populateEditMultiple(QWidget *w) {
  KstViewLegendWidget *widget = dynamic_cast<KstViewLegendWidget*>(w);
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

  widget->_font->insertItem(0, QString(" "));
  widget->_font->setCurrentIndex(widget->_font->count()-1);

  widget->_fontSize->setMinimum(widget->_fontSize->minimum() - 1);
  widget->_fontSize->setSpecialValueText(QString(" "));
  widget->_fontSize->setValue(widget->_fontSize->minimum());

  widget->_vertical->setTristate();
  widget->_vertical->setCheckState(Qt::PartiallyChecked);

// xxx  widget->_fontColor->setColor(QColor());

  widget->_transparent->setTristate();
  widget->_transparent->setCheckState(Qt::PartiallyChecked);
/* xxx
  widget->_boxColors->setForeground(QColor());
  widget->_boxColors->setBackground(QColor());
*/
  widget->_margin->setMinimum(widget->_margin->minimum() - 1);
  widget->_margin->setSpecialValueText(QString(" "));
  widget->_margin->setValue(widget->_margin->minimum());

  widget->_scaleLineWidth->setMinimum(widget->_scaleLineWidth->minimum() - 1);
  widget->_scaleLineWidth->setSpecialValueText(QString(" "));
  widget->_scaleLineWidth->setValue(widget->_scaleLineWidth->minimum());

  widget->_border->setMinimum(widget->_border->minimum() - 1);
  widget->_border->setSpecialValueText(QString(" "));
  widget->_border->setValue(widget->_border->minimum());

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
  int mm = qMax(0, margin);

  if (_legendMargin != mm) {
    _legendMargin = mm;
    setDirty();
  }
}


int KstViewLegend::legendMargin() const {
  return _legendMargin;
}


int KstViewLegend::scaleLineWidth() const {
  return _scaleLineWidth;
}


void KstViewLegend::setScaleLineWidth(int scaleLineWidth) {
  if (scaleLineWidth < 1) {
    scaleLineWidth = 1;
  }

  if (_scaleLineWidth != scaleLineWidth) {
    _scaleLineWidth = scaleLineWidth;
    setDirty();
  }
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
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      KstViewLegendList sub;

// xxx      sub = viewWindow->view()->findChildrenType<KstViewLegend>(true);
      rc += sub;
    }
  }

  return rc;
}


namespace {
KstViewObject *create_KstViewLegend() {
  return new KstViewLegend;
}


KstGfxMouseHandler *handler_KstViewLegend() {
  return new KstGfxLegendMouseHandler;
}

KST_REGISTER_VIEW_OBJECT(Legend, create_KstViewLegend, handler_KstViewLegend)
}

#include "kstviewlegend.moc"
