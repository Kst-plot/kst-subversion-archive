/***************************************************************************
                   linefitplugin.cpp
                             -------------------
    begin                : 09/08/06
    copyright            : (C) 2006 The University of Toronto
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
#include "linefitplugin.h"

#include <qstylesheet.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include <kstdatacollection.h>

static const QString& X_COORDINATES_IN = KGlobal::staticQString("X Array");
static const QString& Y_COORDINATES_IN = KGlobal::staticQString("Y Array");

static const QString& X_COORDINATES_OUT = KGlobal::staticQString("X Interpolated");
static const QString& Y_COORDINATES_OUT = KGlobal::staticQString("Y Interpolated");

static const QString& A = KGlobal::staticQString("a");
static const QString& B = KGlobal::staticQString("b");
static const QString& CHI2 = KGlobal::staticQString("chi^2");

K_EXPORT_COMPONENT_FACTORY( kst_linefit,
    KGenericFactory<LineFit>( "kst_linefit" ) )

LineFit::LineFit( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstDataObject() {
  m_init = false;
}

LineFit::~LineFit() {
}

bool LineFit::isValid() const {
  return m_init;
}

KstObject::UpdateType LineFit::update(int updateCounter) {
  if (isValid()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  KstVectorPtr xIn = *_inputVectors.find(X_COORDINATES_IN);
  KstVectorPtr yIn = *_inputVectors.find(Y_COORDINATES_IN);

  if (!xIn || !yIn) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool depUpdated = force;

  depUpdated = UPDATE == xIn->update(updateCounter) || depUpdated;
  depUpdated = UPDATE == yIn->update(updateCounter) || depUpdated;

  linefit();

  m_init = true;

  setLastUpdateResult(UPDATE); // make sure that provider callbacks work

  KstVectorPtr xOut = *_outputVectors.find(X_COORDINATES_OUT);
  KstVectorPtr yOut = *_outputVectors.find(Y_COORDINATES_OUT);

  vectorRealloced(xOut, xOut->value(), 2);
  xOut->setDirty();
  xOut->setNewAndShift(xOut->length(), xOut->numShift());
  xOut->update(updateCounter);

  vectorRealloced(yOut, yOut->value(), 2);
  yOut->setDirty();
  yOut->setNewAndShift(yOut->length(), yOut->numShift());
  yOut->update(updateCounter);

  KstScalarPtr a = *_outputScalars.find(A);
  KstScalarPtr b = *_outputScalars.find(B);
  KstScalarPtr chi2 = *_outputScalars.find(CHI2);

  a->update(updateCounter);
  b->update(updateCounter);
  chi2->update(updateCounter);

  return setLastUpdateResult(UPDATE);
}

void LineFit::linefit() {
  int i = 0;
  double a = 0.0, b = 0.0, sx = 0.0, sy = 0.0, sxoss = 0.0, st2 = 0.0, chi2 = 0.0;
  double xScale;

  KstVectorPtr xIn = *_inputVectors.find(X_COORDINATES_IN);
  KstVectorPtr yIn = *_inputVectors.find(Y_COORDINATES_IN);

  KstVectorPtr xOut = *_outputVectors.find(X_COORDINATES_OUT);
  KstVectorPtr yOut = *_outputVectors.find(Y_COORDINATES_OUT);

  KstScalarPtr _a = *_outputScalars.find(A);
  KstScalarPtr _b = *_outputScalars.find(B);
  KstScalarPtr _chi2 = *_outputScalars.find(CHI2);

  if (yIn->length() < 1 || xIn->length() < 1) {
    return /*-1*/;
  }

  if (xOut->length() != 2) {
    xOut->resize( 2, false );
    }
  if (yOut->length() != 2) {
    yOut->resize( 2, false );
  }

  if (yIn->length() == 1) {
    xOut->value()[0] = xIn->value()[0];
    xOut->value()[1] = xIn->value()[xIn->length() - 1];
    yOut->value()[0] = yIn->value()[0];
    yOut->value()[1] = yIn->value()[0];
    _a->setValue( yIn->value()[0] );
    _b->setValue( 0 );
    _chi2->setValue( chi2 );
    return /*0*/;
  }

  xScale = xIn->length()/yIn->length();

  for (i = 0; i < yIn->length(); i++) {
    double z = xScale*i;
    long int idx = long(rint(z));
    double skew = z - floor(z); /* [0..1] */
    long int idx2 = idx + 1;
    sy += yIn->value()[i];
    while (idx2 >= yIn->length()) {
      idx2--;
    }
    sx += xIn->value()[idx] + (xIn->value()[idx2] - xIn->value()[idx])*skew;
  }

  sxoss = sx / xIn->length();

  for (i = 0; i < xIn->length(); i++) {
    double t = xIn->value()[i] - sxoss;
    st2 += t * t;
    b += t * yIn->value()[i];
  }

  b /= st2;
  a = (sy - sx*b)/xIn->length();

  /* could put goodness of fit, etc, in here */

  xOut->value()[0] = xIn->value()[0];
  xOut->value()[1] = xIn->value()[xIn->length()-1];
  yOut->value()[0] = a+b*xOut->value()[0];
  yOut->value()[1] = a+b*xOut->value()[1];

  for (i = 0; i < xIn->length(); i++) {
    double z = xScale*i;
    long int idx = long(rint(z));
    double skew = z - floor(z); /* [0..1] */
    long int idx2 = idx + 1;
    double newX;
    double ci;
    while (idx2 >= xIn->length()) {
      idx2--;
    }
    newX = xIn->value()[idx] + (xIn->value()[idx2] - xIn->value()[idx])*skew;
    ci = yIn->value()[i] - a - b*newX;
    chi2 += ci*ci;
  }

  _a->setValue( a );
  _b->setValue( b );
  _chi2->setValue( chi2 );
}

QString LineFit::propertyString() const {
  return "linefit";
}

KstDataObjectPtr LineFit::makeDuplicate(KstDataObjectDataObjectMap&) {
  return 0;
}

void LineFit::_showDialog() {
  //KMessageBox::information( 0, "insert linefit config widget here :)", "linefitconfig" );
}

void LineFit::load(const QDomElement &e) {
  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "iscalar") {
        _inputScalarLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "istring") {
        _inputStringLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "ovector") {
        KstVectorPtr v;
        if (e.attribute("scalarList", "0").toInt()) {
          v = new KstVector(e.text(), 0, this, true);
        } else {
          v = new KstVector(e.text(), 0, this, false);
        }
        _outputVectors.insert(e.attribute("name"), v);
        KST::addVectorToList(v);
      } else if (e.tagName() == "oscalar") {
        KstScalarPtr sp = new KstScalar(e.text(), this);
        _outputScalars.insert(e.attribute("name"), sp);
      } else if (e.tagName() == "ostring") {
        KstStringPtr sp = new KstString(e.text(), this);
        _outputStrings.insert(e.attribute("name"), sp);
      }
    }
    n = n.nextSibling();
  }
}

void LineFit::save(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<plugin name=\"Line Fit\"" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstVectorMap::Iterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    ts << l2 << "<ivector name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ivector>" << endl;
  }
  for (KstScalarMap::Iterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    ts << l2 << "<iscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</iscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    ts << l2 << "<istring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</istring>" << endl;
  }
  for (KstVectorMap::Iterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    ts << l2 << "<ovector name=\"" << QStyleSheet::escape(i.key());
    if (i.data()->isScalarList()) {
      ts << "\" scalarList=\"1";
    }
    ts << "\">" << QStyleSheet::escape(i.data()->tagName())
        << "</ovector>" << endl;
  }
  for (KstScalarMap::Iterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    ts << l2 << "<oscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</oscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    ts << l2 << "<ostring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ostring>" << endl;
  }
  ts << indent << "</plugin>" << endl;
}

#include "linefitplugin.moc"
