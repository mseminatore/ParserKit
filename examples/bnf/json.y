%{
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string>
#include <map>

unsigned yylineno = 1;
FILE *YYIN = stdin;

#define DEFAULT_NUM_BUF 256
#define DEFAULT_TEXT_BUF 2048

struct YYSTYPE
{
protected:
	int character;
	float number;
	std::string text;
	int token;

public:
	YYSTYPE()
	{
		character = token = 0;
		number = 0.0f;
	}

	YYSTYPE(const bool &rhs)
	{
		setNumber(rhs ? 1 : 0);
	}

	void empty()
	{
		number = 0;
		character = token = 0;
		text = "";
	}

	void setNumber(float num)
	{
		number = num;
		character = token = 0;
		text = "";
	}

	float asNumber() { return number; }	

	void setCharacter(int c)
	{
		character = c;
		token = 0;
		number = 0.0f;
		text = "";
	}

	void setToken(int c)
	{
		character = c;
		token = c;
		number = 0.0f;
		text = "";
	}

	int asChar() { return character; }

	void setText(const std::string &str)
	{
		text = str;
		character = token = 0;
		number = 0.0f;
	}

	std::string &asString() { return text; }

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

value: STRING { printf("%s\n", vs.back().asString().c_str()); }
	| NUM { printf("%3.2f\n", vs.back().asNumber()); }
	| object
	| array
	| TRUE { $$ = true; }
	| FALSE { $$ = false; }
	| NULL
	;

%%

//
using TokenTable = std::map<std::string, int>;

TokenTable keywords = {
	{ "true", TS_TRUE },
	{ "false", TS_FALSE },
	{ "null", TS_NULL }
};

//
void yyerror(const char *str)
{
	puts(str);
}

//
int backslash(int c)
{
	static char translation_tab[] = "b\bf\fn\nr\rt\t";

	if (c != '\\')
		return c;

	c = getc(YYIN);
	if (islower(c) && strchr(translation_tab, c))
		return strchr(translation_tab, c)[1];

	return c;
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
		*cptr++ = backslash(c);
		c = getc(YYIN);
	}

	// make sure its asciiz
	*cptr = '\0';

	yylval.setText(buf);
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

	yylval.setNumber((float)atof(buf));
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

	// make sure we clear out any previous data
	yylval.empty();

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

		auto iterTokens = keywords.find(_strlwr(buf));
		if (iterTokens != keywords.end())
		{
			yylval.setToken(iterTokens->second);
			return iterTokens->second;
		}

		yylval.setText(buf);
		return TS_STRING;
	}

	yylval.setCharacter(chr);
	return chr;
}

//
int main()
{
	jsonparser parser(yylex);

	parser.setDebug(true);
	parser.yyparse();
	
	return 0;
}