#pragma once

#include <map>
#include <stack>
#include <vector>
#include <string>
#include <cstdio>

#define YYLOG stdout
#define YYBUFSIZE 2048

class TableParser
{
protected:
	using Symbols = int;
	using Rule = int;
	using LexicalAnalyzer = int(*)();

	LexicalAnalyzer yylex;

	// parsing table
	std::map< Symbols, std::map<Symbols, Rule> > table;
	
	std::map<Symbols, Rule> actions;

	// symbol stack
	std::stack<Symbols> ss;

	bool yydebug = false;

public:
	TableParser(LexicalAnalyzer lexer) { yylex = lexer; };
	~TableParser() = default;

	void setDebug(bool onoff) {
		yydebug = onoff;
	}

	virtual int yyparse();
	
	virtual void yyerror(const std::string &str);
	virtual void yywarning(const std::string &str);
	virtual void yylog(const char *fmt, ...);

	virtual void initTable() = 0;
	virtual int yyrule(int rule) = 0;
	virtual int yyaction(int action) = 0;

	virtual void tokenMatch(int token) = 0;
};

// ---------------------------------------------------------------------------
// PrattParser — Top-Down Operator Precedence (Pratt) parser base class.
//
// Generated subclasses override:
//   initPrattTable() — populate m_infixBP from %left/%right/%nonassoc decls
//   nud(token, val)  — null denotation: handle prefix/atom expressions
//   led(op,l,r)      — left denotation: handle infix binary expressions
//
// Binding powers (stored in m_infixBP):
//   Left-assoc  %left  op at level L:  {lbp=L, rbp=L}
//   Right-assoc %right op at level L:  {lbp=L, rbp=L-1}
// ---------------------------------------------------------------------------
template<typename ValueType>
class PrattParser
{
protected:
	using LexicalAnalyzer = int(*)();

	LexicalAnalyzer m_yylex;
	ValueType*      m_yylval;      // pointer to user's global yylval
	int             m_lookahead = 0;
	bool            m_debug = false;

	struct BP { int lbp; int rbp; };
	std::map<int, BP> m_infixBP;   // token → binding powers

	void advance()
	{
		m_lookahead = m_yylex();
	}

	void matchToken(int expected)
	{
		if (m_lookahead != expected)
		{
			char msg[64];
			snprintf(msg, sizeof(msg), "expected token '%c' (%d), got '%c' (%d)",
			         expected, expected, m_lookahead, m_lookahead);
			yyerror(msg);
		}
		advance();
	}

	// Core Pratt expression parser.
	// Parses and returns the value of the tightest sub-expression whose
	// infix operators all have lbp > minBP.
	ValueType parseExpr(int minBP = 0)
	{
		ValueType lval = *m_yylval;
		int t = m_lookahead;
		advance();
		ValueType left = nud(t, lval);

		while (true)
		{
			auto it = m_infixBP.find(m_lookahead);
			if (it == m_infixBP.end() || it->second.lbp <= minBP)
				break;
			int op = m_lookahead;
			advance();
			ValueType right = parseExpr(m_infixBP[op].rbp);
			left = led(op, left, right);
		}
		return left;
	}

	virtual void initPrattTable() = 0;
	virtual ValueType nud(int token, ValueType val) = 0;
	virtual ValueType led(int op, ValueType left, ValueType right) = 0;

public:
	PrattParser(LexicalAnalyzer lex, ValueType* yylval)
		: m_yylex(lex), m_yylval(yylval) {}

	virtual ~PrattParser() = default;

	void setDebug(bool onoff) { m_debug = onoff; }

	virtual void yyerror(const std::string& msg)
	{
		fprintf(stderr, "error: %s\n", msg.c_str());
	}

	virtual int yyparse()
	{
		initPrattTable();
		advance();
		parseExpr(0);
		return 0;
	}
};
