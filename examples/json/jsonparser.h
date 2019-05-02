#pragma once

#include "../../baseparser.h"
#include "json.h"

class JSONParser : public BaseParser
{
protected:

public:
	JSONParser();
	virtual ~JSONParser();
	
	int DoToken(int token) override;
	void DoObject(JSONValue &node);
	void DoArray(JSONValue &node);
	void DoValue(JSONValue &node);
};