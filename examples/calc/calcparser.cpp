//
// A four-function calculator (+ - * / ^) with unary minus, built by
// implementing Pratt-style precedence climbing directly against
// BaseParser::lookahead/match() — no code generation involved.
//
#include "calcparser.h"
#include <math.h>

//
// calc has no keywords — only literals and single-char operators
//
TokenTable _tokenTable[] =
{
	{ nullptr, TV_DONE }
};

//
//
//
static bool isBinOp(int op)
{
	return op == '+' || op == '-' || op == '*' || op == '/' || op == '^';
}

//
// The stock LexicalAnalyzer::yylex() always folds a leading '+' or '-'
// into the number that follows it as a sign, so "2 + 3" would otherwise
// tokenize as [INTVAL(2), INTVAL(3)] with the '+' silently swallowed —
// there would be no way to see it as a binary operator. This override
// keeps '+'/'-' as their own single-character tokens; negative numeric
// literals are instead handled by the grammar's own unary-minus
// production (see CalcParser::parsePrimary()).
//
class CalcLexer : public LexicalAnalyzer
{
public:
	CalcLexer(TokenTable *atokenTable, BaseParser *pParser, YYSTYPE *pyylval)
		: LexicalAnalyzer(atokenTable, pParser, pyylval) {}

	int yylex() override
	{
		int chr = skipLeadingWhiteSpace();

		if (chr == '+' || chr == '-')
			return chr;

		ungetChar(chr);
		return LexicalAnalyzer::yylex();
	}
};

//
//
//
CalcParser::CalcParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<CalcLexer>(_tokenTable, this, &yylval);

	m_lexer->setHexNumbers(true);   // 0x... literals
}

//
// binding power of each binary operator; higher binds tighter
//
int CalcParser::precedence(int op)
{
	switch (op)
	{
	case '+': case '-': return 1;
	case '*': case '/': return 2;
	case '^':            return 3;
	default:             return 0;
	}
}

//
//
//
bool CalcParser::isRightAssoc(int op)
{
	return op == '^';
}

//
//
//
double CalcParser::apply(int op, double left, double right)
{
	switch (op)
	{
	case '+': return left + right;
	case '-': return left - right;
	case '*': return left * right;
	case '/': return left / right;
	case '^': return pow(left, right);
	default:  return 0.0;
	}
}

//
// primary: INTVAL | FLOATVAL | '(' expr ')' | '-' primary
//
double CalcParser::parsePrimary()
{
	double val = 0.0;

	switch (lookahead)
	{
	case TV_INTVAL:
		val = yylval.ival;
		match(TV_INTVAL);
		break;

	case TV_FLOATVAL:
		val = yylval.fval;
		match(TV_FLOATVAL);
		break;

	case '(':
		match('(');
		val = parseExpr(0);
		match(')');
		break;

	case '-':
		match('-');
		val = -parsePrimary();
		break;

	default:
		yyerror("expected a number or '('");
		break;
	}

	return val;
}

//
// Precedence climbing: parses the tightest-binding chain of operators
// whose precedence is at least minPrec, recursing with a raised minPrec
// for left-associative operators (so they don't re-swallow their own
// level) or the same precedence for right-associative ones (so they do).
//
double CalcParser::parseExpr(int minPrec)
{
	double left = parsePrimary();

	while (isBinOp(lookahead) && precedence(lookahead) >= minPrec)
	{
		int op = lookahead;
		int nextMinPrec = isRightAssoc(op) ? precedence(op) : precedence(op) + 1;

		match(op);
		double right = parseExpr(nextMinPrec);
		left = apply(op, left, right);
	}

	return left;
}

//
// line: expr [';']
//
void CalcParser::parseLine()
{
	double val = parseExpr(0);

	if (lookahead == ';')
		match(';');

	printf("%g\n", val);
}

//
// program: line*
//
int CalcParser::yyparse()
{
	BaseParser::yyparse();

	while (lookahead != TV_DONE)
		parseLine();

	return 0;
}
