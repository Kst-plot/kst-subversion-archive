/***************************************************************************
                          kstequation.cpp: Equations for KST
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

/** A class for handling equations for kst
 *@author C. Barth Netterfield
 */

#include <assert.h>
#include <math.h>
#include <stdlib.h>

// include files for Qt
#include <qstylesheet.h>

// include files for KDE
#include <kdebug.h>
#include <klocale.h>

// application specific includes
#include "dialoglauncher.h"
#include "enodes.h"
#include "eparse-eh.h"
#include "kstdatacollection.h"
#include "kstdefaultnames.h"
#include "kstdebug.h"
#include "kstequation.h"
#include "kstsvector.h"

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);

const QString KstEquation::XVECTOR = "X";
const QString KstEquation::OUTVECTOR = "O"; // Output (slave) vector


KstEquation::KstEquation(const QString& in_tag, const QString& equation, double x0, double x1, int nx)
: KstDataObject() {

  KstVectorPtr xvector;
  QString vtag = KST::suggestVectorName(QString( "(%1..%2)" ).arg( x0 ).arg( x1 ) );

  xvector = new KstSVector(x0, x1, nx, vtag);
  KST::addVectorToList( xvector );

  _doInterp = false;
  _xVector = _inputVectors.insert(XVECTOR, xvector);

  commonConstructor(in_tag, equation);
  setDirty();
}


KstEquation::KstEquation(const QString& in_tag, const QString& equation, KstVectorPtr xvector, bool do_interp)
: KstDataObject() {
  _doInterp = do_interp; //false;
  _xVector = _inputVectors.insert(XVECTOR, xvector);

  commonConstructor(in_tag, equation);
  setDirty();
}


KstEquation::KstEquation(const QDomElement &e)
: KstDataObject(e) {
  QString in_tag, equation;

  int ns = -1;
  double x0 = 0.0, x1 = 1.0;
  QString xvtag;

  _doInterp = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull()) { // the node was really an element.
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
      } else if (e.tagName() == "interpolate") {
        _doInterp = true;
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

    KstVectorPtr xvector;
    QString vtag = KST::suggestVectorName(
      QString( "(%1..%2)" ).arg( x0 ).arg( x1 ) );

    xvector = new KstSVector(x0, x1, ns, vtag);
    KST::addVectorToList( xvector );

    _doInterp = false;
    _xVector = _inputVectors.insert(XVECTOR, xvector);
  } else {
    _xVector = _inputVectors.end();
  }
  commonConstructor(in_tag, equation);
}


KstEquation::~KstEquation() {
  delete _pe;
  _pe = 0L;
}

void KstEquation::commonConstructor(const QString& in_tag, const QString& in_equation) {
  _ns = 2;
  _pe = 0L;
  _typeString = i18n("Equation");
  KstObject::setTagName(in_tag);

  KstVectorPtr yv = new KstVector(tagName()+"-sv" , 2);
  KST::addVectorToList(yv);
  _yVector = _outputVectors.insert(OUTVECTOR, yv);
  yv->setProvider(this);

  _isValid = false;
  _numNew = _numShifted = 0;

  setEquation(in_equation);
}


const KstCurveHintList *KstEquation::curveHints() const {
  _curveHints->clear();
  _curveHints->append(new KstCurveHint(i18n("Equation Curve"), (*_xVector)->tagName(), (*_yVector)->tagName()));
  return _curveHints;
}


bool KstEquation::isValid() const {
  return _isValid;
}


KstObject::UpdateType KstEquation::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  bool xUpdated = false;
  bool usedUpdated = false;

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  if (!_pe) {
    return setLastUpdateResult(NO_CHANGE);
  }

  assert(update_counter >= 0);

  if (_xVector == _inputVectors.end()) {
    _xVector = _inputVectors.find(XVECTOR);
    if (!*_xVector) { // This is technically sort of fatal
      return setLastUpdateResult(NO_CHANGE);
    }
  }

  KstVectorPtr v = *_xVector;

  xUpdated = KstObject::UPDATE == v->update(update_counter);

  Equation::Context ctx;
  ctx.sampleCount = _ns;
  ctx.xVector = v;
  usedUpdated = KstObject::UPDATE == _pe->update(update_counter, &ctx);

  KstObject::UpdateType rc = NO_CHANGE; // if force, rc = UPDATE anyway.
  if (force || xUpdated || usedUpdated) {
    _isValid = FillY(force);
    rc = UPDATE;
  }
  v = *_yVector;
  v->setDirty();
  v->update(update_counter);

  return setLastUpdateResult(rc);
}


void KstEquation::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<equationobject>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;

  // Reparse the equation, then write it back out in text so that we can update
  // any vectors or scalars that had name changes, but we don't get affected by
  // the optimizer
  if (!_equation.isEmpty()) {
    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_equation.latin1());
    ParsedEquation = 0L;
    int rc = yyparse();
    Equation::Node *en = static_cast<Equation::Node*>(ParsedEquation);
    if (rc == 0 && en) {
      if (!en->takeVectors(VectorsUsed)) {
        KstDebug::self()->log(i18n("Equation [%1] failed to find its vectors when saving.  Resulting Kst file may have issues.").arg(_equation), KstDebug::Warning);
      }
      QString etext = en->text();
      ts << l2 << "<equation>" << QStyleSheet::escape(etext) << "</equation>" << endl;
    }
    delete en;
    ParsedEquation = 0L;
  }

  ts << l2 << "<xvector>" << QStyleSheet::escape((*_xVector)->tagName()) << "</xvector>" << endl;
  if (_doInterp) {
    ts << l2 << "<interpolate/>" << endl;
  }

  ts << indent << "</equationobject>" << endl;
}


void KstEquation::setEquation(const QString& in_fn) {
  // assert(*_xVector); - ugly, we have to allow this here due to
  // document loading with vector lazy-loading
  setDirty();
  _equation = in_fn;
  VectorsUsed.clear();
  _inputScalars.clear();
  _ns = 2; // reset the updating
  delete _pe;
  _pe = 0L;
  if (!_equation.isEmpty()) {
    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_equation.latin1());
    int rc = yyparse();
    _pe = static_cast<Equation::Node*>(ParsedEquation);
    if (rc == 0 && _pe) {
      Equation::Context ctx;
      ctx.sampleCount = _ns;
      ctx.xVector = *_xVector;
      Equation::FoldVisitor vis(&ctx, &_pe);
      KstStringMap sm;
      _pe->collectObjects(VectorsUsed, _inputScalars, sm);
      _pe->update(-1, &ctx);
    } else {
      // Parse error
      KstDebug::self()->log(i18n("Equation [%1] failed to parse.  Errors follow.").arg(_equation), KstDebug::Warning);
      for (QStringList::ConstIterator i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
        KstDebug::self()->log(i18n("Parse Error: %1").arg(*i), KstDebug::Warning);
      }
      delete (Equation::Node*)ParsedEquation;
    }
    ParsedEquation = 0L;
  }
  _isValid = _pe != 0L;
}


void KstEquation::setExistingXVector(KstVectorPtr in_xv, bool do_interp) {
  KstVectorPtr v = _inputVectors[XVECTOR];
  if (v) {
    if (v == in_xv) {
      return;
    }
    v->writeUnlock();
  }

  setDirty();

  _inputVectors.erase(XVECTOR);
  in_xv->writeLock();
  _xVector = _inputVectors.insert(XVECTOR, in_xv);

  _ns = 2; // reset the updating
  _doInterp = do_interp;
}


void KstEquation::setTagName(const QString &in_tag) {
  KstObject::setTagName(in_tag);
  (*_yVector)->setTagName(in_tag+"-sv");
}


/************************************************************************/
/*                                                                      */
/*                      Fill Y: Evaluates the equation                  */
/*                                                                      */
/************************************************************************/
bool KstEquation::FillY(bool force) {
  int v_shift, v_new;
  int i0;
  int ns;

  // determine value of Interp
  if (_doInterp) {
    ns = (*_xVector)->length();
    for (KstVectorMap::ConstIterator i = VectorsUsed.begin(); i != VectorsUsed.end(); ++i) {
      if (i.data()->length() > ns) {
        ns = i.data()->length();
      }
    }
  } else {
    ns = (*_xVector)->length();
  }

  if (_ns != (*_xVector)->length() || ns != (*_xVector)->length() ||
      (*_xVector)->numShift() != (*_xVector)->numNew()) {
    _ns = ns;

    KstVectorPtr yv = *_yVector;
    if (!yv->resize(_ns)) {
      // FIXME: handle error?
      return false;
    }
    yv->zero();
    i0 = 0; // other vectors may have diffent lengths, so start over
    v_shift = _ns;
  } else {
    // calculate shift and new samples
    // only do shift optimization if all used vectors are same size and shift
    v_shift = (*_xVector)->numShift();
    v_new = (*_xVector)->numNew();

    for (KstVectorMap::ConstIterator i = VectorsUsed.begin(); i != VectorsUsed.end(); ++i) {
      if (v_shift != i.data()->numShift()) {
        v_shift = _ns;
      }
      if (v_new != i.data()->numNew()) {
        v_shift = _ns;
      }
      if (_ns != i.data()->length()) {
        v_shift = _ns;
      }
    }

    if (v_shift > _ns/2 || force) {
      i0 = 0;
      v_shift = _ns;
    } else {
      KstVectorPtr yv = *_yVector;
      for (int i = v_shift; i < _ns; i++) {
        yv->value()[i - v_shift] = yv->value()[i];
      }
      i0 = _ns - v_shift;
    }
  }

  _numShifted = (*_yVector)->numShift() + v_shift;
  if (_numShifted > _ns) {
    _numShifted = _ns;
  }

  _numNew = _ns - i0 + (*_yVector)->numNew();
  if (_numNew > _ns) {
    _numNew = _ns;
  }

  (*_yVector)->setNewAndShift(_numNew, _numShifted);

  double *rawv = (*_yVector)->value();
  KstVectorPtr iv = (*_xVector);

  Equation::Context ctx;
  ctx.sampleCount = _ns;
  ctx.xVector = iv;

  if (!_pe) {
    if (_equation.isEmpty()) {
      return true;
    }

    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_equation.latin1());
    int rc = yyparse();
    _pe = static_cast<Equation::Node*>(ParsedEquation);
    if (_pe && rc == 0) {
      Equation::FoldVisitor vis(&ctx, &_pe);
      KstStringMap sm;
      _pe->collectObjects(VectorsUsed, _inputScalars, sm);
      ParsedEquation = 0L;
    } else {
      delete (Equation::Node*)ParsedEquation;
      ParsedEquation = 0L;
      return false;
    }
  }

  for (ctx.i = i0; ctx.i < _ns; ++ctx.i) {
    ctx.x = iv->interpolate(ctx.i, _ns);
    rawv[ctx.i] = _pe->value(&ctx);
  }

  return true;
}


QString KstEquation::propertyString() const {
  return equation();
}


void KstEquation::_showDialog() {
  KstDialogs::showEquationDialog(tagName());
}


bool KstEquation::uses(KstObjectPtr p) const {
  // Inefficient to call values().contains()
  return KstDataObject::uses(p) || VectorsUsed.values().contains(kst_cast<KstVector>(p)) || _inputScalars.values().contains(kst_cast<KstScalar>(p));
}

// vim: ts=2 sw=2 et
