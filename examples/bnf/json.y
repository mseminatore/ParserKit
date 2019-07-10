%{

#include <stdio.h>
#include <ctype.h>
#include <string>

unsigned yylineno = 1;
FILE *YYIN = stdin;

#define DEFAULT_NUM_BUF 256
#define DEFAULT_TEXT_BUF 2048

struct YYSTYPE
{
	int character;
	float number;
	std::string text;
} yylval;

%}

%token STRING NUM TRUE FALSE NULL
%start value

%%

object: '{' key_values '}' { puts("found an object!"); }
	;

key_values:
	| STRING ':' value more_key_values
	;

more_key_values:
	| ',' key_values
	;

array: '[' values ']'
	;

values:
	| value more_values
	;

more_values:
	| ',' values
	;

value: STRING { printf("%s\n", lvalStack.top().text.c_str()); }
	| NUM
	| object
	| array
	| TRUE
	| FALSE
	| NULL
	;

%%

//
void yyerror(const char *str)
{
	puts(str);
}

//======================================================================
// characters which are considered to be whitespace
//======================================================================
bool iswhitespace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r') ? true : false;
}

//======================================================================
// 
//======================================================================
int getStringLiteral()
{
	int c;
	char buf[DEFAULT_TEXT_BUF];
	char *cptr = buf;

	c = getc(YYIN);

	while (c != '"' && cptr < &buf[sizeof(buf)])
	{
		if (c == '\n' || c == EOF)
			yyerror("missing quote");

		// build up our string, translating escape chars
		*cptr++ = c;
		c = getc(YYIN);
	}

	// make sure its asciiz
	*cptr = '\0';

	yylval.text = buf;
	return TS_STRING;
}

//
int getNumber()
{
	int c;
	char buf[DEFAULT_NUM_BUF];
	char *bufptr = buf;

	while (isdigit((c = getc(YYIN))) || c == '.')
		*bufptr++ = c;
	
	// need to put back the last character
	ungetc(c, YYIN);

	// make sure string is asciiz
	*bufptr = '\0';

	yylval.number = (float)atof(buf);

	return TS_NUM;
}

// skip any leading WS
int skipLeadingWhiteSpace()
{
	int chr;

	do
	{
		chr = getc(YYIN);
		if (chr == '\n')
		{
			yylineno++;
		}
	} while (iswhitespace(chr));

	return chr;
}

//
int yylex()
{
	int chr;
	char buf[DEFAULT_TEXT_BUF];
	char *pBuf = buf;

	// skip any leading WS
	chr = skipLeadingWhiteSpace();

	// look for a number value
	if (isdigit(chr))
	{
		ungetc(chr, YYIN);
		return getNumber();
	}

	// look for string literals
	if (chr == '"')
	{
		return getStringLiteral();
	}

	// look for keywords or ID
	if (isalpha(chr))
	{
		// get the token
		do 
		{
			*pBuf++ = ((char)chr);
		} while ((chr = getc(YYIN)) != EOF && (isalnum(chr) || chr == '_'));
		
		ungetc(chr, YYIN);
	
		// make sure its asciiz
		*pBuf = 0;

		if (!_stricmp(buf, "true"))
			return TS_TRUE;

		if (!_stricmp(buf, "false"))
			return TS_FALSE;

		if (!_stricmp(buf, "null"))
			return TS_NULL;

		yylval.text = buf;
		return TS_STRING;
	}

	yylval.character = chr;
	return chr;
}

//
int main()
{
	jsonparser parser(yylex);

	parser.yyparse();
	
	return 0;
}