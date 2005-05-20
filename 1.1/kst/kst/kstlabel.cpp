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

// include files for Qt
#include <qregexp.h>
#include <qstylesheet.h>

// include files for KDE
#include <kdebug.h>
#include <klocale.h>

// application specific includes
#include "kst.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstlabel.h"
#include "ksttimers.h"

#define MAX_DEPTH_SUB_SUPER 30

KstLabel::KstLabel()
: KstShared(), KstRWLock(), _dirty(false) {
  Rotation = 0.0;
  _interpret = true;
  _x = 0.0;
  _y = 0.0;
  _justification = CxBy;
  v_offset = 0;
  _usePlotColor = true;
  _color.setRgb(0, 0, 0);
  FontName = KstApp::inst()->defaultFont();
  SymbolFontName = "Symbol";
  Size = 0;
  _doScalarReplacement = true;
  IsSample = false;
  Width = -1; // invalidate the width
  LineSpacing = -1;
}


KstLabel::KstLabel(const QString &in_text,
                   KstJustifyType in_j,
                   float in_rotation,
                   float in_X, float in_Y,
                   bool is_sample)
: KstShared(), KstRWLock(), _dirty(false) {
  Text = in_text;
  Rotation = in_rotation;
  _interpret = true;
  _x = in_X;
  _y = in_Y;
  _justification = in_j;
  v_offset = 0;
  _usePlotColor = true;
  _color.setRgb(0, 0, 0);
  FontName = KstApp::inst()->defaultFont();
  SymbolFontName = "Symbol";
  Size = 0;
  _doScalarReplacement = true;
  IsSample = is_sample;
  Width = -1; // invalidate the width
  LineSpacing = -1;
}


KstLabel::KstLabel(const KstLabel* label)
: KstShared(), KstRWLock(), _dirty(false) {
  Text                = label->Text;
  Rotation            = label->Rotation;
  _interpret          = label->_interpret;
  _x                  = label->_x;
  _y                  = label->_y;
  _justification      = label->_justification;
  v_offset            = label->v_offset;
  _usePlotColor       = label->_usePlotColor;
  _color              = label->_color;
  FontName            = label->FontName;
  SymbolFontName      = label->SymbolFontName;
  Size                = label->Size;
  _doScalarReplacement = label->_doScalarReplacement;
  IsSample            = label->IsSample;
  Width               = -1; // invalidate the width
  LineSpacing         = -1;
}


KstLabel::~KstLabel() {
}


void KstLabel::setResized() {
  Width = -1; // invalidate the width
  LineSpacing = -1;
}


void KstLabel::setText(const QString &in_text) {
  if (Text != in_text) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
#ifdef BENCHMARK
    kdDebug() << "label: old text was [" << Text << "] new text is [" << in_text << "]" << endl;
#endif
    setDirty();
  }
  Text = in_text;
}


const QString& KstLabel::text() const {
  return Text;
}


const QColor& KstLabel::color() const {
  return _color;
}


void KstLabel::setColor(const QColor &color) {
  setDirty(_color != color);
  _color = color;
}


bool KstLabel::usePlotColor() const {
  return _usePlotColor;
}


void KstLabel::setUsePlotColor(bool usePlotColor) {
  setDirty(_usePlotColor != usePlotColor);
  _usePlotColor = usePlotColor;
}


void KstLabel::setRotation(float in_rotation) {
  if (Rotation != in_rotation) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
    setDirty();
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
  if (_justification != in_j) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
    setDirty();
  }
  _justification = in_j;
}


/** Return the width of the label */
int KstLabel::width(QPainter &p) {
  // FIXME: this is still expensive.
  if (Width < 0) {
    draw(p, 0, 0, true, false); // Do a dummy draw to evaluate the width
#ifdef BENCHMARK
    kdDebug() << "Did draw in width()" << endl;
#endif
  }
  return int(Width);
}


int KstLabel::ascent(QPainter &p) {
  if (Text.isEmpty()) return 0;
  int Ascent = 0;
  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);

  p.save();
  p.setFont(TextFont);
  Ascent = p.fontMetrics().boundingRect("0").height();
  p.restore();

  return Ascent;
}


int KstLabel::rotatedWidth(QPainter &p) {
  return int(fabs((double)width(p) * cos(rotationRadians())) + fabs((double)lineSpacing(p) * sin(rotationRadians())));
}


int KstLabel::rotatedHeight(QPainter &p) {
  return int(fabs((double)width(p) * sin(rotationRadians())) + fabs((double)lineSpacing(p) * cos(rotationRadians())));
}


// FIXME: expensive - cache it?
int KstLabel::lineSpacing(QPainter &p) {
  if (LineSpacing >= 0) {
    return LineSpacing;
  }

  if (Text.isEmpty()) {
    return 0;
  }

  QFont TextFont(FontName, fontSize(p), QFont::Normal, false);

  p.save();
  p.setFont(TextFont);
  LineSpacing = p.fontMetrics().lineSpacing();
  p.restore();

  return LineSpacing;
}


struct FPType {
  FPType() { up = down = locked = true; x = dy = 0.0; size = 0; }
  double dy;
  double x;
  int size;
  bool locked;
  bool up, down;
};


struct GreekCharType {
  QString label;
  QString c;
};


void KstLabel::draw(QPainter &p, int px, int py, bool justify, bool doDraw) {
  int label_width;
  int font_height;
  int s;
  unsigned i;
  double dRotationRadians = 3.1415926535897932333796 * Rotation / 180.0;
  double x;
  double y;
  double y_upper;
  double y_lower;
  double x0;
  int i_g;
  int n_cr = 0;
  bool is_greek;
  bool terminate = false;
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
                                     {"\\Omicron", "O"}, {"\\omicron", "o"},
                                     {"\\Pi", "P"}, {"\\pi", "p"},
                                     {"\\Theta", "Q"}, {"\\theta", "q"},
                                     {"\\Rho", "R"}, {"\\rho", "r"},
                                     {"\\Sigma", "S"}, {"\\sigma", "s"},
                                     {"\\Tau", "T"}, {"\\tau", "t"},
                                     {"\\Upsilon", "U"}, {"\\upsilon", "u"},
                                     {"\\Omega", "W"}, {"\\omega", "w"},
                                     {"\\Xi", "X"}, {"\\xi", "x"},
                                     {"\\Psi", "Y"}, {"\\psi", "y"},
                                     {"\\Zeta", "Z"}, {"\\zeta", "z"},
                                     {"\\sum", QChar(229)},
                                     {"\\int", QChar(242)},
  };
  static const int N_GREEK = sizeof( GC ) / sizeof( GreekCharType );
  p.save();
  p.setFont(TextFont);
  p.translate(px, py);
  p.rotate(Rotation);
  p.setClipping(false);

  if (!_usePlotColor) {
    p.setPen(QPen(_color));
  }

  if (doDraw) {
    label_width = width(p);
    font_height = p.fontMetrics().height(); //ascent(p);
  } else {
    label_width = font_height = 0;
  }

  x = y = 0;

  if (justify) {
    switch (_justification) {
      case CxBy:
        x = -label_width/2;
        break;
      case CxTy:
        x = -label_width/2;
        y = font_height;
        break;
      case CxCy:
        x = -label_width/2;
        y = font_height/2;
        break;
      case LxBy:
        break;
      case LxTy:
        y = font_height;
        break;
      case LxCy:
        y = font_height/2;
        break;
      case RxBy:
        x = -label_width;
        break;
      case RxTy:
        x = -label_width;
        y = font_height;
        break;
      case RxCy:
        x = -label_width;
        y = font_height/2;
        break;
      default:
        KstDebug::self()->log(i18n("Undefined justification %1 in label \"%2\".").arg(_justification).arg(Text), KstDebug::Debug);
        break;
    }
  }

  y_upper = y;
  y_lower = y;
  x0 = x;
  processedText = Text;
  processedText.replace("\\t", "\t"); // \\t doesn't get evaluated properly FIXME
  processedText.replace("\tau", "\\tau"); // put back the mistakes caused by the hack above
  processedText.replace("\theta", "\\theta"); // put back the mistakes caused by the hack above

  if (_doScalarReplacement) {
    ScalarsUsed.clear();
    KST::scalarList.lock().readLock();
    for (KstScalarList::iterator it = KST::scalarList.begin(); it != KST::scalarList.end(); ++it) {
      if (processedText.contains((*it)->tagLabel())) {
        ScalarsUsed.append(*it);
        processedText.replace((*it)->tagLabel(), (*it)->label());
      }
    }
    KST::scalarList.lock().readUnlock();
    KST::stringList.lock().readLock();
    for (KstStringList::iterator it = KST::stringList.begin(); it != KST::stringList.end(); ++it) {
      if (processedText.contains((*it)->tagLabel())) {
        StringsUsed.append(*it);
        processedText.replace((*it)->tagLabel(), (*it)->value());
      }
    }
    KST::stringList.lock().readUnlock();
  }

  uiLength = processedText.length();
  Width = 0;
  LineSpacing = -1;

  p.setFont(TextFont);
  int savedSize = fontSize(p);
  int tabWidth = p.fontMetrics().width("MMMMMMMM"); // TeXbook says em is a standard width
  p.setTabStops(tabWidth);

  for (i = 0; i < uiLength; i++) {
    C = processedText[i];
    if (_interpret &&
        (C == '^' || C == '_' || C == '{' || C == '}' || C == '\\')) {
      if (!strOut.isEmpty()) {
        p.setFont(TextFont);
        QSize sz = p.fontMetrics().size(Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut, -1, tabWidth);
        if (doDraw) {
          p.drawText(int(x), int(y) - sz.height() - int(fP[i_fp].dy), sz.width(), sz.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut);
        }
        x += sz.width();
        if (y - fP[i_fp].dy - sz.height() < y_upper) {
          y_upper = y - fP[i_fp].dy - sz.height();
        }
        if (y + fP[i_fp].dy > y_lower) {
          y_lower = y + fP[i_fp].dy;
        }
        strOut = QString::null;
        if (i_fp > 0) {
          s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp-1].size)));
          if (s < 5) {
            s = 5; // no smaller than 5pt font!
          }
        } else {
          s = savedSize;
        }
        TextFont.setPointSize(s);
      }

      if (C == '^') {  // Superscript
        if (i_fp+1 < MAX_DEPTH_SUB_SUPER) {
          i_fp++;
          terminate = false;
          fP[i_fp].locked = false;
          fP[i_fp].size = fP[i_fp-1].size - 1;
          fP[i_fp].dy = fP[i_fp-1].dy + p.fontMetrics().ascent()*0.5;
          fP[i_fp].x = x;
          fP[i_fp].up = true;
          fP[i_fp].down = false;
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
          terminate = false;
          fP[i_fp].locked = false;
          fP[i_fp].size = fP[i_fp-1].size-1;
          fP[i_fp].dy = fP[i_fp-1].dy - p.fontMetrics().height()*0.2;
          fP[i_fp].x = x;
          fP[i_fp].up = false;
          fP[i_fp].down = true;
          s = int(double(fontSize(p))*pow(1.3, double(fP[i_fp].size)));
          if (s < 5) {
            s = 5; // no smaller than 5pt font!
          }
          TextFont.setPointSize(s);
        }
      } else if (processedText[i] == '{') {
        fP[i_fp].locked = true;
        fP[i_fp].up = false;
        fP[i_fp].down = false;
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
          terminate = true;
          QSize sz = p.fontMetrics().size(Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, str, -1, tabWidth);
          if (doDraw) {
            p.drawText(int(x),int(y) - sz.height() - int(fP[i_fp].dy), sz.width(), sz.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, str);
          }

          x += sz.width();
          if (y - fP[i_fp].dy - sz.height() < y_upper) {
            y_upper = y - fP[i_fp].dy - sz.height();
          }
          if (y + fP[i_fp].dy > y_lower) {
            y_lower = y + fP[i_fp].dy;
          }
        } else {
          if (processedText.find("it", i+1, false) == (int)i+1 ) {
            TextFont.setItalic(true);
            i += 2;
            if (processedText.at(i+1) == ' ') {
              i++;
            }
          } else if (processedText.find("rm", i+1, false) == (int)i+1 ) {
            TextFont.setItalic(false);
            i += 2;
            if (processedText.at(i+1) == ' ') {
              i++;
            }
          } else if (processedText[i+1] == 'n') {
            // New Line here
            if ( n_cr == 0 ) {
              v_offset = int(y - y_upper);
            }
            n_cr++;
            if (Width < (x-x0)) {
              Width = x - x0;
            }
            x = x0;
            y += p.fontMetrics().lineSpacing();

            i++;
            if (processedText.at(i+1) == ' ') {
              i++;
            }
          } else {
            i++;
            terminate = true;
            strOut += processedText.at(i);
          }
        }
      }
    } else {
      terminate = true;
      strOut += processedText[i];
    }

    // do we need to terminate a superscript or subscript?
    if (_interpret && i_fp > 0 && fP[i_fp].locked == false && terminate) {
      if (!strOut.isEmpty()) {
        p.setFont(TextFont);
        QSize sz = p.fontMetrics().size(Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut, -1, tabWidth);
        if (doDraw) {
          p.drawText(int(x), int(y) - sz.height() - int(fP[i_fp].dy), sz.width(), sz.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut);
        }
        x += sz.width();
        if (y - fP[i_fp].dy - sz.height() < y_upper) {
          y_upper = y - fP[i_fp].dy - sz.height();
        }
        if (y + fP[i_fp].dy > y_lower) {
          y_lower = y + fP[i_fp].dy;
        }
        strOut = QString::null;
      }
      if ((processedText.at(i+1) == '_' && fP[i_fp].up) || (processedText.at(i+1) == '^' && fP[i_fp].down)) {
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

  if (!strOut.isEmpty()) {
    p.setFont(TextFont);
    QSize sz = p.fontMetrics().size(Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut, -1, tabWidth);
    if (doDraw) {
      p.drawText(int(x), int(y) - sz.height() - int(fP[i_fp].dy), sz.width(), sz.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::ExpandTabs, strOut);
    }
    x += sz.width();

    if (y - fP[i_fp].dy - sz.height() < y_upper) {
      y_upper = y - fP[i_fp].dy - sz.height();
    }
    if (y + fP[i_fp].dy > y_lower) {
      y_lower = y + fP[i_fp].dy;
    }
  }

  p.restore();
  if (Width < x-x0) {
    Width = x - x0;
  }

  rect.setRect((int)(px + x0), (int)(py + y_upper),
               (int)Width, (int)(y_lower - y_upper + 1));
  rect.normalize();

  if (n_cr == 0) {
    v_offset = (int)(y - y_upper);
  } // else it has already set

  if (Rotation == 0.0) {
    extents = QRegion(rect);
  } else {
    QPointArray points(5);
    int ixBase = 0;
    int iyBase = 0;
    int ixOld = 0;
    int iyOld = 0;
    int ix;
    int iy;

    switch (_justification) {
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

    for (i = 0; i < 5; i++) {
      switch (i) {
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

    extents = QRegion(points);
  }
}


void KstLabel::save(QTextStream &ts, const QString& indent, bool save_pos) {
  QString l2 = indent + "  ";
  ts << indent << "<text>" << QStyleSheet::escape(Text) << "</text>" << endl;
  if (_interpret) {
    ts << indent << "<interpret/>" << endl;
  }
  ts << indent << "<rotation>" << Rotation << "</rotation>" << endl;
  if (save_pos) {
    ts << indent << "<justify>" << _justification << "</justify>" << endl;
    ts << indent << "<xpos>" << _x << "</xpos>" << endl;
    ts << indent << "<ypos>" << _y << "</ypos>" << endl;
  }
  ts << indent << "<fontfamily>" << QStyleSheet::escape(FontName) << "</fontfamily>" << endl;
  ts << indent << "<symbolfontfamily>" << QStyleSheet::escape(SymbolFontName) << "</symbolfontfamily>" << endl;
  ts << indent << "<size>" << Size << "</size>" << endl;
  if (!_usePlotColor) {
    ts << indent << "<useusercolor/>" << endl;
  }
  ts << indent << "<color>" << QStyleSheet::escape(_color.name()) << "</color>" << endl;
}


void KstLabel::read(const QDomElement &e) {
  Rotation = 0.0;
  Text = QString::null;
  _justification = CxBy;
  _x = _y = 0.0;
  SymbolFontName = "Symbol";
  FontName = KstApp::inst()->defaultFont();
  Size = 0;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull()) { // the node was really an element.
      if (e.tagName() == "text") {
        setText(e.text());
      } else if  (e.tagName() == "justify") {
        _justification = (KstJustifyType) e.text().toInt();
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
      } else if (e.tagName() == "useusercolor") {
        _usePlotColor = false;
      } else if (e.tagName() == "color") {
        _color.setNamedColor(e.text());
      }
    }
    n = n.nextSibling();
  }
}


void KstLabel::setSize(int in_size) {
  if (in_size != Size) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
    setDirty();
  }
  Size = in_size;
}


void KstLabel::setFontName(const QString &in_fontName) {
  if (in_fontName != FontName) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
    setDirty();
  }
  FontName = in_fontName;
}


int KstLabel::size() const {
  return Size;
}


int KstLabel::fontSize(QPainter &p) {
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

  //if (s<12.0) s = 0.5*s + 6;

  if (s < 6.0) {
    s = 6.0;
  }

  return int(s);
}


QString KstLabel::fontName() const {
  return FontName;
}


void KstLabel::setDoScalarReplacement(bool in_do) {
  if (in_do != _doScalarReplacement) {
    Width = -1; // invalidate the width
    LineSpacing = -1;
    setDirty();
  }
  _doScalarReplacement = in_do;
}


bool KstLabel::doScalarReplacement() const {
  return _doScalarReplacement;
}


void KstLabel::setInterpreted(bool interpreted) {
  setDirty(_interpret != interpreted);
  _interpret = interpreted;
}


bool KstLabel::interpreted() const {
  return _interpret;
}


void KstLabel::setDirty(bool dirty) {
  _dirty = dirty;
}


bool KstLabel::dirty() const {
  return _dirty;
}


// Use for dragging labels
KstLabelDrag::KstLabelDrag(QWidget *dragSource, const QString& window, KstPlotBasePtr p, int n, const QPoint& hotSpot, const QPixmap &pm)
: QDragObject(dragSource), _label(n), _window(window), _plot(p), _hs(hotSpot) {
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
    ds << _window << _plot->tagName() << _label << _hs;
  }
  return a;
}

// vim: ts=2 sw=2 et
