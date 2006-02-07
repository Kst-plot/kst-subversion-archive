/***************************************************************************
                          kstequationcurve.cpp: Power Spectra for KST
                             -------------------
    begin                : Fri Feb 10 2002
    copyright            : (C) 2002 by C. Barth Netterfield
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

/** A class for handling power spectra for kst
 *@author C. Barth Netterfield
 */

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <klocale.h>
#include <kdebug.h>

#include <qstylesheet.h>

#include "enodes.h"
#include "kstdoc.h"
#include "kstdatacollection.h"
#include "kstequationcurve.h"
#include "ksteqdialog_i.h"

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);

const QString KstEquationCurve::XVECTOR = "X";
const QString KstEquationCurve::OUTVECTOR = "O"; // Output (slave) vector

KstEquationCurve::KstEquationCurve(const QString &in_tag,
                                   const QString &equation,
                                   double x0, double x1,
                                   int nx, const QColor &in_color)
: KstBaseCurve() {
  _staticX = true;
  DoInterp = false;
  _xVector = _inputVectors.insert(XVECTOR, KstVector::generateVector(x0, x1, nx, QString::null));
  commonConstructor(in_tag, equation, in_color);
  update();
}

KstEquationCurve::KstEquationCurve(const QString &in_tag,
                                   const QString &equation,
                                   KstVectorPtr xvector,
                                   bool do_interp,
                                   const QColor &in_color)
: KstBaseCurve() {
  _staticX = false;
  DoInterp = do_interp; //false;
  _xVector = _inputVectors.insert(XVECTOR, xvector);

  commonConstructor(in_tag, equation, in_color);
  update();
}

KstEquationCurve::KstEquationCurve(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag, equation;
  QColor in_color("magenta");

  int ns = -1;
  double x0 = 0.0, x1 = 1.0;
  QString xvtag = QString::null;

  setHasPoints(false);
  setHasLines(true);

  _staticX = false;
  DoInterp = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "equation") {
        equation = e.text();
      } else if (e.tagName() == "x0") {
        x0 = e.text().toDouble();
      } else if (e.tagName() == "x1") {
        x1 = e.text().toDouble();
      } else if (e.tagName() == "ns") {
        ns = e.text().toInt();
      } else if (e.tagName() == "xvtag") {
        xvtag = e.text();
      } else if (e.tagName() == "xvector") {
        _inputVectorLoadQueue.append(qMakePair(XVECTOR, e.text()));
      } else if (e.tagName() == "color") {
        in_color.setNamedColor(e.text());
      } else if (e.tagName() == "hasLines") {
        HasLines = (e.text() != "0");
      } else if (e.tagName() == "hasPoints") {
        HasPoints = (e.text() != "0");
      } else if (e.tagName() == "interpolate") {
        DoInterp = true;
      } else if (e.tagName() == "pointType") {
        Point.setType(e.text().toInt());
      }
    }
    n = n.nextSibling();
  }

  if (_inputVectorLoadQueue.isEmpty()) {
    if (ns < 0) {
      ns = 2;
    }
    if (x0 == x1) {
      x1 = x0 + 2;
    }
    _staticX = true;
    DoInterp = false;
    _xVector = _inputVectors.insert(XVECTOR, KstVector::generateVector(x0, x1, ns, xvtag));
  } else {
    _xVector = _inputVectors.end();
  }
  commonConstructor(in_tag, equation, in_color);
}

KstEquationCurve::~KstEquationCurve() {
  if (_staticX) {
    _xVector = _inputVectors.end();
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }

  delete _pe;
  _pe = 0L;
}


bool KstEquationCurve::loadInputs() {
  if (!_staticX) {
    return KstDataObject::loadInputs();
  } else {
    update();
    return true;
  }
}

void KstEquationCurve::commonConstructor(const QString &in_tag,
                                         const QString &in_equation,
                                         const QColor &in_color) {
  _pe = 0L;
  _typeString = i18n("Equation");
  setHasPoints(false);
  setHasLines(true);

  int NS = 2;

  NumUsed = 0;
  Color = in_color;
  KstObject::setTagName(in_tag);

  KstVectorPtr yv = new KstVector(in_tag + "-sv", NS);
  KST::addVectorToList(yv);
  _yVector = _outputVectors.insert(OUTVECTOR, yv);
  yv->zero();

  IsValid = false;
  NumNew = NumShifted = 0;

  Equation = in_equation;
}

bool KstEquationCurve::isValid() const {
  return IsValid;
}

KstObject::UpdateType KstEquationCurve::update(int update_counter) {
  bool force = false;

  if (KstObject::checkUpdateCounter(update_counter)) {
    return NO_CHANGE;
  }

  if (update_counter <= 0) {
    force = true;
  } else {
    for (unsigned i = 0; i < VectorsUsed.count(); i++) {
      VectorsUsed[i]->update(update_counter);
    }
  }

  if (_xVector == _inputVectors.end()) {
    _xVector = _inputVectors.find(XVECTOR);
    if (!*_xVector) { // This is technically sort of fatal
      return NO_CHANGE;
    }
  }

  KstVectorPtr v = *_xVector;
  MaxX = v->max();
  MinX = v->min();
  MeanX= v->mean();
  MinPosX = v->minPos();

  IsValid = FillY(force);

  v = *_yVector;

  v->update(update_counter);

  MaxY = v->max();
  MinY = v->min();
  MeanY = v->mean();
  MinPosY = v->minPos();

  return UPDATE;
}

void KstEquationCurve::getPoint(int i, double &x, double &y) {
  x = (*_xVector)->interpolate(i, NS);
  y = (*_yVector)->value()[i];
}

void KstEquationCurve::save(QTextStream &ts) {
  ts << " <equation>" << endl;
  ts << "  <tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << "  <equation>" << QStyleSheet::escape(Equation) << "</equation>" << endl;
  if (_staticX) {
    ts << "  <x0>" << QString::number((*_xVector)->min()) << "</x0>" << endl;
    ts << "  <x1>" << QString::number((*_xVector)->max()) << "</x1>" << endl;
    ts << "  <ns>" << QString::number((*_xVector)->sampleCount()) << "</ns>" << endl;
    ts << "  <xvtag>" << QStyleSheet::escape((*_xVector)->tagName()) << "</xvtag>" << endl;
  } else {
    ts << "  <xvector>" << QStyleSheet::escape((*_xVector)->tagName()) << "</xvector>" << endl;
    if (DoInterp) {
      ts << "  <interpolate/>" << endl;
    }
  }

  ts << "  <color>" << Color.name() << "</color>" << endl;
  ts << "  <hasLines>" << HasLines << "</hasLines>" << endl;
  ts << "  <hasPoints>" << HasPoints << "</hasPoints>" << endl;
  ts << "  <pointType>" << Point.getType() << "</pointType>" << endl;
  ts << " </equation>" << endl;
}

void KstEquationCurve::setEquation(const QString& in_fn) {
  Equation = in_fn;
  VectorsUsed.clear();
  NS = 2; // reset the updating
  if (_pe) {
    delete _pe;
    _pe = 0L;
  }
}

void KstEquationCurve::setExistingXVector(KstVectorPtr in_xv, bool do_interp) {
  if (_staticX) {
    _xVector = _inputVectors.end();
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }

  _xVector = _inputVectors.insert(XVECTOR, in_xv);

  NS = 2; // reset the updating
  _staticX = false;
  DoInterp = do_interp;
}

void KstEquationCurve::setStaticXVector(double xmin, double xmax, int n) {
  if (_staticX) {
    _xVector = _inputVectors.end();
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }
  _staticX = true;
  _xVector = _inputVectors.insert(XVECTOR, KstVector::generateVector(xmin, xmax, n, QString::null));
}


QString KstEquationCurve::getYLabel() const {
  return Equation;
}

QString KstEquationCurve::getXLabel() const {
  return (*_xVector)->label();
}

KstCurveType KstEquationCurve::type() const {
  return KST_EQUATIONCURVE;
}

void KstEquationCurve::setTagName(const QString &in_tag) {
  KstObject::setTagName(in_tag);
  (*_yVector)->setTagName(in_tag+"-sv");
  if (_staticX) {
    (*_xVector)->setTagName(in_tag);
  }
}

bool KstEquationCurve::slaveVectorsUsed() const {
  return _staticX;
}


/************************************************************************/
/*                                                                      */
/*                     	Fill Y: Evaluates the equation                 	*/
/*                                                                     	*/
/************************************************************************/
bool KstEquationCurve::FillY(bool force) {
  unsigned i_v;
  int v_shift, v_new;
  int i0;
  int ns;

  // determine value of Interp
  if (DoInterp) {
    ns = (*_xVector)->sampleCount();
    for (i_v = 0; i_v < VectorsUsed.count(); i_v++) {
      if (VectorsUsed[i_v]->sampleCount() > ns) {
        ns = VectorsUsed[i_v]->sampleCount();
      }
    }
    if ((*_xVector)->sampleCount() > 0) {
      Interp = (ns+1) / (*_xVector)->sampleCount();
    } else { // avoid divide by zero
      Interp = 1;
    }
  } else {
    Interp = 1;
  }

  if (NS != (*_xVector)->sampleCount() || Interp != 1 ||
      (*_xVector)->numShift() != (*_xVector)->numNew()) {
    NS = (*_xVector)->sampleCount()*Interp - 1;

    KstVectorPtr yv = *_yVector;
    yv->resize(NS);
    yv->zero();
    i0 = 0; // other vectors may have diffent lengths, so start over
    v_shift = NS;
  } else {
    // calculate shift and new samples
    // only do shift optimization if all used vectors are same size and shift
    v_shift = (*_xVector)->numShift()*Interp;
    v_new = (*_xVector)->numNew()*Interp;

    for (i_v = 0; i_v < VectorsUsed.count(); i_v++) {
      if (v_shift != VectorsUsed[i_v]->numShift()) {
        v_shift = NS;
      }
      if (v_new != VectorsUsed[i_v]->numNew()) {
        v_shift = NS;
      }
      if (NS != VectorsUsed[i_v]->sampleCount()) {
        v_shift = NS;
      }
    }

    if (v_shift > NS/2 || force) {
      i0 = 0;
      v_shift = NS;
    } else {
      KstVectorPtr yv = *_yVector;
      for (int i = v_shift; i < NS; i++) {
        yv->value()[i - v_shift] = yv->value()[i];
      }
      i0 = NS - v_shift;
    }
  }

  NumShifted = (*_yVector)->numShift() + v_shift;
  if (NumShifted > NS) {
    NumShifted = NS;
  }

  NumNew = NS - i0 + (*_yVector)->numNew();
  if (NumNew > NS) {
    NumNew = NS;
  }

  (*_yVector)->SetNewAndShift(NumNew, NumShifted);

  double *rawv = (*_yVector)->value();
  KstVectorPtr iv = (*_xVector);

  Equation::Context ctx;
  ctx.sampleCount = NS;

  if (!_pe) {
    if (Equation.isEmpty()) {
      return true;
    }

    yy_scan_string(Equation.latin1());
    int rc = yyparse();
    if (rc == 0) {
      _pe = static_cast<Equation::Node*>(ParsedEquation);
      _pe->fold(&ctx);
      _pe->collectVectors(VectorsUsed);
      ParsedEquation = 0L;
    } else {
      delete (Equation::Node*)ParsedEquation;
      ParsedEquation = 0L;
      return false;
    }
  }

  for (ctx.i = i0; ctx.i < NS; ++ctx.i) {
    ctx.x = iv->interpolate(ctx.i, NS);
    rawv[ctx.i] = _pe->value(&ctx);
  }

  return true;
}

QString KstEquationCurve::propertyString() const {
  return equation();
}

void KstEquationCurve::_showDialog() {
  KstEqDialogI::globalInstance()->show_I(tagName());
}

// vim: ts=2 sw=2 et
