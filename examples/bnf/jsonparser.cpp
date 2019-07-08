

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

#include "jsonparser.h"

void jsonparser::initTable() {
	// End Of File marker is last thing we'll see!
	ss.push(EOF);

	// push the start symbol
	ss.push(NTS_value);

	// setup the LL(1) parse table
	table[NTS_array]['['] = 1;
	table[NTS_key_values][TS_STRING] = 2;
	table[NTS_key_values]['}'] = 3;
	table[NTS_more_key_values][','] = 4;
	table[NTS_more_key_values]['}'] = 5;
	table[NTS_more_values][','] = 6;
	table[NTS_more_values][']'] = 7;
	table[NTS_object]['{'] = 8;
	table[NTS_value][TS_FALSE] = 9;
	table[NTS_value][TS_NULL] = 10;
	table[NTS_value][TS_NUM] = 11;
	table[NTS_value][TS_STRING] = 12;
	table[NTS_value][TS_TRUE] = 13;
	table[NTS_value]['['] = 14;
	table[NTS_value]['{'] = 15;
	table[NTS_values][TS_FALSE] = 16;
	table[NTS_values][TS_NULL] = 17;
	table[NTS_values][TS_NUM] = 18;
	table[NTS_values][TS_STRING] = 19;
	table[NTS_values][TS_TRUE] = 20;
	table[NTS_values]['['] = 21;
	table[NTS_values][']'] = 22;
	table[NTS_values]['{'] = 23;
}

int jsonparser::yyrule(int rule)
{
	switch (rule)
	{
		// array -> '[' values ']' 
		case 1:
			ss.pop();
			ss.push(']');
			ss.push(NTS_values);
			ss.push('[');
			break;

		// key_values -> STRING ':' value more_key_values 
		case 2:
			ss.pop();
			ss.push(NTS_more_key_values);
			ss.push(NTS_value);
			ss.push(':');
			ss.push(TS_STRING);
			break;

		// key_values -> 
		case 3:
			ss.pop();
			break;

		// more_key_values -> ',' key_values 
		case 4:
			ss.pop();
			ss.push(NTS_key_values);
			ss.push(',');
			break;

		// more_key_values -> 
		case 5:
			ss.pop();
			break;

		// more_values -> ',' values 
		case 6:
			ss.pop();
			ss.push(NTS_values);
			ss.push(',');
			break;

		// more_values -> 
		case 7:
			ss.pop();
			break;

		// object -> '{' key_values '}' 
		case 8:
			ss.pop();
			ss.push('}');
			ss.push(NTS_key_values);
			ss.push('{');
			break;

		// value -> FALSE 
		case 9:
			ss.pop();
			ss.push(TS_FALSE);
			break;

		// value -> NULL 
		case 10:
			ss.pop();
			ss.push(TS_NULL);
			break;

		// value -> NUM 
		case 11:
			ss.pop();
			ss.push(TS_NUM);
			break;

		// value -> STRING 
		case 12:
			ss.pop();
			ss.push(TS_STRING);
			break;

		// value -> TRUE 
		case 13:
			ss.pop();
			ss.push(TS_TRUE);
			break;

		// value -> array 
		case 14:
			ss.pop();
			ss.push(NTS_array);
			break;

		// value -> object 
		case 15:
			ss.pop();
			ss.push(NTS_object);
			break;

		// values -> value more_values 
		case 16:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> value more_values 
		case 17:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> value more_values 
		case 18:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> value more_values 
		case 19:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> value more_values 
		case 20:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> value more_values 
		case 21:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		// values -> 
		case 22:
			ss.pop();
			break;

		// values -> value more_values 
		case 23:
			ss.pop();
			ss.push(NTS_more_values);
			ss.push(NTS_value);
			break;

		default:
			yyerror("parsing table defaulted");
			return 0;
			break;
	}
	return rule;
}

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

	yylval.text = cptr;
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