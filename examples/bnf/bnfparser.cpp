//
// This parser parses BNF files
//

#define _CRT_SECURE_NO_WARNINGS

#include "bnfparser.h"
#include "bnflexer.h"

//
// Table of lexemes and tokens to be recognized by the lexer
//
static TokenTable _tokenTable[] =
{
	{ "token",	  TV_TOKEN },
	{ "start",	  TV_START },
	{ "left",	  TV_LEFT },
	{ "right",	  TV_RIGHT },
	{ "nonassoc", TV_NONASSOC },
	{ "%%",		  TV_PERCENTS},
	{ "%{",		  TV_PERCENT_LBRACE },
	{ "%}",		  TV_PERCENT_RBRACE },

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
	int actionIndex = 1;

	yylog("Identify non-terminals");
	yylog("----------------------");

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

			std::string action;

			// see if there is an Action
			if (lookahead == '{')
			{
				char str[DEFAULT_TEXT_BUF];
				m_lexer->copyUntilChar('}', '{', str);
				action = str;
				match('{');
				match('}');
			}

			// add production to our list of productions
			Production prod(lhs, rhs, action, actionIndex++);
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

		switch (lookahead)
		{
		case TV_TOKEN:
			match(TV_TOKEN);

			while (lookahead == TV_ID)
			{
				tokens.insert(yylval.sym->lexeme);

				// all tokens are terminals
				terminals.insert(yylval.sym->lexeme);

				match(TV_ID);
			}
			break;

		case TV_START:
			match(TV_START);
			startSymbol = yylval.sym->lexeme;
			match(TV_ID);
			break;

		case TV_LEFT:
		case TV_RIGHT:
		case TV_NONASSOC:
		{
			bool rightAssoc = (lookahead == TV_RIGHT);
			bool nonAssoc   = (lookahead == TV_NONASSOC);
			match(lookahead);

			while (lookahead == TV_ID || lookahead == TV_CHARVAL)
			{
				std::string opName;

				if (lookahead == TV_CHARVAL)
				{
					opName = std::string(1, yylval.char_val);
					terminals.insert(opName);
				}
				else
				{
					opName = yylval.sym->lexeme;
					tokens.insert(opName);
					terminals.insert(opName);
				}

				m_operatorDecls[opName] = OperatorDecl{m_nextPrecLevel, rightAssoc, nonAssoc};
				match(lookahead);
			}
			m_nextPrecLevel++;
			break;
		}
		}
	}
}

//
// Write out the symbol enumeration
//
void BNFParser::OutputSymbols()
{
	fputs("#include \"tableparser.h\"\n\n", yyhout);
//	fputs("enum class Tokens {\n\tERROR = 256,\n", yyhout);
	fputs("\n// Tokens\n", yyout);
	fputs("enum {\n\tERROR = 256,\n", yyout);

	// output all the Terminals
	fputs("\n\t// Terminal symbols\n", yyout);
	for (auto iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		fprintf(yyout, "\tTS_%s,\n", iter->c_str());
	}

	if (!isPrattMode())
	{
		// output all the non-Terminals (only needed for LL(1) table)
		fputs("\n\t// Non-Terminal symbols\n", yyout);
		for (auto iter = nonTerminals.begin(); iter != nonTerminals.end(); iter++)
		{
			fprintf(yyout, "\tNTS_%s,\n", iter->c_str());
		}
	}

	fputs("\tLAST_TOKEN\n", yyout);
	fputs("};\n\n", yyout);

	if (!isPrattMode())
	{
		// output all the action symbols (only needed for LL(1))
		fputs("// Action symbols\n", yyout);
		fputs("enum {\n", yyout);
		fputs("\tFIRST_ACTION = LAST_TOKEN,\n", yyout);

		for (auto i = 0; i < productions.size(); i++)
		{
			auto str = getRule(productions[i].lhs, productions[i].rhs);
			fprintf(yyout, "\tACTION_%d,\t//%s\n", productions[i].rhs.actionIndex, str.c_str());
		}

		fputs("};\n\n", yyout);
	}

	if (isPrattMode())
	{
		// Pratt parser class declaration
		fprintf(yyhout, "class %s : public PrattParser<YYSTYPE> {\n", outputFileName.c_str());
		fputs("protected:\n", yyhout);
		fputs("\tvoid initPrattTable() override;\n", yyhout);
		fputs("\tYYSTYPE nud(int token, YYSTYPE val) override;\n", yyhout);
		fputs("\tYYSTYPE led(int op, YYSTYPE left, YYSTYPE right) override;\n", yyhout);
		fputs("public:\n", yyhout);
		fprintf(yyhout, "\t%s(LexicalAnalyzer lex) : PrattParser<YYSTYPE>(lex, &yylval) {}\n", outputFileName.c_str());
		fputs("\tint yyparse() override;\n", yyhout);
		fputs("};\n", yyhout);
	}
	else
	{
		// LL(1) TableParser class declaration
		fprintf(yyhout, "class %s : public TableParser {\n", outputFileName.c_str());
		fputs("protected:\n", yyhout);
		fputs("\tstd::vector<std::pair<Symbols, YYSTYPE>> vs;\n\n", yyhout);
		fputs("\tvoid initTable() override;\n", yyhout);
		fputs("\tint yyrule(int rule) override;\n", yyhout);
		fputs("\tint yyaction(int action) override;\n\n", yyhout);
		fputs("\tvoid tokenMatch(int token) override\n\t{\n\t\tvs.push_back(std::make_pair(token, yylval));\n\t\tyylog(\"Pushed (%d, %f) onto the value stack: %zd\\n\", token, yylval, vs.size());\n\t}\n", yyhout);
		fputs("\tvoid pop(int count) { for (int i = 0; i < count; i++) vs.pop_back(); yylog(\"\\nPopping %d items from value stack.\\n\", count); }", yyhout);
		fprintf(yyhout, "\npublic:\n\t%s(LexicalAnalyzer lexer) : TableParser(lexer) {}\n", outputFileName.c_str());
		fputs("};\n", yyhout);
	}
}

//
void BNFParser::OutputProductions()
{
	yylog("\nProductions");
	yylog("-----------");

	auto iter = productions.begin();
	for (; iter != productions.end(); iter++)
	{
		Production prod = *iter;
		yylog("%s: ", prod.lhs.c_str());

		auto symbols = prod.rhs.symbols.begin();
		for (; symbols != prod.rhs.symbols.end(); symbols++)
		{
			yylog("%s ", symbols->name.c_str());
		}

		yylog(";");
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

	yylog("\nNullable non-terminals");
	yylog("----------------------");

	for (auto iter = nullable.begin(); iter != nullable.end(); iter++)
	{
		yylog("%s is nullable\n", (*iter).c_str());
	}

	yylog("\nFirst sets");
	yylog("----------");

	for (auto iter = first.begin(); iter != first.end(); iter++)
	{
		yylog("First %s: [", iter->first.c_str());
		for (auto rhs = iter->second.begin(); rhs != iter->second.end(); rhs++)
		{
			yylog("%s ", (*rhs).c_str());
		}
		yylog("]");
	}

	yylog("\nFollow sets");
	yylog("-----------");

	for (auto iter = follow.begin(); iter != follow.end(); iter++)
	{
		yylog("Follow %s: [", iter->first.c_str());
		for (auto rhs = iter->second.begin(); rhs != iter->second.end(); rhs++)
		{
			yylog("%s ", (*rhs).c_str());
		}
		yylog("]");
	}

	yylog("\nParse table rules");
	yylog("-----------------");

	// foreach production
	for (auto iter = productions.begin(); iter != productions.end(); iter++)
	{
		auto lhs = iter->lhs;
		auto rhs = iter->rhs.symbols;
		
		auto followSet = follow.find(lhs)->second;

		if (AreAllNullable(0, rhs.size(), rhs))
		{
			for (auto followy = followSet.begin(); followy != followSet.end(); followy++)
			{
				auto result = parseTable[lhs].insert(std::pair<std::string, RightHandSide>(*followy, RightHandSide(rhs, iter->rhs.action, iter->rhs.actionIndex)));
				if (!result.second)
				{
					auto sym = m_pSymbolTable->lookup(lhs.c_str());
					yywarning(Position(sym), "Conflict! Production for non-terminal '%s' is ambiguous.", lhs.c_str());
				}

				yylog("(%s, %s): %s -> ", lhs.c_str(), (*followy).c_str(), lhs.c_str());
				for (auto i = rhs.begin(); i != rhs.end(); i++)
				{
					yylog("%s ", i->name.c_str());
				}
				yylog("");
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
					auto result = parseTable[lhs].insert(std::pair<std::string, RightHandSide>(*firsty, RightHandSide(rhs, iter->rhs.action, iter->rhs.actionIndex)));
					if (!result.second)
					{
						auto sym = m_pSymbolTable->lookup(lhs.c_str());
						yywarning(Position(sym), "Conflict! Production for non-terminal '%s' is ambiguous.", lhs.c_str());
					}

					yylog("(%s, %s): %s -> ", lhs.c_str(), (*firsty).c_str(), lhs.c_str());
					for (auto rhsIter = rhs.begin(); rhsIter != rhs.end(); rhsIter++)
					{
						yylog("%s ", rhsIter->name.c_str());
					}
					yylog("");
				}
			}
		}
	}
}

//
bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;

	while (start_pos != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos = str.find(from);
	}
	return true;
}


//
void BNFParser::TemplateReplace(std::string &str, size_t symbolCount)
{
	char buf[256], buf2[256];

	sprintf(buf, "vs[vs.size() - %d].second", (int)symbolCount + 1);
	replace(str, "$$", buf);

	sprintf(buf, "vs[vs.size() - %d].second", (int)symbolCount + 2);
	replace(str, "$<", buf);

	for (auto i = 1; i <= symbolCount; i++)
	{
		sprintf(buf, "$%d", i);
		sprintf(buf2, "vs[vs.size() - %d].second", (int)(1 + symbolCount - i));
		replace(str, buf, buf2);
	}
}

//
std::string BNFParser::getRule(const std::string &lhs, RightHandSide &rhs)
{
	std::string str = lhs;

	str += " -> ";

	for (auto i = rhs.symbols.begin(); i != rhs.symbols.end(); i++)
	{
		char *prefix = "", *postfix = "";

		if (i->type == SymbolType::CharTerminal)
		{
			prefix = postfix = "'";
		}

		str += prefix;
		str += i->name;
		str += postfix;
		str += " ";
	}

	return str;
}

//
void BNFParser::OutputTable()
{
	fprintf(yyout, "#include \"%s.h\"\n\n", outputFileName.c_str());
	fprintf(yyout, "void %s::initTable() {\n", outputFileName.c_str());

	fputs("\t// End Of File marker is last thing we'll see!\n\tss.push(0);\n\n", yyout);

	// push the start symbol
	fprintf(yyout, "\t// push the start symbol\n\tss.push(NTS_%s);\n\n", startSymbol.c_str());

	fputs("\t// setup the LL(1) parse table\n", yyout);

	// write out the parser table rules
	auto index = 1;
	for (auto entry = parseTable.begin(); entry != parseTable.end(); entry++)
	{
		for (auto item = entry->second.begin(); item != entry->second.end(); item++)
		{
			char *prefix = "TS_", *postfix = "";
			auto sym = m_pSymbolTable->lookup(item->first.c_str());
			if (!sym)
			{
				prefix = postfix = "'";
			}

			if (item->first == "")
				fprintf(yyout, "\ttable[NTS_%s][0] = %d;\n", entry->first.c_str(), index++);
			else
				fprintf(yyout, "\ttable[NTS_%s][%s%s%s] = %d;\n", entry->first.c_str(), prefix, item->first.c_str(), postfix, index++);
		}
	}

	fputs("\n", yyout);

	// write out the parser table actions
	for (auto entry = parseTable.begin(); entry != parseTable.end(); entry++)
	{
		for (auto item = entry->second.begin(); item != entry->second.end(); item++)
		{
//			if (item->second.action != "")
				fprintf(yyout, "\tactions[NTS_%s] = ACTION_%d;\n", entry->first.c_str(), item->second.actionIndex);
		}
	}

	fputs("}\n\n", yyout);

	fprintf(yyout, "int %s::yyrule(int rule)\n{\n", outputFileName.c_str());
	fprintf(yyout, "\tswitch (rule)\n\t{\n");

	index = 1;
	for (auto nt = parseTable.begin(); nt != parseTable.end(); nt++)
	{
		auto entry = *nt;
		for (auto t = entry.second.begin(); t != entry.second.end(); t++)
		{
			auto str = getRule(entry.first, t->second);

			fprintf(yyout, "\t\t// %s\n", str.c_str());
			fprintf(yyout, "\t\tcase %d:\n", index++);
			fprintf(yyout, "\t\t\tyylog(\"%s\\n\");\n", str.c_str());
			fputs("\t\t\ttokenMatch(ss.top());\n\t\t\tss.pop();\n", yyout);

//			if (t->second.action != "")
			{
				fprintf(yyout, "\t\t\tss.push(ACTION_%d);\n", t->second.actionIndex);
			}

			for (auto i = t->second.symbols.rbegin(); i != t->second.symbols.rend(); i++)
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
	fputs("\treturn rule;\n}\n\n", yyout);

	// generate the actions handler
	fprintf(yyout, "int %s::yyaction(int action)\n{\n", outputFileName.c_str());
	fprintf(yyout, "\tswitch (action)\n\t{\n");

	std::set<int> actions;
	for (auto nt = parseTable.begin(); nt != parseTable.end(); nt++)
	{
		auto entry = *nt;
		for (auto t = entry.second.begin(); t != entry.second.end(); t++)
		{
			if (actions.insert(t->second.actionIndex).second == false)
				continue;

			auto str = getRule(entry.first, t->second);
			fprintf(yyout, "\t// %s\n", str.c_str());

			fprintf(yyout, "\tcase ACTION_%d:\n", t->second.actionIndex);
			fputs("\t\t{\n", yyout);
			fprintf(yyout, "\t\t\tyylog(\"Action: %s\\n\");\n", str.c_str());

			if (t->second.symbols.size())
			{
				// if there is an action then output that code
				if (t->second.action != "")
				{
					TemplateReplace(t->second.action, t->second.symbols.size());
					fprintf(yyout, "\t\t\t%s\n", t->second.action.c_str());
				}
				else {
					// default is to left-propagate the first symbols value
					fprintf(yyout, "\t\t\tvs[vs.size() - %zd].second = vs[vs.size() - %zd].second;\n", t->second.symbols.size() + 1, t->second.symbols.size());
				}

				fprintf(yyout, "\t\t\tpop(%zd);\n", t->second.symbols.size());
			}
			else
			{
				fputs("\t\t\t// do nothing!\n", yyout);
			}

			fputs("\t\t}\n\t\tbreak;\n\n", yyout);
		}
	}

	fputs("\tdefault:\n\t\treturn 0;\n\t\tbreak;\n", yyout);
	fputs("\t}\n", yyout);
	fputs("\treturn action;\n}\n\n", yyout);
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
			for (auto symbolIndex = 0; symbolIndex < prod.rhs.symbols.size(); symbolIndex++)
			{
				auto rhs = prod.rhs.symbols[symbolIndex];

				if (nullable.find(rhs.name) == nullable.end())
					nullSoFar = false;

				// insert first[Yi] into first[X]
				if (0 == symbolIndex || nullSoFar)
				{
					auto rhsSet = first[rhs.name];

					for (auto rhsIter = rhsSet.begin(); rhsIter != rhsSet.end(); rhsIter++)
					{
						auto result = first[prod.lhs].insert(*rhsIter);
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

	// initialize follow set with (start,EOF) and EOF for all nullables
	follow[startSymbol].insert("");
	for (auto nullIter = nullable.begin(); nullIter != nullable.end(); nullIter++)
	{
		follow[*nullIter].insert("");
	}

	do
	{
		done = true;

		// foreach production
		auto prodIter = productions.begin();
		for (; prodIter != productions.end(); prodIter++)
		{
			Production prod = *prodIter;
		
			// foreach symbol
			for (auto i = 0; i < prod.rhs.symbols.size(); i++)
			{
				auto rhs = prod.rhs.symbols;
				auto Yi = rhs[i].name;

				if (rhs[i].type == SymbolType::Nonterminal)
				{
					if ((i == prod.rhs.symbols.size() - 1 || AreAllNullable(i + 1, rhs.size(), rhs)))
					{
						// insert follow[X] in follow[Yi]
						auto X = prod.lhs;
						auto followSet = follow[X];
						for (auto rhsIter = followSet.begin(); rhsIter != followSet.end(); rhsIter++)
						{
							auto result = follow[Yi].insert(*rhsIter);
							if (result.second)
								done = false;
						}
					}

					for (auto j = i + 1; j < prod.rhs.symbols.size(); j++)
					{
						if (i + 1 == j || AreAllNullable(i + 1, j - 1, rhs))
						{
							// insert first[Yj] in follow[Yi]
							auto Yj = prod.rhs.symbols[j].name;
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

			auto symbols = prod.rhs.symbols.begin();
			if (symbols == prod.rhs.symbols.end())
			{
				auto result = nullable.insert(prod.lhs);
				if (result.second)
					done = false;
			}
			else
			{
				auto nullableCount = 0;
				for (; symbols != prod.rhs.symbols.end(); symbols++)
				{
					if (nullable.find(symbols->name) != nullable.end())
						nullableCount++;
				}

				if (nullableCount == prod.rhs.symbols.size())
				{
					auto result = nullable.insert(prod.lhs);
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
		// copy contents to output file
		m_lexer->copyUntilChar('%', 0, yyout);

		match(lookahead);

		match(TV_PERCENT_RBRACE);
	}

	DoTokens();
	
	match(TV_PERCENTS);

	DoRules();

	match(TV_PERCENTS);
	
	OutputProductions();

	if (!isPrattMode())
	{
		GenerateTable();
	}
	else
	{
		DetectExprNonterminals();
	}

	OutputSymbols();

	if (!isPrattMode())
	{
		OutputTable();
	}
	else
	{
		OutputPrattTable();
	}

	// copy tail of file to output
	fputs("\n", yyout);

	if (lookahead == TV_ID)
		fputs(yylval.sym->lexeme.c_str(), yyout);
	else
		fputc(lookahead, yyout);

	m_lexer->copyToEOF(yyout);

	return 0;
}

// ---------------------------------------------------------------------------
// Returns true when the production is an infix binary expression:
//   X → X op X   where op is declared in %left / %right / %nonassoc
// ---------------------------------------------------------------------------
bool BNFParser::isInfixProduction(const Production& prod) const
{
	const auto& rhs = prod.rhs.symbols;
	return rhs.size() == 3
		&& rhs[0].type == SymbolType::Nonterminal && rhs[0].name == prod.lhs
		&& rhs[2].type == SymbolType::Nonterminal && rhs[2].name == prod.lhs
		&& m_operatorDecls.count(rhs[1].name) > 0;
}

// Returns the C++ token expression for a symbol: 'x' for char terminals,
// TS_NAME for named terminals.
std::string BNFParser::symbolTokenName(const Symbol& sym) const
{
	if (sym.type == SymbolType::CharTerminal)
		return std::string("'") + sym.name + "'";
	return "TS_" + sym.name;
}

// Returns the C++ token expression for an operator name.
std::string BNFParser::opTokenName(const std::string& name) const
{
	if (name.length() == 1 && !isalnum((unsigned char)name[0]))
		return std::string("'") + name + "'";
	return "TS_" + name;
}

// ---------------------------------------------------------------------------
// Identifies which non-terminals are "expression" non-terminals — those that
// have at least one infix binary production (X → X op X).
// ---------------------------------------------------------------------------
void BNFParser::DetectExprNonterminals()
{
	for (const auto& prod : productions)
	{
		if (isInfixProduction(prod))
			m_exprNonterminals.insert(prod.lhs);
	}
}

// ---------------------------------------------------------------------------
// OutputPrattTable — generates the three methods that make up a Pratt parser:
//
//   initPrattTable()  — populate m_infixBP from the %left/%right declarations
//   nud(token, val)   — null denotation: prefix/atom cases
//   led(op, l, r)     — left denotation: infix binary cases
//   yyparse()         — entry point; drives parseExpr() and runs the start action
// ---------------------------------------------------------------------------
void BNFParser::OutputPrattTable()
{
	fprintf(yyout, "#include \"%s.h\"\n\n", outputFileName.c_str());

	// --- initPrattTable() ---
	fprintf(yyout, "void %s::initPrattTable() {\n", outputFileName.c_str());
	for (const auto& kv : m_operatorDecls)
	{
		const auto& opName = kv.first;
		const auto& decl   = kv.second;
		int lbp = decl.level;
		int rbp = decl.rightAssoc ? decl.level - 1 : decl.level;
		fprintf(yyout, "\tm_infixBP[%s] = {%d, %d};\n",
		        opTokenName(opName).c_str(), lbp, rbp);
	}
	fprintf(yyout, "}\n\n");

	// --- nud() — handles atoms and prefix expressions ---
	fprintf(yyout, "YYSTYPE %s::nud(int token, YYSTYPE val) {\n", outputFileName.c_str());
	fprintf(yyout, "\tswitch (token) {\n");

	for (const auto& prod : productions)
	{
		if (!m_exprNonterminals.count(prod.lhs) || isInfixProduction(prod))
			continue;

		const auto& rhs = prod.rhs.symbols;
		if (rhs.empty())
			continue;  // epsilon — not representable in nud

		const auto& first = rhs[0];
		fprintf(yyout, "\tcase %s:\n\t{\n", symbolTokenName(first).c_str());

		// Parse remaining symbols (indices 1 .. rhs.size()-1)
		for (size_t k = 1; k < rhs.size(); k++)
		{
			const auto& sym = rhs[k];
			if (sym.type == SymbolType::Nonterminal && m_exprNonterminals.count(sym.name))
			{
				fprintf(yyout, "\t\tYYSTYPE s%zu = parseExpr(0);\n", k + 1);
			}
			else
			{
				// structural terminal — match and discard
				fprintf(yyout, "\t\tmatchToken(%s);\n", symbolTokenName(sym).c_str());
			}
		}

		if (!prod.rhs.action.empty())
		{
			std::string action = prod.rhs.action;
			replace(action, "$$", "_result");
			replace(action, "$1", "val");
			for (size_t k = 2; k <= rhs.size(); k++)
			{
				char from[16], to[16];
				snprintf(from, sizeof(from), "$%zu", k);
				snprintf(to,   sizeof(to),   "s%zu", k);
				replace(action, from, to);
			}
			fprintf(yyout, "\t\tYYSTYPE _result = YYSTYPE{};\n");
			fprintf(yyout, "\t\t%s\n", action.c_str());
			fprintf(yyout, "\t\treturn _result;\n");
		}
		else
		{
			fprintf(yyout, "\t\treturn val;\n");
		}

		fprintf(yyout, "\t}\n");
	}

	fprintf(yyout, "\tdefault:\n");
	fprintf(yyout, "\t\tyyerror(\"unexpected token in expression\");\n");
	fprintf(yyout, "\t\treturn YYSTYPE{};\n");
	fprintf(yyout, "\t}\n}\n\n");

	// --- led() — handles infix binary expressions ---
	fprintf(yyout, "YYSTYPE %s::led(int op, YYSTYPE left, YYSTYPE right) {\n", outputFileName.c_str());
	fprintf(yyout, "\tswitch (op) {\n");

	for (const auto& prod : productions)
	{
		if (!m_exprNonterminals.count(prod.lhs) || !isInfixProduction(prod))
			continue;

		const auto& op = prod.rhs.symbols[1];
		fprintf(yyout, "\tcase %s:\n\t{\n", symbolTokenName(op).c_str());

		if (!prod.rhs.action.empty())
		{
			std::string action = prod.rhs.action;
			replace(action, "$$", "_result");
			replace(action, "$1", "left");
			replace(action, "$3", "right");
			replace(action, "$2", "(YYSTYPE)op");
			fprintf(yyout, "\t\tYYSTYPE _result = YYSTYPE{};\n");
			fprintf(yyout, "\t\t%s\n", action.c_str());
			fprintf(yyout, "\t\treturn _result;\n");
		}
		else
		{
			fprintf(yyout, "\t\treturn left;\n");
		}

		fprintf(yyout, "\t}\n");
	}

	fprintf(yyout, "\tdefault:\n");
	fprintf(yyout, "\t\tyyerror(\"unknown operator\");\n");
	fprintf(yyout, "\t\treturn YYSTYPE{};\n");
	fprintf(yyout, "\t}\n}\n\n");

	// --- yyparse() — drives the parse from the start symbol ---
	fprintf(yyout, "int %s::yyparse() {\n", outputFileName.c_str());
	fprintf(yyout, "\tinitPrattTable();\n");
	fprintf(yyout, "\tadvance();\n");

	// Find the first production for the start symbol and generate its body
	for (const auto& prod : productions)
	{
		if (prod.lhs != startSymbol)
			continue;

		const auto& rhs = prod.rhs.symbols;

		for (size_t k = 0; k < rhs.size(); k++)
		{
			const auto& sym = rhs[k];
			if (sym.type == SymbolType::Nonterminal && m_exprNonterminals.count(sym.name))
			{
				fprintf(yyout, "\tYYSTYPE s%zu = parseExpr(0);\n", k + 1);
			}
			else if (sym.type == SymbolType::CharTerminal)
			{
				fprintf(yyout, "\tmatchToken('%s');\n", sym.name.c_str());
			}
			else
			{
				fprintf(yyout, "\tmatchToken(TS_%s);\n", sym.name.c_str());
			}
		}

		if (!prod.rhs.action.empty())
		{
			std::string action = prod.rhs.action;
			for (size_t k = 1; k <= rhs.size(); k++)
			{
				char from[16], to[16];
				snprintf(from, sizeof(from), "$%zu", k);
				snprintf(to,   sizeof(to),   "s%zu", k);
				replace(action, from, to);
			}
			replace(action, "$$", "_result");
			fprintf(yyout, "\t%s\n", action.c_str());
		}

		break;  // only the first start-symbol production
	}

	fprintf(yyout, "\treturn 0;\n}\n\n");
}
