#include "tableparser.h"

enum {
	ERROR = 256,
	// Terminal symbols
	TS_FALSE,
	TS_NULL,
	TS_NUM,
	TS_STRING,
	TS_TRUE,
	// Non-Terminal symbols
	NTS_array,
	NTS_file,
	NTS_key_values,
	NTS_more_values,
	NTS_object,
	NTS_value,
};

class jsonparser : public tableparser {
protected:
	void initTable() override;
	void yyrule(int rule) override;
};
