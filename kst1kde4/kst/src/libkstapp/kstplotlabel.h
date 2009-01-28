/***************************************************************************
                            kstplotviewlabel.h
                             ----------------
    begin                : Jun 22 2005
    copyright            : (C) 2005 by The University of Toronto
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

#ifndef KSTPLOTLABEL_H
#define KSTPLOTLABEL_H

#include "labelparser.h"

#include "kstvector.h"

class KstPlotLabel {
  public:
    explicit KstPlotLabel(float rotation = 0.0);
    KstPlotLabel(const QString& txt, const QString& font, int fontSize, KstLJustifyType justify = 0L, float rotation = 0.0);
    ~KstPlotLabel();

    void setText(const QString& text);
    const QString& text() const;

    void setRotation(float rotation);
    float rotation() const { return _rotation; }

    void setJustification(KstLJustifyType Justify);
    KstLJustifyType justification() const { return _justify; }

    void setInterpreted(bool interpreted);
    bool interpreted() const;

    void setFontName(const QString& fontName);
    const QString& fontName() const;

    void setFontSize(int size);
    int fontSize() const;
    void updateAbsFontSize(int x_pix, int y_pix);

    void setDataPrecision(int prec);
    int dataPrecision() const;

    int ascent() const;
    int lineSpacing() const;

    void setDoScalarReplacement(bool in_do);
    bool doScalarReplacement() const;

    void paint(QPainter& p);

    QSize size() const;

    void load(const QDomElement& e);
    void save(QTextStream& ts, const QString& indent = QString::null, bool save_pos = false);

    KstPlotLabel& operator=(const KstPlotLabel&);

  private:
    void commonConstructor(const QString& txt, const QString& font, int fontSize, KstLJustifyType justify, float rotation);
    void drawToPainter(Label::Parsed *lp, QPainter& p);
    void computeTextSize();
    void reparse();

    float _rotation;
    double _cosr; // absolute cos of the rotation angle
    double _sinr; // absolute sin of the rotation angle
    QString _txt;
    QString _fontName;

    bool _replace : 1;
    bool _interpret : 1;
    int _dataPrecision : 6;
    int _fontSize;
    int _absFontSize;
    int _textWidth;
    int _textHeight;
    int _ascent;

    KstLJustifyType _justify;
    Label::Parsed *_parsed;
    int _lineSpacing;

    KstScalarMap _scalarsUsed;
    KstStringMap _stringsUsed;
    KstVectorMap _vectorsUsed;
};

#endif
