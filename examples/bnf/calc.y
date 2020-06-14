%{
#define _CRT_SECURE_NO_WARNINGS
#define YYSTYPE double
YYSTYPE yylval;

#include <stdio.h>
#include <ctype.h>
%}

%token NUMBER

%%

expr: term expr_tail { $$ = $1; printf("Result is %4f\n", $$); }
    ;

expr_tail:
    | '+' term expr_tail { $< = $< + $2; }
    | '-' term expr_tail { $< = $< - $2; }
    ;

term: factor term_tail
    ;

term_tail:
    | '*' factor term_tail { $< = $< * $2; }
    | '/' factor term_tail { $< = $< / $2; }
    ;

factor: NUMBER
    |'(' expr ')' { $$ = $2; }
    ;

%%

int lineno = 1;

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

//    if (c == '\n')
//        lineno++;

    return c;
}

int main(int argc, char *argv[])
{
	calc parser(yylex);

	parser.setDebug(true);
	parser.yyparse();
	
	return 0;
}