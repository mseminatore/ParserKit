#pragma once

#include "../../baseparser.h"
#include <set>

class BNFParser : public BaseParser
{
protected:
	enum class SymbolType
	{
		Terminal,
		CharTerminal,
		Nonterminal
	};

	struct Symbol {
		SymbolType type;
		std::string name;
	};

	std::string startSymbol;

	using SymbolList = std::vector<Symbol>;
	using Production = std::pair<std::string, SymbolList>;
	using Productions = std::vector<Production>;

	Productions productions;

	// the list of terminals
	using TokenSet = std::set<std::string>;
	TokenSet tokens, terminals, nonTerminals;

	std::map<std::string, std::map<std::string, SymbolList>> parseTable;

	//
	std::set<std::string> nullable;

	using TerminalSets = std::map<std::string, std::set<std::string>>;
	TerminalSets first, follow;

	void ComputeNullable();
	void ComputeFirst();
	void ComputeFollow();

	void GenerateTable();
	void OutputTokens();
	void OutputProductions();
	bool AreAllNullable(size_t start, size_t end, const SymbolList &symbols);

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};