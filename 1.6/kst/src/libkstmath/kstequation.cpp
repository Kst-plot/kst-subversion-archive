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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

// include files for Qt
#include <qstylesheet.h>

// include files for KDE
#include <klocale.h>

// application specific includes
#include "defaultprimitivenames.h"
#include "dialoglauncher.h"
#include "enodes.h"
#include "eparse-eh.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstequation.h"
#include "kstsvector.h"

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);

const QString KstEquation::XINVECTOR = "X";
const QString KstEquation::XOUTVECTOR = "XO"; // Output (slave) vector
const QString KstEquation::YOUTVECTOR = "O"; // Output (slave) vector


KstEquation::KstEquation(const QString& in_tag, const QString& equation, double x0, double x1, int nx)
: KstDataObject() {

  KstVectorPtr xvector;
  QString vtag = KST::suggestVectorName(QString( "(%1..%2)" ).arg( x0 ).arg( x1 ) );

  xvector = new KstSVector(x0, x1, nx, KstObjectTag(vtag, QStringList(in_tag)));

  _doInterp = false;
  _xInVector = _inputVectors.insert(XINVECTOR, xvector);

  commonConstructor(in_tag, equation);
  setDirty();
}


KstEquation::KstEquation(const QString& in_tag, const QString& equation, KstVectorPtr xvector, bool do_interp)
: KstDataObject() {
  _doInterp = do_interp; //false;
  _xInVector = _inputVectors.insert(XINVECTOR, xvector);

  commonConstructor(in_tag, equation);
  setDirty();
}


KstEquation::KstEquation(const QDomElement &e)
: KstDataObject(e) {
  KstObjectTag xvtag = KstObjectTag::invalidTag;
  QString in_tag, equation;
  double x0 = 0.0;
  double x1 = 1.0;
  bool haveVector = false;
  int ns = -1;

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
        xvtag = KstObjectTag::fromString(e.text());
      } else if (e.tagName() == "xvector") {
        _inputVectorLoadQueue.append(qMakePair(XINVECTOR, e.text()));
        haveVector = true;
      } else if (e.tagName() == "interpolate") {
        _doInterp = true;
      }
    }
    n = n.nextSibling();
  }

  if (!haveVector) {
    if (ns < 0) {
      ns = 2;
    }
    if (x0 == x1) {
      x1 = x0 + 2.0;
    }

    KstObjectTag vtag = KstObjectTag::invalidTag;
    if (!xvtag.isValid()) {
      vtag = KST::suggestUniqueVectorTag(KstObjectTag(QString("(%1..%2)").arg(x0).arg(x1), KstObjectTag::globalTagContext));
    } else {
      vtag = xvtag;
    }

    KstVectorPtr xvector = new KstSVector(x0, x1, ns, vtag);

    _doInterp = false;
    _xInVector = _inputVectors.insert(XINVECTOR, xvector);
  } else {
    _xInVector = _inputVectors.end();
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
  _type = "Equation";
  KstObject::setTagName(KstObjectTag::fromString(in_tag));

  KstVectorPtr xv = new KstVector(KstObjectTag("xsv", tag()), 2, this);
  _xOutVector = _outputVectors.insert(XOUTVECTOR, xv);

  KstVectorPtr yv = new KstVector(KstObjectTag("sv", tag()), 2, this);
  _yOutVector = _outputVectors.insert(YOUTVECTOR, yv);

  _isValid = false;
  _numNew = _numShifted = 0;

  setEquation(in_equation);
}


const KstCurveHintList *KstEquation::curveHints() const {
  _curveHints->clear();
  _curveHints->append(new KstCurveHint(i18n("Equation Curve"), 
                      (*_xOutVector)->tagName(), (*_yOutVector)->tagName()));
  return _curveHints;
}


bool KstEquation::isValid() const {
  return _isValid;
}


KstObject::UpdateType KstEquation::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

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

  if (recursed()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  assert(update_counter >= 0);

  if (_xInVector == _inputVectors.end()) {
    _xInVector = _inputVectors.find(XINVECTOR);
    if (!*_xInVector) { // This is technically sort of fatal
      return setLastUpdateResult(NO_CHANGE);
    }
  }

  writeLockInputsAndOutputs();

  KstVectorPtr v = *_xInVector;

  xUpdated = KstObject::UPDATE == v->update(update_counter);

  Equation::Context ctx;
  ctx.sampleCount = _ns;
  ctx.xVector = v;
  usedUpdated = _pe && KstObject::UPDATE == _pe->update(update_counter, &ctx);

  KstObject::UpdateType rc = NO_CHANGE; // if force, rc = UPDATE anyway.
  if (force || xUpdated || usedUpdated) {
    _isValid = FillY(force, usedUpdated);
    rc = UPDATE;
  }
  v = *_yOutVector;
  if (rc == UPDATE) {
    v->setDirty();
  }
  v->update(update_counter);

  unlockInputsAndOutputs();

  return setLastUpdateResult(rc);
}


void KstEquation::reparse() {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  if (!_equation.isEmpty()) {
    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_equation.latin1());
    ParsedEquation = 0L;
    int rc = yyparse();
    Equation::Node *en = static_cast<Equation::Node*>(ParsedEquation);
    if (rc == 0 && en) {
      if (en->takeVectorsAndScalars(_vectorsUsed, _scalarsUsed)) {
        _equation = en->text();

        KstStringMap sm;
        _vectorsUsed.clear();
        _scalarsUsed.clear();

        en->collectObjects(_vectorsUsed, _scalarsUsed, sm);

        setupConnections();
      } else {
        kstdWarning() << "Equation [" << _equation << "] failed to find its vectors/scalars when re-parsing." << endl;
      }
    }
    delete en;
    ParsedEquation = 0L;
  }
}


void KstEquation::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<equationobject>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tag().tagString()) << "</tag>" << endl;

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
      QString etext;
      if (en->takeVectorsAndScalars(_vectorsUsed, _scalarsUsed)) {
        etext = en->text();
      } else {
        kstdWarning() << "Equation [" << _equation << "] failed to find its vectors/scalars when saving. The resulting Kst file may have issues." << endl;
        etext = _equation;
      }
      ts << l2 << "<equation>" << QStyleSheet::escape(etext) << "</equation>" << endl;
    }
    delete en;
    ParsedEquation = 0L;
  }

  ts << l2 << "<xvector>" << QStyleSheet::escape((*_xInVector)->tag().tagString()) << "</xvector>" << endl;
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
  setRecursed(false);

  _vectorsUsed.clear();
  _scalarsUsed.clear();

  _ns = 2; // reset the updating
  delete _pe;
  _pe = 0L;
  if (!_equation.isEmpty()) {
    Equation::mutex().lock();
    yy_scan_string(_equation.latin1());
    int rc = yyparse();
    _pe = static_cast<Equation::Node*>(ParsedEquation);
    if (rc == 0 && _pe) {
      ParsedEquation = 0L;
      Equation::mutex().unlock();
      Equation::Context ctx;
      ctx.sampleCount = _ns;
      ctx.xVector = *_xInVector;
      Equation::FoldVisitor vis(&ctx, &_pe);
      KstStringMap sm;

      if (_pe->collectObjects(_vectorsUsed, _scalarsUsed, sm)) {
        setupConnections();
        _pe->update(-1, &ctx);
      } else {
        //we have bad objects...
        KstDebug::self()->log(i18n("Equation [%1] references non-existent objects.").arg(_equation), KstDebug::Error);
        delete (Equation::Node*)ParsedEquation;
        ParsedEquation = 0L;
        Equation::mutex().unlock();
      }
    } else {
      // Parse error
      KstDebug::self()->log(i18n("Equation [%1] failed to parse.  Errors follow.").arg(_equation), KstDebug::Warning);
      for (QStringList::ConstIterator i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
        KstDebug::self()->log(i18n("Parse Error: %1").arg(*i), KstDebug::Warning);
      }
      delete (Equation::Node*)ParsedEquation;
      ParsedEquation = 0L;
      _pe = 0L;
      Equation::mutex().unlock();
    }
  }
  _isValid = _pe != 0L;
}


void KstEquation::setExistingXVector(KstVectorPtr in_xv, bool do_interp) {
  KstVectorPtr v = _inputVectors[XINVECTOR];
  if (v == in_xv) {
    return;
  }

  setDirty();

  _inputVectors.erase(XINVECTOR);
  _xInVector = _inputVectors.insert(XINVECTOR, in_xv);

  _ns = 2; // reset the updating
  _doInterp = do_interp;
}


void KstEquation::setTagName(const QString &in_tag) {
  KstObjectTag newTag(in_tag, tag().context());  // FIXME: always the same context?

  if (newTag == tag()) {
    return;
  }

  KstObject::setTagName(newTag);
  (*_xOutVector)->setTagName(KstObjectTag("xsv", tag()));
  (*_yOutVector)->setTagName(KstObjectTag("sv", tag()));
}


/************************************************************************/
/*                                                                      */
/*                      Fill Y: Evaluates the equation                  */
/*                                                                      */
/************************************************************************/
bool KstEquation::FillY(bool force, bool usedUpdated) {
  int v_shift = 0;
  int v_new;
  int i0 = 0;
  int ns;

  writeLockInputsAndOutputs();

  // determine value of Interp
  if (_doInterp) {
    ns = (*_xInVector)->length();
    for (KstVectorMap::ConstIterator i = _vectorsUsed.begin(); i != _vectorsUsed.end(); ++i) {
      if (i.data()->length() > ns) {
        ns = i.data()->length();
      }
    }
  } else {
    ns = (*_xInVector)->length();
  }

  if (_ns != (*_xInVector)->length() || ns != (*_xInVector)->length() ||
      (*_xInVector)->numShift() != (*_xInVector)->numNew()) {
    _ns = ns;

    KstVectorPtr xv = *_xOutVector;
    KstVectorPtr yv = *_yOutVector;
    if (!xv->resize(_ns)) {
      // FIXME: handle error?
      unlockInputsAndOutputs();
      return false;
    }
    if (!yv->resize(_ns)) {
      // FIXME: handle error?
      unlockInputsAndOutputs();
      return false;
    }
    yv->zero();
    i0 = 0; // other vectors may have diffent lengths, so start over
    v_shift = _ns;
  } else if (usedUpdated) {
    i0 = 0;
    v_shift = _ns;
  } else {
    // calculate shift and new samples
    // only do shift optimization if all used vectors are same size and shift
    v_shift = (*_xInVector)->numShift();
    v_new = (*_xInVector)->numNew();

    for (KstVectorMap::ConstIterator i = _vectorsUsed.begin(); i != _vectorsUsed.end(); ++i) {
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
      KstVectorPtr xv = *_xOutVector;
      KstVectorPtr yv = *_yOutVector;
      for (int i = v_shift; i < _ns; i++) {
        yv->value()[i - v_shift] = yv->value()[i];
        xv->value()[i - v_shift] = xv->value()[i];
      }
      i0 = _ns - v_shift;
    }
  }

  _numShifted = (*_yOutVector)->numShift() + v_shift;
  if (_numShifted > _ns) {
    _numShifted = _ns;
  }

  _numNew = _ns - i0 + (*_yOutVector)->numNew();
  if (_numNew > _ns) {
    _numNew = _ns;
  }

  (*_xOutVector)->setNewAndShift(_numNew, _numShifted);
  (*_yOutVector)->setNewAndShift(_numNew, _numShifted);

  double *rawxv = (*_xOutVector)->value();
  double *rawyv = (*_yOutVector)->value();
  KstVectorPtr iv = (*_xInVector);

  Equation::Context ctx;
  ctx.sampleCount = _ns;
  ctx.xVector = iv;

  if (!_pe) {
    if (_equation.isEmpty()) {
      unlockInputsAndOutputs();
      return true;
    }

    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_equation.latin1());
    int rc = yyparse();
    _pe = static_cast<Equation::Node*>(ParsedEquation);
    if (_pe && rc == 0) {
      Equation::FoldVisitor vis(&ctx, &_pe);
      KstStringMap sm;
      _pe->collectObjects(_vectorsUsed, _scalarsUsed, sm);
      setupConnections();
      ParsedEquation = 0L;
    } else {
      delete (Equation::Node*)ParsedEquation;
      ParsedEquation = 0L;
      _pe = 0L;
      unlockInputsAndOutputs();
      return false;
    }
  }

  for (ctx.i = i0; ctx.i < _ns; ++ctx.i) {
    rawxv[ctx.i] = iv->value(ctx.i);
    ctx.x = iv->interpolate(ctx.i, _ns);
    rawyv[ctx.i] = _pe->value(&ctx);
  }

  if (!(*_xOutVector)->resize(iv->length())) {
    // FIXME: handle error?
    unlockInputsAndOutputs();
    return false;
  }

  unlockInputsAndOutputs();
  return true;
}


QString KstEquation::propertyString() const {
  return equation();
}


void KstEquation::showNewDialog() {
  KstDialogs::self()->showEquationDialog();
}


void KstEquation::showEditDialog() {
  KstDialogs::self()->showEquationDialog(tagName(), true);
}


KstDataObjectPtr KstEquation::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  QString name(tagName() + '\'');
  while (KstData::self()->dataTagNameNotUnique(name, false)) {
    name += '\'';
  }
  KstEquationPtr eq = new KstEquation(name, _equation, _inputVectors[XINVECTOR], _doInterp);
  duplicatedMap.insert(this, KstDataObjectPtr(eq));
  return KstDataObjectPtr(eq);
}


void KstEquation::replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject) {
  QString newExp = _equation;

  // replace all occurences of outputVectors, outputScalars from oldObject
  for (KstVectorMap::Iterator j = oldObject->outputVectors().begin(); j != oldObject->outputVectors().end(); ++j) {
    QString oldTag = j.data()->tagName();
    QString newTag = ((newObject->outputVectors())[j.key()])->tagName();
    newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]");
  }

  for (KstScalarMap::Iterator j = oldObject->outputScalars().begin(); j != oldObject->outputScalars().end(); ++j) {
    QString oldTag = j.data()->tagName();
    QString newTag = ((newObject->outputScalars())[j.key()])->tagName();
    newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]");
  }

  // and dependencies on matrix stats (there won't be matrices themselves in the expression)
  for (KstMatrixMap::Iterator j = oldObject->outputMatrices().begin(); j != oldObject->outputMatrices().end(); ++j) {
    QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
    for (; scalarDictIter.current(); ++scalarDictIter) {
      QString oldTag = scalarDictIter.current()->tagName();
      QString newTag = ((((newObject->outputMatrices())[j.key()])->scalars())[scalarDictIter.currentKey()])->tagName();
      newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]"); 
    }
  }

  // only replace _inputVectors
  for (KstVectorMap::Iterator j = oldObject->outputVectors().begin(); j != oldObject->outputVectors().end(); ++j) {
    for (KstVectorMap::Iterator k = _inputVectors.begin(); k != _inputVectors.end(); ++k) {
      if (j.data().data() == k.data().data()) {
        // replace input with the output from newObject
        _inputVectors[k.key()] = (newObject->outputVectors())[j.key()]; 
      }
    }
    // and dependencies on vector stats
    QDictIterator<KstScalar> scalarDictIter(j.data()->scalars());
    for (; scalarDictIter.current(); ++scalarDictIter) {
      QString oldTag = scalarDictIter.current()->tagName();
      QString newTag = ((((newObject->outputVectors())[j.key()])->scalars())[scalarDictIter.currentKey()])->tagName();
      newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]"); 
    }
  }

  setEquation(newExp);
}


void KstEquation::replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector) {
  QString oldTag = oldVector->tagName();
  QString newTag = newVector->tagName();

  // replace all occurences of oldTag with newTag
  QString newExp = _equation.replace("[" + oldTag + "]", "[" + newTag + "]");

  // also replace all occurences of scalar stats for the oldVector
  QDictIterator<KstScalar> scalarDictIter(oldVector->scalars());
  for (; scalarDictIter.current(); ++scalarDictIter) {
    QString oldTag = scalarDictIter.current()->tagName();
    QString newTag = ((newVector->scalars())[scalarDictIter.currentKey()])->tagName();
    newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]"); 
  }

  setEquation(newExp);

  // do the dependency replacements for _inputVectors, but don't call parent function as it
  // replaces _inputScalars 
  for (KstVectorMap::Iterator j = _inputVectors.begin(); j != _inputVectors.end(); ++j) {
    if (j.data() == oldVector) {
      _inputVectors[j.key()] = newVector;
    }
  }
}


void KstEquation::replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix) {
  QString newExp = _equation;

  // also replace all occurences of scalar stats for the oldMatrix
  QDictIterator<KstScalar> scalarDictIter(oldMatrix->scalars());
  for (; scalarDictIter.current(); ++scalarDictIter) {
    QString oldTag = scalarDictIter.current()->tagName();
    QString newTag = ((newMatrix->scalars())[scalarDictIter.currentKey()])->tagName();
    newExp = newExp.replace("[" + oldTag + "]", "[" + newTag + "]"); 
  }

  setEquation(newExp);
}


bool KstEquation::uses(KstObjectPtr p) const {
  // check _vectorsUsed in addition to _input*'s
  if (KstVectorPtr vect = kst_cast<KstVector>(p)) {
    for (KstVectorMap::ConstIterator j = _vectorsUsed.begin(); j != _vectorsUsed.end(); ++j) {
      if (j.data() == vect) {
        return true;
      }
    }
  } else if (KstDataObjectPtr obj = kst_cast<KstDataObject>(p) ) {
    // check all connections from this expression to p
    for (KstVectorMap::Iterator j = obj->outputVectors().begin(); j != obj->outputVectors().end(); ++j) {
      for (KstVectorMap::ConstIterator k = _vectorsUsed.begin(); k != _vectorsUsed.end(); ++k) {
        if (j.data() == k.data()) {
          return true;
        }
      }
    }
  }
  return KstDataObject::uses(p);
}


void KstEquation::setupConnections() {
  for (KstScalarMap::iterator i = _scalarsUsed.begin(); i != _scalarsUsed.end(); ++i) {
    disconnect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
    connect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
  }
  for (KstVectorMap::iterator i = _vectorsUsed.begin(); i != _vectorsUsed.end(); ++i) {
    disconnect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
    connect(i.data(), SIGNAL(tagChanged()), this, SLOT(reparse()));
  }
}

#include "kstequation.moc"
