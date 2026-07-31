#pragma once

#include "../../baseparser.h"

// -------------------------------------------------------------------------
// IniParser — parses classic INI files ([section] + key = value)
//
// Demonstrates SymbolTable::push()/pop(): each [section] gets its own
// symbol table scope, so the same key name can be reused across sections
// without colliding, and a key stops being resolvable once its section's
// scope has been popped.
// -------------------------------------------------------------------------
class IniParser : public BaseParser
{
public:
	IniParser();
	virtual ~IniParser() = default;

	int yyparse() override;

	void DoSection();
	void DoPair();
};
