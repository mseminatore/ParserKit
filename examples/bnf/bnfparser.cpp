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
	puts("enum Class Tokens {\nERROR = 256,");

	auto iter = tokens.begin();
	for (; iter != tokens.end(); iter++)
	{
		printf("%s,\n", iter->c_str());
	}

	puts("};");
}

//
void BNFParser::OutputProductions()
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
//for each production X->Y1Y2...Yk
//	for each i from 1 to k, each j from i + 1 to k
//		if Y1...Yi - 1 are all nullable(or if i = 1)
//			then FIRST[X] = FIRST[X] u FIRST[Yi]
//		if Yi + 1...Yk are all nullable(or if i = k)
//			then FOLLOW[Yi] = FOLLOW[Yi] u FOLLOW[X]
//		if Yi + 1...Yj - 1 are all nullable(or if i + 1 = j)
//			then FOLLOW[Yi] = FOLLOW[Yi] u FIRST[Yj]
//
void BNFParser::GenerateTable()
{
	ComputeNullable();

	// for all Terminals, First[T] = {T}
	auto tokenIter = tokens.begin();
	for (; tokenIter != tokens.end(); tokenIter++)
	{
		first[*tokenIter].insert(*tokenIter);
	}

	auto done = true;

	do
	{
		done = true;

		auto prodIter = productions.begin();
		for (; prodIter != productions.end(); prodIter++)
		{
			Production prod = *prodIter;

			auto nullSoFar = true;
			for (auto i = 0; i < prod.second.size(); i++)
			{
				auto rhs = prod.second[i];

				if (nullable.find(rhs.name) == nullable.end())
					nullSoFar = false;

				// insert first[Yi] into first[X]
				if (rhs.type == SymbolType::Terminal || 0 == i || nullSoFar)
				{
					auto rhsSet = first[rhs.name];

					for (auto i = rhsSet.begin(); i != rhsSet.end(); i++)
					{
						auto result = first[prod.first].insert(*i);
						if (result.second)
							done = false;
					}
				}
			}
		}
	} while (!done);
}

//
//for each production X->Y1Y2...Yk
//	if Y1...Yk are all nullable(or if k = 0)
//		then nullable[X] = true
//
void BNFParser::ComputeNullable()
{
	auto done = true;

	do
	{
		done = true;

		auto prodIter = productions.begin();
		for (; prodIter != productions.end(); prodIter++)
		{
			Production prod = *prodIter;

			auto symbols = prod.second.begin();
			if (symbols == prod.second.end())
			{
				auto result = nullable.insert(prod.first);
				if (result.second)
					done = false;
			}
			else
			{
				auto nullableCount = 0;
				for (; symbols != prod.second.end(); symbols++)
				{
					if (nullable.find(symbols->name) != nullable.end())
						nullableCount++;
				}

				if (nullableCount == prod.second.size())
				{
					auto result = nullable.insert(prod.first);
					if (result.second)
						done = false;
				}
			}
		}
	} while (!done);
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
