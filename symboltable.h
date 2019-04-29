#ifndef __SYMBOL_H
#define __SYMBOL_H

//
//
//
enum SymbolType
{
	stUndef,

	stEnum,				// enums are integers
	stStringLiteral,	// string literal values
	stFloat,
	stInteger, 
	stChar, 
	stDefine,

	stNumSymbolTypes
};

//
// Ensure that the array and enum stay in sync
//
//C_ASSERT(ARRAYSIZE(asSymbolTypeSize) == stNumSymbolTypes);

//
//
//
struct SymbolEntry
{
	// common
	std::string		lexeme;
	SymbolType		type;
	int				srcLine;
	std::string		srcFile;

	unsigned			isReferenced:1;
	
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
	}
};

//
//
//
class SymbolTable
{
protected:
	typedef std::map<std::string, SymbolEntry> SymbolMap;
	typedef std::list<SymbolMap> SymbolStack;

	SymbolStack m_symbolTable;
	SymbolEntry *m_pCurrentSymbol;

	int m_iCurrentDepth;
	int m_iMaxDepth;

public:
	typedef SymbolStack::iterator stack_iterator;
	typedef SymbolMap::iterator map_iterator;

protected:
	map_iterator m_globalIter;

public:
	SymbolTable(int maxDepth = 10);
	virtual ~SymbolTable();

	SymbolEntry *lookup(const char *lexeme);
	SymbolEntry *reverse_lookup(int ival);
	SymbolEntry *install(const char *lexeme, SymbolType type);
	
	SymbolEntry *GetFirstGlobal();
	SymbolEntry *GetNextGlobal();

	// iterators for accessing the symbol table stack
	stack_iterator begin_stack()	{ return m_symbolTable.begin(); }
	stack_iterator end_stack()		{ return m_symbolTable.end(); }

	const char *GetTypeName(SymbolType st);

	void Push();
	void Pop();

	void DumpContents();
	void DumpUnreferencedSymbolsAtCurrentLevel();
};

#endif	// __SYMBOL_H
