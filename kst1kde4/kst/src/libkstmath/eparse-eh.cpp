/***************************************************************************
                               eparse-eh.cpp
                             -------------------
    begin                : Nov. 24, 2004
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

#include <QString>
#include <QStringList>

#include "kst_export.h"

namespace Equation {
  KST_EXPORT QStringList errorStack;
}


extern "C" const char *EParseErrorUnknown;
extern "C" const char *EParseErrorEmpty;
extern "C" const char *EParseErrorEmptyArg;
extern "C" const char *EParseErrorTwoOperands;
extern "C" const char *EParseErrorEmptyParentheses;
extern "C" const char *EParseErrorMissingClosingParenthesis;
extern "C" const char *EParseErrorNoImplicitMultiply;
extern "C" const char *EParseErrorRequiresOperand;
extern "C" const char *EParseErrorToken;

const char *EParseErrorUnknown = QT_TR_NOOP("parse error");
const char *EParseErrorEmpty = QT_TR_NOOP("Equation is empty.");
const char *EParseErrorEmptyArg = QT_TR_NOOP("Function argument is empty.");
const char *EParseErrorTwoOperands = QT_TR_NOOP("Two operands are required.");
const char *EParseErrorEmptyParentheses = QT_TR_NOOP("Empty parentheses are forbidden except in function calls.");
const char *EParseErrorMissingClosingParenthesis = QT_TR_NOOP("Closing parenthesis is missing.");
const char *EParseErrorNoImplicitMultiply = QT_TR_NOOP("Term must be followed by an operator. Implicit multiplication is not supported.");
const char *EParseErrorRequiresOperand = QT_TR_NOOP("This operator requires an operand.");
const char *EParseErrorToken = QT_TR_NOOP("Unknown character '%1'.");

extern "C" void yyClearErrors() {
  Equation::errorStack.clear();
}


extern "C" int yyErrorCount() {
  return Equation::errorStack.count();
}


extern "C" void yyerror(const char *s) {
  Equation::errorStack << QObject::tr(s);
}


extern "C" void yyerrortoken(char c) {
  Equation::errorStack << QObject::tr(EParseErrorToken).arg(c);
}
