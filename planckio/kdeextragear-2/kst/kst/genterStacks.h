/***************************************************************************
                          genterStacks: stacks for equation parsing:
                             -------------------
    begin                : 1990: modified for c++/kst June 2002
    copyright            : (C) 1990, 2002 by C. Barth Netterfield
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

/** A class for stacks for equation parsing

 *@author C. Barth Netterfield
 */
#ifndef GENTERSTACKS_H
#define GENTERSTACKS_H

#define STACKDEPTH 30
#define STRLEN 160
#define OPLEN 5
#define NUMMONOPS 21

class ValStack {
public:
  ValStack();
  ~ValStack();
  int Push(char *value);
  void LPush(char *value);
  void Pop(char *value);
  void reset();
  int Ptr();
private:
  char Stack[STACKDEPTH][STRLEN];
  int ptr;
};

class OpStack {
public:
  OpStack();
  ~OpStack();
  int Push(char *op);
  void Pop(char *op);
  char *Peek();
  void cPeek(char *op);
  int Ptr();
  void reset();
private:
  char Stack[STACKDEPTH][OPLEN];
  int ptr;
};

class NumStack {
public:
  NumStack();
  ~NumStack();
  void Push(double N);
  double Pop();
  void reset();
private:
  double stack[STACKDEPTH];
  int ptr;
};

class GQ {
public:
  GQ();
  ~GQ();
  void DQueue(char *s);
  void Queue(char *s);
  int ePtr();
  void reset();
  void rewind();
  bool error();
private:
  char Q[STACKDEPTH][STRLEN];
  int EPtr;
  int SPtr;
};

int IsNum(char*);
int IsMonop(char*);
int IsBinop(char*);
int Precedence(char*);

#endif
