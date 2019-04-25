#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "symboltable.h"

//
class BaseParser
{
protected:
	std::unique<CLexer> m_lexer;

	// 
	YYSTYPE yylval;
	
	// next token in the parse stream
	int lookahead;
	
	// total error count
	int m_iErrorCount;

	// total warning count
	int m_iWarningCount;

	// was symbol table allocated by the parser
	bool m_bAllocatedSymbolTable;

	// 
	std::unique<SymbolTable> m_pSymbolTable;

public:
	BaseParser(std::unique<CLexer> theLexer);
	virtual ~BaseParser();

	CLexer *getLexer() const { return m_lexer.get(); }

	int GetErrorCount()		{ return m_iErrorCount; }
	int GetWarningCount()	{ return m_iWarningCount; }

	virtual bool Init(std::unique<SymbolTable> pSymbolTable = NULL);
	virtual int Parse(const char *filename);
	virtual int ParseData(char *textToParse, const char *fileName, LPVOID pUserData);
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

#endif	//__PARSER_H

