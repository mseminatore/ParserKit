#pragma once

#include "../../baseparser.h"

// -------------------------------------------------------------------------
// CalcParser — a four-function calculator with unary minus and a
// right-associative '^' (power) operator.
//
// Unlike examples/bnf (which *generates* a PrattParser<T> from a .y
// grammar file), this parser implements precedence climbing by hand,
// directly against BaseParser's lookahead/match() primitives — showing
// what the generated code in examples/bnf/tableparser.h is automating.
// -------------------------------------------------------------------------
class CalcParser : public BaseParser
{
public:
	CalcParser();
	virtual ~CalcParser() = default;

	int yyparse() override;

protected:
	void parseLine();
	double parseExpr(int minPrec);
	double parsePrimary();

	static int precedence(int op);
	static bool isRightAssoc(int op);
	static double apply(int op, double left, double right);
};
