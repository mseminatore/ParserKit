#ifndef __SYMBOL_H
#define __SYMBOL_H

//
//
//
enum StorageType
{
	stoUndef,
	stoExtern,
	stoStatic,
	stoRegister,
	stoPort,
	stoAuto,
	stoTypedef,

	stoNumStorageTypes
};

//
//
//
enum TypeModifier
{
	tmUndef,

	tmConst,
	tmVolatile,

	tmNumTypeModifiers
};

//
//
//
enum SymbolType
{
	stUndef, 

	stVoid, 
	stEnum,				// enums are integers
	stStringLiteral,	// string literal values
	stFloat,
	stInteger, 
	stChar, 
	stFunction,
	stUnsigned,
	stSigned,
	stShort,
	stLong,
	stBool,				// language extension
	stStruct,
	stString,			// variable of type string
	stDefine,

	stNumSymbolTypes
};

//
// Define size of basic types
//
const short asSymbolTypeSize[] = 
{
	0,	// stUndef
	2,	// stVoid
	1,	// stEnum
	2,	// stStringLiteral
	4,	// stFloat
	2,	// stInteger
	1,	// stChar
	2,	// stFunction
	2,	// stUnsigned
	2,	// stSigned
	2,	// stShort
	4,	// stLong
	1,	// stBool
	1,	// stStruct
	1,	// stString
	2	// stDefine
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
	SymbolType		returnType;
	StorageType		storage;
	
	unsigned			pointerCount;	// represents the number of times variable can be dereferenced

	unsigned			address;		// for port and global variables, the memory address

	// list of types, used for function arg lists and structs
	std::vector<SymbolType> parameterTypes;

	unsigned			isConst:1;
	unsigned			isReferenced:1;
	unsigned			isInterrupt:1;
	unsigned			sawReturn:1;		// for function symbols, whether a return was encountered
	unsigned			isPointer:1;		// for variables, whether it is a ponter type
	unsigned			isBit:1;			// for variables, whether the variable is a bit
	
	// if this symbol represents a literal value 
	union
	{
		int		ival;		// integer value held by this entry
		float	fval;		// float value held by this entry
		char	char_val;	// char value held by this entry
		bool	bval;		// boolean value held by this entry
	};

	// if this symbol represents a bit within a byte, the bit number
	char bitnum;

	//
	//
	//
	SymbolEntry()
	{
		srcLine			= -1;
		ival			= 0;
		type			= stUndef;
		storage			= stoUndef;
		isConst			= 0;
		isReferenced	= 0;
		isInterrupt		= 0;
		sawReturn		= 0;
		isPointer		= 0;
		isBit			= 0;
		bitnum			= -1;
		pointerCount	= 0;
		address			= 0;
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

	unsigned m_uiAddress;

public:
	typedef SymbolStack::iterator stack_iterator;
	typedef SymbolMap::iterator map_iterator;

protected:
	map_iterator m_globalIter;

public:
	SymbolTable();
	virtual ~SymbolTable();

	bool Init(int maxDepth = 10);

	SymbolEntry *lookup(const char *lexeme);
	SymbolEntry *reverse_lookup(int ival);
	SymbolEntry *install(const char *lexeme, SymbolType type);
	
	SymbolEntry *GetFirstGlobal();
	SymbolEntry *GetNextGlobal();

	// iterators for accessing the symbol table stack
	stack_iterator begin_stack()	{ return m_symbolTable.begin(); }
	stack_iterator end_stack()		{ return m_symbolTable.end(); }

	unsigned GetSizeOfType(SymbolType st);
	char *GetTypeName(SymbolType st);

	void SetAddress(unsigned addr)	{ m_uiAddress = addr; }
	unsigned GetNextAddress(SymbolEntry *pSym);

	void Push();
	void Pop();

	void DumpContents();
	void DumpUnreferencedSymbolsAtCurrentLevel();
};

#endif	// __SYMBOL_H
