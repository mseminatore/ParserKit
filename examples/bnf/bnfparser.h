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

	struct RightHandSide
	{
		SymbolList symbols;
		std::string action;
		int actionIndex;

		RightHandSide(SymbolList _symbols, std::string _action, int _index)
		{
			symbols = _symbols;
			action = _action;
			actionIndex = _index;
		}
	};

	struct Production
	{
		std::string lhs;
		RightHandSide rhs;

		Production(std::string _lhs, SymbolList _symbols, std::string _action, int _index) : rhs(_symbols, _action, _index)
		{
			lhs = _lhs;
		}
	};

	using Productions = std::vector<Production>;
	Productions productions;

	// the list of terminals
	using TokenSet = std::set<std::string>;
	TokenSet tokens, terminals, nonTerminals;

	std::map<std::string, std::map<std::string, RightHandSide>> parseTable;

	//
	std::set<std::string> nullable;

	using TerminalSets = std::map<std::string, std::set<std::string>>;
	TerminalSets first, follow;

	void ComputeNullable();
	void ComputeFirst();
	void ComputeFollow();

	void GenerateTable();
	
	void OutputSymbols();
	void OutputProductions();
	void OutputTable();

	bool AreAllNullable(size_t start, size_t end, const SymbolList &symbols);

public:
	BNFParser();
	virtual ~BNFParser() = default;
	
	int yyparse() override;

	void DoTokens();
	void DoRules();
};