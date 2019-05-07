#ifndef __BASEPARSER_H
#define __BASEPARSER_H

#include <stdio.h>
#include <assert.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include "lexer.h"
#include "symboltable.h"

#define SMALL_BUFFER	512

//
class BaseParser
{
public:
	bool yydebug = false;

protected:
	// the lexical analyzer
	std::unique_ptr<LexicalAnalzyer> m_lexer;

	// the value filled in by the lexical analyzer
	YYSTYPE yylval;
	
	// next token in the parse stream
	int lookahead;
	
	// total error count
	unsigned m_errorCount;

	// total warning count
	unsigned m_warningCount;

	// our symbol table
	std::unique_ptr<SymbolTable> m_pSymbolTable;

public:
	BaseParser();
	virtual ~BaseParser();

	unsigned getErrorCount()		{ return m_errorCount; }
	unsigned getWarningCount()		{ return m_warningCount; }

	virtual int Parse(const char *filename);
	virtual int ParseData(char *textToParse, const char *fileName, void *pUserData);
	virtual int DoToken(int token);

	virtual void yyerror(const char *fmt, ...);
	virtual void OutputErrorMessage(const char *msg);

	virtual void yywarning(const char *fmt, ...);
	virtual void OutputWarningMessage(const char *msg);

	virtual void expected(int token);
	virtual void match(int token);

	virtual void yylog(const char *fmt, ...);

	// these methods delegate their work to the symbol table object
	SymbolEntry *installSymbol(char *lexeme, SymbolType st = stUndef)
	{
		return m_pSymbolTable->install(lexeme, st);
	}

	SymbolEntry *lookupSymbol(char *lexeme)
	{
		return m_pSymbolTable->lookup(lexeme);
	}

	// callback from the lexer when new identifier is encountered
	void addNewVar(SymbolEntry *sym) {}
};

#endif	//__BASEPARSER_H

