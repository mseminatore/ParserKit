#ifndef __CPARSER_H
#define __CPARSER_H

#include "parser.h"

class CodeGen;

//
//
//
class CParser : public BaseParser
{
protected:
	unsigned m_iLabel;

	SymbolEntry *m_pCurrentFunction;
	CodeGen *m_pCodeGen;

	// parser support functions
	unsigned NewLabel() { return m_iLabel++; }

	// match end of statement
	void EOS()	{ match(';'); }

	bool IsIntegerType(SymbolType st);

	// classification methods
	bool isOrop(int c);
	bool isAndop(int c);
	bool isRelop(int c);
	bool isAddop(int c);
	bool isMulop(int c);
	bool isType(int c);

	// relational methods
	SymbolType Equivalent();
	SymbolType NotEquivalent();
	SymbolType Less();
	SymbolType Greater();
	SymbolType GreaterEq();
	SymbolType LessEq();
	SymbolType Relation();
	SymbolType NotFactor();
	SymbolType ShiftLeft(void);
	SymbolType ShiftRight(void);

	// logical methods
	SymbolType BoolExpr();
	SymbolType BoolTerm();

	void BoolOr();
	void BoolAnd();
	void BoolXor();

	// arithmetic methods
	SymbolType Factor();
	SymbolType NegFactor();
	SymbolType FirstFactor();
	SymbolType Multiply();
	SymbolType Divide();
	SymbolType Remainder();
	SymbolType MoreTerms(SymbolType typeLeft);
	SymbolType Term();
	SymbolType FirstTerm();
	SymbolType Add();
	SymbolType Subtract();
	SymbolType Expr();

	// statements
	void CompoundStatement(int breakLabel, int continueLabel);
	void Statement(int breakLabel, int continueLabel);
	void Assignment();
	bool AreTypesAssignmentCompatible(SymbolType typeLeft, SymbolType typeRight);
	void DoReturn(SymbolEntry *symFunction);

	// control statements
	void DoIf(int breakLabel, int continueLabel);
	void DoWhile();
	void DoSwitch();
	void DoFor();

	// types
	SymbolType Type();
	void DoEnumeration();
	void DoStruct();

	// pre-processor
	void DoInclude();
	void DoDefine();
	void DoHash();

	// decls
	StorageType Storage();
	SymbolEntry *OneDeclaration();
	TypeModifier Modifier();

	int DoDeclaration();

	//	void FunctionDeclaration(LocationType storage);
	int ArgList(SymbolEntry *sym);

	void DoFunctionCall();

//	void FunctionDefinitions();
	void FunctionDefinition();
	void DeclareFormalParameters();
	//	int LocalDeclarations();
	int GetBit(SymbolEntry *sym);

public:
	CParser();
	~CParser();

	int DoToken(int token);
};

#endif	//__CPARSER_H
