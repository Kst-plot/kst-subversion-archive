/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdualcolorbutton.h"
#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QBitmap>

class KDualColorButton::KDualColorPrivate
{
public:
    QWidget* dialogParent;
};

KDualColorButton::KDualColorButton(QWidget *parent, const char *name, QWidget* dialogParent)
  : QWidget(parent),
    d (new KDualColorPrivate)
{
    if (!dialogParent && parent) {
	d->dialogParent = parent;
    } else {
	d->dialogParent = dialogParent;
    }
/* xxx
    arrowBitmap = new QBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap); // heh
    resetPixmap = new QPixmap((const char **)dcolorreset_xpm);
*/
    fg = QBrush(Qt::black, Qt::SolidPattern);
    bg = QBrush(Qt::white, Qt::SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::KDualColorButton(const QColor &fgColor, const QColor &bgColor,
                                   QWidget *parent, const char *name, QWidget* dialogParent)
  : QWidget(parent),
    d (new KDualColorPrivate)
{
    d->dialogParent = dialogParent;
/* xxx
    arrowBitmap = new QBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap);
    resetPixmap = new QPixmap((const char **)dcolorreset_xpm);
*/
    fg = QBrush(fgColor, Qt::SolidPattern);
    bg = QBrush(bgColor, Qt::SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::~KDualColorButton()
{
  delete d;
  delete arrowBitmap;
  delete resetPixmap;
}

QColor KDualColorButton::foreground() const
{
    return fg.color();
}

QColor KDualColorButton::background() const
{
    return bg.color();
}

KDualColorButton::DualColor KDualColorButton::current() const
{
    return curColor;
}

QColor KDualColorButton::currentColor() const
{
    return (curColor == Background ? bg.color() : fg.color());
}

QSize KDualColorButton::sizeHint() const
{
    return QSize(34, 34);
}

void KDualColorButton::setForeground(const QColor &c)
{
    fg = QBrush(c, Qt::SolidPattern);
    repaint();

    emit fgChanged(fg.color());
}

void KDualColorButton::setBackground(const QColor &c)
{
    bg = QBrush(c, Qt::SolidPattern);
    repaint();

    emit bgChanged(bg.color());
}

void KDualColorButton::setCurrentColor(const QColor &c)
{
    if(curColor == Background)
        bg = QBrush(c, Qt::SolidPattern);
    else
        fg = QBrush(c, Qt::SolidPattern);
    repaint();
}

void KDualColorButton::setCurrent(DualColor s)
{
    curColor = s;
    repaint();
}

void KDualColorButton::metrics(QRect &fgRect, QRect &bgRect)
{
    fgRect = QRect(0, 0, width()-14, height()-14);
    bgRect = QRect(14, 14, width()-14, height()-14);
}

void KDualColorButton::paintEvent(QPaintEvent *)
{
    QRect fgRect, bgRect;
    QPainter p(this);
    QPalette palette;

    metrics(fgRect, bgRect);
    QBrush defBrush = palette.brush(QPalette::Normal, QPalette::Button);

    qDrawShadeRect(&p, bgRect, palette, curColor == Background, 2, 0,
                   isEnabled() ? &bg : &defBrush);
    qDrawShadeRect(&p, fgRect, palette, curColor == Foreground, 2, 0,
                   isEnabled() ? &fg : &defBrush);
    p.setPen(palette.color(QPalette::Normal, QPalette::Shadow));
/* xxx
    p.drawPixmap(fgRect.right()+2, 0, *arrowBitmap);
    p.drawPixmap(0, fgRect.bottom()+2, *resetPixmap);
*/
}

void KDualColorButton::dragEnterEvent(QDragEnterEvent *ev)
{
// xxx    ev->accept(isEnabled() && KColorDrag::canDecode(ev));
}

void KDualColorButton::dropEvent(QDropEvent *ev)
{
/* xxx
    QColor c;
    if(KColorDrag::decode(ev, c)){
        if(curColor == Foreground){
            fg.setColor(c);
            emit fgChanged(c);
        }
        else{
            bg.setColor(c);
            emit(bgChanged(c));
        }
        repaint(false);
    }
*/
}

void KDualColorButton::mousePressEvent(QMouseEvent *ev)
{
    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);
    mPos = ev->pos();
    tmpColor = curColor;
    dragFlag = false;
    if(fgRect.contains(mPos)){
        curColor = Foreground;
        miniCtlFlag = false;
    }
    else if(bgRect.contains(mPos)){
        curColor = Background;
        miniCtlFlag = false;
   }
    else if(ev->pos().x() > fgRect.width()){
        // We handle the swap and reset controls as soon as the mouse is
        // is pressed and ignore further events on this click (mosfet).
        QBrush c = fg;
        fg = bg;
        bg = c;
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    else if(ev->pos().x() < bgRect.x()){
        fg.setColor(Qt::black);
        bg.setColor(Qt::white);
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    repaint();
}


void KDualColorButton::mouseMoveEvent(QMouseEvent *ev)
{
/* xxx
    if(!miniCtlFlag){
        int delay = KGlobalSettings::dndEventDelay();
        if(ev->x() >= mPos.x()+delay || ev->x() <= mPos.x()-delay ||
           ev->y() >= mPos.y()+delay || ev->y() <= mPos.y()-delay) {
            KColorDrag *d = new KColorDrag( curColor == Foreground ?
                                            fg.color() : bg.color(),
                                            this);
            d->dragCopy();
            dragFlag = true;
        }
    }
*/
}

void KDualColorButton::mouseReleaseEvent(QMouseEvent *ev)
{
    if(!miniCtlFlag){
        QRect fgRect, bgRect;

        metrics(fgRect, bgRect);
        if(dragFlag)
            curColor = tmpColor;
        else if(fgRect.contains(ev->pos()) && curColor == Foreground){
            if(tmpColor == Background){
                curColor = Foreground;
                emit currentChanged(Foreground);
            }
            else{
                QColor newColor = fg.color();
                newColor = QColorDialog::getColor(newColor, d->dialogParent);
                if(newColor.isValid()) {
                    fg.setColor(newColor);
                    emit fgChanged(newColor);
                }
            }
        }
        else if(bgRect.contains(ev->pos()) && curColor == Background){
            if(tmpColor == Foreground){
                curColor = Background;
                emit currentChanged(Background);
            }
            else{
                QColor newColor = bg.color();
                newColor = QColorDialog::getColor(newColor, d->dialogParent);
                if(newColor.isValid()) {
                    bg.setColor(newColor);
                    emit bgChanged(newColor);
                }
            }
        }
        repaint();
        dragFlag = false;
    }
    else
        miniCtlFlag = false;
}

void KDualColorButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdualcolorbutton.moc"

