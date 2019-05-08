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

	{ nullptr,	TV_DONE }
};

//
//
//
JSONParser::JSONParser() : BaseParser()
{
	m_lexer = std::make_unique<LexicalAnalzyer>(_tokenTable, this, &yylval);
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
void JSONParser::DoObject(JSONValue &node)
{
	yylog("Found new object");

	match('{');
	
	// match key-value pairs
	while (lookahead == TV_STRING) 
	{
		yylog("Found new key: %s", yylval.sym->lexeme.c_str());

		auto result = node.o->key_values.insert( std::pair<std::string, std::unique_ptr<JSONValue>>(yylval.sym->lexeme, std::make_unique<JSONValue>()) );

		match(TV_STRING);
		match(':');

		DoValue(*(*(result.first)).second);

		if (lookahead == ',')
			match(',');
	}

	match('}');
}

//
// An array is an ordered collection of values. An array begins with [ (left bracket) and ends with ] (right bracket). Values are separated by , (comma).
//
void JSONParser::DoArray(JSONValue &node)
{
	yylog("Found new array");

	match('[');
	
	// match array elements
	// match key-value pairs
	while (lookahead != ']')
	{
		std::unique_ptr<JSONValue> val = std::make_unique<JSONValue>();

		DoValue(*val);

		node.a->elements.push_back(std::move(val));

		if (lookahead == ',')
			match(',');
	}

	match(']');
}

//
// From JSON.org grammar:
//
// A value can be a string in double quotes, or a number, or true or false or null, or an object or an array. These structures can be nested.
//	'STRING', 'NUMBER', 'NULL', 'TRUE', 'FALSE', '{', '['
//
void JSONParser::DoValue(JSONValue &node)
{
	switch (lookahead)
	{
	case TV_STRING:
		node.value_type = JSONValue::ValueType::String;
		node.s = yylval.sym->lexeme;

		yylog("'%s'", yylval.sym->lexeme.c_str());
		match(lookahead);
		break;
	
	case TV_INTVAL:
		node.value_type = JSONValue::ValueType::Number;
		node.n = (float)yylval.ival;

		yylog("%d", yylval.ival);

		match(lookahead);
		break;

	case TV_FLOATVAL:
		node.value_type = JSONValue::ValueType::Number;
		node.n = yylval.fval;

		yylog("%f", yylval.fval);

		match(lookahead);
		break;
	
	case TV_NULL:
		node.value_type = JSONValue::ValueType::Null;

		yylog(m_lexer->getLexemeFromToken(lookahead));
		match(lookahead);
		break;

	case TV_TRUE:
	case TV_FALSE:
		node.value_type = JSONValue::ValueType::Boolean;
		node.b = lookahead == TV_TRUE ? true : false;

		yylog(m_lexer->getLexemeFromToken(lookahead));
		match(lookahead);
		break;

	case '{':
		node.value_type = JSONValue::ValueType::Object;
		node.o = std::make_unique<JSONObject>();

		DoObject(node);
		break;

	case '[':
		node.value_type = JSONValue::ValueType::Array;
		node.a = std::make_unique<JSONArray>();

		DoArray(node);
		break;
	}
}

//
//
//
int JSONParser::yyparse()
{
	BaseParser::yyparse();

	JSONValue root;

	DoValue(root);

	if (yydebug)
		root.dumpAll();

	return 0;
}
