//
// This parser parses JSON files per definition at https://json.org
//
#include "jsonparser.h"

enum
{
	// pre-processor
	TV_TRUE = TV_USER,
	TV_FALSE,
	TV_NULL,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "true",	TV_TRUE },
	{ "false",	TV_FALSE},
	{ "null",	TV_NULL},

	{ NULL,		TV_DONE }
};

//
//
//
JSONParser::JSONParser() : BaseParser()
{
	m_lexer = std::make_unique<LexicalAnalzyer>(_tokenTable, this, &yylval);
	yydebug = true;
}

//
//
//
JSONParser::~JSONParser() 
{

}

//
// An object is an unordered set of name/value pairs. An object begins with { (left brace) and ends with } (right brace). Each name is followed by : (colon) 
// and the name/value pairs are separated by , (comma).
//
void JSONParser::DoObject()
{
	yylog("Found new object");

	match('{');
	
	// match key-value pairs
	while (lookahead == TV_STRING) 
	{
		yylog("Found new key: %s", yylval.sym->lexeme.c_str());

		match(TV_STRING);
		match(':');

		DoValue();

		if (lookahead == ',')
			match(',');
	}

	match('}');
}

//
// An array is an ordered collection of values. An array begins with [ (left bracket) and ends with ] (right bracket). Values are separated by , (comma).
//
void JSONParser::DoArray()
{
	yylog("Found new array");

	match('[');
	
	// match array elements
	// match key-value pairs
	while (lookahead != ']')
	{
		DoValue();

		if (lookahead == ',')
			match(',');
	}

	match(']');
}

//
// From JSON.org grammar
//
// A value can be a string in double quotes, or a number, or true or false or null, or an object or an array. These structures can be nested.
//
void JSONParser::DoValue()
{
	//Expecting 'STRING', 'NUMBER', 'NULL', 'TRUE', 'FALSE', '{', '['
	switch (lookahead)
	{
	case TV_STRING:
		yylog("'%s'", yylval.sym->lexeme.c_str());
		match(lookahead);
		break;
	
	case TV_INTVAL:
		yylog("%d", yylval.ival);

		match(lookahead);
		break;

	case TV_FLOATVAL:
		yylog("%f", yylval.fval);

		match(lookahead);
		break;
	
	case TV_NULL:
	case TV_TRUE:
	case TV_FALSE:
		yylog(m_lexer->GetLexemeFromToken(lookahead));
		match(lookahead);
		break;

	case '{':
		DoObject();
		break;

	case '[':
		DoArray();
		break;
	}
}

//
//
//
int JSONParser::DoToken(int token)
{
	DoValue();
	
	return 0;
}
