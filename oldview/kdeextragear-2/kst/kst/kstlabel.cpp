/***************************************************************************
                          kstlabel.cpp  -  description
                             -------------------
    begin                : Fri Sep 22 2000
    copyright            : (C) 2000 by cbn
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include <qptrlist.h>
#include <qstylesheet.h>
#include <qtextstream.h>

#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstlabel.h"

#define MAX_DEPTH_SUB_SUPER	30

KstLabel::KstLabel() {
  Text = QString::null;
  Rotation = 0.0;
  _interpret = true;
  _x = 0.0;
  _y = 0.0;
  Justify = CxBy;
  v_offset = 0;
  
  FontName = "helvetica";
  SymbolFontName = "Symbol";
  Size = 0;

  doScalarReplacement = true;
  IsSample = false;
  Width = -1; // invalidate the width
}

KstLabel::KstLabel(const QString &in_text,
                   KstJustifyType in_j,
                   float in_rotation,
                   float in_X, float in_Y,
                   bool is_sample) {

  Text = in_text;
  Rotation = in_rotation;
  _interpret = true;
  _x = in_X;
  _y = in_Y;
  Justify = in_j;
  v_offset = 0;
  
  FontName = "helvetica";
  SymbolFontName = "Symbol";
  Size = 0;

  doScalarReplacement = true;
  IsSample = is_sample;
  Width = -1; // invalidate the width
}

KstLabel::~KstLabel(){
}

void KstLabel::setText(const QString &in_text) {
  if (Text != in_text) {
    Width = -1; // invalidate the width
  }
  Text = in_text;
}

const QString& KstLabel::text() const {
  return Text;
}

void KstLabel::setRotation(float in_rotation) {
  if (Rotation != in_rotation) {
    Width = -1; // invalidate the width
  }
  Rotation = in_rotation;
}

void KstLabel::setRelPosition(float in_X, float in_Y) {
  _x = in_X;
  _y = in_Y;
}

void KstLabel::offsetRelPosition(float offset_X, float offset_Y) {
  _x += offset_X;
  _y += offset_Y;
}

void KstLabel::setJustification(KstJustifyType in_j) {
  if (Justify != in_j) {
    Width = -1; // invalidate the width
  }
  Justify = in_j;
}

/** Return the width of the label */
int KstLabel::width(QPainter &p) {
  // FIXME: this is still expensive.
  if (Width < 0) {
    draw(p, 0, 0, true, false); // Do a dummy draw to evaluate the width
  }
  return int(Width);
}

int KstLabel::ascent(QPainter &p) {
  int Ascent = 0;
  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);

  p.save();
  p.setFont(TextFont);
  Ascent = p.fontMetrics().boundingRect("1234567890").height();
  p.restore();

  return Ascent;
}

// FIXME: expensive - cache it?
int KstLabel::lineSpacing(QPainter &p) {
  int LineSpacing = 0;
  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);

  p.save();
  p.setFont(TextFont);
  LineSpacing = p.fontMetrics().lineSpacing();
  p.restore();

  return LineSpacing;
}

typedef struct FPType {
  double dy;
  double x;
  int size;
  bool locked;
};

typedef struct GreekCharType {
  QString label;
  QString c;
};

void KstLabel::draw(QPainter &p, int px, int py, bool bJustify, bool doDraw) {
  int w;
  int h;
  int s;
  unsigned i;
  double dRotationRadians = 3.1415926535897932333796 * Rotation / 180.0; 
  double x;
  double y;
  double y_upper;
  double y_lower;
  double x0;
  int i_g;
  bool is_greek;
  bool bTerminate = false;
  unsigned uiLength;
  QChar C;
  QRect rect;
  QRegion rgn;
  QString str;
  QString strOut;
  QString subProcessedText;
  QString processedText;
  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);
  QFont SymbolFont(SymbolFontName, fontSize(p), QFont::Normal, false);	
  int i_fp = 0;
  FPType fP[MAX_DEPTH_SUB_SUPER];
  static const GreekCharType GC[] = {{"\\Alpha", "A"}, {"\\alpha", "a"},
                                     {"\\Beta", "B"}, {"\\beta", "b"},
                                     {"\\Chi", "C"}, {"\\chi", "c"},
                                     {"\\Delta", "D"}, {"\\delta", "d"},
                                     {"\\Epsilon", "E"}, {"\\epsilon", "e"},
                                     {"\\Phi", "F"}, {"\\phi", "f"},
                                     {"\\Gamma", "G"}, {"\\gamma", "g"},
                                     {"\\Eta", "H"}, {"\\eta", "h"},
                                     {"\\Iota", "I"}, {"\\iota", "i"},
                                     {"\\Kappa", "K"}, {"\\kappa", "k"},
                                     {"\\Lambda", "L"}, {"\\lambda", "l"},
                                     {"\\Mu", "M"}, {"\\mu", "m"},
                                     {"\\Nu", "N"}, {"\\nu", "n"},
                                     {"\\Pi", "P"}, {"\\pi", "p"},
                                     {"\\Theta", "Q"}, {"\\theta", "q"},
                                     {"\\Rho", "R"}, {"\\rho", "r"},
                                     {"\\Sigma", "S"}, {"\\sigma", "s"},
                                     {"\\Tau", "T"}, {"\\tau", "t"},
                                     {"\\Omega", "W"}, {"\\omega", "w"},
                                     {"\\Psi", "Y"}, {"\\psi", "y"},
                                     {"\\Zeta", "Z"}, {"\\zeta", "z"},
                                     {"\\sum", "�"}, {"\\int", "�"}
  };
  static const int N_GREEK = sizeof( GC ) / sizeof( GreekCharType );

  p.save();
  p.setFont(TextFont);
  p.translate(px, py);
  p.rotate(Rotation);
  p.setClipping(false);

  if(doDraw) {
    w = width(p);
    h = ascent(p);
  } else {
    w = h = 0;
  }

  fP[0].locked = true;
  fP[0].dy = 0;
  fP[0].size = 0;
  fP[0].x = 0;
  x = y = 0;

  if(bJustify) {
    switch (Justify) {
    case CxBy:
      x = -w/2;
      break;
    case CxTy:
      x = -w/2;
      y = h;
      break;
    case CxCy:
      x = -w/2;
      y = h/2;
      break;
    case LxBy:
      break;
    case LxTy:
      y = h;
      break;
    case LxCy:
      y = h/2;
      break;
    case RxBy:
      x = -w;
      break;
    case RxTy:
      x = -w;
      y = h;
      break;
    case RxCy:
      x = -w;
      y = h/2;
      break;
    default:
      KstDebug::self()->log(i18n("Undefined justification %1 in label \"%2\".").arg(Justify).arg(Text), KstDebug::Debug);
      break;
    }
  }

  y_upper = y;
  y_lower = y;
  x0 = x;
  processedText = Text;

  if (doScalarReplacement) {
    ScalarsUsed.clear();
    KST::scalarList.lock().readLock();
    for (KstScalarList::iterator it = KST::scalarList.begin(); it != KST::scalarList.end(); ++it) {
      if (processedText.contains((*it)->tagLabel())) {
        ScalarsUsed.append(*it);
        processedText.replace((*it)->tagLabel(), (*it)->label());
      }
    }
    KST::scalarList.lock().readUnlock();
  }

  uiLength = processedText.length();
  for (i = 0; i < uiLength; i++) {    
    C = processedText[i];
    if( _interpret &&
        (C == '^' || C == '_' || C == '{' || C == '}' || C == '\\') ) {
      if( !strOut.isEmpty() ) {
        p.setFont( TextFont );
        if( doDraw ) {
          p.drawText(int(x),int(y + fP[i_fp].dy), strOut);
        }      
        x += p.fontMetrics().width(strOut);
        if( y + fP[i_fp].dy - (double)p.fontMetrics().ascent() < y_upper ) {
          y_upper = y + fP[i_fp].dy - (double)p.fontMetrics().ascent();
        }
        if( y + fP[i_fp].dy + (double)p.fontMetrics().descent() > y_lower ) {
          y_lower = y + fP[i_fp].dy + (double)p.fontMetrics().descent();
        }
        strOut = QString::null;
      }
        
      if (C == '^') {  // Superscript
        if (i_fp+1 < MAX_DEPTH_SUB_SUPER) {
          i_fp++;
          bTerminate  = false;
          fP[i_fp].locked = false;
          fP[i_fp].size = fP[i_fp-1].size - 1;
          fP[i_fp].dy = fP[i_fp-1].dy - p.fontMetrics().ascent()*0.4;
          fP[i_fp].x = x;
          if (p.fontMetrics().rightBearing(C) < 0) {
            x -= 2.0 * p.fontMetrics().rightBearing(C);
          }
          s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
          if (s < 5) {
            s = 5; // no smaller than 5pt font!
          }
          TextFont.setPointSize(s);
        }
      } else if (processedText[i] == '_') { // Subscript
        if (i_fp+1 < MAX_DEPTH_SUB_SUPER) {
          i_fp++;
          bTerminate  = false;
          fP[i_fp].locked = false;
          fP[i_fp].size = fP[i_fp-1].size-1;
          fP[i_fp].dy = fP[i_fp-1].dy + p.fontMetrics().height()*0.2;
          fP[i_fp].x = x;
          s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
          if (s < 5) {
            s = 5; // no smaller than 5pt font!
          }
          TextFont.setPointSize(s);
        }
      } else if (processedText[i] == '{') {
        fP[i_fp].locked = true;
      } else if (processedText[i] == '}') {
        if (i_fp > 0) {
          if (processedText[i+1] == '_' || processedText[i+1] == '^') {
            x = fP[i_fp].x;
          }
          i_fp--;
        }
      } else if( processedText[i] == '\\' ) {
        is_greek = false;
        subProcessedText = processedText.mid(i);
        for (i_g = 0; i_g < N_GREEK; i_g++) {
          if (subProcessedText.startsWith(GC[i_g].label)) {
            is_greek = true;
            str = GC[i_g].c;
            i += GC[i_g].label.length()-1;
            if (processedText[i+1] == ' ') {
              i++;
            }
            break;
          }
        }

        if (is_greek) {
          s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
          if (s < 5) {
            s = 5; // no smaller than 5pt font!
          }
          SymbolFont.setPointSize(s);
          p.setFont(SymbolFont);
          bTerminate = true;
          if( doDraw ) {
            p.drawText(int(x),int(y + fP[i_fp].dy), str);
          } 
          
          x += p.fontMetrics().width(str);
          if( y + fP[i_fp].dy - (double)p.fontMetrics().ascent() < y_upper ) {
            y_upper = y + fP[i_fp].dy - (double)p.fontMetrics().ascent();
          }
          if( y + fP[i_fp].dy + (double)p.fontMetrics().descent() > y_lower ) {
            y_lower = y + fP[i_fp].dy + (double)p.fontMetrics().descent();
          }
        } else {
          if (processedText.find( "it", i+1, FALSE ) == (int)i+1 ) {
            TextFont.setItalic(true);
            i += 2;
            if (processedText.at(i+1) == ' ') {
              i++;
            }
          } else if (processedText.find( "rm", i+1, FALSE ) == (int)i+1 ) {
            TextFont.setItalic(false);
            i += 2;
            if (processedText.at(i+1) == ' ') {
              i++;
            }
          } else {
            i++;
            bTerminate = true;
            strOut += processedText.at( i );  
          }
        }
      }
    } else {
      bTerminate = true;
      strOut += processedText[i];
    }
  
    //
    // do we need to terminate a superscript or subscript?
    //
    if (_interpret && i_fp > 0 && fP[i_fp].locked == false && bTerminate ) {
      if( !strOut.isEmpty( ) ) {
        p.setFont( TextFont );
        if( doDraw ) {
          p.drawText(int(x),int(y + fP[i_fp].dy), strOut);
        }
        x += p.fontMetrics().width(strOut);
        if( y + fP[i_fp].dy - (double)p.fontMetrics().ascent() < y_upper ) {
          y_upper = y + fP[i_fp].dy - (double)p.fontMetrics().ascent();
        }
        if( y + fP[i_fp].dy + (double)p.fontMetrics().descent() > y_lower ) {
          y_lower = y + fP[i_fp].dy + (double)p.fontMetrics().descent();      
        }
        strOut = QString::null;
      }
      if (processedText.at(i+1) == '_' || processedText.at(i+1) == '^') {
        x = fP[i_fp].x;
      } 
      
      i_fp--;
      s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
      if (s < 5) {
        s = 5; // no smaller than 5pt font!
      }
      TextFont.setPointSize(s);
    } 
  }
  
  if( !strOut.isEmpty( ) ) {
    p.setFont( TextFont );
    if( doDraw ) {
      p.drawText(int(x),int(y + fP[i_fp].dy), strOut);
    }      
    
    x += p.fontMetrics().width(strOut);
    if( y + fP[i_fp].dy - (double)p.fontMetrics().ascent() < y_upper ) {
      y_upper = y + fP[i_fp].dy - (double)p.fontMetrics().ascent();
    }
    if( y + fP[i_fp].dy + (double)p.fontMetrics().descent() > y_lower ) {
      y_lower = y + fP[i_fp].dy + (double)p.fontMetrics().descent();      
    }
  }
  
  p.restore();
  Width = x - x0;

  rect.setRect((int)(px + x0), (int)(py + y_upper), 
               (int)Width, (int)(y_lower - y_upper + 1));
  rect.normalize( );
  
  v_offset = (int)(y - y_upper);
  
  if( Rotation == 0.0 ) {
    QRegion rgn( rect );
    
    extents = rgn;
  } else {
    QPointArray points( 5 );
    int ixBase = 0;
    int iyBase = 0;
    int	ixOld = 0;
    int	iyOld = 0;
    int ix;
    int iy;
    
    switch( Justify ){
    case CxBy:
      ixBase = rect.left() + rect.width()/2;
      iyBase = rect.bottom();
      break;
    case CxTy:
      ixBase = rect.left() + rect.width()/2;
      iyBase = rect.top();
      break;
    case CxCy:
      ixBase = rect.left() + rect.width()/2;
      iyBase = rect.top() + rect.height()/2;
      break;
    case LxBy:
      ixBase = rect.left();
      iyBase = rect.bottom();
      break;
    case LxTy:
      ixBase = rect.left();
      iyBase = rect.top();
      break;
    case LxCy:
      ixBase = rect.left();
      iyBase = rect.top() + rect.height()/2;
      break;
    case RxBy:
      ixBase = rect.right();
      iyBase = rect.bottom();
      break;
    case RxTy:
      ixBase = rect.right();
      iyBase = rect.top();
      break;
    case RxCy:
      ixBase = rect.right();
      iyBase = rect.top() + rect.height()/2;
      break;
    default:
      ixBase = rect.left();
      iyBase = rect.bottom();
      break;
    }
    
    for( i=0; i<5; i++ ) {
      switch( i ) {
      case 0:
      case 4:
        ixOld = rect.left();
        iyOld = rect.top();
        break;
      case 1:
        ixOld = rect.right();
        iyOld = rect.top();
        break;
      case 2:
        ixOld = rect.right();
        iyOld = rect.bottom();
        break;
      case 3:
        ixOld = rect.left();
        iyOld = rect.bottom();
        break;
      }
      ix = ixBase + (int)((double)( ixOld - ixBase )*cos( -dRotationRadians ) + (double)( iyOld - iyBase)*sin( -dRotationRadians ));
      iy = iyBase + (int)((double)( ixOld - ixBase )*sin(  dRotationRadians ) + (double)( iyOld - iyBase)*cos( -dRotationRadians ));
      points.setPoint( i, ix, iy );
    }
    
    QRegion rgn( points );
    
    extents = rgn; 
  }
}

void KstLabel::save(QTextStream &ts) {
  ts << "    <text>" << QStyleSheet::escape(Text) << "</text>" << endl;
  if (_interpret) {
    ts << "    <interpret/>" << endl;
  }
  ts << "    <justify>" << Justify << "</justify>" << endl;
  ts << "    <rotation>" << Rotation << "</rotation>" << endl;
  ts << "    <xpos>" << _x << "</xpos>" << endl;
  ts << "    <ypos>" << _y << "</ypos>" << endl;
  ts << "    <fontfamily>" << FontName << "</fontfamily>" << endl;
  ts << "    <symbolfontfamily>" << SymbolFontName << "</symbolfontfamily>" << endl;
  ts << "    <size>" << Size << "</size>" << endl;
}

void KstLabel::read(QDomElement &e) {
  Rotation = 0.0;
  Text = QString::null;
  Justify = CxBy;
  _x = _y = 0.0;
  SymbolFontName = "Symbol";

  FontName = "helvetica";
  Size = 0;

  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "text") {
        setText(e.text());
      } else if  (e.tagName() == "justify") {
        Justify = (KstJustifyType) e.text().toInt();
      } else if  (e.tagName() == "rotation") {
        Rotation = e.text().toFloat();
      } else if  (e.tagName() == "interpret") {
        _interpret = true;
      } else if  (e.tagName() == "xpos") {
        _x = e.text().toFloat();
      } else if (e.tagName() == "ypos") {
        _y = e.text().toFloat();
      } else if (e.tagName() == "fontfamily") {
        FontName = e.text();
      } else if (e.tagName() == "symbolfontfamily") {
        SymbolFontName = e.text();
      } else if (e.tagName() == "fontsize") {
        Size = e.text().toInt() - 12;
      } else if (e.tagName() == "size") {
        Size = e.text().toInt();
      }
    }
    n = n.nextSibling();
  }
}


void KstLabel::setSize(int in_size) {
  Width = -1; // invalidate the width
  Size = in_size;
}

void KstLabel::setFontName(const QString &in_fontName) {
  Width = -1; // invalidate the width
  FontName = in_fontName;
}

int KstLabel::size() {
  return Size;
}

int KstLabel::fontSize(QPainter &p) {
  int x_pix, y_pix;
  double x_s, y_s, s;

  if (IsSample)
    return Size+12;

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

  //if (s<12.0) s = 0.5*s + 6;

  if (s < 6.0)
    s = 6.0;

  return int(s);
}

QString KstLabel::fontName() {
  return FontName;
}

void KstLabel::setDoScalarReplacement(bool in_do) {
  if (in_do != doScalarReplacement) {
    Width = -1; // invalidate the width
  }
  doScalarReplacement = in_do;
}


// Use for dragging labels
KstLabelDrag::KstLabelDrag(QWidget *dragSource, int p, int n, const QPoint& hotSpot, const QPixmap &pm)
: QDragObject(dragSource), _plot(p), _label(n), _hs(hotSpot) {
  setPixmap(pm, hotSpot);
}


KstLabelDrag::~KstLabelDrag() {
}


const char *KstLabelDrag::format(int i) const {
  if (i == 0) {
    return "application/x-kst-label-number";
  } else {
    return 0L;
  }
}


QByteArray KstLabelDrag::encodedData(const char *mime) const {
  QByteArray a;

  if (QCString(mime) == "application/x-kst-label-number") {
    QDataStream ds(a, IO_WriteOnly);
    ds << _plot << _label << _hs;
  }
  return a;
}

// vim: ts=2 sw=2 et
