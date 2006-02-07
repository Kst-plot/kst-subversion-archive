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

#include "enodes.h"

#include <kdebug.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kstdatacollection.h"

using namespace Equation;

Node::Node() {
}


Node::~Node() {
}


void Node::fold(Context*) {
}


void Node::collectVectors(KstVectorList&) {
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


void BinaryNode::fold(Context *ctx) {
  if (_left->isConst() && dynamic_cast<Number*>(_left) == 0L) {
    double v = _left->value(ctx);
    delete _left;
    _left = new Number(v);
  } else {
    _left->fold(ctx);
  }

  if (_right->isConst() && dynamic_cast<Number*>(_right) == 0L) {
    double v = _right->value(ctx);
    delete _right;
    _right = new Number(v);
  } else {
    _right->fold(ctx);
  }
}


void BinaryNode::collectVectors(KstVectorList& c) {
  _left->collectVectors(c);
  _right->collectVectors(c);
}

/////////////////////////////////////////////////////////////////
Addition::Addition(Node *left, Node *right)
: BinaryNode(left, right) {
  //printf("%p: New Addition: %p + %p\n", (void*)this, (void*)left, (void*)right);
}


Addition::~Addition() {
}


double Addition::value(Context *ctx) {
  return _left->value(ctx) + _right->value(ctx);
}


bool Addition::isConst() {
  return _left->isConst() && _right->isConst();
}


/////////////////////////////////////////////////////////////////
Subtraction::Subtraction(Node *left, Node *right)
: BinaryNode(left, right) {
  //printf("%p: New Subtraction: %p - %p\n", (void*)this, (void*)left, (void*)right);
}


Subtraction::~Subtraction() {
}


double Subtraction::value(Context *ctx) {
  return _left->value(ctx) - _right->value(ctx);
}


bool Subtraction::isConst() {
  return _left->isConst() && _right->isConst();
}


/////////////////////////////////////////////////////////////////
Multiplication::Multiplication(Node *left, Node *right)
: BinaryNode(left, right) {
  //printf("%p: New Multiplication: %p - %p\n", (void*)this, (void*)left, (void*)right);
}


Multiplication::~Multiplication() {
}


double Multiplication::value(Context *ctx) {
  return _left->value(ctx) * _right->value(ctx);
}


bool Multiplication::isConst() {
  return _left->isConst() && _right->isConst();
}


/////////////////////////////////////////////////////////////////
Division::Division(Node *left, Node *right)
: BinaryNode(left, right) {
  //printf("%p: New Division: %p - %p\n", (void*)this, (void*)left, (void*)right);
}


Division::~Division() {
}


double Division::value(Context *ctx) {
  return _left->value(ctx) / _right->value(ctx);
}


bool Division::isConst() {
  return _left->isConst() && _right->isConst();
}


/////////////////////////////////////////////////////////////////
Power::Power(Node *left, Node *right)
: BinaryNode(left, right) {
  //printf("%p: New Power: %p - %p\n", (void*)this, (void*)left, (void*)right);
}


Power::~Power() {
}


double Power::value(Context *ctx) {
  return pow(_left->value(ctx), _right->value(ctx));
}


bool Power::isConst() {
  return _left->isConst() && _right->isConst();
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


static double step(double x) {
  return x > 0.0 ? 1.0 : 0.0;
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
  {"step", &step},
  {"tan",  &tan},
  {"tanh", &tanh},
  {0, 0}
};


Function::Function(char *name, ArgumentList *args)
: Node(), _name(name), _args(args), _f(0L) {
  //printf("%p: New Function: %s - %p\n", (void*)this, name, (void*)args);
  for (int i = 0; FTable[i].name; ++i) {
    if (strcasecmp(FTable[i].name, name) == 0) {
      _f = (void*)FTable[i].func;
      break;
    }
  }
}


Function::~Function() {
  free(_name);
  _name = 0L;
  delete _args;
  _args = 0L;
  _f = 0L;
}


double Function::value(Context *ctx) {
  if (!_f) {
    return ctx->noPoint;
  }

  // FIXME: support multiple arguments
  double x = _args->at(0, ctx);
  return ((double (*)(double))_f)(x);
}


bool Function::isConst() {
  return _args->isConst();
}


void Function::collectVectors(KstVectorList& c) {
  _args->collectVectors(c);
}

/////////////////////////////////////////////////////////////////
ArgumentList::ArgumentList()
: Node() {
  //printf("%p: New Argument List\n", (void*)this);
  _args.setAutoDelete(true);
}


ArgumentList::~ArgumentList() {
}


void ArgumentList::appendArgument(Node *arg) {
  _args.append(arg);
}


double ArgumentList::at(int arg, Context *ctx) {
  Node *n = _args.at(arg);
  if (n) {
    return n->value(ctx);
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


void ArgumentList::collectVectors(KstVectorList& c) {
  for (Node *i = _args.first(); i; i = _args.next()) {
    i->collectVectors(c);
  }
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
  //printf("%p: New Identifier: %s\n", (void*)this, name);
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


/////////////////////////////////////////////////////////////////
Data::Data(char *name)
: Node(), _name(name) {
  //printf("%p: New Data Object: %s\n", (void*)this, name);
  QString n = QString(_name).mid(1);
  n.truncate(n.length() - 1);
  _vector = *KST::vectorList.findTag(n);
  if (!_vector) {
    _scalar = *KST::scalarList.findTag(n);
  }
}


Data::~Data() {
  free(_name);
  _name = 0L;
}


double Data::value(Context *ctx) {
  if (_vector) {
    return _vector->interpolate(ctx->i, ctx->sampleCount);
  } else if (_scalar) {
    return _scalar->value();
  } else {
    return ctx->noPoint;
  }
}


bool Data::isConst() {
  return false;
}


void Data::collectVectors(KstVectorList& c) {
  if (_vector && !c.contains(_vector)) {
    c.append(_vector);
  }
}


/////////////////////////////////////////////////////////////////
Number::Number(double n)
: Node(), _n(n) {
  //printf("%p: New Number: %lf\n", (void*)this, n);
}


Number::~Number() {
}


double Number::value(Context*) {
  return _n;
}


bool Number::isConst() {
  return true;
}

// vim: ts=2 sw=2 et
