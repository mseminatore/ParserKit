#pragma once

#include "../../baseparser.h"

class XMLParser : public BaseParser
{
protected:

public:
	XMLParser();
	virtual ~XMLParser() = default;
	
	int yyparse() override;

	void DoEntity();
	void DoMarkup();
};