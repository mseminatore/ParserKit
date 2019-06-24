#ifndef __BNFLEXER_H
#define __BNFLEXER_H

//
enum {
	TV_PERCENTS = TV_USER,
	TV_PERCENT_LBRACE,
	TV_PERCENT_RBRACE,
	TV_TOKEN,
	TV_START
};

//
class BNFLexer : public LexicalAnalyzer
{
public:
	BNFLexer(TokenTable *atokenTable, BaseParser *pParser, YYSTYPE *pyylval) : LexicalAnalyzer(atokenTable, pParser, pyylval) 
	{
		m_bCStyleComments	= true;
		m_bCharLiterals		= true;
	}

	int specialTokens(int chr) override;
};

#endif	//__BNFLEXER_H
