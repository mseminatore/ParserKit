#pragma once

#include "../../baseparser.h"

// -------------------------------------------------------------------------
// ScriptParser — a tiny statement language:
//
//   x = expr;
//   print expr;
//   include "path";
//
// Demonstrates:
//   • LexicalAnalyzer::pushFile() for nested #include-style file inclusion.
//     popFile() happens automatically at EOF — see
//     LexicalAnalyzer::specialTokens() in lexer.cpp.
//   • BaseParser::parseData() for parsing an in-memory script (see the
//     "-e" option in script.cpp).
//   • Overriding BaseParser::yyerror() to recover from an error instead of
//     exiting on the first one, so a whole script is checked in one pass.
// -------------------------------------------------------------------------
class ScriptParser : public BaseParser
{
public:
	ScriptParser();
	virtual ~ScriptParser() = default;

	using BaseParser::yyerror;

	int yyparse() override;
	void yyerror(const char *fmt, ...) override;

	void DoStmt();
	double DoExpr();
	double DoTerm();
	double DoFactor();
};
