#pragma once

#include "../../baseparser.h"
#include <set>

class BNFParser : public BaseParser
{
protected:
	enum class SymbolType
	{
		Terminal,
		Nonterminal
	};

	struct Symbol {
		SymbolType type;
		std::string name;
	};

	// 
	using RuleList = std::vector<int>;
	using Rules = std::map<std::string, RuleList>;
	Rules rules;

	using SymbolList = std::vector<Symbol>;
	using Production = std::pair<std::string, SymbolList>;
	using Productions = std::vector<Production>;

	Productions productions;

	// the list of terminals
	using TokenSet = std::set<std::string>;
	TokenSet tokens;

	//
	std::set<std::string> nullable;

	using TerminalSets = std::map<std::string, std::set<std::string>>;
	TerminalSets first;
	TerminalSets follow;

	void GenerateTable();
	void OutputTokens();

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};