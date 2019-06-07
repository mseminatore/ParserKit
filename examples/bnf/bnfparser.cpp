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

enum {
	stNonTerminal = stUser,
	stTerminal
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
		std::string lhs;

		SymbolList rhs;

		// match the non-terminal name
		yylval.sym->type = stNonTerminal;
		lhs = yylval.sym->lexeme;

		// TODO - error if we see the same nonTerminal more than once!?
		auto result = nonTerminals.insert(lhs);
		if (!result.second)
			yyerror("Duplicate non-terminal (%s) found on LHS of rules!", lhs.c_str());

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
					yylval.sym->type = stTerminal;
				}
				else {
					symbol.type = SymbolType::Nonterminal;
					symbol.name = yylval.sym->lexeme;
					yylval.sym->type = stNonTerminal;
				}

				rhs.push_back(symbol);

				match(lookahead);
			}

			// add production to our list of productions
			Production prod(lhs, rhs);
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
			tokens.insert(yylval.sym->lexeme);
			match(TV_ID);
		}
	}
}

//
void BNFParser::OutputTokens()
{
	puts("enum Class Tokens {\nERROR = 256,");

	// output all the Terminals
	for (auto iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		printf("\t%s,\n", iter->c_str());
	}

	// output all the non-Terminals
	for (auto iter = nonTerminals.begin(); iter != nonTerminals.end(); iter++)
	{
		printf("\t%s,\n", iter->c_str());
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
//
//
void BNFParser::GenerateTable()
{
	// TODO - look for first/first conflicts
	
	// make sure all Nonterminals are on lhs of a production
	auto sym = m_pSymbolTable->getFirstGlobal();
	while (sym)
	{
		if (sym->type == stNonTerminal && nonTerminals.find(sym->lexeme) == nonTerminals.end())
		{
			yywarning(Position(sym), "Non-terminal (%s) missing from left-hand side rule", sym->lexeme.c_str());
		}

		sym = m_pSymbolTable->getNextGlobal();
	};

	ComputeNullable();

	ComputeFirst();
	ComputeFollow();

	for (auto iter = nullable.begin(); iter != nullable.end(); iter++)
	{
		printf("%s is nullable\n", (*iter).c_str());
	}

	for (auto iter = first.begin(); iter != first.end(); iter++)
	{
		printf("First %s: ", iter->first.c_str());
		for (auto rhs = iter->second.begin(); rhs != iter->second.end(); rhs++)
		{
			printf("%s ", (*rhs).c_str());
		}
		puts("");
	}

	for (auto iter = follow.begin(); iter != follow.end(); iter++)
	{
		printf("Follow %s: ", iter->first.c_str());
		for (auto rhs = iter->second.begin(); rhs != iter->second.end(); rhs++)
		{
			printf("%s ", (*rhs).c_str());
		}
		puts("");
	}

	// foreach production
	for (auto iter = productions.begin(); iter != productions.end(); iter++)
	{
		auto lhs = iter->first;
		auto rhs = iter->second;
		
		for (auto termIter = tokens.begin(); termIter != tokens.end(); termIter++)
		{
			auto Yi = rhs[0].name;
			auto T = first.find(Yi);

			if (T->first == *termIter)
			{
				printf("(%s, %s): %s -> ", lhs.c_str(), termIter->c_str(), lhs.c_str());
				for (auto i = rhs.begin(); i != rhs.end(); i++)
				{
					printf("%s ", i->name.c_str());
				}
				puts("");
			}
		}
	}
}

//
bool BNFParser::AreAllNullable(int start, int end, const SymbolList &symbols)
{
	bool allNullable = true;

	for (auto i = start; i < end; i++)
	{
		if (nullable.find(symbols[i].name) == nullable.end())
			allNullable = false;
	}

	if (allNullable)
		return true;
	else
		return false;
}

//
//for each production X->Y1Y2...Yk
//	for each i from 1 to k, each j from i + 1 to k
//		if Y1...Yi - 1 are all nullable(or if i = 1)
//			then FIRST[X] = FIRST[X] u FIRST[Yi]
//
void BNFParser::ComputeFirst()
{
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

		// foreach production
		auto prodIter = productions.begin();
		for (; prodIter != productions.end(); prodIter++)
		{
			Production prod = *prodIter;

			auto nullSoFar = true;

			// foreach symbol
			for (auto symbolIndex = 0; symbolIndex < prod.second.size(); symbolIndex++)
			{
				auto rhs = prod.second[symbolIndex];

				if (nullable.find(rhs.name) == nullable.end())
					nullSoFar = false;

				// insert first[Yi] into first[X]
				if (rhs.type == SymbolType::Terminal || 0 == symbolIndex || nullSoFar)
				{
					auto rhsSet = first[rhs.name];

					for (auto rhsIter = rhsSet.begin(); rhsIter != rhsSet.end(); rhsIter++)
					{
						auto result = first[prod.first].insert(*rhsIter);
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
//	for each i from 1 to k, each j from i + 1 to k
//		if Yi + 1...Yk are all nullable(or if i = k)
//			then FOLLOW[Yi] = FOLLOW[Yi] u FOLLOW[X]
//		if Yi + 1...Yj - 1 are all nullable(or if i + 1 = j)
//			then FOLLOW[Yi] = FOLLOW[Yi] u FIRST[Yj]
//
void BNFParser::ComputeFollow()
{
	auto done = true;

	do
	{
		done = true;

		// foreach production
		auto prodIter = productions.begin();
		for (; prodIter != productions.end(); prodIter++)
		{
			Production prod = *prodIter;
		
			// foreach symbol
			for (auto i = 0; i < prod.second.size(); i++)
			{
				auto rhs = prod.second;

				if (rhs[i].type != SymbolType::Terminal && (i == prod.second.size() - 1 || AreAllNullable(i + 1, rhs.size(), rhs)))
				{
					// insert follow[X] in follow[Yi]
					auto rhsSet = follow[prod.first];
					for (auto rhsIter = rhsSet.begin(); rhsIter != rhsSet.end(); rhsIter++)
					{
						auto result = follow[rhs[i].name].insert(*rhsIter);
						if (result.second)
							done = false;
					}
				}

				for (auto j = i + 1; j < prod.second.size(); j++)
				{
					if (i + 1 == j || AreAllNullable(i + 1, j - 1, rhs))
					{
						// insert first[Yj] in follow[Yi]
						auto rhsSet = first[prod.second[j].name];
						for (auto rhsIter = rhsSet.begin(); rhsIter != rhsSet.end(); rhsIter++)
						{
							auto result = follow[rhs[i].name].insert(*rhsIter);
							if (result.second)
								done = false;
						}
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
	OutputProductions();

	GenerateTable();

	return 0;
}
