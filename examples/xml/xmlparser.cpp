//
// This parser parses JSON files per definition at https://json.org
//
#include "xmlparser.h"

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
XMLParser::XMLParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<LexicalAnalyzer>(_tokenTable, this, &yylval);
}

//
//
//
void XMLParser::DoMarkup()
{
	while (lookahead != TV_DONE)
	{
		// look for text
		if (lookahead == TV_ID)
		{
			match(TV_ID);
		}
		else if (lookahead == '<')
		{
			match('<');

			// if this is an end-tag we are done
			if (lookahead == '/')
			{
				match('/');
				return;
			}

			DoEntity();
		}

	}
}

//
//
//
void XMLParser::DoEntity()
{
	std::string entityName;

	entityName = yylval.sym->lexeme;

	match(TV_ID);

	// match zero or more attributes
	while (lookahead == TV_ID) 
	{
		match(TV_ID);
		match('=');
		match(TV_STRING);
	}

	// see if this is a self-closed tag
	if (lookahead == '/')
	{
		match(lookahead);
		match('>');
		return;
	}

	match('>');

	DoMarkup();

	if (entityName != yylval.sym->lexeme)
		yyerror("incorrect or missing end tag: %s", entityName);

	match(TV_ID);
	match('>');
}

//
//
//
int XMLParser::yyparse()
{
	BaseParser::yyparse();

	// match first start tag
	match('<');

	DoEntity();

	return 0;
}
