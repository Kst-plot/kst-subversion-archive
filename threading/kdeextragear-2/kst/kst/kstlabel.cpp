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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <kdebug.h>
#include <qptrlist.h>
#include <qstylesheet.h>
#include <qtextstream.h>


#include "kstlabel.h"
#include "kstdatacollection.h"

KstLabel::KstLabel(const QString &in_text,
                   KstJustifyType in_j,
                   float in_rotation,
                   float in_X, float in_Y,
                   bool is_sample) {

  Text = in_text;
  Rotation = in_rotation;
  _x = in_X;
  _y = in_Y;
  Justify = in_j;

  FontName = "helvetica";
  Size = 0;

  SymbolFontName = "Symbol";
  QFont SymbolFont(SymbolFontName, 12, QFont::Normal, false);

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

void KstLabel::setJustification(KstJustifyType in_j) {
  if (Justify != in_j) {
    Width = -1; // invalidate the width
  }
  Justify = in_j;
}

/** Return the width of the label */
int KstLabel::width(QPainter &p) {
  // FIXME: this is still expensive.
  if (Width < 0)
    draw(p, 0, 0, false); // Do a dummy draw to evaluate the width
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


void KstLabel::draw(QPainter &p, int px, int py, bool doDraw) {
  int w,h;
  int s;
  double x,y;
  double x0;
  QString C = " ";
  int i_g;
  bool is_greek;
  const int N_GREEK = 44;
  QString processedText;

  // FIXME: This parsing is really slow.  We should cache the parse results.
  //        We should also use a faster parsing algorithm.

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
                                     {"\\sum", "Â"}, {"\\int", "Ú"}
  };

  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);
  QFont SymbolFont(SymbolFontName, fontSize(p), QFont::Normal, false);

  int i_fp = 0;
  FPType fP[30]; // max depth of sub and supers is 30

  p.save();
  p.setFont(TextFont);
  p.translate(px, py);

  p.setClipping(false);

  if (doDraw) {
    w = width(p);
    h = ascent(p);
  } else {
    w = h = 0;
  }

  p.rotate(Rotation);

  fP[0].locked = true;
  fP[0].dy = 0;
  fP[0].size = 0;
  fP[0].x = 0;
  x = y = 0;

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
        kdWarning() << "undefined KstJustify in kstlabel::draw: "
             << Justify << endl;
        break;
  }

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

  i_fp = 0;
  for (unsigned i = 0; i < processedText.length(); i++) {
    if (processedText[i] == '^') {  /* Superscript */
      if (i_fp < 29) {
        i_fp++;
        fP[i_fp].locked = false;
        fP[i_fp].size = fP[i_fp-1].size - 1;
        fP[i_fp].dy = fP[i_fp-1].dy - p.fontMetrics().ascent()*0.4;
        fP[i_fp].x = x;
        if (p.fontMetrics().rightBearing(C.at(0)) < 0) {
          x -= 2.0 * p.fontMetrics().rightBearing(C.at(0));
        }
      }
    } else if (processedText[i] == '_') { /* Subscript */
      if (i_fp < 29) {
        i_fp++;
        fP[i_fp].locked = false;
        fP[i_fp].size = fP[i_fp-1].size-1;
        fP[i_fp].dy = fP[i_fp-1].dy + p.fontMetrics().height()/5;
        fP[i_fp].x = x;
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
    } else {
      s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
      if (s < 5)
        s = 5; // no smaller than 5pt font!
      TextFont.setPointSize(s);
      SymbolFont.setPointSize(s);

      is_greek = false;
      QString subProcessedText = processedText.mid(i);
      for (i_g = 0; i_g < N_GREEK; i_g++) {
        if (subProcessedText.startsWith(GC[i_g].label)) {
          is_greek = true;
          C = GC[i_g].c;
          i += GC[i_g].label.length()-1;
          if (processedText[i+1] == ' ')
            i++;
          break;
        }
      }

      if (is_greek) {
        p.setFont(SymbolFont);
      } else {
        p.setFont(TextFont);

        if (processedText[i] == '\\') {
          if (processedText[i+1] == '_') {
            C = "_";
            i++;
          } else if (processedText[i+1] == '^') {
            C = "^";
            i++;
          } else if (processedText[i+1] == 'i' && processedText[i+2] == 't') {
            TextFont.setItalic(true);
            i += 2;
            if (processedText[i+1] == ' ')
              i++;
            C = QString::null;
          } else if (processedText[i+1] == 'r' && processedText[i+2] == 'm') {
            TextFont.setItalic(false);
            i += 2;
            if (processedText[i+1] == ' ')
              i++;
            C = QString::null;
          } else {
            C = processedText[i];
            p.setFont(TextFont);
          }
        } else {
          C = processedText[i];
          p.setFont(TextFont);
        }
      }

      if (doDraw)
        p.drawText(int(x),int(y + fP[i_fp].dy), C);

      x += p.fontMetrics().width(C);

      if ((i_fp > 0) && (fP[i_fp].locked == false)) {
        if (processedText[i+1] == '_' || processedText[i+1] == '^') {
          x = fP[i_fp].x;
        }
        i_fp--;
      }
    }
  }
  p.restore();
  Width = x - x0;

  if (doDraw) {
    extents.setRect((int)(px + x0), (int)(py + y-h), (int)Width, h);
  }
}

void KstLabel::save(QTextStream &ts) {
  ts << "    <text>" << QStyleSheet::escape(Text) << "</text>" << endl;
  ts << "    <justify>" << Justify << "</justify>" << endl;
  ts << "    <rotation>" << Rotation << "</rotation>" << endl;
  ts << "    <xpos>" << _x << "</xpos>" << endl;
  ts << "    <ypos>" << _y << "</ypos>" << endl;
  ts << "    <fontfamily>" << FontName << "</fontfamily>" << endl;
  ts << "    <symbolfontfamily>" << SymbolFontName << "</symbolfontfamily>" << endl;
  ts << "    <size>" << Size << "</size>" << endl;
}

const QString& KstLabel::text() const {
  return Text;
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
