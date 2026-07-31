//
// This parser parses simple INI-style configuration files:
//
//   [section]
//   key = value
//
#include "iniparser.h"

enum
{
	TV_TRUE = TV_USER,
	TV_FALSE,
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "true",	TV_TRUE },
	{ "false",	TV_FALSE },

	{ nullptr,	TV_DONE }
};

//
//
//
IniParser::IniParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<LexicalAnalyzer>(_tokenTable, this, &yylval);

	// INI files commonly use either '#' or ';' to start a comment
	m_lexer->setUnixComments(true);
	m_lexer->setASMComments(true);
}

//
// pair: ID '=' value
//
void IniParser::DoPair()
{
	std::string key = yylval.sym->lexeme;
	match(TV_ID);
	match('=');

	SymbolEntry *sym = installSymbol(const_cast<char *>(key.c_str()));

	switch (lookahead)
	{
	case TV_STRING:
	{
		std::string val = yylval.sym->lexeme;
		sym->type = stStringLiteral;
		match(TV_STRING);
		printf("    %s = \"%s\"\n", key.c_str(), val.c_str());
		break;
	}

	case TV_INTVAL:
	{
		int val = yylval.ival;
		sym->type = stInteger;
		sym->ival = val;
		match(TV_INTVAL);
		printf("    %s = %d\n", key.c_str(), val);
		break;
	}

	case TV_FLOATVAL:
	{
		float val = yylval.fval;
		sym->type = stFloat;
		sym->fval = val;
		match(TV_FLOATVAL);
		printf("    %s = %f\n", key.c_str(), val);
		break;
	}

	case TV_TRUE:
	case TV_FALSE:
	{
		bool val = (lookahead == TV_TRUE);
		sym->type = stUser;
		sym->bval = val;
		match(lookahead);
		printf("    %s = %s\n", key.c_str(), val ? "true" : "false");
		break;
	}

	default:
	{
		// bare word value, e.g. host = localhost
		std::string val = yylval.sym->lexeme;
		sym->type = stStringLiteral;
		match(TV_ID);
		printf("    %s = %s\n", key.c_str(), val.c_str());
		break;
	}
	}
}

//
// section: '[' ID ']' pair*
//
void IniParser::DoSection()
{
	match('[');

	std::string name = yylval.sym->lexeme;
	match(TV_ID);
	match(']');

	printf("[%s]\n", name.c_str());

	// give this section its own scope so its keys can't collide with keys
	// of the same name declared in any other section
	yylog("push() — new symbol table scope for section '%s'", name.c_str());
	m_pSymbolTable->push();

	while (lookahead == TV_ID)
		DoPair();

	// everything installed above disappears with the scope — a key is only
	// resolvable via lookupSymbol() while its section's scope is on top
	yylog("pop() — discarding symbol table scope for section '%s'", name.c_str());
	m_pSymbolTable->pop();
}

//
// config: section*
//
int IniParser::yyparse()
{
	BaseParser::yyparse();

	while (lookahead == '[')
		DoSection();

	return 0;
}
