/***************************************************************************
                              kstviewlabel.h
                             ----------------
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

#ifndef KSTVIEWLABEL_H
#define KSTVIEWLABEL_H

#include "kstbackbuffer.h"
#include "kstborderedviewobject.h"
#include "kstscalar.h"

class KstViewLabelDialogI;

class KstViewLabel : public KstBorderedViewObject {
  Q_OBJECT
  public:
    KstViewLabel(const QString& txt, KstVLJustifyType justify = 0L,
        float rotation = 0.0);
    virtual ~KstViewLabel();

    void setText(const QString& text);
    const QString& text() const;

    void setRotation(float rotation);
    float rotation() const { return _rotation; }

    void setJustification(KstVLJustifyType Justify);
    KstVLJustifyType justification() const { return _justify; }

    int ascent() const;

    /** Interpret special characters, default = true */
    void setInterpreted(bool interpreted);

    void setFontName(const QString& fontName);
    const QString& fontName() const;

    virtual void save(QTextStream& ts, const QString& indent = QString::null);

    void setDoScalarReplacement(bool in_do);

    virtual void paint(KstPaintType type, QPainter& p);
    virtual void resize(const QSize&);

  public slots:
    void adjustSizeForText();
    virtual void edit();

  protected:
    virtual KstViewObjectFactoryMethod factory() const;
    virtual bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
    virtual void readBinary(QDataStream& str);
    virtual void writeBinary(QDataStream& str);

  private:
    void drawToBuffer(LabelChunk*);
    void computeTextSize(LabelChunk*);

    float _rotation;
    QString _txt;
    QString _symbolFontName;
    QString _fontName;
    KstScalarList _scalarsUsed;

    bool _replace : 1;
    bool _interpret : 1;
    int _fontSize : 6;
    int _textWidth, _textHeight, _ascent;
    KstVLJustifyType _justify;
    KstBackBuffer _backBuffer;
    LabelChunk *_chunk;
    KstViewLabelDialogI *_editDialog;
};

#endif
// vim: ts=2 sw=2 et
