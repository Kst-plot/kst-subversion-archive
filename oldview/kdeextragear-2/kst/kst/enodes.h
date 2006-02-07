/***************************************************************************
                                  enodes.h
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

#ifndef ENODES_H
#define ENODES_H

#include <qptrlist.h>

#include "kstvector.h"
#include "kstscalar.h"

#define EQ_FALSE 0.0
#define EQ_TRUE  1.0

namespace Equation {

  class Context {
    public:
      Context() : i(0), x(0.0), noPoint(0.0), sampleCount(0) {}
      ~Context() {}

      long i;
      double x;
      double noPoint;
      long sampleCount;
  };

  class Node {
    public:
      Node();
      virtual ~Node();

      virtual bool isConst() = 0;
      virtual void fold(Context*);
      virtual void collectVectors(KstVectorList& c);
      virtual double value(Context*) = 0;
  };

  class BinaryNode : public Node {
    public:
      BinaryNode(Node *left, Node *right);
      virtual ~BinaryNode();
      virtual void fold(Context*);
      virtual void collectVectors(KstVectorList& c);

    protected:
      Node *_left, *_right;
  };


  class ArgumentList : public Node {
    friend class Function;
    public:
      ArgumentList();
      virtual ~ArgumentList();

      void appendArgument(Node *arg);

      // Makes no sense for this type
      virtual double value(Context*) { return 0.0; }

      virtual bool isConst();
      virtual void collectVectors(KstVectorList& c);
      double at(int, Context*);

    protected:
      QPtrList<Node> _args;
  };


  class Function : public Node {
    public:
      Function(char *name, ArgumentList *args);
      virtual ~Function();

      virtual bool isConst();
      virtual double value(Context*);
      virtual void collectVectors(KstVectorList& c);

    protected:
      char *_name;
      ArgumentList *_args;
      void *_f;
  };
 

  class Number : public Node {
    public:
      Number(double n);
      virtual ~Number();

      virtual bool isConst();
      virtual double value(Context*);

    protected:
      double _n;
  };


  class Identifier : public Node {
    public:
      Identifier(char *name);
      virtual ~Identifier();

      virtual bool isConst();
      virtual double value(Context*);

    protected:
      char *_name;
      double *_const;
  };


  class Data : public Node {
    public:
      Data(char *name);
      virtual ~Data();

      virtual bool isConst();
      virtual double value(Context*);
      virtual void collectVectors(KstVectorList& c);

    protected:
      char *_name;
      KstVectorPtr _vector;
      KstScalarPtr _scalar;
  };


#define CreateNode(x)                     \
  class x : public BinaryNode {           \
    public:                               \
      x(Node *left, Node *right);         \
      virtual ~x();                       \
      virtual bool isConst();             \
      virtual double value(Context*);     \
  };

CreateNode(Addition)
CreateNode(Subtraction)
CreateNode(Multiplication)
CreateNode(Division)
CreateNode(Power)
CreateNode(And)
CreateNode(Or)
CreateNode(LessThan)
CreateNode(LessThanEqual)
CreateNode(GreaterThan)
CreateNode(GreaterThanEqual)
CreateNode(EqualTo)
#undef CreateNode

}

#endif

// vim: ts=2 sw=2 et
