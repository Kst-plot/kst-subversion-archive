/***************************************************************************
                             labelrenderer.cpp
                             ------------------
    begin                : Jun 17 2005
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

#include "labelrenderer.h"

#include "enodes.h"
#include "kstdatacollection.h"
#include "ksttimers.h"
#include "labelparser.h"

#include "ksdebug.h"
#include <kglobal.h>

#include <iostream>

void renderLabel(RenderContext& rc, Label::Chunk *fi) {
  // FIXME: RTL support
  int oldSize = rc.size;
  int oldY = rc.y;
  int oldX = rc.x;
  
  while (fi) {
    if (fi->vOffset != Label::Chunk::None) {
      if (fi->vOffset == Label::Chunk::Up) {
        rc.y -= int(0.4 * rc.fontHeight());
      } else { // Down
        rc.y += int(0.4 * rc.fontHeight());
      }
      if (rc.size > 5) {
        rc.size = (rc.size*2)/3;
      }
    }

    if (rc.fontSize() != rc.size) {
      QFont f = rc.font();
      f.setPointSize(rc.size);
      rc.setFont(f);
    }

    if (fi->group) { // groups have no contents
      fi = fi->next;
      continue;
    }

    if (fi->linebreak) {
      rc.x = oldX;
      rc.y += rc.fontAscent() + rc.fontDescent() + 1;
      fi = fi->next;
      continue;
    }

    if (!rc.substitute && (fi->scalar || fi->vector)) {
      QString txt = QString("[") + fi->text + "]";
      if (rc.p) {
        rc.p->drawText(rc.x, rc.y, txt);
      }
      rc.x += rc.fontWidth(txt);
    } else if (fi->scalar) { // Do scalar/string substitution
      QString txt;
      if (!fi->text.isEmpty() && fi->text[0] == '=') {
        // Parse and evaluate as an equation
        bool ok = false;
        txt = QString::number(Equation::interpret(fi->text.mid(1).latin1(), &ok), 'g', rc.precision);
      } else {
        KST::scalarList.lock().readLock();
        KstScalarPtr scp = *KST::scalarList.findTag(fi->text);
        KST::scalarList.lock().readUnlock();
        if (scp) {
          scp->readLock();
          txt = QString::number(scp->value(), 'g', rc.precision);
          scp->readUnlock();
        } else {
          KST::stringList.lock().readLock();
          KstStringPtr stp = *KST::stringList.findTag(fi->text);
          KST::stringList.lock().readUnlock();
          if (stp) {
            stp->readLock();
            txt = stp->value();
            stp->readUnlock();
          }
        }
      }
      if (rc.p) {
        rc.p->drawText(rc.x, rc.y, txt);
      }
      rc.x += rc.fontWidth(txt);
    } else if (fi->vector) {
      QString txt;
      KST::vectorList.lock().readLock();
      KstVectorPtr vp = *KST::vectorList.findTag(fi->text);
      KST::vectorList.lock().readUnlock();
      if (vp) {
        if (!fi->expression.isEmpty()) {
          // Parse and evaluate as an equation
          bool ok = false;
          // FIXME: make more efficient: cache the parsed equation
          double idx = Equation::interpret(fi->expression.latin1(), &ok);
          if (ok) {
            vp->readLock();
            txt = QString::number(vp->value()[int(idx)], 'g', rc.precision);
            vp->readUnlock();
          } else {
            txt = "NAN";
          }
        }
      }
      if (rc.p) {
        rc.p->drawText(rc.x, rc.y, txt);
      }
      rc.x += rc.fontWidth(txt);
    } else if (fi->tab) {
      int tabWidth = rc.fontWidth("MMMMMMMM");
      rc.x += tabWidth - (rc.x - rc.xStart) % tabWidth;
    } else {
      if (rc.p) {
#ifdef BENCHMARK
        QTime t;
        t.start();
#endif
        rc.p->drawText(rc.x, rc.y, fi->text);
#ifdef BENCHMARK
        kstdDebug() << "Renderer did draw, time: " << t.elapsed() << endl;
#endif
      }
      rc.x += rc.fontWidth(fi->text);
    }

    if (!rc.p) {
      // No need to compute ascent and descent when really painting
      rc.ascent = QMAX(rc.ascent, -rc.y + rc.fontAscent());
      if (-rc.y - rc.fontDescent() < 0) {
        rc.descent = QMAX(rc.descent, rc.fontDescent() + rc.y);
      }
    }

    int xNext = rc.x;
    if (fi->up) {
      int xPrev = rc.x;
      renderLabel(rc, fi->up);
      xNext = kMax(xNext, rc.x);
      rc.x = xPrev;
    }

    if (fi->down) {
      renderLabel(rc, fi->down);
      xNext = kMax(xNext, rc.x);
    }

    rc.x = xNext;
    rc.xMax = kMax(rc.xMax, rc.x);

    fi = fi->next;
  }

  rc.size = oldSize;
  rc.y = oldY;
}

// vim: ts=2 sw=2 et
