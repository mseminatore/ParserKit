//
// This parser parses BNF files
//
#include "bnfparser.h"

enum
{
	// pre-processor
	TV_TOKEN = TV_USER,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "token",	TV_TOKEN },

	{ nullptr,	TV_DONE }
};

//
//
//
BNFParser::BNFParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<LexicalAnalzyer>(_tokenTable, this, &yylval);
}

//
void BNFParser::DoRules()
{
	// match the non-terminal name
	yylog("Found non-terminal: %s", yylval.sym->lexeme.c_str());

	match(TV_ID);

	match(':');

	// match one or more rules
	do 
	{
		// a rule is zero or more symbols. Symbols are non-terminals and/or terminals (ie. tokens)
		while (lookahead == TV_ID || lookahead == TV_CHARVAL)
		{
			match(lookahead);
		}

		// TODO - add rule to our list of rules

	} while (lookahead == '|' && match('|'));

	match(';');
}

//
void BNFParser::DoTokens()
{
	// tokens are optional
	if (lookahead != '%')
		return;

	while (lookahead == '%')
	{
		match('%');
		match(TV_TOKEN);

		while (lookahead == TV_ID)
		{
			// TODO - set the type to stToken
			// TODO - assign an enumeration value to the token

			match(TV_ID);
		}
	}
}

//
//
//
int BNFParser::yyparse()
{
	BaseParser::yyparse();

	DoTokens();
	
	match('%');
	match('%');

	DoRules();

	match('%');
	match('%');

	return 0;
}
