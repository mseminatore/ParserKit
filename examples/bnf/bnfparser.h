#pragma once

#include "../../baseparser.h"

class BNFParser : public BaseParser
{
protected:
	// 
	using RuleList = std::vector<int>;
	using Rules = std::map<std::string, RuleList>;
	Rules rules;

	// the list of tokens
	using TokenList = std::vector<std::string>;
	TokenList tokens;
	
	void OutputTokens();

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};