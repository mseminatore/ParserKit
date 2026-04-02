%{
#define _CRT_SECURE_NO_WARNINGS
#define YYSTYPE double
YYSTYPE yylval;

#include <stdio.h>
#include <ctype.h>
%}

%token NUMBER

%left  '+' '-'
%left  '*' '/'

%%

calc: expr   { printf("Result is %f\n", $1); }
    ;

expr: NUMBER
    | '(' expr ')'  { $$ = $2; }
    | expr '+' expr { $$ = $1 + $3; }
    | expr '-' expr { $$ = $1 - $3; }
    | expr '*' expr { $$ = $1 * $3; }
    | expr '/' expr { $$ = $1 / $3; }
    ;

%%

int yylex()
{
    int c;

    yylval = 0.0;
    
    while ((c=getchar()) == ' ' || c == '\t' || c == '\n')
        ;

    if (c == EOF)
        return 0;

    if (c == '.' || isdigit(c)) 
    {
        ungetc(c, stdin);
        scanf("%lf", &yylval);
        return TS_NUMBER;
    }

    return c;
}

int main(int argc, char *argv[])
{
	calc parser(yylex);

    // set to true to see parsing details
//	parser.setDebug(true);
	parser.yyparse();

	printf("Successful parse.\n");
	
	return 0;
}