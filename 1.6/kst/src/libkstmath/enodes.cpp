/***************************************************************************
                                 enodes.cpp
                                 ----------      
    begin                : Feb 12 2004
    copyright            : (C) 2004 The University of Toronto
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
#include <string.h>
#include <unistd.h>

#include <klocale.h>

#include <qmutex.h>
#include <qregexp.h>

#include "enodes.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstmath.h"
#include "plugincollection.h"

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);
extern "C" struct yy_buffer_state *yy_scan_bytes(const char*, int);
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern "C" void yy_delete_buffer(YY_BUFFER_STATE);

using namespace Equation;

static QMutex _mutex;

QMutex& Equation::mutex() {
  return _mutex;
}


double Equation::interpret(const char *txt, bool *ok, int len) {
  if (!txt || !*txt) {
    if (ok) {
      *ok = false;
    }
    return 0.0;
  }

  mutex().lock();
  YY_BUFFER_STATE b;
  if (len > 0) {
    b = yy_scan_bytes(txt, len);
  } else {
    b = yy_scan_string(txt);
  }
  int rc = yyparse();
  yy_delete_buffer(b);
  if (rc == 0) {
    Equation::Node *eq = static_cast<Equation::Node*>(ParsedEquation);
    ParsedEquation = 0L;
    mutex().unlock();
    Equation::Context ctx;
    ctx.sampleCount = 2;
    ctx.noPoint = KST::NOPOINT;
    ctx.x = 0.0;
    ctx.xVector = 0L;
    Equation::FoldVisitor vis(&ctx, &eq);
    double v = eq->value(&ctx);
    delete eq;
    if (ok) {
      *ok = true;
    }
    return v;
  } else {
    ParsedEquation = 0L;
    mutex().unlock();
    if (ok) {
      *ok = false;
    }
    return 0.0;
  }
}


Node::Node() {
  _parentheses = false;
}


Node::~Node() {
}


bool Node::collectObjects(KstVectorMap&, KstScalarMap&, KstStringMap&) {
  return true;
}


bool Node::takeVectorsAndScalars(const KstVectorMap&, const KstScalarMap&) {
  return true;
}


void Node::visit(NodeVisitor* v) {
  v->visitNode(this);
}


KstObject::UpdateType Node::update(int counter, Context *ctx) {
  Q_UNUSED(counter)
  Q_UNUSED(ctx)
  return KstObject::NO_CHANGE;
}


/////////////////////////////////////////////////////////////////
BinaryNode::BinaryNode(Node *left, Node *right)
: Node(), _left(left), _right(right) {
}


BinaryNode::~BinaryNode() {
  delete _left;
  _left = 0L;
  delete _right;
  _right = 0L;
}


bool BinaryNode::collectObjects(KstVectorMap& v, KstScalarMap& s, KstStringMap& t) {
  bool ok = true;
  ok = _left->collectObjects(v, s, t) ? ok : false;
  ok = _right->collectObjects(v, s, t) ? ok : false;
  return ok;
}


bool BinaryNode::takeVectorsAndScalars(const KstVectorMap& vm, const KstScalarMap& sm) {
  bool rc = _left->takeVectorsAndScalars(vm, sm);
  rc = _right->takeVectorsAndScalars(vm, sm) && rc;
  return rc;
}


void BinaryNode::visit(NodeVisitor* v) {
  v->visitBinaryNode(this);
}


Node *& BinaryNode::left() {
  return _left;
}


Node *& BinaryNode::right() {
  return _right;
}


KstObject::UpdateType BinaryNode::update(int counter, Context *ctx) {
  KstObject::UpdateType l = _left->update(counter, ctx);
  KstObject::UpdateType r = _right->update(counter, ctx);

  return (l == KstObject::UPDATE || r == KstObject::UPDATE) ? KstObject::UPDATE : KstObject::NO_CHANGE;
}


/////////////////////////////////////////////////////////////////
Addition::Addition(Node *left, Node *right)
: BinaryNode(left, right) {
}


Addition::~Addition() {
}


double Addition::value(Context *ctx) {
  return _left->value(ctx) + _right->value(ctx);
}


bool Addition::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Addition::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "+" + _right->text() + ")";
  } else {
    return _left->text() + "+" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
Subtraction::Subtraction(Node *left, Node *right)
: BinaryNode(left, right) {
}


Subtraction::~Subtraction() {
}


double Subtraction::value(Context *ctx) {
  return _left->value(ctx) - _right->value(ctx);
}


bool Subtraction::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Subtraction::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "-" + _right->text() + ")";
  } else {
    return _left->text() + "-" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
Multiplication::Multiplication(Node *left, Node *right)
: BinaryNode(left, right) {
}


Multiplication::~Multiplication() {
}


double Multiplication::value(Context *ctx) {
  return _left->value(ctx) * _right->value(ctx);
}


bool Multiplication::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Multiplication::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "*" + _right->text() + ")";
  } else {
    return _left->text() + "*" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
Division::Division(Node *left, Node *right)
: BinaryNode(left, right) {
}


Division::~Division() {
}


double Division::value(Context *ctx) {
  return _left->value(ctx) / _right->value(ctx);
}


bool Division::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Division::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "/" + _right->text() + ")";
  } else {
    return _left->text() + "/" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
Modulo::Modulo(Node *left, Node *right)
: BinaryNode(left, right) {
}


Modulo::~Modulo() {
}


double Modulo::value(Context *ctx) {
  return fmod(_left->value(ctx), _right->value(ctx));
}


bool Modulo::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Modulo::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "%" + _right->text() + ")";
  } else {
    return _left->text() + "%" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
Power::Power(Node *left, Node *right)
: BinaryNode(left, right) {
}


Power::~Power() {
}


double Power::value(Context *ctx) {
  return pow(_left->value(ctx), _right->value(ctx));
}


bool Power::isConst() {
  return _left->isConst() && _right->isConst();
}


QString Power::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + "^" + _right->text() + ")";
  } else {
    return _left->text() + "^" + _right->text();
  }
}


/////////////////////////////////////////////////////////////////

static double cot(double x) {
  return 1.0/tan(x);
}


static double csc(double x) {
  return 1.0/sin(x);
}


static double sec(double x) {
  return 1.0/cos(x);
}


static struct {
  const char *name;
  double (*func)(double);
} FTable[] = {
  {"abs",  &fabs},
  {"acos", &acos},
  {"asin", &asin},
  {"atan", &atan},
  {"cbrt", &cbrt},
  {"cos",  &cos},
  {"cosh", &cosh},
  {"cot",  &cot},
  {"csc",  &csc},
  {"exp",  &exp},
  {"log",  &log10},
  {"ln",   &log},
  {"sec",  &sec},
  {"sin",  &sin},
  {"sinh", &sinh},
  {"sqrt", &sqrt},
  {"tan",  &tan},
  {"tanh", &tanh},
  {0, 0}
};


Function::Function(char *name, ArgumentList *args)
: Node(), _name(name), _args(args), _f(0L), _cStylePlugin(0L), _dataObjectPlugin(0L) {
  _argCount = 1; // Presently no functions take != 1 argument
  _inPid = 0L;
  _inScalars = 0L;
  _inVectors = 0L;
  _outScalars = 0L;
  _outVectors = 0L;
  _inArrayLens = 0L;
  _outArrayLens = 0L;
  _outputIndex = EQ_INDEX_ERROR;
  _localData = 0L;
  _outputVectorCnt = 0;
  _inputVectorCnt = 0;

  if (strcasecmp("plugin", name) == 0) {
    Identifier *pn = dynamic_cast<Identifier*>(_args->node(0));

    if (pn) {
      //
      // first check for the C-style plugins...
      //
      _cStylePlugin = PluginCollection::self()->plugin(pn->name());
      if (!_cStylePlugin) {
        //
        // the user may be using the readable name -
        //  we should certainly support this as it is the only name the user ever sees...
        //
        QMap<QString,QString>::ConstIterator rc;

        rc = PluginCollection::self()->readableNameList().find(pn->name());
        if (rc != PluginCollection::self()->readableNameList().end()) {
          _cStylePlugin = PluginCollection::self()->plugin(*rc);
        }
      }

      if (_cStylePlugin) {
        const QValueList<Plugin::Data::IOValue>& itable = _cStylePlugin->data()._inputs;
        const QValueList<Plugin::Data::IOValue>& otable = _cStylePlugin->data()._outputs;
        unsigned ignore;

        Plugin::countScalarsVectorsAndStrings(itable, _inputScalarCnt, _inputVectorCnt, _inputStringCnt, _inPid);
        Plugin::countScalarsVectorsAndStrings(otable, _outputScalarCnt, _outputVectorCnt, _outputStringCnt, ignore);
        assert(_inputStringCnt == 0 && _outputStringCnt == 0); // FIXME: implement support for strings
        _inScalars = new double[_inputScalarCnt];
        _outScalars = new double[_outputScalarCnt];
        _inVectors = new double*[_inputVectorCnt];
        _outVectors = new double*[_outputVectorCnt];
        _inArrayLens = new int[_inputVectorCnt];
        _outArrayLens = new int[_outputVectorCnt];
        memset(_outVectors, 0, _outputVectorCnt*sizeof(double*));
        memset(_outArrayLens, 0, _outputVectorCnt*sizeof(int));
      }

      //
      // now check for the KstDataObject plugins...
      //
      if (!_cStylePlugin) {
        _dataObjectPlugin = kst_cast<KstBasicPlugin>(KstDataObject::createPlugin(pn->name()));
        if (_dataObjectPlugin) {
          QStringList vectors = _dataObjectPlugin->outputVectorList();
          QStringList scalars = _dataObjectPlugin->outputScalarList();

          KstWriteLocker pl(_dataObjectPlugin);

          for (QStringList::ConstIterator it = vectors.begin(); it != vectors.end(); ++it) {
            _dataObjectPlugin->setOutputVector(*it, QString::null);
          }

          for (QStringList::ConstIterator it = scalars.begin(); it != scalars.end(); ++it) {
            _dataObjectPlugin->setOutputScalar(*it, QString::null);
          }
        }
      }

      if (!_cStylePlugin && !_dataObjectPlugin) {
        KstDebug::self()->log(i18n("Equation was unable to load plugin %1.").arg(pn->name()), KstDebug::Warning);
      }
    } else {
      KstDebug::self()->log(i18n("A plugin call in an equation requires the first argument to be the name of the plugin."), KstDebug::Warning);
    }
  } else {
    for (int i = 0; FTable[i].name; ++i) {
      if (strcasecmp(FTable[i].name, name) == 0) {
        _f = (void*)FTable[i].func;
        break;
      }
    }
  }
}


Function::~Function() {
  free(_name);
  _name = 0L;
  delete _args;
  _args = 0L;
  _f = 0L;
  if (_localData) {
    if (!_cStylePlugin->freeLocalData(&_localData)) {
      free(_localData);
    }
    _localData = 0L;
  }

  _cStylePlugin = 0L;
  _dataObjectPlugin = 0L;

  delete[] _inScalars;
  delete[] _inVectors;
  delete[] _outScalars;
  for (uint i = 0; i < _outputVectorCnt; ++i) {
    free(_outVectors[i]);
  }
  delete[] _outVectors;
  delete[] _inArrayLens;
  delete[] _outArrayLens;
}


KstObject::UpdateType Function::updateCStylePlugin(Context *ctx) {
  const QValueList<Plugin::Data::IOValue>& itable = _cStylePlugin->data()._inputs;
  uint itcnt = 0, vitcnt = 0, cnt = 0;

  // populate the input scalars and vectors
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      Data *d = dynamic_cast<Data*>(_args->node(cnt + 1));
      if (d && d->_vector) {
        _inVectors[vitcnt] = d->_vector->value();
        _inArrayLens[vitcnt++] = d->_vector->length();
      } else {
        Identifier *pn = dynamic_cast<Identifier*>(_args->node(cnt + 1));
        if (pn && 0 == strcmp(pn->name(), "x")) {
          if (!ctx->xVector) {
            _outputIndex = EQ_INDEX_ERROR;
            // hope we recover later
            return KstObject::NO_CHANGE;
          }
          _inVectors[vitcnt] = ctx->xVector->value();
          _inArrayLens[vitcnt++] = ctx->xVector->length();
        } else {
          _outputIndex = EQ_INDEX_ERROR;
          KstDebug::self()->log(i18n("Plugin %2 failed when called from equation.  Argument %1 was not found.").arg(cnt + 1).arg(_cStylePlugin->data()._name), KstDebug::Warning);
          return KstObject::NO_CHANGE;
        }
      }
      ++cnt;
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      Node *n = _args->node(cnt + 1);
      _inScalars[itcnt++] = n->value(ctx);
      ++cnt;
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      _inScalars[itcnt++] = getpid();
    }
  }

  int rc;

  if (_cStylePlugin->data()._localdata) {
    rc = _cStylePlugin->call(_inVectors, _inArrayLens, _inScalars, _outVectors, _outArrayLens, _outScalars, &_localData);
  } else {
    rc = _cStylePlugin->call(_inVectors, _inArrayLens, _inScalars, _outVectors, _outArrayLens, _outScalars);
  }

  _outputIndex = EQ_INDEX_ERROR;
  if (rc != 0) {
    KstDebug::self()->log(i18n("Plugin %1 failed when called from equation.").arg(_cStylePlugin->data()._name), KstDebug::Warning);
    return KstObject::NO_CHANGE;
  }

  if (!_cStylePlugin->data()._filterOutputVector.isEmpty()) {
    int loc = 0;
    bool found = false;
    const QValueList<Plugin::Data::IOValue>& otable = _cStylePlugin->data()._outputs;

    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        if ((*it)._name == _cStylePlugin->data()._filterOutputVector) {
          found = true;
          break;
        }
        loc++;
      }
    }
    if (found) {
      _outputIndex = loc;
    }
  }

  if (_outputIndex == EQ_INDEX_ERROR) {
    if (_outputVectorCnt > 0) {
      if (_outVectors[0] && _outArrayLens[0] > 1) {
        _outputIndex = 0;
      }
    } else if (_outputScalarCnt > 0 && _outScalars) { // make sense?
      _outputIndex = -1;
    }
  }

  return KstObject::UPDATE;
}


KstObject::UpdateType Function::updateDataObjectPlugin(int counter, Context *ctx) {
  KstObject::UpdateType ut = KstObject::UPDATE;
  QStringList vectors;
  QStringList scalars;
  QStringList outVectors;
  QStringList outScalars;
  uint cnt = 0;

  // populate the input scalars and vectors
  vectors = _dataObjectPlugin->inputVectorList();
  for (QStringList::ConstIterator it = vectors.begin(); it != vectors.end(); ++it) {
    Data *d = dynamic_cast<Data*>(_args->node(cnt + 1));
    if (d && d->_vector) {
      _dataObjectPlugin->setInputVector(*it, d->_vector);
    } else {
      Identifier *pn = dynamic_cast<Identifier*>(_args->node(cnt + 1));
      if (pn && 0 == strcmp(pn->name(), "x")) {
        if (!ctx->xVector) {
          _outputIndex = EQ_INDEX_ERROR;
          // hope we recover later
          return KstObject::NO_CHANGE;
        }
        _dataObjectPlugin->setInputVector(*it, ctx->xVector);
      } else {
        _outputIndex = EQ_INDEX_ERROR;
        KstDebug::self()->log(i18n("Plugin %2 failed when called from equation.  Argument %1 was not found.").arg(cnt + 1).arg(_dataObjectPlugin->tagName()), KstDebug::Warning);
        return KstObject::NO_CHANGE;
      }
    }
    ++cnt;
  }

  scalars = _dataObjectPlugin->inputScalarList();
  for (QStringList::ConstIterator it = scalars.begin(); it != scalars.end(); ++it) {
    Data *d = dynamic_cast<Data*>(_args->node(cnt + 1));
    if (d && d->_scalar) {
      _dataObjectPlugin->setInputScalar(*it, d->_scalar);
    } else {
      Node *n = _args->node(cnt + 1);
      if (n) {
        KstScalarPtr sp = new KstScalar(KstObjectTag::fromString(*it), 0L, n->value(ctx), true, false, false);
        if (sp) {
          _dataObjectPlugin->setInputScalar(*it, sp);
        }
      }
    }
    ++cnt;
  }

  {
    KstWriteLocker pl(_dataObjectPlugin);

    _dataObjectPlugin->update(counter);
  }

  _outputIndex = EQ_INDEX_ERROR;
  outVectors = _dataObjectPlugin->outputVectorList();

  if (_outputIndex == EQ_INDEX_ERROR) {
    if (outVectors.count() > 0) {
      KstVectorPtr vectorOut = _dataObjectPlugin->outputVector(outVectors.first());

      if (vectorOut && vectorOut->length() > 1) {
        _outputIndex = 0;
      }
    } else if (_dataObjectPlugin->outputScalarList().count() > 0) {
      _outputIndex = -1;
    }
  }
  return ut;
}


KstObject::UpdateType Function::update(int counter, Context *ctx) {
  KstObject::UpdateType ut = _args->update(counter, ctx);

  if (ut != KstObject::NO_CHANGE || counter != -1) {
    if (_cStylePlugin) {
      ut = updateCStylePlugin(ctx);
    } else if (_dataObjectPlugin) {
      ut = updateDataObjectPlugin(counter, ctx);
    } else {
      ut = KstObject::NO_CHANGE;
    }
  } else {
    ut = KstObject::NO_CHANGE;
  }

  return ut;
}


double Function::evaluateCStylePlugin(Context *ctx) {
  if (_outputIndex >= 0) {
    return ::kstInterpolate(_outVectors[_outputIndex], _outArrayLens[_outputIndex], ctx->i, ctx->sampleCount);
  } else if (_outputIndex == EQ_INDEX_ERROR) {
    return ctx->noPoint;
  } else { // make sense?
    return _outScalars[abs(_outputIndex) - 1];
  }

  return ctx->noPoint;
}


double Function::evaluateDataObjectPlugin(Context *ctx) {
  if (_outputIndex == EQ_INDEX_ERROR) {
    return ctx->noPoint;
  } else if (_outputIndex >= 0) {
    if (int(_dataObjectPlugin->outputVectorList().count()) > _outputIndex) {
      KstVectorPtr vector;

      vector = _dataObjectPlugin->outputVector(_dataObjectPlugin->outputVectorList()[_outputIndex]);
      if (vector) {
        return vector->interpolate(ctx->i, ctx->sampleCount);
      }
    }
  } else {
    int index = abs(_outputIndex) - 1;

    if (int(_dataObjectPlugin->outputScalarList().count()) > index) {
      KstScalarPtr scalar;

      scalar = _dataObjectPlugin->outputScalar(_dataObjectPlugin->outputScalarList()[index]);
      if (scalar) {
        return scalar->value();
      }
    }
  }

  return ctx->noPoint;
}


double Function::value(Context *ctx) {
  if (_cStylePlugin) {
    return evaluateCStylePlugin(ctx);
  } else if (_dataObjectPlugin) {
    return evaluateDataObjectPlugin(ctx);
  }

  if (!_f) {
    return ctx->noPoint;
  }

  if (_argCount == 1) {
    double x = _args->at(0, ctx);
    return ((double (*)(double))_f)(x);
  } else if (_argCount > 1) {
    double *x = new double[_argCount];
    for (int i = 0; i < _argCount; ++i) {
      x[i] = _args->at(i, ctx);
    }
    delete[] x;
    return ((double (*)(double*))_f)(x);
  } else {
    return ((double (*)())_f)();
  }
}


bool Function::isConst() {
  return _args->isConst();
}


bool Function::isPlugin() const {
  return _cStylePlugin != 0L;
}


bool Function::collectObjects(KstVectorMap& v, KstScalarMap& s, KstStringMap& t) {
  return _args->collectObjects(v, s, t);
}


bool Function::takeVectorsAndScalars(const KstVectorMap& vm, const KstScalarMap& sm) {
  return _args->takeVectorsAndScalars(vm, sm);
}


QString Function::text() const {
  return QString::fromLatin1(_name) + "(" + _args->text() + ")";
}


/////////////////////////////////////////////////////////////////
ArgumentList::ArgumentList()
: Node() {
  _args.setAutoDelete(true);
}


ArgumentList::~ArgumentList() {
}


void ArgumentList::appendArgument(Node *arg) {
  _args.append(arg);
}


double ArgumentList::at(int arg, Context *ctx) {
  if (arg < (int)_args.count()) {
    Node *n = _args.at(arg);
    if (n) {
      return n->value(ctx);
    }
  }
  return ctx->noPoint;
}


bool ArgumentList::isConst() {
  for (Node *i = _args.first(); i; i = _args.next()) {
    if (!i->isConst()) {
      return false;
    }
  }
  return true;
}


bool ArgumentList::collectObjects(KstVectorMap& v, KstScalarMap& s, KstStringMap& t) {
  bool ok = true;
  for (Node *i = _args.first(); i; i = _args.next()) {
    ok = i->collectObjects(v, s, t) ? ok : false;
  }
  return ok;
}


bool ArgumentList::takeVectorsAndScalars(const KstVectorMap& vm, const KstScalarMap& sm) {
  bool rc = true;
  for (Node *i = _args.first(); i; i = _args.next()) {
    rc = i->takeVectorsAndScalars(vm, sm) && rc;
  }
  return rc;
}


Node *ArgumentList::node(int idx) {
  if (idx < (int)_args.count()) {
    return _args.at(idx);
  } else {
    return 0L;
  }
}


KstObject::UpdateType ArgumentList::update(int counter, Context *ctx) {
  bool updated = false;
  for (Node *i = _args.first(); i; i = _args.next()) {
    updated = updated || KstObject::UPDATE == i->update(counter, ctx);
  }
  return updated ? KstObject::UPDATE : KstObject::NO_CHANGE;
}


QString ArgumentList::text() const {
  QString rc;
  bool first = true;
  QPtrListIterator<Node> it(_args);
  const Node *i;
  while ( (i = it.current()) ) {
    if (!first) {
      rc += ", ";
    } else {
      first = false;
    }
    rc += i->text();
    ++it;
  }
  return rc;
}


/////////////////////////////////////////////////////////////////
static struct {
  const char *name;
  double value;
} ITable[] = {
  {"e", 2.7128182846},
  {"pi", 3.1415926536},
  {0, 0.0}
};

Identifier::Identifier(char *name)
: Node(), _name(name), _const(0L) {
  for (int i = 0; ITable[i].name; ++i) {
    if (strcasecmp(ITable[i].name, name) == 0) {
      _const = &ITable[i].value;
      break;
    }
  }
}


Identifier::~Identifier() {
  free(_name);
  _name = 0L;
}


const char *Identifier::name() const {
  return _name;
}


double Identifier::value(Context *ctx) {
  if (_const) {
    return *_const;
  } else if (_name[0] == 'x' && _name[1] == 0) {
    return ctx->x;
  } else {
    return ctx->noPoint;
  }
}


bool Identifier::isConst() {
  return _const != 0L || !(_name[0] == 'x' && _name[1] == 0);
}


QString Identifier::text() const {
  return _name;
}


/////////////////////////////////////////////////////////////////
Data::Data(char *name)
: Node(), _isEquation(false), _equation(0L) {
  if (name[0] == '=') {
    _tagName = QString(&name[1]).stripWhiteSpace();
    _isEquation = true;
  } else if (strchr(name, '[')) {
    _tagName = QString(name).stripWhiteSpace();
    QRegExp re("(.*)\\[(.*)\\]");
    int hit = re.search(_tagName);
    if (hit > -1 && re.numCaptures() == 2) {
      _vector = *KST::vectorList.findTag(re.cap(1));
      if (_vector) {
        _vectorIndex = re.cap(2);
      }
    }
  } else {
    _tagName = QString(name).stripWhiteSpace();
    _vector = *KST::vectorList.findTag(_tagName);
    if (!_vector) {
      _scalar = *KST::scalarList.findTag(_tagName);
    }
  }
  free(name);
  name = 0L;
}


Data::~Data() {
  delete _equation;
  _equation = 0L;
}


double Data::value(Context *ctx) {
  if (_isEquation) {
    if (!_equation) {
      mutex().lock();
      YY_BUFFER_STATE b = yy_scan_bytes(_tagName.latin1(), _tagName.length());
      int rc = yyparse();
      yy_delete_buffer(b);
      if (rc == 0 && ParsedEquation) {
        _equation = static_cast<Equation::Node*>(ParsedEquation);
        ParsedEquation = 0L;
        mutex().unlock();
        Equation::Context ctx;
        ctx.sampleCount = 2;
        ctx.noPoint = KST::NOPOINT;
        ctx.x = 0.0;
        ctx.xVector = 0L;
        Equation::FoldVisitor vis(&ctx, &_equation);
      } else {
        ParsedEquation = 0L;
        mutex().unlock();
        _isEquation = false;
        return ctx->noPoint;
      }
    }
    return _equation->value(ctx);
  } else if (_vector) {
    if (!_equation && !_vectorIndex.isEmpty()) {
      mutex().lock();
      YY_BUFFER_STATE b = yy_scan_bytes(_vectorIndex.latin1(), _vectorIndex.length());
      int rc = yyparse();
      yy_delete_buffer(b);
      if (rc == 0 && ParsedEquation) {
        _equation = static_cast<Equation::Node*>(ParsedEquation);
        ParsedEquation = 0L;
        mutex().unlock();
        Equation::Context ctx;
        ctx.sampleCount = 2;
        ctx.noPoint = KST::NOPOINT;
        ctx.x = 0.0;
        ctx.xVector = 0L;
        Equation::FoldVisitor vis(&ctx, &_equation);
      } else {
        ParsedEquation = 0L;
        mutex().unlock();
        _vectorIndex = QString::null;
        _vector = 0L;
        return ctx->noPoint;
      }
    }
    if (_equation) {
      // Note: should we use a fresh context here?
      return _vector->value(int(_equation->value(ctx)));
    }
    return _vector->interpolate(ctx->i, ctx->sampleCount);
  } else if (_scalar) {
    return _scalar->value();
  } else {
    return ctx->noPoint;
  }
}


bool Data::isConst() {
  return (_isEquation && _equation) ? _equation->isConst() : false;
}


bool Data::collectObjects(KstVectorMap& v, KstScalarMap& s, KstStringMap& t) {
  if (_isEquation) {
    if (_equation) {
      _equation->collectObjects(v, s, t);
    }
  } else if (_vector && !v.contains(_tagName)) {
    v.insert(_tagName, _vector);
  } else if (_scalar && !s.contains(_tagName)) {
    s.insert(_tagName, _scalar);
  } else if (!_scalar && !_vector) {
    KstDebug::self()->log(i18n("Equation has unknown object [%1].").arg(_tagName), KstDebug::Error);
    return false;
  }
  return true;
}


bool Data::takeVectorsAndScalars(const KstVectorMap& vm, const KstScalarMap& sm) {
  if (_isEquation) {
    if (_equation) {
      return _equation->takeVectorsAndScalars(vm, sm);
    }
    return false;
  }

  if (_vector) {
    if (vm.contains(_tagName)) {
      _vector = vm[_tagName];
    } else {
      return false;
    }
  } else if (_scalar) {
    if (sm.contains(_tagName)) {
      _scalar = sm[_tagName];
    } else {
      return false;
    }
  } else {
    if (vm.contains(_tagName)) {
      _vector = vm[_tagName];
    } else if (sm.contains(_tagName)) {
      _scalar = sm[_tagName];
    } else {
      return false;
    }
  }

  return true;
}


KstObject::UpdateType Data::update(int counter, Context *ctx) {
  Q_UNUSED(ctx)
  if (_isEquation) {
    if (_equation) {
      return _equation->update(counter, ctx);
    }
  } else if (_vector) {
    KstWriteLocker l(_vector);
    return _vector->update(counter);
  } else if (_scalar) {
    KstWriteLocker l(_scalar);
    return _scalar->update(counter);
  }
  return KstObject::NO_CHANGE;
}


QString Data::text() const {
  if (_isEquation) {
    return QString("[=") + _tagName + "]";
  } else if (_vector) {
    return _vector->tagLabel();
  } else if (_scalar) {
    return _scalar->tagLabel();
  } else {
    return QString::null;
  }
}

/////////////////////////////////////////////////////////////////
Number::Number(double n)
: Node(), _n(n) {
}


Number::~Number() {
}


double Number::value(Context*) {
  return _n;
}


bool Number::isConst() {
  return true;
}


QString Number::text() const {
  if (_parentheses) {
    return QString("(") + QString::number(_n, 'g', 15) + ")";
  } else {
    return QString::number(_n, 'g', 15);
  }
}


/////////////////////////////////////////////////////////////////
Negation::Negation(Node *node)
: Node(), _n(node) {
}


Negation::~Negation() {
  delete _n;
  _n = 0L;
}


double Negation::value(Context *ctx) {
  double v = _n->value(ctx);
  return (v == v) ? -v : v;
}


bool Negation::isConst() {
  return _n->isConst();
}


QString Negation::text() const {
  if (_parentheses) {
    return QString("(-") + _n->text() + ")";
  } else {
    return QString("-") + _n->text();
  }
}


/////////////////////////////////////////////////////////////////
LogicalNot::LogicalNot(Node *node)
: Node(), _n(node) {
}


LogicalNot::~LogicalNot() {
  delete _n;
  _n = 0L;
}


double LogicalNot::value(Context *ctx) {
  double v = _n->value(ctx);
  return (v == v) ? (v == 0.0) : 1.0;
}


bool LogicalNot::isConst() {
  return _n->isConst();
}


QString LogicalNot::text() const {
  if (_parentheses) {
    return QString("(!") + _n->text() + ")";
  } else {
    return QString("!") + _n->text();
  }
}


/////////////////////////////////////////////////////////////////
BitwiseAnd::BitwiseAnd(Node *left, Node *right)
: BinaryNode(left, right) {
}


BitwiseAnd::~BitwiseAnd() {
}


double BitwiseAnd::value(Context *ctx) {
  return long(_left->value(ctx)) & long(_right->value(ctx));
}


bool BitwiseAnd::isConst() {
  return _left->isConst() && _right->isConst();
}


QString BitwiseAnd::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("&") + _right->text() + ")";
  } else {
    return _left->text() + QString("&") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
BitwiseOr::BitwiseOr(Node *left, Node *right)
: BinaryNode(left, right) {
}


BitwiseOr::~BitwiseOr() {
}


double BitwiseOr::value(Context *ctx) {
  return long(_left->value(ctx)) | long(_right->value(ctx));
}


bool BitwiseOr::isConst() {
  return _left->isConst() && _right->isConst();
}


QString BitwiseOr::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("|") + _right->text() + ")";
  } else {
    return _left->text() + QString("|") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
LogicalAnd::LogicalAnd(Node *left, Node *right)
: BinaryNode(left, right) {
}


LogicalAnd::~LogicalAnd() {
}


double LogicalAnd::value(Context *ctx) {
  return (_left->value(ctx) && _right->value(ctx)) ? EQ_TRUE : EQ_FALSE;
}


bool LogicalAnd::isConst() {
  return _left->isConst() && _right->isConst();
}


QString LogicalAnd::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("&&") + _right->text() + ")";
  } else {
    return _left->text() + QString("&&") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
LogicalOr::LogicalOr(Node *left, Node *right)
: BinaryNode(left, right) {
}


LogicalOr::~LogicalOr() {
}


double LogicalOr::value(Context *ctx) {
  return (_left->value(ctx) || _right->value(ctx)) ? EQ_TRUE : EQ_FALSE;
}


bool LogicalOr::isConst() {
  return _left->isConst() && _right->isConst();
}


QString LogicalOr::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("||") + _right->text() + ")";
  } else {
    return _left->text() + QString("||") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
LessThan::LessThan(Node *left, Node *right)
: BinaryNode(left, right) {
}


LessThan::~LessThan() {
}


double LessThan::value(Context *ctx) {
  return _left->value(ctx) < _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool LessThan::isConst() {
  return _left->isConst() && _right->isConst();
}


QString LessThan::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("<") + _right->text() + ")";
  } else {
    return _left->text() + QString("<") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
LessThanEqual::LessThanEqual(Node *left, Node *right)
: BinaryNode(left, right) {
}


LessThanEqual::~LessThanEqual() {
}


double LessThanEqual::value(Context *ctx) {
  return _left->value(ctx) <= _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool LessThanEqual::isConst() {
  return _left->isConst() && _right->isConst();
}


QString LessThanEqual::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("<=") + _right->text() + ")";
  } else {
    return _left->text() + QString("<=") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
GreaterThan::GreaterThan(Node *left, Node *right)
: BinaryNode(left, right) {
}


GreaterThan::~GreaterThan() {
}


double GreaterThan::value(Context *ctx) {
  return _left->value(ctx) > _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool GreaterThan::isConst() {
  return _left->isConst() && _right->isConst();
}


QString GreaterThan::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString(">") + _right->text() + ")";
  } else {
    return _left->text() + QString(">") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
GreaterThanEqual::GreaterThanEqual(Node *left, Node *right)
: BinaryNode(left, right) {
}


GreaterThanEqual::~GreaterThanEqual() {
}


double GreaterThanEqual::value(Context *ctx) {
  return _left->value(ctx) >= _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool GreaterThanEqual::isConst() {
  return _left->isConst() && _right->isConst();
}


QString GreaterThanEqual::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString(">=") + _right->text() + ")";
  } else {
    return _left->text() + QString(">=") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
EqualTo::EqualTo(Node *left, Node *right)
: BinaryNode(left, right) {
}


EqualTo::~EqualTo() {
}


double EqualTo::value(Context *ctx) {
  return _left->value(ctx) == _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool EqualTo::isConst() {
  return _left->isConst() && _right->isConst();
}


QString EqualTo::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("==") + _right->text() + ")";
  } else {
    return _left->text() + QString("==") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////
NotEqualTo::NotEqualTo(Node *left, Node *right)
: BinaryNode(left, right) {
 }


NotEqualTo::~NotEqualTo() {
}


double NotEqualTo::value(Context *ctx) {
  return _left->value(ctx) != _right->value(ctx) ? EQ_TRUE : EQ_FALSE;
}


bool NotEqualTo::isConst() {
  return _left->isConst() && _right->isConst();
}


QString NotEqualTo::text() const {
  if (_parentheses) {
    return QString("(") + _left->text() + QString("!=") + _right->text() + ")";
  } else {
    return _left->text() + QString("!=") + _right->text();
  }
}


/////////////////////////////////////////////////////////////////

NodeVisitor::NodeVisitor() {
}


NodeVisitor::~NodeVisitor() {
}


/////////////////////////////////////////////////////////////////

FoldVisitor::FoldVisitor(Context* ctxIn, Node** rootNode) : NodeVisitor(), _ctx(ctxIn) {
  if ((*rootNode)->isConst() && dynamic_cast<Number*>(*rootNode) == 0L) {
    double v = (*rootNode)->value(ctxIn);
    delete *rootNode;
    *rootNode = new Number(v);
  } else {
    (*rootNode)->visit(this);
  }
  _ctx = 0L; // avoids context being marked as 'still reachable'
}


FoldVisitor::~FoldVisitor() {
}


void FoldVisitor::visitNode(Node*) {
  // useful?
}


void FoldVisitor::visitBinaryNode(BinaryNode *n) {
  if (n->left()->isConst() && dynamic_cast<Number*>(n->left()) == 0L) {
    double v = n->left()->value(_ctx);
    delete n->left();
    n->left() = new Number(v);
  } else {
    n->left()->visit(this);
  }

  if (n->right()->isConst() && dynamic_cast<Number*>(n->right()) == 0L) {
    double v = n->right()->value(_ctx);
    delete n->right();
    n->right() = new Number(v);
  } else {
    n->right()->visit(this);
  }
}

