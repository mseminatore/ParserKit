//
// This parser parses a tiny statement language with variables, printing,
// and file inclusion:
//
//   x = 1 + 2 * 3;
//   print x;
//   include "lib.script";
//
#include "scriptparser.h"
#include <stdarg.h>

enum
{
	TV_PRINT = TV_USER,
	TV_INCLUDE,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "print",   TV_PRINT },
	{ "include", TV_INCLUDE },

	{ nullptr, TV_DONE }
};

//
// The stock LexicalAnalyzer::yylex() always folds a leading '+' or '-'
// into the number that follows it as a sign, so "2 + 3" would otherwise
// tokenize as [INTVAL(2), INTVAL(3)] with the '+' silently swallowed —
// there would be no way to see it as a binary operator. This override
// keeps '+'/'-' as their own single-character tokens; negative numeric
// literals are instead handled by the grammar's own unary-minus
// production (see ScriptParser::DoFactor()).
//
class ScriptLexer : public LexicalAnalyzer
{
public:
	ScriptLexer(TokenTable *atokenTable, BaseParser *pParser, YYSTYPE *pyylval)
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
ScriptParser::ScriptParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<ScriptLexer>(_tokenTable, this, &yylval);

	m_lexer->setCPPComments(true);   // // comments
	m_lexer->setHexNumbers(true);    // 0x... literals
}

//
// Overrides the library default (LexicalAnalyzer::yyerror, which prints and
// calls exit() on the very first error). Instead we report the error and
// then skip tokens up to the next ';' (or end of input) so parsing can
// continue — the script gets checked in one pass and every error is
// reported, not just the first.
//
void ScriptParser::yyerror(const char *fmt, ...)
{
	char buf[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
	vsnprintf(buf, sizeof(buf), fmt, argptr);
	va_end(argptr);

	fprintf(stderr, "%s(%d) : error near column %d: %s\n",
		m_lexer->getFile().c_str(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_errorCount++;

	// panic-mode recovery: resynchronize at the next statement boundary
	while (lookahead != ';' && lookahead != TV_DONE)
		match();

	if (lookahead == ';')
		match(';');
}

//
// factor: INTVAL | FLOATVAL | ID | '(' expr ')' | '-' factor
//
double ScriptParser::DoFactor()
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

	case TV_ID:
	{
		// the lexer auto-installs every identifier it sees (type stUndef)
		// so it always has a SymbolEntry to point yylval.sym at — a type
		// still stUndef here means the name was never actually assigned
		SymbolEntry *sym = yylval.sym;
		bool undefined = (sym->type == stUndef);

		if (undefined)
		{
			// yyerror()'s panic-mode recovery already consumes this
			// token (and everything up to the next ';'), so don't
			// also try to match() it below
			yyerror("undefined variable '%s'", sym->lexeme.c_str());
		}
		else
		{
			match(TV_ID);
		}

		val = undefined ? 0.0 : sym->fval;
		break;
	}

	case '(':
		match('(');
		val = DoExpr();
		match(')');
		break;

	case '-':
		match('-');
		val = -DoFactor();
		break;

	default:
		yyerror("expected a number, variable, or '('");
		break;
	}

	return val;
}

//
// term: factor (('*' | '/') factor)*
//
double ScriptParser::DoTerm()
{
	double left = DoFactor();

	while (lookahead == '*' || lookahead == '/')
	{
		int op = lookahead;
		match(op);
		double right = DoFactor();

		if (op == '*')
			left *= right;
		else if (right == 0.0)
			yyerror("division by zero");
		else
			left /= right;
	}

	return left;
}

//
// expr: term (('+' | '-') term)*
//
double ScriptParser::DoExpr()
{
	double left = DoTerm();

	while (lookahead == '+' || lookahead == '-')
	{
		int op = lookahead;
		match(op);
		double right = DoTerm();
		left = (op == '+') ? left + right : left - right;
	}

	return left;
}

//
// stmt: ID '=' expr ';'
//     | 'print' expr ';'
//     | 'include' STRING ';'
//
void ScriptParser::DoStmt()
{
	unsigned errsBefore = m_errorCount;

	switch (lookahead)
	{
	case TV_PRINT:
	{
		match(TV_PRINT);
		double val = DoExpr();

		// if DoExpr() already hit an error, its recovery has consumed the
		// terminating ';' for us — don't try to match it again
		if (m_errorCount == errsBefore)
		{
			printf("%g\n", val);
			match(';');
		}
		break;
	}

	case TV_INCLUDE:
	{
		match(TV_INCLUDE);
		std::string path = yylval.sym->lexeme;
		match(TV_STRING);

		if (m_errorCount == errsBefore)
		{
			if (m_lexer->pushFile(path.c_str()) != 0)
				yyerror("could not open include file '%s'", path.c_str());
			else
				match(';');
		}
		break;
	}

	case TV_ID:
	{
		std::string name = yylval.sym->lexeme;
		match(TV_ID);
		match('=');
		double val = DoExpr();

		if (m_errorCount == errsBefore)
		{
			SymbolEntry *sym = installSymbol(const_cast<char *>(name.c_str()));
			sym->type = stFloat;
			sym->fval = val;
			match(';');
		}
		break;
	}

	default:
		yyerror("expected a statement");
		break;
	}
}

//
// program: stmt*
//
int ScriptParser::yyparse()
{
	BaseParser::yyparse();

	while (lookahead != TV_DONE)
		DoStmt();

	return 0;
}
