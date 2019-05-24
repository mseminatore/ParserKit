#pragma once

#include "../../baseparser.h"

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

	using Production = std::map<std::string, std::vector<Symbol>>;
	using Productions = std::vector<Production>;

	// the list of terminals
	using TokenList = std::vector<std::string>;
	TokenList tokens;

	std::vector<int> nullable;

	void OutputTokens();

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};