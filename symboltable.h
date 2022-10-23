#pragma once

#ifndef __SYMBOL_H
#define __SYMBOL_H

#define ARRAY_SIZE(p)	(size_t(sizeof(p) / sizeof(p[0])))

// Define symbol types
enum
{
	stUndef,			// undefined symbol

	stEnum,				// enums are integers
	stStringLiteral,	// string literal values
	stFloat,			// floating point value
	stInteger,			// integer value
	stChar,				// character value
	stDefine,			// preprocessor define
	stUser,				// user-defined symbol type

	stNumSymbolTypes
};

using SymbolType = int;

// Define symbol table entry
struct SymbolEntry
{
	// common
	std::string		lexeme;		// text of symbol
	SymbolType		type;		// type of the symbol
	int				srcLine;	// source file line number
	std::string		srcFile;	// source file name
	bool			global;		// is this a global var

	unsigned		isReferenced:1;	// was this symbol referenced
	
	// if this symbol represents a literal value 
	union
	{
		int		ival;		// integer value held by this entry
		float	fval;		// float value held by this entry
		char	char_val;	// char value held by this entry
		bool	bval;		// boolean value held by this entry
	};

	//
	//
	//
	SymbolEntry()
	{
		srcLine			= -1;
		ival			= 0;
		type			= stUndef;
		isReferenced	= 0;
		global			= false;
	}
};

// Represents a source file position
struct Position
{
	int				srcLine;
	std::string		srcFile;
	int				srcColumn;

	Position(std::string &file, int line, int col)
	{
		srcLine = line;
		srcFile = file;
		srcColumn = col;
	}

	Position(const SymbolEntry *sym)
	{
		srcLine = sym->srcLine;
		srcFile = sym->srcFile;
		srcColumn = 0;
	}
};

// Define symbol table class
class SymbolTable
{
protected:
	using SymbolMap = std::map<std::string, SymbolEntry>;
	using SymbolStack = std::list<SymbolMap>;

	SymbolStack m_symbolTable;
	SymbolEntry *m_pCurrentSymbol;

public:
	using stack_iterator = SymbolStack::iterator;
	using map_iterator = SymbolMap::iterator;

protected:
	map_iterator m_globalIter;

	const char *getTypeString(int type);

public:
	SymbolTable();
	virtual ~SymbolTable() = default;

	SymbolEntry *lookup(const char *lexeme);
	SymbolEntry *reverse_lookup(int ival);
	SymbolEntry *install(const char *lexeme, SymbolType type);
	
	SymbolEntry *getFirstGlobal();
	SymbolEntry *getNextGlobal();

	// iterators for accessing the symbol table stack
	stack_iterator begin_stack()	{ return m_symbolTable.begin(); }
	stack_iterator end_stack()		{ return m_symbolTable.end(); }

	const char *getTypeName(SymbolType st);

	void push();
	void pop();

	void dumpContents();
	void dumpUnreferencedSymbolsAtCurrentLevel();
};

#endif	// __SYMBOL_H
