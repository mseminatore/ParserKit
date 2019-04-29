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

#define ARRAY_SIZE(p)	(size_t(sizeof(p) / sizeof(p[0])))
#define SMALL_BUFFER	512

//
class BaseParser
{
protected:
	// the lexical analyzer
	std::unique_ptr<LexicalAnalzyer> m_lexer;

	// the 
	YYSTYPE yylval;
	
	// next token in the parse stream
	int lookahead;
	
	// total error count
	int m_iErrorCount;

	// total warning count
	int m_iWarningCount;

	// was symbol table allocated by the parser
	bool m_bAllocatedSymbolTable;

	// our symbol table
	std::unique_ptr<SymbolTable> m_pSymbolTable;

public:
	BaseParser(std::unique_ptr<LexicalAnalzyer> theLexer, std::unique_ptr<SymbolTable> pSymbolTable = nullptr);
	virtual ~BaseParser();

	LexicalAnalzyer *getLexer() const { return m_lexer.get(); }

	int GetErrorCount()		{ return m_iErrorCount; }
	int GetWarningCount()	{ return m_iWarningCount; }

	virtual int Parse(const char *filename);
	virtual int ParseData(char *textToParse, const char *fileName, void *pUserData);
	virtual int DoToken(int token);

	virtual void yyerror(const char *fmt, ...);
	virtual void OutputErrorMessage(const char *msg);

	virtual void yywarning(const char *fmt, ...);
	virtual void OutputWarningMessage(const char *msg);

	virtual void expected(int token);
	virtual void match(int token);

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
	void AddNewVar(SymbolEntry *sym) {}
};

#endif	//__BASEPARSER_H

