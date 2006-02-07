
%{
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "enodefactory.h"

extern int yylex();
void yyerror(char *s);

void *ParsedEquation = 0L;

%}

%union {
        char *data;
        double number;
	void *n; /* tree node */
       }


%token T_NUMBER T_IDENTIFIER T_DATA

%left <operator> T_ADD T_SUBTRACT
%left <operator> T_MULTIPLY T_DIVIDE
%nonassoc U_SUBTRACT

%start START

%%

START		:	EQUATION
			{ $<n>$ = ParsedEquation = $<n>1; }
		;

EQUATION	:	EQUATION T_ADD TERM
			{ $<n>$ = NewAddition($<n>1, $<n>3); }
		|	EQUATION T_SUBTRACT TERM 
			{ $<n>$ = NewSubtraction($<n>1, $<n>3); }
		|	TERM
			{ $<n>$ = $<n>1; }
		;

TERM		:	TERM T_MULTIPLY EXPONENTORNOT
			{ $<n>$ = NewMultiplication($<n>1, $<n>3); }
		|	TERM T_DIVIDE EXPONENTORNOT
			{ $<n>$ = NewDivision($<n>1, $<n>3); }
		|	EXPONENTORNOT
			{ $<n>$ = $<n>1; }
		;

EXPONENTORNOT	:	ATOMIC '^' ATOMIC
			{ $<n>$ = NewPower($<n>1, $<n>3); }
		|	ATOMIC
			{ $<n>$ = $<n>1; }
		;

ATOMIC		:	T_SUBTRACT ATOMIC %prec U_SUBTRACT
			{ $<n>$ = NewSubtraction(NewNumber(0), $<n>2); /* optimize me? */}
		|	'(' EQUATION ')'
			{ $<n>$ = $<n>2; }
		|	T_IDENTIFIER
			{ $<n>$ = NewIdentifier($<data>1); }
		|	T_DATA
			{ $<n>$ = NewData($<data>1); }
		|	T_IDENTIFIER '(' ')'
			{ $<n>$ = NewFunction($<data>1, NewArgumentList()); }
		|	T_IDENTIFIER '(' ARGUMENTS ')'
			{ $<n>$ = NewFunction($<data>1, $<n>3); }
		|	T_NUMBER
			{ $<n>$ = NewNumber($<number>1); }
		;

ARGUMENTS	:	ARGLIST
			{ $<n>$ = $<n>1; }
		;

ARGLIST		:	ARGUMENTS ',' ARGUMENT
			{ AppendArgument($<n>1, $<n>3); $<n>$ = $<n>1; }
		|	ARGUMENT
			{ $<n>$ = NewArgumentList(); AppendArgument($<n>$, $<n>1); }
		|	ARGUMENTS ',' error
			{ $<n>$ = $<n>1; }
		;

ARGUMENT	:	EQUATION
			{ $<n>$ = $<n>1; }
		;

%%

void yyerror(char *s) {
  printf("ERROR: %s\n", s);
}

