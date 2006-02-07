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

#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <klocale.h>
#include <kdebug.h>

#include <qstylesheet.h>

#include "kstdoc.h"
#include "kstdatacollection.h"
#include "kstequationcurve.h"
#include "ksteqdialog_i.h"

const QString KstEquationCurve::XVECTOR = "X";
const QString KstEquationCurve::OUTVECTOR = "O"; // Output (slave) vector

KstEquationCurve::KstEquationCurve(const QString &in_tag,
                                   const QString &equation,
                                   double x0, double x1,
                                   int nx, const QColor &in_color)
: KstBaseCurve() {
  _staticX = true;
  DoInterp = false;
  _inputVectors.insert(XVECTOR, KstVector::generateVector(x0, x1, nx, QString::null));
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
  _inputVectors.insert(XVECTOR, xvector);

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
    _inputVectors.insert(XVECTOR, KstVector::generateVector(x0, x1, ns, xvtag));
  }
  commonConstructor(in_tag, equation, in_color);
}

KstEquationCurve::~KstEquationCurve() {
  if (_staticX) {
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }
}


void KstEquationCurve::loadInputs() {
  if (!_staticX) {
    KstDataObject::loadInputs();
  } else {
    update();
  }
}

void KstEquationCurve::commonConstructor(const QString &in_tag,
                                         const QString &in_equation,
                                         const QColor &in_color) {
  _typeString = i18n("Equation");
  Equation = in_equation;
  setHasPoints(false);
  setHasLines(true);

  int NS = 2;

  NumUsed = 0;
  Color = in_color;
  _tag = in_tag;

  KstVectorPtr yv = new KstVector(_tag + "-sv", NS);
  _outputVectors.insert(OUTVECTOR, yv);
  yv->zero();

  IsValid = false;
  NumNew = NumShifted = 0;
}

bool KstEquationCurve::isValid() const {
  return IsValid;
}

// FIXME: Optimize me, especially map lookups.
KstObject::UpdateType KstEquationCurve::update(int update_counter) {
  bool force = false;

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  if (update_counter <= 0){
    force = true;
  } else {
    unsigned int i;
    for (i = 0; i < VectorsUsed.count(); i++) {
      VectorsUsed[i]->update(update_counter);
    }
  }

  MaxX = _inputVectors[XVECTOR]->max();
  MinX = _inputVectors[XVECTOR]->min();
  MeanX= _inputVectors[XVECTOR]->mean();
  MinPosX = _inputVectors[XVECTOR]->minPos();

  preProcess();

  IsValid = FillRPNQ();

  IsValid = FillY(force) && IsValid;

  _outputVectors[OUTVECTOR]->update(update_counter);

  MaxY = _outputVectors[OUTVECTOR]->max();
  MinY = _outputVectors[OUTVECTOR]->min();
  MeanY = _outputVectors[OUTVECTOR]->mean();
  MinPosY = _outputVectors[OUTVECTOR]->minPos();

  return UPDATE;
}

// FIXME: Optimize me, especially map lookups.
void KstEquationCurve::getPoint(int i, double &x, double &y) {
  KstVectorPtr xv = _inputVectors[XVECTOR];
  KstVectorPtr yv = _outputVectors[OUTVECTOR];
  x = xv->interpolate(i, NS);
  y = yv->value()[i];
}

void KstEquationCurve::save(QTextStream &ts) {
  ts << " <equation>" << endl;
  ts << "  <tag>" << QStyleSheet::escape(_tag) << "</tag>" << endl;
  ts << "  <equation>" << QStyleSheet::escape(Equation) << "</equation>" << endl;
  if (_staticX) {
    ts << "  <x0>" << QString::number(_inputVectors[XVECTOR]->min()) << "</x0>" << endl;
    ts << "  <x1>" << QString::number(_inputVectors[XVECTOR]->max()) << "</x1>" << endl;
    ts << "  <ns>" << QString::number(_inputVectors[XVECTOR]->sampleCount()) << "</ns>" << endl;
    ts << "  <xvtag>" << QStyleSheet::escape(_inputVectors[XVECTOR]->tagName()) << "</xvtag>" << endl;
  } else {
    ts << "  <xvector>" << QStyleSheet::escape(_inputVectors[XVECTOR]->tagName()) << "</xvector>" << endl;
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

void KstEquationCurve::setEquation(const QString &in_fn) {
  Equation = in_fn;
  NS = 2; // reset the updating
}

void KstEquationCurve::setExistingXVector(KstVectorPtr in_xv, bool do_interp) {
  if (_staticX) {
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }

  _inputVectors[XVECTOR] = in_xv;

  NS = 2; // reset the updating
  _staticX = false;
  DoInterp = do_interp;
}

void KstEquationCurve::setStaticXVector(double xmin, double xmax, int n) {
  if (_staticX) {
    KST::vectorList.lock().writeLock();
    KST::vectorList.remove(_inputVectors[XVECTOR]);
    KST::vectorList.lock().writeUnlock();
  }
  _staticX = true;
  _inputVectors[XVECTOR] = KstVector::generateVector(xmin, xmax, n, QString::null);
}


QString KstEquationCurve::getYLabel() const {
  return Equation;
}

QString KstEquationCurve::getXLabel() const {
  return _inputVectors[XVECTOR]->label();
}

KstCurveType KstEquationCurve::type() const {
  return KST_EQUATIONCURVE;
}

void KstEquationCurve::setTagName(const QString &in_tag) {
  _tag = in_tag;
  _outputVectors[OUTVECTOR]->setTagName(in_tag+"-sv");
  if (_staticX) {
    _inputVectors[XVECTOR]->setTagName(in_tag);
  }
}

bool KstEquationCurve::slaveVectorsUsed() const {
  return _staticX;
}

/************************************************************************/
/*                                                                     	*/
/*              PreProcess: Minuses, e, PI, convert to lower, curves    */
/*                          and vectors                                 */
/*                                                                     	*/
/************************************************************************/
void KstEquationCurve::preProcess() {
  char TempString[1024];
  char ts[1020];
  int i,j,Prev;
  int i_t;
  unsigned i_v;
  bool replaced;

  strncpy(String, Equation.latin1(), 1020);

  strcpy(TempString, String);

  VectorsUsed.clear();
  ScalarsUsed.clear();

  if (TempString[0]=='-') TempString[0]='!';
  for (j=0, i=0; TempString[i]!='\0'; ) {
    Prev = (i>0 ? i-1 : 0 );
    if (TempString[i]=='e' && tolower(TempString[i+1]) != 'x' &&
        tolower(TempString[i+1]) != 'c' && tolower(TempString[i+1])!='p' &&
        (TempString[Prev]<'0' || TempString[Prev]>'9') &&
        (TempString[i+1]<'0'  || TempString[i+1]>'9')) {
      strcpy(&String[j],"2.7128182846");
      TempString[i] = 'f'; // remind later stages that we found e
      j+=12;
      i++;
    } else if (tolower(TempString[i])=='p'
        && tolower(TempString[i+1]) == 'i'
        && (TempString[Prev] < '0'
          || TempString[Prev] > '9')
        && (TempString[i+2]< '0'
          || TempString[i+2]> '9')) {
      strcpy(&String[j],"3.1415926536");
      j+=12;
      i+=2;
    } else if (TempString[i]=='-' &&
        (TempString[Prev]=='*' || TempString[Prev]=='/' ||
         TempString[Prev]=='+' || TempString[Prev]=='-' ||
         TempString[Prev]=='(' || TempString[Prev]=='E' ||
         TempString[Prev]=='^')|| TempString[Prev]=='e') {
      String[j++]='!';
      i++;
    } else if (TempString[i]==' ') {
      i++;
    } else if (TempString[i]=='[') {
      int i_brace = 1;
      for (i_t = i+1; (TempString[i_t]!='\0') && (i_brace>0); i_t++) {
        if (TempString[i_t]=='[')
          i_brace++;
        if (TempString[i_t]==']')
          i_brace--;
      }
      i_t--;

      if (i_t - i - 1>1019)
        i_t = i + 1019;

      strncpy(ts,TempString+i+1, i_t - i - 1);
      ts[i_t-i-1] = '\0';
      replaced = false;
      KST::vectorList.lock().readLock();
      for (i_v = 0; i_v < KST::vectorList.count(); i_v++) {
        // FIXME: "- 'a'" probably only allows 26 vectors in an equation
        if (KST::vectorList[i_v]->tagName() == ts) {
          String[j++] = 'V';
          String[j++] = 'a' + i_v;
          VectorsUsed.append(KST::vectorList[i_v]);
          i_v = KST::vectorList.count();
          replaced = true;
        }
      }
      KST::vectorList.lock().readUnlock();
      if (!replaced) {
        KstReadLocker ml(&KST::scalarList.lock());
        for (KstScalarList::iterator it = KST::scalarList.begin();
            it != KST::scalarList.end();
            ++it) {
          if ((*it)->tagName() == ts) {
            ScalarsUsed.append(*it);
            for (unsigned k = 0; k < (*it)->label().length(); k++) {
              if ((*it)->label()[k] == '-') {
                String[j] = '!';
              } else {
                String[j] = (*it)->label()[k];
              }
              j++;
            }
            String[j] = '\0';
          }
        }
      }
      i = i_t+1;
    } else {
      String[j++]=tolower(TempString[i++]);
    }
  }
  String[j]='\0';
}

/************************************************************************/
/*                                                                      */
/*                      Fill RPN Q: Converts String to RPN              */
/*                                                                      */
/************************************************************************/
bool KstEquationCurve::FillRPNQ() {
  bool not_done = true;
  bool Ok=true, Error=false;

  int i=0,j;
  char Tmp[300]="",Tmp2[300],Tmp1[300],Tmp3[7];

  vstack.reset();
  opstack.reset();
  numstack.reset();
  gq.reset();

  while(not_done) {
    if (IsNum(&String[i])!=0) {
      i+=vstack.Push(&String[i]);
    } else if (IsBinop(&String[i])!=0) {
      while (opstack.Ptr()>0 &&
          Precedence(opstack.Peek()) >=
          Precedence(&String[i])) {
        opstack.cPeek(Tmp);
        if (IsBinop(Tmp)!=0) {
          vstack.Pop(Tmp2);
          vstack.Pop(Tmp1);
          opstack.Pop(Tmp3);
          sprintf(Tmp,"%.140s;%.140s;%.6s;",Tmp1,Tmp2,Tmp3);
          vstack.LPush(Tmp);
        } else {
          vstack.Pop(Tmp1);
          opstack.Pop(Tmp2);
          sprintf(Tmp,"%.140s;%.6s;",Tmp1,Tmp2);
          vstack.LPush(Tmp);
        }
      }
      i+=opstack.Push(&String[i]);
    } else if (IsMonop(&String[i])!=0) {
      i+=opstack.Push(&String[i]);
    } else if (String[i]=='(') {
      i+=opstack.Push(&String[i]);
    } else if (String[i]==')') {
      opstack.cPeek(Tmp);
      if (Tmp[0]!='(') {
        Ok=true;
        while (Ok) {
          if (IsBinop(Tmp)!=0) {
            vstack.Pop(Tmp2);
            vstack.Pop(Tmp1);
            opstack.Pop(Tmp3);
            sprintf(Tmp,"%.140s;%.140s;%.6s;",Tmp1,Tmp2,Tmp3);
            vstack.LPush(Tmp);
          } else {
            vstack.Pop(Tmp1);
            opstack.Pop(Tmp2);
            sprintf(Tmp,"%.140s;%.6s;",Tmp1,Tmp2);
            vstack.LPush(Tmp);
          }
          opstack.cPeek(Tmp);
          if (Tmp[0]=='(' || Tmp[0]=='#') Ok=false;
        }
      }
      if (Tmp[0]=='#') {
        Error=true;
        not_done=false;
      }
      opstack.Pop(Tmp);
      i++;
    } else {
      not_done=false;
      if (String[i] != '\0') Error=true;
    }
  }
  if (IsBinop(&String[i-1])!=0 || IsMonop(&String[i-1])!=0)
    Error=true;
  for(opstack.cPeek(Tmp);Tmp[0]!='#';opstack.cPeek(Tmp)) {
    if (Tmp[0]=='(') Error=true;
    if (IsBinop(Tmp)!=0) {
      vstack.Pop(Tmp2);
      vstack.Pop(Tmp1);
      opstack.Pop(Tmp3);
      sprintf(Tmp,"%.140s;%.140s;%.6s;",Tmp1,Tmp2,Tmp3);
      vstack.LPush(Tmp);
    } else {
      vstack.Pop(Tmp1);
      opstack.Pop(Tmp2);
      sprintf(Tmp,"%.140s;%.6s;",Tmp1,Tmp2);
      vstack.LPush(Tmp);
    }
  }
  vstack.Pop(Tmp);

  // was the equation too long?
  if (strlen(Tmp)>140) Error = true;

  for (i=0;Tmp[i]!='\0';) {
    for (j=0;Tmp[i+j]!=';'&& Tmp[i+j]!='\0';j++) {
      Tmp1[j]=Tmp[j+i];
    }
    Tmp1[j++]='\0';
    gq.Queue(Tmp1);
    i+=j;
  }

  if (vstack.Ptr() > 0) Error = true;
  if (gq.ePtr() == 0) Error = true;
  if (gq.error()) Error = true;

  return !Error;
}

/************************************************************************/
/*                                                                      */
/*                     	Fill Y: Evaluates the equation                 	*/
/*                                                                     	*/
/************************************************************************/
// FIXME: Optimize me
bool KstEquationCurve::FillY(bool force) {
  int i;
  unsigned r = 0;
  double X, A,B;
  char Done, MError='n';
  unsigned i_v;
  int v_shift, v_new;
  int i0;
  int ns;

  KstReadLocker rl(&KST::vectorList.lock());
  // determine value of Interp
  if (DoInterp) {
    ns = _inputVectors[XVECTOR]->sampleCount();
    for (i_v=0; i_v < VectorsUsed.count(); i_v++) {
      if (VectorsUsed[i_v]->sampleCount() > ns) {
        ns = VectorsUsed[i_v]->sampleCount();
      }
    }
    if (_inputVectors[XVECTOR]->sampleCount() > 0) {
      Interp = (ns+1) / (_inputVectors[XVECTOR]->sampleCount());
    } else { // avoid divide by zero
      Interp = 1;
    }
  } else {
    Interp = 1;
  }

  if (NS != _inputVectors[XVECTOR]->sampleCount() || Interp != 1 ||
      _inputVectors[XVECTOR]->numShift() != _inputVectors[XVECTOR]->numNew() ||
       ScalarsUsed.count()) {
    NS = _inputVectors[XVECTOR]->sampleCount()*Interp-1;

    KstVectorPtr yv = _outputVectors[OUTVECTOR];
    yv->resize(NS);
    yv->zero();
    i0 = 0; // other vectors may have diffent lengths, so start over
    v_shift = NS;

    for (i =0; i < MAX_DIV_REG; i++)
      DivReg[i] = 0;
    DivRegPtr = 0;
  } else {
    // calculate shift and new samples
    // only do shift optimization if all used vectors are same size and shift
    v_shift = _inputVectors[XVECTOR]->numShift()*Interp;
    v_new = _inputVectors[XVECTOR]->numNew()*Interp;

    for (i_v = 0; i_v < VectorsUsed.count(); i_v++) {
      if (v_shift != VectorsUsed[i_v]->numShift())
        v_shift = NS;
      if (v_new != VectorsUsed[i_v]->numNew())
        v_shift = NS;
      if (NS != VectorsUsed[i_v]->sampleCount())
        v_shift = NS;
    }

    if ((v_shift > NS/2) || (force)) {
      i0 = 0;
      v_shift = NS;
    } else {
      KstVectorPtr yv = _outputVectors[OUTVECTOR];
      for (i = v_shift; i < NS; i++) {
        yv->value()[i - v_shift] = yv->value()[i];
      }
      i0 = NS - v_shift;
    }
  }

  NumShifted = _outputVectors[OUTVECTOR]->numShift() + v_shift;
  if (NumShifted > NS)
    NumShifted = NS;

  NumNew = NS - i0 + _outputVectors[OUTVECTOR]->numNew();
  if (NumNew > NS)
    NumNew = NS;

  _outputVectors[OUTVECTOR].data()->SetNewAndShift(NumNew, NumShifted);

  if (i0 == 0) {
    for (i = 0; i < MAX_DIV_REG; i++)
      DivReg[i] = 0;
    DivRegPtr = 0;
  }

  double *rawv = _outputVectors[OUTVECTOR]->value();
  KstVectorPtr iv = _inputVectors[XVECTOR];

  for(i = i0; i < NS; ++i) {
    //X = (float)i*x_step + MinX;
    X = iv->interpolate(i, NS);
    gq.rewind();
    numstack.reset();
    for (Done='n';Done=='n';) {
      gq.DQueue(String);
      if (String[0]=='#'){
        Done='y';
      } else if (String[0]=='x') {
        numstack.Push(X);
      } else if (String[0]=='V') { // a vector
        r = String[1] - 'a';
        if (r < KST::vectorList.count()) {
          numstack.Push(KST::vectorList[r]->interpolate(i, NS));
        } else {
          MError='y';
        }
      } else if (IsNum(String) != 0) {
        numstack.Push(atof(String));
      } else if (String[0]=='*') {
        numstack.Push(numstack.Pop()*numstack.Pop());
      } else if (String[0]=='+') {
        numstack.Push(numstack.Pop()+numstack.Pop());
      } else if (String[0]=='-') {
        A=numstack.Pop();
        numstack.Push(numstack.Pop()-A);
      } else if (String[0]=='^') {
        A=numstack.Pop();
        numstack.Push(pow(numstack.Pop(),A));
      } else if(String[0]=='/') {
        A=numstack.Pop();
        if ((A>0 && DivReg[DivRegPtr]<0) ||
            (A<0 && DivReg[DivRegPtr]>0) || A==0) {
          MError='y';
        }
        DivReg[DivRegPtr++]=A;

        if (MError!='y') numstack.Push(numstack.Pop()/A);

      } else if (String[0]=='!') {
        numstack.Push(-numstack.Pop());
      } else if (strcmp(String,"sinh")==0) {
        numstack.Push(sinh(numstack.Pop()));
      } else if (strcmp(String,"sin")==0) {
        numstack.Push(sin(numstack.Pop()));
      } else if (strcmp(String,"cosh")==0) {
        numstack.Push(cosh(numstack.Pop()));
      } else if (strcmp(String,"cos")==0) {
        numstack.Push(cos(numstack.Pop()));
      } else if (strcmp(String,"sec")==0) {
        A=cos(numstack.Pop());

        if ((A>0 && DivReg[DivRegPtr]<0) ||
            (A<0 && DivReg[DivRegPtr]>0) || A==0) {
          MError='y';
        }
        DivReg[DivRegPtr++]=A;

        if (MError!='y') numstack.Push(1/A);

      } else if (strcmp(String,"csc")==0) {
        A=sin(numstack.Pop());

        if ((A>0 && DivReg[DivRegPtr]<0) ||
            (A<0 && DivReg[DivRegPtr]>0) || A==0) {
          MError='y';
        }
        DivReg[DivRegPtr++]=A;

        if (MError!='y') numstack.Push(1/A);

      } else if (strcmp(String,"tanh")==0) {
        A=numstack.Pop();
        numstack.Push(sinh(A)/cosh(A));
      } else if (strcmp(String,"tan")==0) {
        A=numstack.Pop();
        B=sin(A);
        A=cos(A);

        if ((A>0 && DivReg[DivRegPtr]<0) ||
            (A<0 && DivReg[DivRegPtr]>0) || A==0) {
          MError='y';
        }
        DivReg[DivRegPtr++]=A;

        if (MError!='y') numstack.Push(B/A);

      } else if (strcmp(String,"cot")==0) {
        A=numstack.Pop();
        B=cos(A);
        A=sin(A);

        if ((A>0 && DivReg[DivRegPtr]<0) ||
            (A<0 && DivReg[DivRegPtr]>0) || A==0) {
          MError='y';
        }
        DivReg[DivRegPtr++]=A;

        if (MError!='y') numstack.Push(B/A);

      } else if (strcmp(String,"exp")==0) {
        numstack.Push(exp(numstack.Pop()));
      } else if (strcmp(String,"abs")==0) {
        numstack.Push(fabs(numstack.Pop()));
      } else if (strcmp(String,"ln")==0) {
        numstack.Push(log(numstack.Pop()));
      } else if (strcmp(String,"log")==0) {
        numstack.Push(log10(numstack.Pop()));
      } else if (strcmp(String,"sqrt")==0) {
        numstack.Push(sqrt(numstack.Pop()));
      } else if (strcmp(String,"asin")==0) {
        numstack.Push(asin(numstack.Pop()));
      } else if (strcmp(String,"acos")==0) {
        numstack.Push(acos(numstack.Pop()));
      } else if (strcmp(String,"atan")==0) {
        numstack.Push(atan(numstack.Pop()));
      } else if (strcmp(String,"cbrt")==0) {
        A = numstack.Pop();
        if (A == 0) {
          numstack.Push(0);
        } else if (A > 0) {
          numstack.Push(pow(A,.3333333333333));
        } else {
          numstack.Push(-pow(-A,.333333333333));
        }
      } else if (strcmp(String,"step")==0) {
        A = numstack.Pop();
        numstack.Push(A > 0 ? 0 : 1);
      }
    }
    if (MError == 'n') {
      rawv[i] = numstack.Pop();
    } else {
      rawv[i] = KST::NOPOINT;
    }
    MError = 'n';
    DivRegPtr = 0;
  }

  return true;

}

QString KstEquationCurve::propertyString() const {
  return equation();
}

void KstEquationCurve::_showDialog() {
  KstEqDialogI::globalInstance()->show_I(tagName());
}

// vim: sw=2 ts=2 et
