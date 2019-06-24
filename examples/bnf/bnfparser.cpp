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
	{ "start",	TV_START },
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

		// set the start symbol if one was not defined previously
		if (startSymbol == "")
		{
			startSymbol = lhs;
		}

		// error if we see the same nonTerminal more than once!
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
					symbol.type = SymbolType::CharTerminal;

					// character values are terminal symbols
					terminals.insert(symbol.name);
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

		if (lookahead == TV_TOKEN)
		{
			match(TV_TOKEN);

			while (lookahead == TV_ID)
			{
				tokens.insert(yylval.sym->lexeme);

				// all tokens are terminals
				terminals.insert(yylval.sym->lexeme);

				match(TV_ID);
			}
		}
		else if (lookahead == TV_START)
		{
			match(TV_START);
			
			startSymbol = yylval.sym->lexeme;
			
			match(TV_ID);
		}
	}
}

//
void BNFParser::OutputTokens()
{
	fputs("#include \"tableparser.h\"\n\n", yyhout);
	fputs("enum {\n\tERROR = 256,\n", yyhout);
//	fputs("enum Class Tokens {\n\tERROR = 256,\n", yyhout);

	// output all the Terminals
	fputs("\t// Terminal symbols\n", yyhout);
	for (auto iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		fprintf(yyhout, "\tTS_%s,\n", iter->c_str());
	}

	// output all the non-Terminals
	fputs("\t// Non-Terminal symbols\n", yyhout);
	for (auto iter = nonTerminals.begin(); iter != nonTerminals.end(); iter++)
	{
		fprintf(yyhout, "\tNTS_%s,\n", iter->c_str());
	}

	fputs("};\n\n", yyhout);

	fprintf(yyhout, "class %s : public tableparser {\n", outputFileName.c_str());
	fputs("protected:\n", yyhout);
	fputs("\tvoid initTable() override;\n", yyhout);
	fputs("\tvoid yyrule(int rule) override;\n", yyhout);
	fputs("};\n", yyhout);
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
		
		auto followSet = follow.find(lhs)->second;

		if (AreAllNullable(0, rhs.size(), rhs))
		{
			for (auto followy = followSet.begin(); followy != followSet.end(); followy++)
			{
				auto result = parseTable[lhs].insert(std::pair<std::string, SymbolList>(*followy, rhs));
				if (!result.second)
				{
					auto sym = m_pSymbolTable->lookup(lhs.c_str());
					yywarning(Position(sym), "Conflict! Production for non-terminal '%s' is ambiguous.", lhs.c_str());
				}

				printf("(%s, %s): %s -> ", lhs.c_str(), (*followy).c_str(), lhs.c_str());
				for (auto i = rhs.begin(); i != rhs.end(); i++)
				{
					printf("%s ", i->name.c_str());
				}
				puts("");
			}
		}

		// (X, T) = production X -> Y for each T in FIRST[Yi]
		if (rhs.size())
		{
			auto firstSet = first.find(rhs[0].name)->second;
			for (auto firsty = firstSet.begin(); firsty != firstSet.end(); firsty++)
			{
				if (terminals.find(*firsty) != terminals.end())
				{
					auto result = parseTable[lhs].insert(std::pair<std::string, SymbolList>(*firsty, rhs));
					if (!result.second)
					{
						auto sym = m_pSymbolTable->lookup(lhs.c_str());
						yywarning(Position(sym), "Conflict! Production for non-terminal '%s' is ambiguous.", lhs.c_str());
					}

					printf("(%s, %s): %s -> ", lhs.c_str(), (*firsty).c_str(), lhs.c_str());
					for (auto rhsIter = rhs.begin(); rhsIter != rhs.end(); rhsIter++)
					{
						printf("%s ", rhsIter->name.c_str());
					}
					puts("");
				}
			}
		}
	}

	fprintf(yyout, "#include \"%s.h\"\n\n", outputFileName.c_str());
	fprintf(yyout, "void %s::initTable() {\n", outputFileName.c_str());
	
	fputs("\tss.push(0);\n", yyout);
	
	// push the start symbol
	fprintf(yyout, "\tss.push(NTS_%s);\n", startSymbol.c_str());

	// TODO - allow for a specified start symbol
	fputs("\tss.push(0);\n", yyout);

	// write out the parser table
	auto index = 1;
	for (auto nt = parseTable.begin(); nt != parseTable.end(); nt++)
	{
		auto entry = *nt;
		for (auto t = entry.second.begin(); t != entry.second.end(); t++)
		{
			char *prefix = "", *postfix = "";
			auto sym = m_pSymbolTable->lookup(t->first.c_str());
			if (!sym)
			{
				prefix = postfix = "'";
			}

			fprintf(yyout, "\ttable[NTS_%s][%s%s%s] = %d;\n", entry.first.c_str(), prefix, t->first.c_str(), postfix, index++);
		}
	}

	fputs("}\n\n", yyout);

	fprintf(yyout, "void %s::yyrule(int rule)\n{\n", outputFileName.c_str());
	fprintf(yyout, "\tswitch (rule)\n\t{\n");

	index = 1;
	for (auto nt = parseTable.begin(); nt != parseTable.end(); nt++)
	{
		auto entry = *nt;
		for (auto t = entry.second.begin(); t != entry.second.end(); t++)
		{
			fprintf(yyout, "\t\tcase %d:\n", index++);

			fputs("\t\t\tss.pop();\n", yyout);
			for (auto i = t->second.rbegin(); i != t->second.rend(); i++)
			{
				char *prefix = "TS_", *postfix = "";

				if (i->type == SymbolType::Nonterminal)
					prefix = "NTS_";

				if (i->type == SymbolType::CharTerminal)
				{
					postfix = prefix = "'";
				}

				fprintf(yyout, "\t\t\tss.push(%s%s%s);\n", prefix, i->name.c_str(), postfix);
			}
			fputs("\t\t\tbreak;\n\n", yyout);
		}
	}

	fputs("\t\tdefault:\n\t\t\tyyerror(\"parsing table defaulted\");\n\t\t\treturn 0;\n\t\t\tbreak;\n", yyout);
	fputs("\t}\n", yyout);
	fputs("}\n", yyout);
}

//
bool BNFParser::AreAllNullable(size_t start, size_t end, const SymbolList &symbols)
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
	auto terminalIter = terminals.begin();
	for (; terminalIter != terminals.end(); terminalIter++)
	{
		first[*terminalIter].insert(*terminalIter);
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
				if (0 == symbolIndex || nullSoFar)
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
				auto Yi = rhs[i].name;

				if (rhs[i].type == SymbolType::Nonterminal)
				{
					if ((i == prod.second.size() - 1 || AreAllNullable(i + 1, rhs.size(), rhs)))
					{
						// insert follow[X] in follow[Yi]
						auto X = prod.first;
						auto followSet = follow[X];
						for (auto rhsIter = followSet.begin(); rhsIter != followSet.end(); rhsIter++)
						{
							auto result = follow[Yi].insert(*rhsIter);
							if (result.second)
								done = false;
						}
					}

					for (auto j = i + 1; j < prod.second.size(); j++)
					{
						if (i + 1 == j || AreAllNullable(i + 1, j - 1, rhs))
						{
							// insert first[Yj] in follow[Yi]
							auto Yj = prod.second[j].name;
							auto firstSet = first[Yj];
							for (auto rhsIter = firstSet.begin(); rhsIter != firstSet.end(); rhsIter++)
							{
								auto result = follow[Yi].insert(*rhsIter);
								if (result.second)
									done = false;
							}
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
		// TODO - copy contents to output file
		m_lexer->copyUntilChar('%', 0, yyout);

		match(lookahead);

		match(TV_PERCENT_RBRACE);
	}

	DoTokens();
	
	match(TV_PERCENTS);

	DoRules();

	match(TV_PERCENTS);
	
	OutputTokens();
	OutputProductions();

	GenerateTable();

	// copy tail of file to output
	fputc(lookahead, yyout);
	m_lexer->copyToEOF(yyout);

	return 0;
}
