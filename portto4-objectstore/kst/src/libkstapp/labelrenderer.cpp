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
#include "datacollection.h"
#include "dataobjectcollection.h"
#include "ksttimers.h"
#include "labelparser.h"
#include "document.h"
#include "objectstore.h"
#include "application.h"

#include <QDebug>

namespace Label {

void renderLabel(RenderContext& rc, Label::Chunk *fi) {
  // FIXME: RTL support
  int oldSize = rc.size;
  int oldY = rc.y;
  int oldX = rc.x;

  Kst::Document *doc = kstApp->mainWindow()->document();
  Q_ASSERT(doc);
  Kst::ObjectStore *store = doc->objectStore();
  Q_ASSERT(store);

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

    QFont f = rc.font();
    if (rc.fontSize() != rc.size) {
      f.setPointSize(rc.size);
    }

    f.setBold(fi->attributes.bold);
    f.setItalic(fi->attributes.italic);
    f.setUnderline(fi->attributes.underline);
    if (rc.p && fi->attributes.color.isValid()) {
      rc.p->setPen(fi->attributes.color);
    } else if (rc.p) {
      rc.p->setPen(rc.pen);
    }
    rc.setFont(f);

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
    } else if (fi->scalar) { 
      // do scalar/string/fit substitution
      QString txt;
      if (!fi->text.isEmpty() && fi->text[0] == '=') {
        // Parse and evaluate as an equation
        bool ok = false;
        const QString s = fi->text.mid(1);
        const double eqResult(Equations::interpret(store, s.toLatin1(), &ok, s.length()));
        txt = QString::number(eqResult, 'g', rc.precision);
        if (rc._cache) {
          rc._cache->append(DataRef(DataRef::DRExpression, fi->text, QString::null, 0.0, QVariant(eqResult)));
        }
      } else {
        Kst::ObjectPtr op = store->retrieveObject(Kst::ObjectTag::fromString(fi->text));
        Kst::ScalarPtr scp = Kst::kst_cast<Kst::Scalar>(op);
        if (scp) {
          KstReadLocker l(scp);
          txt = QString::number(scp->value(), 'g', rc.precision);
          if (rc._cache) {
            rc._cache->append(DataRef(DataRef::DRScalar, fi->text, QString::null, 0.0, QVariant(scp->value())));
          }
        } else {
          Kst::StringPtr stp = Kst::kst_cast<Kst::String>(op);
          if (stp) {
            KstReadLocker l(stp);
            txt = stp->value();
            if (rc._cache) {
              rc._cache->append(DataRef(DataRef::DRString, fi->text, QString::null, 0.0, QVariant(stp->value())));
            }
          }
        }
      }
      if (rc.p) {
        rc.p->drawText(rc.x, rc.y, txt);
      }
      rc.x += rc.fontWidth(txt);
    } else if (fi->vector) {
      QString txt;
      Kst::VectorPtr vp = Kst::kst_cast<Kst::Vector>(store->retrieveObject(Kst::ObjectTag::fromString(fi->text)));
      if (vp) {
        if (!fi->expression.isEmpty()) {
          // Parse and evaluate as an equation
          bool ok = false;
          // FIXME: make more efficient: cache the parsed equation
          const double idx = Equations::interpret(store, fi->expression.toLatin1(), &ok, fi->expression.length());
          if (ok) {
            KstReadLocker l(vp);
            const double vVal(vp->value()[int(idx)]);
            txt = QString::number(vVal, 'g', rc.precision);
            if (rc._cache) {
              rc._cache->append(DataRef(DataRef::DRVector, fi->text, fi->expression, idx, QVariant(vVal)));
            }
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
      const int tabWidth = rc.fontWidth("MMMMMMMM");
      const int toSkip = tabWidth - (rc.x - rc.xStart) % tabWidth;
      if (rc.p && fi->attributes.underline) {
        const int spaceWidth = rc.fontWidth(" ");
        const int spacesToSkip = tabWidth / spaceWidth + (tabWidth % spaceWidth > 0 ? 1 : 0);
        rc.p->drawText(rc.x, rc.y, QString().fill(' ', spacesToSkip));
      }
      rc.x += toSkip;
    } else {
      if (rc.p) {
#ifdef BENCHMARK
        QTime t;
        t.start();
#endif
        rc.p->drawText(rc.x, rc.y, fi->text);
#ifdef BENCHMARK
        qDebug() << "Renderer did draw, time: " << t.elapsed();
#endif
      }
      rc.x += rc.fontWidth(fi->text);
    }

    if (!rc.p) {
      // No need to compute ascent and descent when really painting
      rc.ascent = qMax(rc.ascent, -rc.y + rc.fontAscent());
      if (-rc.y - rc.fontDescent() < 0) {
        rc.descent = qMax(rc.descent, rc.fontDescent() + rc.y);
      }
    }

    int xNext = rc.x;
    if (fi->group) {
      renderLabel(rc, fi->group);
      xNext = rc.x;
    }

    if (fi->up) {
      int xPrev = rc.x;
      renderLabel(rc, fi->up);
      xNext = qMax(xNext, rc.x);
      rc.x = xPrev;
    }

    if (fi->down) {
      renderLabel(rc, fi->down);
      xNext = qMax(xNext, rc.x);
    }

    rc.x = xNext;
    rc.xMax = qMax(rc.xMax, rc.x);

    fi = fi->next;
  }

  rc.size = oldSize;
  rc.y = oldY;
}

}

// vim: ts=2 sw=2 et
