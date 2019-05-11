#pragma once

#include "../../baseparser.h"

class BNFParser : public BaseParser
{
protected:
	// 
	using RuleList = std::vector<int>;

	using Rules = std::map<std::string, RuleList>;

	Rules rules;

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};