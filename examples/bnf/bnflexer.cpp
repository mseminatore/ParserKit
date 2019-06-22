#include "../../baseparser.h"
#include "bnflexer.h"

//
//
//
int BNFLexer::specialTokens(int chr)
{
	if (chr == '%')
	{
		if (follow('%', 1, 0))
			return TV_PERCENTS;
		
		if (follow('{', 1, 0))
			return TV_PERCENT_LBRACE;

		if (follow('}', 1, 0))
			return TV_PERCENT_RBRACE;
	}

	// we didn't find any of our special tokens so give our parent a chance
	return LexicalAnalyzer::specialTokens(chr);
}

