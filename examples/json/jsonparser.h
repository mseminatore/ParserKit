#pragma once

#include "../../baseparser.h"

class JSONParser : public BaseParser
{
protected:

public:
	JSONParser();
	virtual ~JSONParser();
	
	int DoToken(int token) override;
	void DoObject();
	void DoArray();
	void DoValue();
};