//
// This parser parses BNF files
//
#include "bnfparser.h"
#include "bnflexer.h"

//
// Table of lexemes and tokens to be recognized by the lexer
//
static TokenTable _tokenTable[] =
{
	{ "token",	TV_TOKEN },
	{ "%%",		TV_PERCENTS},
	{ "%{",		TV_PERCENT_LBRACE },
	{ "%}",		TV_PERCENT_RBRACE },

	{ nullptr,	TV_DONE }
};

//
//
//
BNFParser::BNFParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<BNFLexer>(_tokenTable, this, &yylval);
}

//
void BNFParser::DoRules()
{
	while (lookahead == TV_ID)
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
			tokens.push_back(yylval.sym->lexeme);

			match(TV_ID);
		}
	}
}

//
void BNFParser::OutputTokens()
{
	puts("enum Class Tokens {");

	auto iter = tokens.begin();
	for (; iter != tokens.end(); iter++)
	{
		printf("%s,\n", iter->c_str());
	}

	puts("};");
}

//
//
//
int BNFParser::yyparse()
{
	BaseParser::yyparse();

	if (lookahead == TV_PERCENT_LBRACE)
	{
		match(lookahead);

		// TODO - copy contents to output file

		match(TV_PERCENT_RBRACE);
	}

	DoTokens();
	
	match(TV_PERCENTS);

	DoRules();

	match(TV_PERCENTS);
	
	OutputTokens();

	return 0;
}
