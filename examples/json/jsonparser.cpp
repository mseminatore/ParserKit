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
JSONParser::JSONParser() : BaseParser(std::make_unique<LexicalAnalzyer>(_tokenTable, this, &yylval))
{

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
	match('{');
	
	// match key-value pairs
	while (lookahead == TV_ID) 
	{
		match(TV_ID);
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
	match('[');
	
	// match array elements
	// match key-value pairs
	while (lookahead == TV_ID)
	{
		match(TV_ID);

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
		match(lookahead);
		break;
	
	case TV_INTVAL:
	case TV_FLOATVAL:
		match(lookahead);
		break;
	
	case TV_NULL:
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
