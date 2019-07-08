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
	NTS_key_values,
	NTS_more_key_values,
	NTS_more_values,
	NTS_object,
	NTS_value,
	NTS_values,
};

class jsonparser : public TableParser {
protected:
	std::stack<YYSTYPE> lvalStack;
	void initTable() override;
	int yyrule(int rule) override;
	void tokenMatch(int token) override { lvalStack.push(yylval); }
	void ruleMatch(int rule) override { /*while (!lvalStack.empty()) lvalStack.pop();*/ }

public:
	jsonparser(LexicalAnalyzer lexer) : TableParser(lexer) {}
};
