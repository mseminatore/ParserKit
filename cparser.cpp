#include "stdafx.h"
#include "cparser.h"
#include "CodeGen.h"

//
//
//
enum 
{
	// pre-processor
	TV_INCLUDE = TV_USER,
	TV_DEFINE,
	TV_IFDEF,
	TV_IFNDEF,
	TV_ELSEDEF,
	TV_ENDIF,
	TV_PRAGMA,
	TV_ONCE,

	// keywords
	TV_ENUM,
	TV_RETURN,
	TV_STRUCT,
	TV_UNION,
	TV_TYPEDEF,
	TV_SWITCH,
	TV_CASE,
	TV_DEFAULT,
	TV_SIZEOF,
	TV_IF,
	TV_ELSE,
	TV_WHILE,
	TV_DO,
	TV_FOR,
	TV_INTEGER,
	TV_FLOAT,
	TV_SHORT,
	TV_LONG,
	TV_VOID,
	TV_CHAR,
	TV_SIGNED,
	TV_UNSIGNED,
	TV_VOLATILE,
	TV_AUTO,
	TV_REGISTER,
	TV_BREAK,
	TV_CONTINUE,
	TV_CONST,
	TV_STATIC,
	TV_EXTERN,

	// C language extensions
	TV_INTERRUPT,
	TV_BOOL,
	TV_TRUE,
	TV_FALSE,
	TV_PORT,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] = 
{
	{"include",	TV_INCLUDE},
	{"define",	TV_DEFINE},
	{"ifdef",	TV_IFDEF},
	{"ifndef",	TV_IFNDEF},
	{"else",	TV_ELSEDEF},
	{"endif",	TV_ENDIF},
	{"pragma",	TV_PRAGMA},
	{"once",	TV_ONCE},

	// keywords
	{"enum",	TV_ENUM},
	{"struct",	TV_STRUCT},
	{"union",	TV_UNION},
	{"typedef",	TV_TYPEDEF},
	{"if",		TV_IF},
	{"else",	TV_ELSE},
	{"while",	TV_WHILE},
	{"do",		TV_DO},
	{"for",		TV_FOR},
	{"switch",	TV_SWITCH},
	{"case",	TV_CASE},
	{"default", TV_DEFAULT},
	{"break",	TV_BREAK},
	{"continue",TV_CONTINUE},
	{"return",	TV_RETURN},
	{"sizeof",	TV_SIZEOF},

	{"int",		TV_INTEGER},
	{"float",	TV_FLOAT},
	{"void",	TV_VOID},
	{"short",	TV_SHORT},
	{"long",	TV_LONG},
	{"signed",	TV_SIGNED},
	{"unsigned", TV_UNSIGNED},
	{"char",	TV_CHAR},
	{"const",	TV_CONST},
	{"static",	TV_STATIC},
	{"extern",	TV_EXTERN},
	{"auto",	TV_AUTO},
	{"volatile", TV_VOLATILE},
	{"register", TV_REGISTER},

	// C language extensions
	{"true",	TV_TRUE},
	{"false",	TV_FALSE},
	{"bool",	TV_BOOL},
	{"interrupt", TV_INTERRUPT},
	{"port",	TV_PORT},

	{NULL,		TV_DONE}
};

//
//
//
CParser::CParser() : BaseParser(new CLexer(_tokenTable))
{
	m_iLabel = 1;
	m_pCurrentFunction = NULL;

	m_lexer->CaseSensitive();

	m_pCodeGen = new CodeGen();
}

//
//
//
CParser::~CParser()
{
	delete m_pCodeGen;
}


//
//
//
bool CParser::isOrop(int c)
{
	return (c == '|' || c == TV_OR);
}

//
//
//
bool CParser::isAndop(int c)
{
	return (c == '&' || c == TV_AND);
}

//
// recognize relational operators
//
bool CParser::isRelop(int c) 
{
	switch(c)
	{
	case '|':
	case TV_OR:
	case '&':
	case TV_AND:
	case '>':
	case '<':
	case TV_EQ:
	case TV_GE:
	case TV_LE:
	case TV_NE:
	case TV_NOT:
	case TV_SHR:
	case TV_SHL:
		return true;

	default:
		return false;
	}
}

//
// recognize addition operators
//
bool CParser::isAddop(int c)
{
	if(c == '+' || c == '-' || c == TV_SHL || c == TV_SHR)
		return true;

	return false;
}

//
// recognize multiplication operators
//
bool CParser::isMulop(int c)
{
	if(c == '*' || c == '/' || c == '%')
		return true;

	return false;
}

//
//
//
bool CParser::isType(int c)
{
	return 
	(
		c == TV_FLOAT ||
		c == TV_INTEGER ||
		c == TV_CHAR ||
		c == TV_BOOL ||
		c == TV_VOID ||
		c == TV_SIGNED ||
		c == TV_UNSIGNED ||
		c == TV_SHORT ||
		c == TV_LONG
	);
}

//======================================================================
// Parse #includes
//======================================================================
void CParser::DoInclude()
{
	match(TV_INCLUDE);
		m_lexer->SetFile(yylval.sym->lexeme.c_str());
	match(TV_STRING);
}

//
// Parse defines
//
void CParser::DoDefine()
{
	SymbolEntry *sym;

	match(TV_DEFINE);

	sym = yylval.sym;

	// var should be undefined

	if (sym->type != stUndef) 
	{
		yyerror("Duplicate symbol name");
	}

	sym->type	= stDefine;
	sym->ival	= 1;
	
	// defines are always referenced
	sym->isReferenced = 1;

	// match the ID part of: #define ID [ID or integer]
	match(TV_ID);

	// match the optional ID or integer part of: #define ID [ID or integer]
	if (lookahead == TV_ID)
	{
		sym->ival = yylval.sym->ival;
		match(lookahead);
	}
	else if (lookahead == TV_INTVAL)
	{
		sym->ival = yylval.ival;
		match(lookahead);
	}
}

//
// Parse enumerations
//
void CParser::DoEnumeration()
{
	int count = 0;
	SymbolEntry *sym;

	match(TV_ENUM);

	// optional enumeration name

	if (lookahead == TV_ID)
		match(TV_ID);

	match('{');

	while (lookahead == TV_ID) 
	{
		sym = yylval.sym;

		// symbol should be undefined
		if (sym->type != stUndef) 
		{
			yyerror("Duplicate symbol name");
			// note: put a break here to make above an error rather than a warning...
		}

		sym->type = stEnum;

		match(TV_ID);

		// handle intializers
		if (lookahead == '=') 
		{
			match(lookahead);
			count = yylval.ival;
			match(TV_INTVAL);
		}

		sym->ival = count++;

		// (optional) comma
		if (lookahead == ',')
			match(lookahead);
	}

	match('}');
	EOS();
}

//
// Parse hash
//
void CParser::DoHash()
{
	match('#');

	switch(lookahead)
	{
	case TV_INCLUDE:
		DoInclude();
		break;

	case TV_DEFINE:
		DoDefine();
		break;

	case TV_IF:
	case TV_IFNDEF:
	case TV_IFDEF:
		// match the #ifdef
		match(lookahead);

		// match the following ID or number
		match(lookahead);
		break;

	case TV_PRAGMA:
		ICE(1);	// TODO handle pragmas

		// match the #pragma
		match(lookahead);

		// match the next token
		match(lookahead);
		break;

	case TV_ELSEDEF:
		// match the #else
		match(lookahead);
		break;

	case TV_ENDIF:
		// match the #endif
		match(lookahead);
		break;

	default:
		yyerror("unexpected token found in DoHash()");
	}
}

//
// Parse type modifiers
//
TypeModifier CParser::Modifier()
{
	TypeModifier tm = tmUndef;

	switch(lookahead)
	{
	case TV_CONST:
		match(lookahead);
		tm = tmConst;
		break;

	case TV_VOLATILE:
		match(lookahead);
		tm = tmVolatile;
		break;
	}
	
	return tm;
}

//======================================================================
// Match and return the TYPE code
//======================================================================
SymbolType CParser::Type(void)
{
	SymbolType typecode = stUndef;

	switch(lookahead)
	{
	case TV_FLOAT:
		match(TV_FLOAT);
		typecode = stFloat;
		break;

	case TV_INTEGER:
		match(TV_INTEGER);
		typecode = stInteger;
		break;

	case TV_SHORT:
		match(TV_SHORT);
		typecode = stShort;
		break;

	case TV_LONG:
		match(TV_LONG);
		typecode = stLong;
		break;

	case TV_UNSIGNED:
		match(TV_UNSIGNED);
		typecode = stUnsigned;
		break;

	case TV_SIGNED:
		match(TV_SIGNED);
		typecode = stSigned;
		break;

	case TV_CHAR:
		match(TV_CHAR);
		typecode = stChar;
		break;

	case TV_BOOL:
		match(TV_BOOL);
		typecode = stBool;
		break;

	case TV_VOID:
		match(TV_VOID);
		typecode = stVoid;
		break;
	}

	return typecode;
}

//
// Parse C structs
//
void CParser::DoStruct()
{
	match(TV_STRUCT);
	SymbolEntry *sym = yylval.sym;
	sym->type = stStruct;

	match(TV_ID);

	match('{');
		// TODO - match member decls
	match('}');
	EOS();
}

//
// Parse storage declarations
//
StorageType CParser::Storage()
{
	StorageType type = stoUndef;

	switch(lookahead)
	{
	case TV_EXTERN:
		match(lookahead);
		type = stoExtern;
		break;

	case TV_STATIC:
		match(lookahead);
		type = stoStatic;
		break;

	case TV_PORT:
		match(lookahead);
		type = stoPort;
		break;

	case TV_REGISTER:
	case TV_AUTO:
	case TV_TYPEDEF:
		break;
	}

	return type;
}

//
//
//
void CParser::DoReturn(SymbolEntry *symFunction)
{
	SymbolType type;
	bool bReturnBoolExpr = false;

	match(TV_RETURN);

	symFunction->sawReturn = true;

	if (symFunction->returnType != stVoid && lookahead == ';')
	{
		yyerror("function must return a value");
		return;
	}

	// if there is a return value process it
	if (lookahead != ';')
	{
		if (symFunction->returnType == stVoid)
		{
			yyerror("function does not require a return value");
			return;
		}

		bReturnBoolExpr = true;
		type = BoolExpr();

		if (!AreTypesAssignmentCompatible(symFunction->returnType, type))
		{
			yyerror("incompatible return type");
			return;
		}
	}

	// return value should already be in W
	m_pCodeGen->CodeReturn();
}

//
// Return true if the compiler can convert from one type to another
//
bool CParser::AreTypesAssignmentCompatible(SymbolType typeLeft, SymbolType typeRight)
{
	if (typeLeft == typeRight)
		return true;

	switch (typeLeft)
	{
	case stFloat:
//	case stString:
//		if (typeRight == stString || typeRight == stLiteral)
//			break;
		// otherwise intentionally fall-through

	case stUnsigned:
	case stSigned:
	case stLong:
	case stInteger:
		switch(typeRight)
		{
		case stInteger:
		case stLong:
		case stChar:
		case stShort:
		case stSigned:
		case stUnsigned:
			return true;
		}
		break;

	case stShort:
		switch(typeRight)
		{
		case stLong:
		case stInteger:
		case stSigned:
		case stUnsigned:
			yywarning("assignment to smaller type, data may be lost");
		case stChar:
			return true;
		}
		break;

	case stChar:
		switch(typeRight)
		{
		case stShort:
		case stLong:
		case stInteger:
		case stSigned:
		case stUnsigned:
			yywarning("assignment to smaller type, data may be lost");
			return true;
		}
		break;
	}
	
	yyerror("type mismatch");
	return false;
}

//
// Parses: 
//	vardecl:	[extern | static] [const] type ID [= value];
//	function:	[extern | static] [interrupt] type ID(args*) { compound_statement }
//
int CParser::DoDeclaration()
{
	int isConst = 0;
	int isInterrupt = 0;

	StorageType storageType = Storage();

	// functions can't be const
	TypeModifier typeModifier = Modifier();
	if (typeModifier == tmConst)
	{
		isConst = 1;
	}
	
	// TODO - handle volatile here

	if (!isType(lookahead))
	{
		yyerror("unexpected token found");
		return 0;
	}

	SymbolType type = Type();

	// PORT types must be char
	if (storageType == stoPort && type != stChar)
	{
		yyerror("PORT declaration type must be CHAR");
		return 0;
	}

	// variables can't be interrupts
	if (lookahead == TV_INTERRUPT)
	{
		match(lookahead);
		isInterrupt = 1;
	}

	match(TV_ID);
	SymbolEntry *pSym = yylval.sym;
	assert(pSym);

	pSym->storage		= storageType;
	pSym->isConst		= isConst;
	pSym->isInterrupt	= isInterrupt;

	// at this point could still be a function or variable decl
	if (lookahead == '(')
	{
		// TODO - the compiler should handle this case
		assert(!isConst);

		pSym->type = stFunction;
		pSym->returnType = type;

		// functions are always referenced
		pSym->isReferenced = 1;
		pSym->srcLine = m_lexer->getLineNumber();
		pSym->srcFile = m_lexer->getFile();

		m_pCurrentFunction = pSym;

		FunctionDefinition();
	}
	else
	{
		// TODO - the compiler should handle this case
		assert(!isInterrupt);

		pSym->type = type;

		// handle PORT bit references
		if (lookahead == '.' && pSym->storage == stoPort)
		{
			match(lookahead);
			if (lookahead != TV_INTVAL)
			{
				yyerror("expected integer literal");
				return 0;
			}

			int bit = yylval.ival;
			match(TV_INTVAL);
	
		}

		// PORT types must have initializers
		if (storageType == stoPort && lookahead != '=')
		{
			yyerror("PORT types must have address initializer");
			return 0;
		}

		// handle optional initializer
		if (lookahead == '=')
		{
			int sign = 1;

			match(lookahead);

			if (lookahead == '-')
			{
				sign = -1;
				match(lookahead);
			}

			switch(type)
			{
			case stUnsigned:
				if (sign != 1)
					yywarning("assigning signed value to unsigned type");

			case stShort:
			case stSigned:
			case stLong:
			case stInteger:
				if (lookahead != TV_INTVAL)
					yyerror("expected integer");

				pSym->ival = sign * yylval.ival;
				break;

			case stFloat:
				assert(lookahead == TV_FLOATVAL);
				pSym->fval = sign * yylval.fval;
				break;

			case stChar:
				if (lookahead != TV_CHARVAL && lookahead != TV_INTVAL)
					yyerror("expected integer value type");

				if (storageType == stoPort)
				{
					assert(sign == 1);
					pSym->address = yylval.ival;
				}
				else
					pSym->char_val = sign * yylval.char_val;
				break;

			case stBool:
				pSym->bval = lookahead == TV_TRUE ? true : false;
				break;

			default:
				yyerror("unexpected r-value");
			}
			
			match(lookahead);
		}

		// TODO - handle multiple comma separated decls
		EOS();
	}

	return 1;
}

//======================================================================
//
//======================================================================
int CParser::ArgList(SymbolEntry *sym)
{
	SymbolType type = stUndef;
	int count = 1;

	// allow empty arg lists
	if (lookahead == ')')
		return 0;

	type = BoolExpr();
	if (!AreTypesAssignmentCompatible(sym->parameterTypes[count - 1], type))
	{
		yyerror("parameter %d invalid type", count);
		return count;
	}

	while (lookahead == ',')
	{
		count++;

		match(',');

		type = BoolExpr();
		if (!AreTypesAssignmentCompatible(sym->parameterTypes[count - 1], type))
		{
			yyerror("parameter %d invalid type", count);
			return count;
		}
	}

	return count;
}

//======================================================================
// Implement parsing of function call statement
//======================================================================
void CParser::DoFunctionCall()
{
	SymbolEntry *sym;
	int parm_count;

	sym = yylval.sym;
	sym->isReferenced = true;

	match(TV_ID);

	match('(');
	// TODO - gen code to push arguments and return number of parms passed
	parm_count = ArgList(sym);
	if (parm_count != sym->parameterTypes.size())
	{
		yyerror("invalid parameter count");
		return;
	}

	match(')');

	// generate code to push PC and JMP to function
	m_pCodeGen->FunctionCall(sym);

	// restore stack after call, C calling convention
//	m_pCodeGen->pop_parms(parm_count);
}

//======================================================================
// Begin expression grammar
//======================================================================

//
SymbolType CParser::Equivalent(void)
{
	match(TV_EQ);
	SymbolType type = Expr();
	m_pCodeGen->Equal(type);

	return type;
}

//
SymbolType CParser::NotEquivalent(void)
{
	match(TV_NE);
	SymbolType type = Expr();
	m_pCodeGen->NotEqual(type);
	
	return type;
}

//
SymbolType CParser::Less(void)
{
	match('<');

	SymbolType type = Expr();

	switch (type)
	{
	case stUnsigned:
	case stInteger:
	case stChar:
		m_pCodeGen->Less(type);
		break;

	case stShort:
	case stSigned:
	case stFloat:
	case stLong:
	default:
		yyerror("operator '<' unsupported on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Greater(void)
{
	match('>');

	SymbolType type = Expr();

	switch (type)
	{
	case stInteger:
	case stChar:
		m_pCodeGen->Greater(type);
		break;

	case stUnsigned:
	case stSigned:
	case stShort:
	case stLong:
	case stFloat:
	default:
		yyerror("operator '>' unsupported on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::ShiftLeft(void)
{
	match(TV_SHL);

	SymbolType type = Expr();

	switch (type)
	{
	case stInteger:
	case stChar:
		m_pCodeGen->ShiftLeft(type);
		break;

	case stShort:
	case stUnsigned:
	case stSigned:
	case stLong:
	default:
		yyerror("operator '<<' unsupported on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::ShiftRight(void)
{
	match(TV_SHR);

	SymbolType type = Expr();

	switch (type)
	{
	case stInteger:
	case stChar:
		m_pCodeGen->ShiftRight(type);
		break;

	case stShort:
	case stUnsigned:
	case stSigned:
	case stLong:
	default:
		yyerror("operator '>>' unsupported on type");
		return type;
	}

	return type;
}
//
SymbolType CParser::GreaterEq(void)
{
	match(TV_GE);

	SymbolType type = Expr();

	switch (type)
	{
	case stChar:
		m_pCodeGen->GreaterEq(type);
		break;

	case stUnsigned:
	case stShort:
	case stSigned:
	case stLong:
	case stInteger:
	default:
		yyerror("operator '>=' unsupported on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::LessEq(void)
{
	match(TV_LE);

	SymbolType type = Expr();

	switch (type)
	{
	case stChar:
		m_pCodeGen->LessEq(type);
		break;

	case stUnsigned:
	case stShort:
	case stSigned:
	case stLong:
	case stInteger:
	case stFloat:
	default:
		yyerror("operator '<=' unsupported on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Relation(void)
{
	SymbolType typeRight = stUndef;

	SymbolType typeLeft = Expr();

	// is the next operator a relational one?
	if (isRelop(lookahead))
	{
		switch(lookahead)
		{
		case TV_GE:
			typeRight = GreaterEq();
			break;

		case TV_LE:
			typeRight = LessEq();
			break;

		case TV_EQ:
			typeRight = Equivalent();
			break;

		case TV_NE:
			typeRight = NotEquivalent();
			break;

		case '<':
			typeRight = Less();
			break;

		case '>':
			typeRight = Greater();
			break;

		default:
			ICE(1000);
			assert(false);
			break;
		}

		// TODO - We might want to set typeLeft = stUndef in the case below -- mds???
		if (!AreTypesAssignmentCompatible(typeLeft, typeRight))
			yyerror("types are not comparable");

		// results of a relational operator are boolean
		typeLeft = stBool;
	}

	return typeLeft;
}

//
SymbolType CParser::NotFactor(void)
{
	SymbolType type;

	if (lookahead == TV_NOT)
	{
		match(TV_NOT);

		type = stBool;

		Relation();

		m_pCodeGen->Not(type);
	}
	else
		type = Relation();

	return type;
}

//
SymbolType CParser::BoolTerm(void)
{
	SymbolType typeRight;

	SymbolType typeLeft = NotFactor();

	// look for arithmetic AND
	while (lookahead == '&')
	{
		match('&');

		typeRight = NotFactor();

		if (typeLeft != typeRight)
			yyerror("type mismatch");

		m_pCodeGen->And(typeLeft);
	}

	return typeLeft;
}

//
void CParser::BoolOr(void)
{
	match(TV_OR);
	BoolTerm();
	m_pCodeGen->Or(stBool);
}

//
void CParser::BoolAnd(void)
{
	match(TV_AND);
	BoolTerm();
	m_pCodeGen->And(stBool);
}

//
void CParser::BoolXor(void)
{
	match(TV_XOR);
	BoolTerm();
	m_pCodeGen->Xor(stBool);
}

//
SymbolType CParser::BoolExpr(void)
{
	SymbolType type = BoolTerm();

	while (isOrop(lookahead) || isAndop(lookahead))
	{
		switch(lookahead)
		{
		case TV_AND:
			BoolAnd();
			break;

		case TV_OR:
			BoolOr();
			break;

		case TV_XOR:
			BoolXor();
			break;
		}
	}

	return type;
}

//======================================================================
//
//======================================================================
SymbolType CParser::Factor(void)
{
	SymbolType type = stUndef;
	SymbolEntry *sym;

	switch(lookahead)
	{
	case '(':
		// expressions are factors!
		match('(');
			type = BoolExpr();
		match(')');
		break;

	case TV_STRING:
		// strings are char pointers
//		m_pCodeGen->loadvar(yylval.sym);
		match(TV_STRING);
		type = stStringLiteral;
		break;

	case TV_ID:
		{
			// idents can be functions, variables or constants
			sym = yylval.sym;
			if (!sym || sym->type == stUndef)
			{
				yyerror("undeclared variable");
				break;
			}

			// handle function calls
			if (sym->type == stFunction)
			{
				type = sym->returnType;
				DoFunctionCall();
	//			m_pCodeGen->push_return();
				break;
			}

			// handle variables
			match(TV_ID);

			// handle constants
			if (sym->isConst)
			{
				switch(sym->type)
				{
				case stBool:
					m_pCodeGen->LoadConst((sym->bval == true));
					break;

				case stInteger:
					m_pCodeGen->LoadConst(sym->ival);
					break;

				case stFloat:
	//				m_pCodeGen->LoadConst(sym->constVal.fval);
					break;

				case stChar:
					m_pCodeGen->LoadConst(sym->char_val);
					break;

				case stSigned:
				case stUnsigned:
				case stLong:
				default:
					yyerror("invalid constant type");
				}

				type = sym->type;
				sym->isReferenced = true;
				break;
			}

			int bit = -1;
			if (lookahead == '.')
				bit = GetBit(sym);

			m_pCodeGen->LoadVar(sym);

			if (lookahead == TV_INC)
			{
				match(lookahead);
				m_pCodeGen->Inc(sym);
			}
			else if (lookahead == TV_DEC)
			{
				match(lookahead);
				m_pCodeGen->Dec(sym);
			}

			type = sym->type;
			sym->isReferenced = true;
		}
		break;

	case TV_TRUE:
	case TV_FALSE:
		m_pCodeGen->LoadConst((lookahead == TV_TRUE));
		match(lookahead);
		type = stBool;
		break;

/*
	case TV_TOINT:
		match(TV_TOINT);
		match('(');
		type = BoolExpr();
		if (type != stFloat)
		{
			yyerror("toint() must take float as parameter");
			return type;
		}

		match(')');
		m_pCodeGen->FloatToInt();
		type = stInteger;
		break;

	case TV_TOFLOAT:
		match(TV_TOFLOAT);
		match('(');
		type = BoolExpr();
		if (type != stInteger)
		{
			yyerror("tofloat() must take integer as parameter");
			return type;
		}

		match(')');

//		m_pCodeGen->IntToFloat();
		type = stFloat;
		break;
*/

	case TV_INTVAL:
		m_pCodeGen->LoadConst(yylval.ival);
		match(TV_INTVAL);
		type = stInteger;
		break;

	case TV_FLOATVAL:
//		m_pCodeGen->LoadConst(yylval.fval);
		match(TV_FLOATVAL);
		type = stFloat;
		break;

	case TV_CHARVAL:
		m_pCodeGen->LoadConst(yylval.char_val);
		match(TV_CHARVAL);
		type = stChar;
		break;

	default:
		yyerror("Unknown factor");
	}

	return type;
}

//
//
//
SymbolType CParser::NegFactor(void)
{
	SymbolType type;

	switch(lookahead)
	{
	case TV_INTVAL:
//		m_pCodeGen->LoadConst(-yylval.ival);
		match(TV_INTVAL);
		type = stInteger;
		break;

	case TV_FLOATVAL:
//		m_pCodeGen->LoadConst(-yylval.fval);
		match(TV_FLOATVAL);
		type = stFloat;
		break;

	default:
		type = Factor();

		switch (type)
		{
		case stInteger:
//			m_pCodeGen->negate();
			break;

		case stFloat:
//			m_pCodeGen->negatef();
			break;

		default:
			yyerror("operation not support on type");
			return type;
		}

		break;
	}

	return type;
}

//
//
//
SymbolType CParser::FirstFactor(void)
{
	SymbolType type;

	switch(lookahead)
	{
	case '+':
		match('+');
		type = Factor();
		break;

	case '-':
		match('-');
		type = NegFactor();
		break;

	default:
		type = Factor();
	}

	return type;
}

//
SymbolType CParser::Multiply(void)
{
	match('*');

	SymbolType type = Factor();

	switch (type)
	{
	case stInteger:
//		m_pCodeGen->Mul();
		break;

	case stFloat:
//		m_pCodeGen->Mulf();
		break;

	default:
		yyerror("operator '*' not support on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Divide(void)
{
	match('/');

	SymbolType type = Factor();

	switch (type)
	{
	case stInteger:
//		m_pCodeGen->Div();
		break;

	case stFloat:
//		m_pCodeGen->Divf();
		break;

	default:
		yyerror("operator '/' support on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Remainder(void)
{
	match('%');

	SymbolType type = Factor();

	if (type != stInteger)
	{
		yyerror("operator '%' not support on type");
		return type;
	}

//	m_pCodeGen->Mod();
	return type;
}

//
SymbolType CParser::MoreTerms(SymbolType typeLeft)
{
	SymbolType type;

	while (isMulop(lookahead))
	{
		switch(lookahead)
		{
		case '*':
			type = Multiply();
			break;

		case '/':
			type = Divide();
			break;

		case '%':
			type = Remainder();
			break;
		}

		if (!AreTypesAssignmentCompatible(type, typeLeft))
			yyerror("type mismatch");
	}

	return typeLeft;
}

//
SymbolType CParser::Term(void)
{
	SymbolType typeLeft		= Factor();
	SymbolType typeRight	= MoreTerms(typeLeft);

	if (typeLeft != typeRight)
		yyerror("type mismatch");

	return typeLeft;
}

//
SymbolType CParser::FirstTerm(void)
{
	SymbolType typeLeft		= FirstFactor();
	SymbolType typeRight	= MoreTerms(typeLeft);

	if (typeLeft != typeRight)
		yyerror("type mismatch");

	return typeLeft;
}

//
SymbolType CParser::Add(void)
{
	match('+');

	SymbolType type = Term();

	switch (type)
	{
	case stChar:
	case stUnsigned:
	case stLong:
	case stSigned:
	case stShort:
	case stInteger:
		m_pCodeGen->Add(type);
		break;

	case stFloat:
//		m_pCodeGen->Addf();
		break;

	default:
		yyerror("operation not support on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Subtract(void)
{
	match('-');

	SymbolType type = Term();

	switch (type)
	{
	case stChar:
	case stUnsigned:
	case stLong:
	case stSigned:
	case stShort:
	case stInteger:
		m_pCodeGen->Sub(type);
		break;

	case stFloat:
//		m_pCodeGen->Subf();
		break;

	default:
		yyerror("operation not support on type");
		return type;
	}

	return type;
}

//
SymbolType CParser::Expr(void)
{
	SymbolType typeLeft, typeRight;

	typeLeft = FirstTerm();

	while (isAddop(lookahead))
	{
		switch(lookahead)
		{

		case TV_SHL:
			typeRight = ShiftLeft();
			break;

		case TV_SHR:
			typeRight = ShiftRight();
			break;

		case '+':
			typeRight = Add();
			break;

		case '-':
			typeRight = Subtract();
			break;
		}

		if (!AreTypesAssignmentCompatible(typeLeft, typeRight))
			yyerror("type mismatch");
	}

	return typeLeft;
}

//
//
//
int CParser::GetBit(SymbolEntry *sym)
{
	int bit = -1;

	if (sym->type != stStruct && sym->storage != stoPort)
	{
		yyerror("expected struct or port");
		return bit;
	}

	match('.');
	if (lookahead != TV_INTVAL)
	{
		yyerror("expected integer literal");
		return bit;
	}

	bit = yylval.ival;
	match(TV_INTVAL);

	return bit;
}

//
// Statements:
//	
//
void CParser::Statement(int breakLabel, int continueLabel)
{
	switch (lookahead)
	{
	case TV_RETURN:
		DoReturn(m_pCurrentFunction);
		EOS();
		break;

	case TV_BREAK:
		if (breakLabel != -1)
		{
			//				m_pCodeGen->CommentCode(";\n;\tbreak\n;"); 
			m_pCodeGen->Branch(breakLabel);
		}

		match(TV_BREAK);
		EOS();
		break;

	case TV_CONTINUE:
		if (continueLabel != -1)
		{
			//				m_pCodeGen->CommentCode(";\n;\tcontinue\n;"); 
			m_pCodeGen->Branch(continueLabel);
		}

		match(TV_CONTINUE);
		EOS();
		break;

	case TV_IF:
		DoIf(breakLabel, continueLabel);
		//			m_pCodeGen->CommentCode(""); 
		break;

	case TV_FOR:
		DoFor();
		//			m_pCodeGen->CommentCode(""); 
		break;

	case TV_SWITCH:
		NYI();
		//			DoSwitch();
		//			m_pCodeGen->CommentCode(""); 
		break;

	case TV_WHILE:
		DoWhile();
		//			m_pCodeGen->CommentCode(""); 
		break;

	case TV_ID:
		// idents start funcs or assigns
		// functions are statememts
		if (!yylval.sym)
		{
			yyerror("unknown identifier");
			return;
		}

		if (yylval.sym->type == stFunction)
			DoFunctionCall();
		else
			Assignment();

		//			m_pCodeGen->CommentCode(""); 
		EOS();
		break;

	case ';':         /* allow null statements */
		EOS();
		break;

	default:
		yyerror("not a valid statement");
		return;
	}
}

//
bool CParser::IsIntegerType(SymbolType st)
{
	return st == stInteger || 
			st == stChar ||
			st == stUnsigned ||
			st == stSigned ||
			st == stLong ||
			st == stShort;
}

//======================================================================
// Simple assignments or postfix inc/dec
//======================================================================
void CParser::Assignment(void)
{
	SymbolEntry *sym;
	SymbolType typeLeft, typeRight;
	int bitnum = -1;

	// start of L-VAL
	sym = yylval.sym;
	if (!sym || sym->type == stUndef)
	{
		yyerror("undeclared variable");
		return;
	}

	match(TV_ID);

	if (sym->isConst)
	{
		yyerror("can't assign to const lvalue");
		return;
	}

	if (lookahead == '.')
	{
		bitnum = GetBit(sym);
	}

	// mark ID as used
	sym->isReferenced = true;     
	typeLeft = sym->type;

	// look for postfix operators
	if (lookahead == TV_INC || lookahead == TV_DEC)
	{
		// we can only postfix inc/dec integer values
		if (!IsIntegerType(typeLeft))
		{
			yyerror("postfix operator applied to non-integer variable");
			return;
		}

		m_pCodeGen->LoadVar(sym);

		switch(lookahead)
		{
		case TV_INC:
			m_pCodeGen->Inc(sym);
			break;

		case TV_DEC:
			m_pCodeGen->Dec(sym);
			break;
		}

		// generate code to store result => symbol
		m_pCodeGen->Store(sym);

		match(lookahead);
		return;
	}

	match('=');         // assignment operator

	// start of R-VAL
	if (lookahead == TV_STRING)
	{
//		m_pCodeGen->loadvar(yylval.sym);
		match(TV_STRING);
		typeRight = stStringLiteral;
	}
	else
		typeRight = BoolExpr();

	switch (typeLeft)
	{
	case stString:
		if (typeRight == stString || typeRight == stStringLiteral)
			break;
		// otherwise intentionally fall-through

	default:
		if (!AreTypesAssignmentCompatible(typeLeft, typeRight))
		{
			yyerror("type mismatch");
			return;
		}
		break;
	}

	// generate code to store result => symbol
	m_pCodeGen->Store(sym, bitnum);
}

//======================================================================
//
//======================================================================
void CParser::DoIf(int breakLabel, int continueLabel)
{
	int elseLabel, endLabel;

	match(TV_IF);

	match('(');
		BoolExpr();
	match(')');

	elseLabel	= NewLabel();
	endLabel	= elseLabel;

	m_pCodeGen->BranchFalse(elseLabel);

	// true clause
	if (lookahead == '{')
		CompoundStatement(breakLabel, continueLabel);
	else
		Statement(breakLabel, continueLabel);

	// optional else clause
	if (lookahead == TV_ELSE)
	{
		match(TV_ELSE);

		endLabel = NewLabel();
		m_pCodeGen->Branch(endLabel);

		// start of else clause
		m_pCodeGen->PostLabel(elseLabel);

		if (lookahead == '{')
			CompoundStatement(breakLabel, continueLabel);
		else
			Statement(breakLabel, continueLabel);
	}

	m_pCodeGen->PostLabel(endLabel);
}

//======================================================================
//
//======================================================================
void CParser::DoFor()
{
	match(TV_FOR);

	int ltest		= NewLabel();
	int lendloop	= NewLabel();

	match('(');

	// optional for-loop initializer
	if (lookahead != ';')
		Assignment();

	match(';');

	m_pCodeGen->PostLabel(ltest);

	if (lookahead != ';')
		BoolExpr();

	m_pCodeGen->BranchFalse(lendloop);

	match(';');

	// optional for-loop inc/dec-rementer
	if (lookahead != ')')
		BoolExpr();

	match(')');

	if (lookahead == '{')
		CompoundStatement(lendloop, ltest);
	else
		Statement(lendloop, ltest);

	m_pCodeGen->Branch(ltest);
	m_pCodeGen->PostLabel(lendloop);
}

//
//
//
void CParser::DoWhile()
{
	int labelTop, labelBottom;

	match(TV_WHILE);

	labelTop	= NewLabel();
	labelBottom = NewLabel();

	m_pCodeGen->PostLabel(labelTop);

	match('(');
		BoolExpr();
	match(')');

	m_pCodeGen->BranchFalse(labelBottom);

	if (lookahead == '{')
		CompoundStatement(labelBottom, labelTop);
	else
		Statement(labelBottom, labelTop);

	m_pCodeGen->Branch(labelTop);
	m_pCodeGen->PostLabel(labelBottom);
}

//
// Parse zero or more statements
//
void CParser::CompoundStatement(int breakLabel, int continueLabel)
{
//	m_pCodeGen->CommentCode("%s", m_lexer->GetCurrentSourceText()); 
	m_pSymbolTable->Push();

	match('{');
	m_pCodeGen->AddInstruction(OP_BEGINBLOCK);

	while (lookahead != '}')
	{
		Statement(breakLabel, continueLabel);
	}

	match('}');
	m_pCodeGen->AddInstruction(OP_ENDBLOCK);

	// Warnings about unreferenced symbols
	m_pSymbolTable->DumpUnreferencedSymbolsAtCurrentLevel();

	m_pSymbolTable->Pop();
}

//======================================================================
// Match the identifier and return its symbol table entry
//======================================================================
SymbolEntry *CParser::OneDeclaration()
{
	// if its not a keyword, it must be an Identifier
	SymbolEntry *sym = yylval.sym;

	// var should be undefined
	if (sym->type != stUndef)
		yyerror("Duplicate variable name");

	// save the file and line information for the identifier
	sym->srcLine = m_lexer->getLineNumber();
	sym->srcFile = m_lexer->getFile();
//	assert(sym->srcFile);

	// get <ident> name
	match(TV_ID);

	return sym;
}

//
// Parses: (args*)
//
void CParser::DeclareFormalParameters()
{
	SymbolType type;
	SymbolEntry *sym;

	while (lookahead != ')')
	{
		type = Type();

		// save parameter type(s) for later to support type checking
		m_pCurrentFunction->parameterTypes.push_back(type);

		sym = OneDeclaration();
		if (sym)
		{
			sym->type		= type;
//			sym->storage	= ltParm;
//			sym->size		= asSymbolTypeSize[type];
//			m_pCodeGen->AddNewVar(sym);
		}

		if (lookahead == ',')
			match(',');
	}
}

//
//
void CParser::FunctionDefinition()
{
	m_pSymbolTable->Push();

	if (!m_pCurrentFunction)
	{
		yyerror("Not in function body.");
		return;
	}

	match('(');
		DeclareFormalParameters();
	match(')');

	m_pCodeGen->BeginFunction(m_pCurrentFunction);

	CompoundStatement(-1, -1);
	if (!m_pCurrentFunction->sawReturn && m_pCurrentFunction->returnType != stVoid)
		yyerror("Function '%s' must return a value", m_pCurrentFunction->lexeme.c_str());

	m_pCodeGen->EndFunction(m_pCurrentFunction);

	m_pCurrentFunction = NULL;

	m_pSymbolTable->DumpUnreferencedSymbolsAtCurrentLevel();
	m_pSymbolTable->Pop();
}

//
//
//
int CParser::DoToken(int token)
{
	while (lookahead != TV_DONE)
	{
		switch (lookahead) 
		{
		case '#':
			DoHash();
			break;

		case TV_ENUM:
			DoEnumeration();
			break;

		case TV_STRUCT:
			DoStruct();
			break;

		default:
			if (!DoDeclaration())
				return 0;
		}
	}

	m_pCodeGen->End();

	m_pCodeGen->WriteModuleHeader();
	m_pCodeGen->WriteDefines(m_pSymbolTable);

	m_pCodeGen->WriteGlobalVars(m_pSymbolTable);
	m_pCodeGen->AssignGlobalMemoryAddresses(m_pSymbolTable);

	// TODO - this should have been generated as IR
	m_pCodeGen->WriteGlobalInitializers(m_pSymbolTable);
	
	// Process the IR and output machine code
	m_pCodeGen->GenerateCode();

	printf("Data segment size: %d bytes\n", m_pCodeGen->GetDataSegmentSize(m_pSymbolTable));
//	printf("Code segment size: %d bytes\n", m_pCodeGen->GetDataSegmentSize(m_pSymbolTable));
	printf("Total lines compiled: %d\n", m_lexer->getTotalLinesCompiled());

	return 0;
}
