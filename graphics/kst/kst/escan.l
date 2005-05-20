%option noyywrap
%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eparse.h"
%}

Number	(0|[1-9][0-9]*)([\.][0-9]+)?([eE][\+\-][0-9]+)?
Data	\[[^\[\]]+\]
Id	[A-Za-z]+

%%

{Number}	{
			yylval.number = atof(yytext);
			/*printf("Found a number %.15f\n", yylval.number);*/
			return T_NUMBER;
		}


{Id}		{
			yylval.data = strdup(yytext);
			/*printf("Found an ID [%s]\n", yylval.data);*/
			return T_IDENTIFIER;
		}

{Data}		{
			yylval.data = strdup(yytext);
			/*printf("Found a data spec [%s]\n", yylval.data);*/
			return T_DATA;
		}

"!"		return T_NOT;

"&"		return T_AND;

"|"		return T_OR;

"&&"		return T_LAND;

"||"		return T_LOR;

"<"		return T_LT;

"<="		return T_LE;

">"		return T_GT;

">="		return T_GE;

"="		return T_EQ;

"=="		return T_EQ;

"!="		return T_NE;

"+"		return T_ADD;

"-"		return T_SUBTRACT;

"*"		return T_MULTIPLY;

"/"		return T_DIVIDE;
 
"%"		return T_MOD;
 
"^"		return T_EXP;
 
"("		return T_OPENPAR;
 
")"		return T_CLOSEPAR;
 
","		return T_COMMA;
 
[ \t]		{}

.		{
			/*printf("Found char '%c'\n", yytext[0]);*/
			yylval.character = yytext[0];
			return T_INVALID;
		}


<<EOF>>		{
			/*printf("yyterminate\n");*/
			yyterminate();
		}

%%

