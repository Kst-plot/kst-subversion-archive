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
#include "kstdataobjectcollection.h"
#include "kstcplugin.h"
#include "ksttimers.h"
#include "labelparser.h"

#include "ksdebug.h"
#include <kglobal.h>

#include <iostream>

bool collectObjects(Label::Parsed *lp, KstVectorMap& vm, KstScalarMap& scm, KstStringMap& stm) {
  if (!lp) {
    return false;
  }

  for (Label::Chunk *fi = lp->chunk; fi; fi = fi->next) {
    if (fi->scalar) {
      KST::scalarList.lock().readLock();
      KstScalarPtr scp = KST::scalarList.retrieveObject(KstObjectTag::fromString(fi->text));
      KST::scalarList.lock().unlock();
      if (scp) {
        scm.insert(fi->text, scp);
        continue;
      }

      KST::stringList.lock().readLock();
      KstStringPtr stp = KST::stringList.retrieveObject(KstObjectTag::fromString(fi->text));
      KST::stringList.lock().unlock();
      if (stp) {
        stm.insert(fi->text, stp);
        continue;
      }
    } else if (fi->vector) {
      KST::vectorList.lock().readLock();
      KstVectorPtr vp = KST::vectorList.retrieveObject(KstObjectTag::fromString(fi->text));
      KST::vectorList.lock().unlock();
      if (vp) {
        vm.insert(fi->text, vp);
        continue;
      }
    } else {
      // not an object reference
      continue;
    }

//    kstdWarning() << "Label references unknown object [" << fi->text << "]." << endl;
    return false;
  }

  return true;
}


// This is a hack to give us back the label text using the current object names
QString labelText(QString txt, Label::Parsed *lp, KstVectorMap& vm, KstScalarMap& scm, KstStringMap& stm) {
  QString label = txt;

  if (!lp) {
    return label;
  }

  for (Label::Chunk *fi = lp->chunk; fi; fi = fi->next) {
    if (fi->scalar) {
      KstScalarPtr scp;
      if (scm.contains(fi->text)) {
        scp = scm[fi->text];
      } else {
        KST::scalarList.lock().readLock();
        scp = KST::scalarList.retrieveObject(KstObjectTag::fromString(fi->text));
        KST::scalarList.lock().unlock();
      }
      if (scp && scp->tag().displayString() != fi->text) {
        label.replace(QString("[%1]").arg(fi->text), QString("[%1]").arg(scp->tag().displayString()));
        scm.insert(scp->tag().displayString(), scp);
        continue;
      }

      KstStringPtr stp;
      if (stm.contains(fi->text)) {
        stp = stm[fi->text];
      } else {
        KST::stringList.lock().readLock();
        stp = KST::stringList.retrieveObject(KstObjectTag::fromString(fi->text));
        KST::stringList.lock().unlock();
      }
      if (stp && stp->tag().displayString() != fi->text) {
        label.replace(QString("[%1]").arg(fi->text), QString("[%1]").arg(stp->tag().displayString()));
        stm.insert(stp->tag().displayString(), stp);
        continue;
      }
    } else if (fi->vector) {
      KstVectorPtr vp;
      if (vm.contains(fi->text)) {
        vp = vm[fi->text];
      } else {
        KST::vectorList.lock().readLock();
        vp = KST::vectorList.retrieveObject(KstObjectTag::fromString(fi->text));
        KST::vectorList.lock().unlock();
      }
      if (vp && vp->tag().displayString() != fi->text) {
        label.replace(QString("[%1[").arg(fi->text), QString("[%1[").arg(vp->tag().displayString()));
        vm.insert(vp->tag().displayString(), vp);
        continue;
      }
    }
  }

  if (label != txt) {
    kstdDebug() << "Updated label \"" << txt << "\" to \"" << label << "\"" << endl;
  }

  return label;
}


void renderLabel(RenderContext& rc, Label::Chunk *fi, const KstVectorMap& vm, const KstScalarMap& scm, const KstStringMap& stm) {
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
        const double eqResult(Equation::interpret(s.latin1(), &ok, s.length()));
        txt = QString::number(eqResult, 'g', rc.precision);
        if (rc._cache) {
          rc._cache->append(DataRef(DataRef::DRExpression, fi->text, QString::null, 0.0, QVariant(eqResult)));
        }
      } else {
        KstScalarPtr scp;
        if (scm.contains(fi->text)) {
          scp = scm[fi->text];
        }
        if (!scp) {
          KST::scalarList.lock().readLock();
          scp = KST::scalarList.retrieveObject(KstObjectTag::fromString(fi->text));
          KST::scalarList.lock().unlock();
        }
        if (scp) {
          scp->readLock();
          txt = QString::number(scp->value(), 'g', rc.precision);
          if (rc._cache) {
            rc._cache->append(DataRef(DataRef::DRScalar, fi->text, QString::null, 0.0, QVariant(scp->value())));
          }
          scp->unlock();
        } else {
          KstStringPtr stp;
          if (stm.contains(fi->text)) {
            stp = stm[fi->text];
          }
          if (!stp) {
            KST::stringList.lock().readLock();
            stp = KST::stringList.retrieveObject(KstObjectTag::fromString(fi->text));
            KST::stringList.lock().unlock();
          }
          if (stp) {
            stp->readLock();
            txt = stp->value();
            if (rc._cache) {
              rc._cache->append(DataRef(DataRef::DRString, fi->text, QString::null, 0.0, QVariant(stp->value())));
            }
            stp->unlock();
          } else {
            KST::dataObjectList.lock().readLock();
            KstDataObjectList::Iterator oi = KST::dataObjectList.findTag(fi->text);
            KST::dataObjectList.lock().unlock();
            if (oi != KST::dataObjectList.end()) {
              KstCPluginPtr fit = kst_cast<KstCPlugin>(*oi);
              if (fit) {
                fit->readLock();
                const QString txtAll = fit->label(rc.precision);
                fit->unlock();

                const QValueList<QString> strList = QStringList::split('\n', txtAll);
                QValueListConstIterator<QString> last = --(strList.end());
                for (QValueListConstIterator<QString> iter = strList.begin(); iter != strList.end(); ++iter) {
                  txt = (*iter);

                  if (iter != last) {
                    if (rc.p) {
                      rc.p->drawText(rc.x, rc.y, txt);
                    } else {
                      rc.ascent = kMax(rc.ascent, -rc.y + rc.fontAscent());
                      if (-rc.y - rc.fontDescent() < 0) {
                        rc.descent = kMax(rc.descent, rc.fontDescent() + rc.y);
                      }
                    }
                    rc.x   += rc.fontWidth(txt);
                    rc.xMax = kMax(rc.xMax, rc.x);

                    rc.x    = oldX;
                    rc.y   += rc.fontAscent() + rc.fontDescent() + 1;
                  }
                }
                if (rc._cache) {
                  rc._cache->append(DataRef(DataRef::DRFit, fi->text, txtAll, rc.precision, QVariant(0.0)));
                }
              }
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
      KstVectorPtr vp;
      if (vm.contains(fi->text)) {
        vp = vm[fi->text];
      }
      if (!vp) {
        KST::vectorList.lock().readLock();
        vp = *KST::vectorList.findTag(fi->text);
        KST::vectorList.lock().unlock();
      }
      if (vp) {
        if (!fi->expression.isEmpty()) {
          // Parse and evaluate as an equation
          bool ok = false;
          // FIXME: make more efficient: cache the parsed equation
          const double idx = Equation::interpret(fi->expression.latin1(), &ok, fi->expression.length());
          if (ok) {
            vp->readLock();
            const double vVal(vp->value()[int(idx)]);
            txt = QString::number(vVal, 'g', rc.precision);
            if (rc._cache) {
              rc._cache->append(DataRef(DataRef::DRVector, fi->text, fi->expression, idx, QVariant(vVal)));
            }
            vp->unlock();
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
        kstdDebug() << "Renderer did draw, time: " << t.elapsed() << endl;
#endif
      }
      rc.x += rc.fontWidth(fi->text);
    }

    if (!rc.p) {
      // No need to compute ascent and descent when really painting
      rc.ascent = kMax(rc.ascent, -rc.y + rc.fontAscent());
      if (-rc.y - rc.fontDescent() < 0) {
        rc.descent = kMax(rc.descent, rc.fontDescent() + rc.y);
      }
    }

    int xNext = rc.x;
    if (fi->group) {
      renderLabel(rc, fi->group, vm, scm, stm);
      xNext = rc.x;
    }

    if (fi->up) {
      int xPrev = rc.x;
      renderLabel(rc, fi->up, vm, scm, stm);
      xNext = kMax(xNext, rc.x);
      rc.x = xPrev;
    }

    if (fi->down) {
      renderLabel(rc, fi->down, vm, scm, stm);
      xNext = kMax(xNext, rc.x);
    }

    rc.x = xNext;
    rc.xMax = kMax(rc.xMax, rc.x);

    fi = fi->next;
  }

  rc.size = oldSize;
  rc.y = oldY;
}
