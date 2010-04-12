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

// xxx #include <klocale.h>
#include "kst_export.h"

namespace Equation {
  KST_EXPORT QStringList errorStack;
}


static const char *EParseErrorUnknown = QT_TR_NOOP("parse error"); // from bison
extern "C" const char *EParseErrorEmpty = QT_TR_NOOP("Equation is empty.");
extern "C" const char *EParseErrorEmptyArg = QT_TR_NOOP("Function argument is empty.");
extern "C" const char *EParseErrorTwoOperands = QT_TR_NOOP("Two operands are required.");
extern "C" const char *EParseErrorEmptyParentheses = QT_TR_NOOP("Empty parentheses are forbidden except in function calls.");
extern "C" const char *EParseErrorMissingClosingParenthesis = QT_TR_NOOP("Closing parenthesis is missing.");
extern "C" const char *EParseErrorNoImplicitMultiply = QT_TR_NOOP("Term must be followed by an operator.  Implicit multiplication is not supported.");
extern "C" const char *EParseErrorRequiresOperand = QT_TR_NOOP("This operator requires an operand.");
extern "C" const char *EParseErrorToken = QT_TR_NOOP("Unknown character '%1'.");


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
