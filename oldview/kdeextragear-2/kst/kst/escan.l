%option noyywrap
%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eparse.h"
%}

Number	(0|[1-9][0-9]*)([\.][0-9]+)?([eE][\+\-][0-9]+)?
Data	\[[A-Za-z_\-0-9\']+\]
Id	[A-Za-z]+

%%

{Number}	{
			/*printf("Found a number\n");*/
			yylval.number = atof(yytext);
			return T_NUMBER;
		}


{Id}		{
			/*printf("Found an ID\n");*/
			yylval.data = strdup(yytext);
			return T_IDENTIFIER;
		}

{Data}		{
			/*printf("Found a data spec\n");*/
			yylval.data = strdup(yytext);
			return T_DATA;
		}

"!"		return T_NOT;

"&"		return T_AND;

"|"		return T_OR;

"<"		return T_LT;

"<="		return T_LE;

">"		return T_GT;

">="		return T_GE;

"="		return T_EQ;

"+"		return T_ADD;

"-"		return T_SUBTRACT;

"*"		return T_MULTIPLY;

"/"		return T_DIVIDE;
 
[ \t]		{}

.		{
			/*printf("Found char '%c'\n", yytext[0]);*/
			return yytext[0];
		}


<<EOF>>		yyterminate();

%%

