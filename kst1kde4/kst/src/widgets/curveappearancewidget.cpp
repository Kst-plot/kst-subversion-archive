/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLineEdit>
#include <QPainter>
#include <QPen>
#include <QTimer>

#include "curveappearancewidget.h"
#include "kstcolorsequence.h"
#include "kstcurvepointsymbol.h"
#include "kstlinestyle.h"
#include "kstsettings.h"

CurveAppearanceWidget::CurveAppearanceWidget(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  connect(_color, SIGNAL(changed(const QColor&)), this, SLOT(modified()));
  connect(_showLines, SIGNAL(clicked()), this, SLOT(modified()));
  connect(_showPoints, SIGNAL(clicked()), this, SLOT(modified()));
  connect(_showBars, SIGNAL(clicked()), this, SLOT(modified()));
  connect(_combo, SIGNAL(activated(int)), this, SLOT(modified()));
  connect(_comboLineStyle, SIGNAL(activated(int)), this, SLOT(modified()));
  connect(_comboPointDensity, SIGNAL(activated(int)), this, SLOT(modified()));
  connect(_spinBoxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(modified()));
  connect(_spinBoxLineWidth->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
  connect(_barStyle, SIGNAL(activated(int)), this, SLOT(modified()));

  connect(_color, SIGNAL(changed(QColor)), this, SLOT(fillCombo()));
  connect(_color, SIGNAL(changed(QColor)), this, SLOT(drawLine()));
  connect(_color, SIGNAL(changed(QColor)), this, SLOT(fillLineStyleCombo()));
  connect(_showLines, SIGNAL(clicked()), this, SLOT(drawLine()));
  connect(_showLines, SIGNAL(toggled(bool)), this, SLOT(enableSettings()));
  connect(_showPoints, SIGNAL(clicked()), this, SLOT(drawLine()));
  connect(_showPoints, SIGNAL(toggled(bool)), this, SLOT(enableSettings()));
  connect(_showBars, SIGNAL(clicked()), this, SLOT(drawLine()));
  connect(_showBars, SIGNAL(toggled(bool)), this, SLOT(enableSettings()));
  connect(_combo, SIGNAL(activated(int)), this, SLOT(drawLine()));
  connect(_combo, SIGNAL(activated(int)), this, SLOT(comboChanged()));
  connect(_comboLineStyle, SIGNAL(activated(int)), this, SLOT(drawLine()));
  connect(_spinBoxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(drawLine()));
  connect(_spinBoxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(drawLine()));
  connect(_barStyle, SIGNAL(activated(int)), this, SLOT(drawLine()));

  reset();

  QTimer::singleShot(0, this, SLOT(drawLine()));
}


CurveAppearanceWidget::~CurveAppearanceWidget() {
}


void CurveAppearanceWidget::modified() {
  emit changed();
}


bool CurveAppearanceWidget::showLines() {
  return _showLines->isChecked();
}


bool CurveAppearanceWidget::showPoints() {
  return _showPoints->isChecked();
}


bool CurveAppearanceWidget::showBars() {
  return _showBars->isChecked();
}


void CurveAppearanceWidget::fillCombo() {
  bool keepBlank;
  QRect rect;
  int currentItem;
  int ptype;

  keepBlank = _combo->count() > 0 && _combo->itemText(0) == " ";
  rect = _combo->style()->subControlRect(
            QStyle::CC_ComboBox, 0L, QStyle::SC_ComboBoxEditField, _combo );

  rect.setLeft( rect.left() + 2 );
  rect.setRight( rect.right() - 2 );
  rect.setTop( rect.top() + 2 );
  rect.setBottom( rect.bottom() - 2 );

  //
  // fill the point type dialog with point types
  //

  QPixmap ppix(rect.width(), rect.height());
  QPainter pp(&ppix);
  QPen pen(color(), 1, Qt::SolidLine);

  pen.setCapStyle(Qt::RoundCap);

  currentItem = _combo->currentIndex();
  _combo->clear();
  pp.setPen(pen);

  if (keepBlank) {
    _combo->insertItem(0, " ");
  }

  for (ptype = 0; ptype < KSTPOINT_MAXTYPE; ptype++) {
    pp.fillRect(pp.window(), QColor("white"));
    KstCurvePointSymbol::draw(ptype, &pp, ppix.width()/2, ppix.height()/2, 0, 600);
    _combo->insertItem(0, QIcon(ppix), QString(""));
  }

  if (currentItem > 0) {
    _combo->setCurrentIndex(currentItem);
  }
}


void CurveAppearanceWidget::setColor( QColor c ) {
  _color->setColor(c);
  drawLine();
}


QColor CurveAppearanceWidget::color() {
  return _color->color();
}


void CurveAppearanceWidget::drawLine() {
  QPixmap pix(_label->contentsRect().height()*7, _label->contentsRect().height());
  QPainter p(&pix);
  QPen pen(color(), lineWidth(), KstLineStyle[lineStyle()]);

  p.fillRect(p.window(), QColor("white"));

  if (showBars()) {
    QRect rectBar;

    rectBar.setLeft((pix.width()-pix.height())/2);
    rectBar.setTop(pix.height()/2);
    rectBar.setWidth(pix.height());
    rectBar.setHeight((pix.height()/2)+1);

    if (barStyle() == 1) {
      p.fillRect(rectBar,QBrush(QColor(color())));
      p.setPen(QPen(QColor("black"),lineWidth(),KstLineStyle[lineStyle()]));
    } else {
      p.setPen(pen);
    }
    p.drawRect(rectBar);
  }

  p.setPen(pen);
  if (_showLines->isChecked()) {
    p.drawLine(1, pix.height()/2, pix.width()-1, pix.height()/2);
  }

  if (_showPoints->isChecked()) {
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    KstCurvePointSymbol::draw(pointType(), &p, pix.width()/2, pix.height()/2, lineWidth(), 600);
  }

  _label->setPixmap(pix);
}


int CurveAppearanceWidget::pointType() {
  if (_combo->count() > 0 && _combo->itemText(0) == " ") {
    return _combo->currentIndex() - 1;
  } else {
    return _combo->currentIndex();
  }
}


void CurveAppearanceWidget::reset(QColor newColor) {
  _showLines->setChecked(true);
  _showPoints->setChecked(false);
  _showBars->setChecked(false);
  _barStyle->setCurrentIndex(1);
  _color->setColor(newColor);
  _spinBoxLineWidth->setValue(KstSettings::globalSettings()->defaultLineWeight);
  _comboPointDensity->setCurrentIndex(0);
  fillLineStyleCombo();
  fillCombo();
  drawLine();
}


void CurveAppearanceWidget::comboChanged() {
  drawLine();
}


void CurveAppearanceWidget::reset() {
  reset(KstColorSequence::next(KstSettings::globalSettings()->backgroundColor));
}


void CurveAppearanceWidget::setUsePoints( bool usePoints ) {
  _showPoints->setEnabled(usePoints);
  _combo->setEnabled(usePoints);
  _textLabelPointStyle->setEnabled(usePoints);
  if (!usePoints && _showPoints->isChecked()) {
    _showPoints->setChecked(false);
    drawLine();
  }
}


void CurveAppearanceWidget::setMustUseLines( bool bMustUseLines ) {
  _showLines->setEnabled(!bMustUseLines);
  if (bMustUseLines) {
    _showLines->setChecked(true);
    _showLines->hide();
    _textLabelLineStyle->setText(QObject::tr("Line type:"));
    drawLine();
  } else {
    _showLines->show();
    _textLabelLineStyle->setText(QObject::tr("Type:"));
  }
}


void CurveAppearanceWidget::redrawCombo() {
    fillCombo();
    fillLineStyleCombo();  
}


void CurveAppearanceWidget::fillLineStyleCombo() {
  bool keepBlank;
  QRect rect;

  keepBlank = _comboLineStyle->count() > 0 && _comboLineStyle->itemText(0) == " ";
  rect = _comboLineStyle->style()->subControlRect(
            QStyle::CC_ComboBox, 0L, QStyle::SC_ComboBoxEditField, _comboLineStyle );

  rect.setLeft(rect.left() + 2);
  rect.setRight(rect.right() - 2);
  rect.setTop(rect.top() + 2);
  rect.setBottom(rect.bottom() - 2);

  //
  // fill the point type dialog with point types
  //

  QPixmap ppix(rect.width(), rect.height());
  QPainter pp(&ppix);
  QPen pen(color(), 0);
  int currentItem;
  int style;

  currentItem = _comboLineStyle->currentIndex();
  _comboLineStyle->clear();

  if (keepBlank) {
    _comboLineStyle->insertItem(0, " ");
  }
    
  for (style = 0; style < (int)KSTLINESTYLE_MAXTYPE; style++) {
    pen.setStyle(KstLineStyle[style]);
    pp.setPen(pen);
    pp.fillRect(pp.window(), QColor("white"));
    pp.drawLine(1, ppix.height()/2, ppix.width()-1, ppix.height()/2);
    _comboLineStyle->insertItem(0, QIcon(ppix), QString(""));
  }
  _comboLineStyle->setCurrentIndex(currentItem);
}


int CurveAppearanceWidget::lineStyle() {
  if (_comboLineStyle->count() > 0 && _comboLineStyle->itemText(0) == " ") {
    return _comboLineStyle->currentIndex() -1;
  } else {
    return _comboLineStyle->currentIndex();
  }
}


int CurveAppearanceWidget::lineWidth() {
  if (_spinBoxLineWidth->text() == " ") {
    return 0;
  } else {
    return _spinBoxLineWidth->value();
  }
}


void CurveAppearanceWidget::setValue( bool hasLines, bool hasPoints, bool hasBars, const QColor &c, int pointType, int lineWidth, int lineStyle, int barStyle, int pointDensity ) {
  fillCombo();
  fillLineStyleCombo();

  _showLines->setChecked(hasLines);
  _showPoints->setChecked(hasPoints);
  _showBars->setChecked(hasBars);
  _color->setColor(c);
  _spinBoxLineWidth->setValue(lineWidth);
  _combo->setCurrentIndex(pointType);
  _barStyle->setCurrentIndex(barStyle);

  if (lineStyle < 0 || lineStyle >= (int)KSTLINESTYLE_MAXTYPE) {
    lineStyle = 0;
  }

  _comboLineStyle->setCurrentIndex(lineStyle);

  if (pointDensity < 0 || pointDensity >= KSTPOINTDENSITY_MAXTYPE) {
    pointDensity = 0;
  }

  _comboPointDensity->setCurrentIndex(pointDensity);

  enableSettings();
  drawLine();
}


void CurveAppearanceWidget::resizeEvent( QResizeEvent * pEvent ) {
  QWidget::resizeEvent(pEvent);

  redrawCombo();
}


int CurveAppearanceWidget::barStyle() {
  int rc;

  if (_barStyle->count() > 0 && _barStyle->itemText(0) == " ") {
    rc = _barStyle->currentIndex() - 1;
  } else {
    rc = _barStyle->currentIndex();
  }

  return rc;
}


int CurveAppearanceWidget::pointDensity() {
  int rc;

  if (_comboPointDensity->count() > 0 && _comboPointDensity->itemText(0) == " ") {
    rc = _comboPointDensity->currentIndex() - 1;
  } else {
    rc = _comboPointDensity->currentIndex();
  }

  return rc;
}


void CurveAppearanceWidget::enableSettings() {
  bool enable;

  enable = showLines() || showBars();
  _comboLineStyle->setEnabled(enable);
  _textLabelLineStyle->setEnabled(enable);

  enable = enable || showPoints();
  _textLabelWeight->setEnabled(enable);
  _spinBoxLineWidth->setEnabled(enable);

  enable = showBars();
  _textLabelBarStyle->setEnabled(enable);
  _barStyle->setEnabled(enable);

  enable = showPoints();  
  _textLabelPointStyle->setEnabled(enable);
  _combo->setEnabled(enable);

  enable = enable && showLines();
  _textLabelPointDensity->setEnabled(enable);
  _comboPointDensity->setEnabled(enable);
}

