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
int JSONParser::DoToken(int token)
{

}
