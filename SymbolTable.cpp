#include <list>
#include <map>
#include "SymbolTable.h"

//
//
//
char *_typeStrings[] = 
{
	"undef", 
	"void", 
	"enum",
	"string literal",
	"float",
	"int", 
	"char", 
	"function", 
	"unsigned",
	"signed",
	"short",
	"long",
	"bool",
	"struct",
	"string",
	"define"
};

// ensure that the array matches the enumeration
//C_ASSERT(ARRAYSIZE(_typeStrings) == stNumSymbolTypes);

//
//
//
SymbolTable::SymbolTable()
{
	m_iCurrentDepth = 0;
	m_iMaxDepth		= 0;
	m_uiAddress		= 0;

	// add the first level to the table
	m_symbolTable.push_back(SymbolMap());
}

//
//
//
SymbolTable::~SymbolTable()
{
	while (m_iCurrentDepth)
	{
		Pop();
		m_iCurrentDepth--;
	}
}

//
//
//
bool SymbolTable::Init(int maxDepth /*= 10*/)
{
	m_iMaxDepth = maxDepth;
	return true;
}

//======================================================================
// Add another depth level to the symbol table
//======================================================================
void SymbolTable::Push()
{
	m_iCurrentDepth++;
	assert(m_iCurrentDepth < m_iMaxDepth);

	m_symbolTable.push_back(SymbolMap());
}

//======================================================================
// Pop a level off the symbol table stack
//======================================================================
void SymbolTable::Pop()
{
	m_symbolTable.pop_back();
	m_iCurrentDepth--;

	// ensure that we don't underflow the stack!
	assert(m_iCurrentDepth >= 0);
}

//
//
//
SymbolEntry *SymbolTable::GetFirstGlobal()
{
	stack_iterator iter = m_symbolTable.begin();
	m_globalIter = (*iter).begin();
	return &m_globalIter->second;
}

//
//
//
SymbolEntry *SymbolTable::GetNextGlobal()
{
	m_globalIter++;
	stack_iterator iter = begin_stack();

	if (m_globalIter == (*iter).end())
		return NULL;

	return &m_globalIter->second;
}

//
//
//
unsigned SymbolTable::GetSizeOfType(SymbolType st)
{
	return asSymbolTypeSize[st];
}

//
//
//
unsigned SymbolTable::GetNextAddress(SymbolEntry *pSym)
{
	unsigned addr = m_uiAddress;

	m_uiAddress += GetSizeOfType(pSym->type);
	return addr;
}

//
//
//
char *SymbolTable::GetTypeName(SymbolType st)
{
	assert(st < stNumSymbolTypes);
	return _typeStrings[st];
}

//
void SymbolTable::DumpContents()
{
	char szText[256];
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	// for each level of table
	for (; riter != m_symbolTable.rend(); riter++)
	{
		iter = (*riter).begin();
		for (; iter != (*riter).end(); iter++)
		{
			if (iter->second.type == stInteger)
				sprintf_s(szText, "%s\t(%u, 0x%08X)\n", iter->first.c_str(), iter->second.ival, iter->second.ival);
			else
				sprintf_s(szText, "%s\t%f\n", iter->first.c_str(), iter->second.fval);

			printf(szText);
		}
	}
}

//===============================================================
// Look for a symbol in the nested symbol stack
//===============================================================
SymbolEntry *SymbolTable::lookup(const char *lexeme)
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	for (; riter != m_symbolTable.rend(); riter++)
	{
		// if we are done with this level, search next highest level
		iter = (*riter).find(lexeme);
		if (iter == (*riter).end())
			continue;

		return &(iter->second);
	}

	// symbol was not found anywhere in the table
	return NULL;
}

//===============================================================
// Look for a symbol in the nested symbol stack
//===============================================================
SymbolEntry *SymbolTable::reverse_lookup(int ival)
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	for (; riter != m_symbolTable.rend(); riter++)
	{
		iter = (*riter).begin();
		for (; iter != (*riter).end(); iter++)
		{
			if (iter->second.ival == ival)
				return &(iter->second);
		}
	}

	// symbol was not found anywhere in the table
	return NULL;
}

//======================================================================
// Install given lexeme in the symbol table at the current level.
// Duplicates are not allowed.
//======================================================================
SymbolEntry *SymbolTable::install(const char *lexeme, SymbolType type)
{
	SymbolMap &currentMap = m_symbolTable.back();

	// see if already in table
	SymbolEntry se;
	se.type = type;
	se.lexeme = lexeme;
	std::pair<SymbolMap::iterator, bool> result = currentMap.insert(SymbolMap::value_type(lexeme, se));

	// if symbol already exist in the table at this level, validate it
	if (!result.second)
		assert(result.first->second.type == type);

	return &(result.first->second);
}

//
void SymbolTable::DumpUnreferencedSymbolsAtCurrentLevel()
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator current_level_riter = m_symbolTable.rbegin();
	SymbolEntry *pSymbol;

	// foreach symbol at the current top of stack
	iter = (*current_level_riter).begin();
	for (; iter != (*current_level_riter).end(); iter++)
	{
		pSymbol = &(iter->second);

		if (!pSymbol->isReferenced)
		{
			printf("%s(%d) : warning: %s '%s' not referenced.\n", 
				pSymbol->srcFile.c_str(),
				pSymbol->srcLine,
				_typeStrings[pSymbol->type],
				pSymbol->lexeme.c_str()
				);
		}
	}
}
