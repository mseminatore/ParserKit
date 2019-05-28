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
// Process all of the grammar Productions
//
void BNFParser::DoRules()
{
	// a production starts with a non-terminal LHS
	while (lookahead == TV_ID)
	{
		std::string nonTerminal;

		SymbolList rhs;

		// match the non-terminal name
		nonTerminal = yylval.sym->lexeme;
		yylog("Found non-terminal: %s", yylval.sym->lexeme.c_str());

		match(TV_ID);

		match(':');

		// match one or more rules
		do
		{
			rhs.clear();

			// a rule is zero or more symbols. Symbols are non-terminals and/or terminals (ie. tokens)
			while (lookahead == TV_ID || lookahead == TV_CHARVAL)
			{
				Symbol symbol;

				if (lookahead == TV_CHARVAL)
				{
					symbol.name = yylval.char_val;
					symbol.type = SymbolType::Terminal;
				}
				else if (tokens.find(yylval.sym->lexeme) != tokens.end())
				{
					symbol.type = SymbolType::Terminal;
					symbol.name = yylval.sym->lexeme;
				}
				else {
					symbol.type = SymbolType::Nonterminal;
					symbol.name = yylval.sym->lexeme;
				}

				rhs.push_back(symbol);

				match(lookahead);
			}

			// add production to our list of productions
			Production prod(nonTerminal, rhs);
			productions.push_back(prod);

		} while (lookahead == '|' && match('|'));

		match(';');
	}
}

//
// All of the Terminals must be pre-defined
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
			tokens.insert(yylval.sym->lexeme);

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
void BNFParser::GenerateTable()
{
	auto iter = productions.begin();
	for (; iter != productions.end(); iter++)
	{
		Production prod = *iter;
		printf("%s: ", prod.first.c_str());

		auto symbols = prod.second.begin();
		for (; symbols != prod.second.end(); symbols++)
		{
			printf("%s ", symbols->name.c_str());
		}

		puts(";");
	}
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
	GenerateTable();

	return 0;
}
