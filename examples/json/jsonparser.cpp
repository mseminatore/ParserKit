#include "jsonparser.h"

enum
{
	// pre-processor
	//TV_INCLUDE = TV_USER,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
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
//
//
void JSONParser::DoObject()
{
	match('{');
	// match key-value pairs
	match('}');
}

//
//
//
void JSONParser::DoArray()
{
	match('[');
	// match array elements
	match(']');
}
//
//
//
int JSONParser::DoToken(int token)
{
	while (lookahead != TV_DONE)
	{
		switch (lookahead)
		{
		case '{':
			DoObject();
			break;

		case '[':
			DoArray();
			break;
		}
	}
	
	return 0;
}
