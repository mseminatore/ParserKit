%{

#include <stdio.h>

%}

%token STRING NUM TRUE FALSE NULL
/* %start file */

%%

file: value
	;

object: '{' key_values '}'
	;

key_values:
	| value more_values
	;

more_values:
	| ',' key_values
	;

array: '[' values ']'
	;

value: STRING
	| NUM
	| object
	| array
	| TRUE
	| FALSE
	| NULL
	;

%%

//
int yylex()
{
	return 0;
}

//
int main()
{
	jsonparser parser();

	parser.
}