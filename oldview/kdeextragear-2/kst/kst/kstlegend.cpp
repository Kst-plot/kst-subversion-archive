/***************************************************************************
                          KstLegend.cpp  -  description
                             -------------------
    begin                : Jan 9th 2000
    copyright            : (C) 2000 by arw
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
#include <stdio.h>
#include <math.h>

#include <kdebug.h>
#include <qptrlist.h>
#include <qstylesheet.h>
#include <qtextstream.h>

#include "kstlegend.h"
#include "kstdatacollection.h"
#include "kstsettings.h"
#include "kstlinestyle.h"

#define MAX_DEPTH_SUB_SUPER	30

KstLegend::KstLegend(
                   KstJustifyType in_j,
                   float in_rotation,
                   float in_X, float in_Y,
                   bool is_sample) {
  Rotation = in_rotation;
  _x = in_X;
  _y = in_Y;
  Justify = in_j;
  v_offset = 0;

  FontName = "helvetica";
  Size = 0;
  bFront = false;
  setColorBackground(KstSettings::globalSettings()->backgroundColor);
  setColorForeground(KstSettings::globalSettings()->foregroundColor);

  doScalarReplacement = true;
  IsSample = is_sample;

  ShowLegend = false;
  LayoutType = LeftTop;
  AlignmentType = AlignmentRight;
}

KstLegend::~KstLegend(){
}

void KstLegend::setSize(int in_size) {
  Size = in_size;
}

void KstLegend::setFontName(const QString &in_fontName) {
  FontName = in_fontName;
}

int KstLegend::size() {
  return Size;
}

QString KstLegend::fontName() {
  return FontName;
}

void KstLegend::setDoScalarReplacement(bool in_do) {
  doScalarReplacement = in_do;
}

void KstLegend::setRotation(float in_rotation) {
  Rotation = in_rotation;
}

void KstLegend::setShow(bool bShow) {
  ShowLegend = bShow;
}

void KstLegend::setFront(bool in_bFront) {
  bFront = in_bFront;
}

void KstLegend::setColorBackground(QColor in_color) {
  colorBackground = in_color;
}

void KstLegend::setColorForeground(QColor in_color) {
  colorForeground = in_color;
}

void KstLegend::setRelPosition(float in_X, float in_Y) {
  _x = in_X;
  _y = in_Y;
}

void KstLegend::offsetRelPosition(float offset_X, float offset_Y) {
  _x += offset_X;
  _y += offset_Y;
}

void KstLegend::setJustification(KstJustifyType in_j) {
  Justify = in_j;
}

void KstLegend::draw(KstBaseCurveList* pCurves, QPainter &p, int px, int py) {
  KstPoint 	tmppoint;
  QFont 		TextFont(FontName, fontSize(p), QFont::Normal, false);
  QPen			penOld;
  QRect		  rectTotal;
  int				iLineSpace;
  int 			iLineHeight;
  int 			iLineWidth;
  int				iLineWidthMax = 0;
  int			  iCharWidth;
  int				iX;
  int 			iY;

  if( ShowLegend ) {
    if( pCurves != 0L ) {
      penOld = p.pen();
      p.setFont(TextFont);
      iLineSpace = p.fontMetrics().lineSpacing();
      iLineHeight = p.fontMetrics().ascent();
      iCharWidth = p.fontMetrics().width("M");

      for(KstBaseCurveList::iterator it = pCurves->begin(); it != pCurves->end(); it++) {
        iLineWidth = p.fontMetrics().width((*it)->tagName());
        if( iLineWidth > iLineWidthMax ) {
          iLineWidthMax = iLineWidth;
        }
      }

      rectTotal.setRect( px, py, iLineWidthMax + 4*iCharWidth, pCurves->count()*iLineSpace );

      QRegion rgn( rectTotal );

      extents = rgn;

      if( bFront ) {
        p.fillRect(rectTotal, QBrush( colorBackground ));

        //
        // draw a border
        //
        int iBorderWidth = iCharWidth / 6;

        if( iBorderWidth > 0 ) {
          QRect rectBorder;
          QPen penBorder(colorForeground, iBorderWidth);

          rectBorder.setRect( rectTotal.left()+iBorderWidth,
                              rectTotal.top()+iBorderWidth,
                              rectTotal.width()-2*iBorderWidth,
                              rectTotal.height()-2*iBorderWidth );
          p.setPen( penBorder );
          p.drawRect( rectTotal );
        }
      }

      for(KstBaseCurveList::iterator it = pCurves->begin(); it != pCurves->end(); it++) {
        iLineWidth = p.fontMetrics().width((*it)->tagName());
        if( it == pCurves->begin() )
        {
          px += iCharWidth / 2;
          py += iLineHeight;
        }
        p.setPen(colorForeground);
        switch( AlignmentType ) {
        case AlignmentCentre:
          iX = px + ((iLineWidthMax - iLineWidth)/2);
          break;
        case AlignmentRight:
          iX = px + iLineWidthMax - iLineWidth;
          break;
        case AlignmentLeft:
        default:
          iX = px;
          break;
        }
        p.drawText( iX, py, (*it)->tagName());

        //
        // draw the corresponding line and point...
        //
        p.setPen(QPen((*it)->getColor(), (*it)->lineWidth(), KstLineStyle[(*it)->lineStyle()]));
        iX = iX + iLineWidth + iCharWidth;
        iY = py - iLineHeight/2;
        if((*it)->hasLines()) {
          p.drawLine(iX, iY, iX+2*iCharWidth, iY);
        }
        if((*it)->hasPoints()) {
          tmppoint.setType((*it)->Point.getType());
          tmppoint.draw(&p, iX+iCharWidth, iY, 600);
        }
        py += iLineSpace;
      }

      p.setPen(penOld);
    }
  }
}

void KstLegend::save(QTextStream &ts) {
  ts << "    <justify>" << Justify << "</justify>" << endl;
  ts << "    <xpos>" << _x << "</xpos>" << endl;
  ts << "    <ypos>" << _y << "</ypos>" << endl;
  ts << "    <fontfamily>" << FontName << "</fontfamily>" << endl;
  ts << "    <symbolfontfamily>" << SymbolFontName << "</symbolfontfamily>" << endl;
  ts << "    <show>" << (int)ShowLegend << "</show>" << endl;
  ts << "    <size>" << Size << "</size>" << endl;
  ts << "    <layout>" << (int)LayoutType << "</layout>" << endl;
  ts << "    <alignment>" << (int)AlignmentType << "</alignment>" << endl;
  ts << "    <front>" << (int)bFront << "</front>" << endl;
  ts << "    <colorback>" << colorBackground.name() << "</colorback>\n";
  ts << "    <colorfore>" << colorForeground.name() << "</colorfore>\n";
}

void KstLegend::read(QDomElement &e) {
  Rotation = 0.0;
  Justify = CxBy;
  _x = _y = 0.0;
  FontName = "helvetica";
  Size = 0;

  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if  (e.tagName() == "justify") {
        Justify = (KstJustifyType)e.text().toInt();
      } else if  (e.tagName() == "xpos") {
        _x = e.text().toFloat();
      } else if (e.tagName() == "ypos") {
        _y = e.text().toFloat();
      } else if (e.tagName() == "fontfamily") {
        FontName = e.text();
      } else if (e.tagName() == "fontsize") {
        Size = e.text().toInt() - 12;
      } else if (e.tagName() == "show") {
        if( e.text().toInt() ) {
          ShowLegend = true;
        } else {
          ShowLegend = false;
        }
      } else if (e.tagName() == "size") {
        Size = e.text().toInt();
      } else if (e.tagName() == "layout") {
        LayoutType = (KstLegendLayoutType)e.text().toInt();
      } else if (e.tagName() == "alignment") {
        AlignmentType = (KstLegendAlignmentType)e.text().toInt();
      } else if (e.tagName() == "front") {
        if( e.text().toInt() ) {
          bFront = true;
        } else {
          bFront = false;
        }
      } else if (e.tagName() == "colorback") {
        colorBackground.setNamedColor(e.text());
      } else if (e.tagName() == "colorfore") {
        colorForeground.setNamedColor(e.text());
      }
    }
    n = n.nextSibling();
  }
}

int KstLegend::fontSize(QPainter &p) {
  int x_pix, y_pix;
  double x_s, y_s, s;

  if (IsSample) {
    return Size+12;
  }

  x_s = y_s = Size + 12.0;

  QRect v = p.window();
  x_pix = v.width();
  y_pix = v.height();

  if (x_pix < y_pix) {
    x_s *= x_pix/540.0;
    y_s *= y_pix/748.0;
  } else {
    y_s *= y_pix/540.0;
    x_s *= x_pix/748.0;
  }

  s = (x_s + y_s)/2.0;

  if (s < 6.0) {
    s = 6.0;
  }

  return int(s);
}


// Use for dragging labels
KstLegendDrag::KstLegendDrag(QWidget *dragSource, int p, const QPoint& hotSpot, const QPixmap &pm)
: QDragObject(dragSource), _plot(p), _hs(hotSpot) {
  setPixmap(pm, hotSpot);
}


KstLegendDrag::~KstLegendDrag() {
}


const char *KstLegendDrag::format(int i) const {
  if (i == 0) {
    return "application/x-kst-legend";
  } else {
    return 0L;
  }
}


QByteArray KstLegendDrag::encodedData(const char *mime) const {
  QByteArray a;

  if (QCString(mime) == "application/x-kst-legend") {
    QDataStream ds(a, IO_WriteOnly);
    ds << _plot << _hs;
  }
  return a;
}

// vim: ts=2 sw=2 et
