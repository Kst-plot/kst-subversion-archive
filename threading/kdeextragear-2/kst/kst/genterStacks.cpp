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

#include "genterStacks.h"

#include <string.h>

static char MonOps[NUMMONOPS][OPLEN] = {
  "sin","cos","tan","sec","csc","cot","cot","step",
  "sinh","cosh","tanh","asin","acos","atan",
  "abs","sqrt","cbrt","exp","ln","log","!"
};

static char BinOps[5] =      {'+','-','*','/','^'};
static int BinPrecidence[5]= { 1,  1,  2,  2,  3};

/**********************************************************************/
/*                                                                    */
/*     Value Stack                                                    */
/*                                                                    */
/**********************************************************************/
ValStack::ValStack() {
  reset();
}

ValStack::~ValStack() {
}

int ValStack::Ptr() {
  return (ptr);
}

void ValStack::reset() {
  for (ptr=0; ptr<STACKDEPTH; ptr++) {
    Stack[ptr][0]='\0';
  }
  ptr = 0;
}

int ValStack::Push(char *value) {
  int i=0;
  bool done=false;

  while (!done) {
    switch (value[i]) {
        case 'x' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
        case '0' :
        case '.' :
          Stack[ptr][i]=value[i];
          if (++i>=STRLEN-1) done=true;
          break;
        case 'V' :
        case 'C' :
          Stack[ptr][i]=value[i];
          if (++i>=STRLEN-1) done=true;
          Stack[ptr][i]=value[i];
          if (++i>=STRLEN-1) done=true;
          break;
        case 'e' :
          Stack[ptr][i]='E';
          if (++i>=STRLEN-1) done=true;
          break;
        case '!' : Stack[ptr][i]='-';
          if (++i>=STRLEN-1) done=true;
          break;
        default  :
          done=true;
          break;
    }
  }

  Stack[ptr][i]='\0';
  if (ptr<STACKDEPTH-1) ptr++;
  return i;
}

void ValStack::LPush(char *value) {
  strncpy(Stack[ptr],value, STRLEN-1);
  if (ptr<STACKDEPTH-1) ptr++;
}

void ValStack::Pop(char *value) {
  if (ptr>0) {
    ptr--;
    strcpy(value, Stack[ptr]);
  } else {
    strcpy(value, "##");
  }
}

/**********************************************************************/
/*                                                                    */
/*     Operator Stack                                                 */
/*                                                                    */
/**********************************************************************/
OpStack::OpStack() {
  reset();
}

OpStack::~OpStack() {
}

void OpStack::reset() {
  for (ptr=0; ptr<STACKDEPTH; ptr++) {
    Stack[ptr][0]='\0';
  }
  ptr=0;
}

int OpStack::Push(char *op) {
  int i,j;

  if (IsBinop(op)!=0) {
    Stack[ptr][0]=op[0];
    Stack[ptr][1]='\0';
  } else if (op[0]=='(') {
    strcpy(Stack[ptr],"(");
  } else if (IsMonop(op)!=0) {
    for (i=0;i<NUMMONOPS;i++) {
      j=strlen(MonOps[i]);
      if (strncmp(op,MonOps[i],j)==0) {
	strcpy(Stack[ptr],MonOps[i]);
      }
    }
  }

  ptr++;
  return (strlen(Stack[ptr-1]));
}

void OpStack::Pop(char *op) {
  if (ptr>0) {
    ptr--;
    strcpy(op,Stack[ptr]);
  } else {
    strcpy(op,"##");
  }
}

char *OpStack::Peek() {
  if (ptr>0) {
    return (Stack[ptr-1]);
  } else {
    return (Stack[0]);
  }
}

void OpStack::cPeek(char* op) {
  if (ptr>0) {
    strcpy(op,Stack[ptr-1]);
  } else {
    strcpy(op,"##");
  }
}

int OpStack::Ptr() {
  return(ptr);
}

/************************************************************************/
/*									*/
/*                           A Simple Number Stack 			*/
/*									*/
/************************************************************************/
NumStack::NumStack() {
  reset();
}

NumStack::~NumStack() {
}

// void NumStack::reset() {
//   ptr = 0;
// }

// void NumStack::Push(double n) {
//   stack[ptr] = n;
//   if (ptr<STACKDEPTH-1) ptr++;
// }

// double NumStack::Pop() {
//   if (ptr>0) {
//     ptr--;
//   }
//   return(stack[ptr]);
// }

/************************************************************************/
/*									*/
/*                           A Simple String Queue			*/
/*									*/
/************************************************************************/
GQ::GQ() {
  reset();
}

GQ::~GQ() {
}

void GQ::Queue(char *s) {
  if (s[0]!='\0') {
    strcpy(Q[EPtr],s);
    if (EPtr<STACKDEPTH-1) EPtr++;
  }
}

int GQ::ePtr() {
  return (EPtr);
}

bool GQ::error() {
  int i;
  for (i=0; i<EPtr; i++) {
    if (Q[i][0]=='#') return (true);
  }

  return (false);
}

/************************************************************************/
/*									*/
/*                           Inqiry Subroutines 			*/
/*									*/
/************************************************************************/
// int IsNum(char *String) {
//   switch (String[0]) {
//   case 'V':
//   case 'C':
//   case 'x':
//   case '0':
//   case '1':
//   case '2':
//   case '3':
//   case '4':
//   case '5':
//   case '6':
//   case '7':
//   case '8':
//   case '9':
//   case '.': return(1);
//   default : return(0);
//   }
// }

// returns 0 if not a binary operation, or returns precedence
int IsBinop(char *String) {
  int i,j=0;

  for (i=0;i<5;i++)
    if (String[0]==BinOps[i])
      j=BinPrecidence[i];

  return j;
}

// returns 0 if not a monary operation, or returns precedence = 4
int IsMonop(char *String) {
  int i;

  for (i=0;i<NUMMONOPS;i++) {
    if (strncmp(String,MonOps[i],strlen(MonOps[i]))==0) return(4);
  }

  return(0);
}

int Precedence(char *String) {
  if (String[0]=='(')
    return 0;
  else if (IsBinop(String)!=0)
    return (IsBinop(String));
  else
    return (IsMonop(String));
}

