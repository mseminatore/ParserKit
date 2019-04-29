
#include "baseparser.h"

//
static const char *_internalTokenLexemes[] = 
{
	"error",
	"EOF",
	"integer value",
	"float value",
	"char value",
	"string literal",
	"identifier",
	"->",
	"^",
	">=",
	"<=",
	"==",
	"!=",
	"!",
	"||",
	"&&"
};

//======================================================================
//
//======================================================================
LexicalAnalzyer::LexicalAnalzyer(TokenTable *aTokenTable, BaseParser *pParser, YYSTYPE *pyylval)
{
	assert(aTokenTable);
	assert(pParser);
	assert(pyylval);

	m_pParser = pParser;
	m_yylval = pyylval;

	m_iTotalLinesCompiled = 0;

	// add tokens to table
	for (; aTokenTable->lexeme; aTokenTable++)
		m_tokenTable[aTokenTable->lexeme] = aTokenTable->token;

	m_yylval		= NULL;

	compare_function	= _stricmp;
	m_bCaseInsensitive	= true;

	m_pParser = NULL;

	m_iCurrentSourceLineIndex = -1;

	for (int i = 0; i < ARRAY_SIZE(m_fdStack); i++)
	{
		m_fdStack[i].fdDocument = NULL;
		m_fdStack[i].pTextData = NULL;
		m_fdStack[i].filename = NULL;
		m_fdStack[i].yylineno = 1;
		m_fdStack[i].column = 0;
	}

	m_iCurrentFD = 0;
}

//
LexicalAnalzyer::~LexicalAnalzyer()
{
	for (int i = 0; i < ARRAY_SIZE(m_fdStack); i++)
	{
		if (m_fdStack[i].fdDocument)
		{
			fclose(m_fdStack[i].fdDocument);
			m_fdStack[i].fdDocument = NULL;
		}

		if (m_fdStack[i].filename)
		{
			free(m_fdStack[i].filename);
			m_fdStack[i].filename	= NULL;
		}
	}
}

//======================================================================
//
//======================================================================
int LexicalAnalzyer::GetChar()
{
	m_fdStack[m_iCurrentFD].column++;

	// get new line if necessary
//	if (m_iCurrentSourceLineIndex == -1)
//		fgets(m_szCurrentSourceLineText, sizeof(m_szCurrentSourceLineText), m_fdStack[m_iCurrentFD].fdDocument);

	if (m_fdStack[m_iCurrentFD].fdDocument)
	{
//		strcat();
		return fgetc(m_fdStack[m_iCurrentFD].fdDocument);
	}

	int c = *m_fdStack[m_iCurrentFD].pTextData;
	m_fdStack[m_iCurrentFD].pTextData++;
	
	return c;
}

//======================================================================
//
//======================================================================
int LexicalAnalzyer::UnGetChar(int c)
{
	m_fdStack[m_iCurrentFD].column--;

	if (m_fdStack[m_iCurrentFD].fdDocument)
		return ungetc(c, m_fdStack[m_iCurrentFD].fdDocument);

	m_fdStack[m_iCurrentFD].pTextData--;
	return 0;
}

//
//
//
void LexicalAnalzyer::yyerror(const char *s)
{
	puts(s);
	fflush(stdout);
	exit(-1);
}

//
//
//
void LexicalAnalzyer::yywarning(const char *s)
{
	puts(s);
	fflush(stdout);
}

//
//
//
void LexicalAnalzyer::CaseSensitive(bool onoff /*= true*/)
{
	compare_function	= (onoff) ? strcmp : _stricmp;
	m_bCaseInsensitive	= onoff;
}

//===============================================================
//
//===============================================================
const char *LexicalAnalzyer::GetLexemeFromToken(int token)
{
	// look for single char tokens
	if (token < 256)
		return (char*)token;

	if (token > 255 && token < TV_USER)
		return _internalTokenLexemes[token - 256];

	TokenTableMap::iterator iter = m_tokenTable.begin();
	for (; iter != m_tokenTable.end(); iter++)
	{
		if (iter->second == token)
			return iter->first.c_str();
	}

	return "(unknown token)";
}

//======================================================================
//
//======================================================================
int LexicalAnalzyer::PopFile()
{
	// if we were processing a file, close it
	if (m_fdStack[m_iCurrentFD].fdDocument)
	{
		assert(m_fdStack[m_iCurrentFD].fdDocument);
		if (m_fdStack[m_iCurrentFD].fdDocument)
		{
			fclose(m_fdStack[m_iCurrentFD].fdDocument);
			m_fdStack[m_iCurrentFD].fdDocument = NULL;
		}

		assert(m_fdStack[m_iCurrentFD].filename);
		if (m_fdStack[m_iCurrentFD].filename)
		{
			free(m_fdStack[m_iCurrentFD].filename);
			m_fdStack[m_iCurrentFD].filename	= NULL;
		}

		m_iCurrentFD--;
		if (m_iCurrentFD == 0)
			return EOF;

		return 0;
	}

	// if we were processing in-memory data, release it
	FreeData(m_fdStack[m_iCurrentFD].pUserData);
	m_fdStack[m_iCurrentFD].pUserData = NULL;
	
	// we assume the pTextData is a subset of the pUserData,
	// and was freed when the data was freed
	m_fdStack[m_iCurrentFD].pTextData = NULL;

	// free the filename of the resource file
	assert(m_fdStack[m_iCurrentFD].filename);
	if (m_fdStack[m_iCurrentFD].filename)
	{
		free(m_fdStack[m_iCurrentFD].filename);
		m_fdStack[m_iCurrentFD].filename	= NULL;
	}

	m_iCurrentFD--;
	if (m_iCurrentFD == 0)
		return EOF;

	return 0;
}

// this is a no-op  meant to be overridden in derived classes
void LexicalAnalzyer::FreeData(void *pUserData)
{
	if (pUserData)
		assert(false);
}

//======================================================================
// Begin processing the given file, pushing the current file onto the
// file descriptor stack.
//======================================================================
int LexicalAnalzyer::SetFile(const char *theFile)
{
	assert(theFile);

	m_iCurrentFD++;
	if (m_iCurrentFD >= ARRAY_SIZE(m_fdStack))
		return -1;

	assert(m_fdStack[m_iCurrentFD].fdDocument == NULL);
	
	FILE *pFile = NULL;
	if (fopen_s(&pFile, theFile, "rt"))
		return -1;

	m_fdStack[m_iCurrentFD].fdDocument = pFile;

	if (!m_fdStack[m_iCurrentFD].fdDocument)
		return -1;

	assert(m_fdStack[m_iCurrentFD].filename == NULL);
	m_fdStack[m_iCurrentFD].filename = _strdup(theFile);

	m_fdStack[m_iCurrentFD].yylineno = 1;

	return 0;
}

//======================================================================
//
//======================================================================
int LexicalAnalzyer::SetData(char *theData, const char *fileName, void *pUserData)
{
	assert(theData);

	m_iCurrentFD++;
	if (m_iCurrentFD >= ARRAY_SIZE(m_fdStack))
		return -1;

	assert(m_fdStack[m_iCurrentFD].pTextData == NULL);
	m_fdStack[m_iCurrentFD].pTextData = theData;

	if (!m_fdStack[m_iCurrentFD].pTextData)
		return -1;

	// hold onto this for later
	m_fdStack[m_iCurrentFD].pUserData = pUserData;

	assert(m_fdStack[m_iCurrentFD].filename == NULL);
	m_fdStack[m_iCurrentFD].filename = _strdup(fileName);

	m_fdStack[m_iCurrentFD].yylineno = 1;

	return 0;
}

//
//
//
int LexicalAnalzyer::GetToken()
{
	// TODO - implement
	return 0;
}

//======================================================================
// valid characters for Identifiers
//======================================================================
bool LexicalAnalzyer::isidval(int c)
{
	// removed: c == '.' || c == '!' || c == '&' || ((char)c) == '�'
	if (isalnum(c) || c == '_')
		return true;

	return false;
}

//======================================================================
// characters which are considered to be whitespace
//======================================================================
bool LexicalAnalzyer::iswhitespace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r') ? true : false;
}

// skip any leading WS
int LexicalAnalzyer::SkipLeadingWhiteSpace()
{
	int chr;

	do
	{
		chr = GetChar();
		if (chr == '\n')
		{
			m_fdStack[m_iCurrentFD].column = 0;
			m_fdStack[m_iCurrentFD].yylineno++;
			m_iTotalLinesCompiled++;
		}
	} while (iswhitespace(chr));

	return chr;
}

//
//
//
int LexicalAnalzyer::backslash(int c)
{
	static char translation_tab[] = "b\bf\fn\nr\rt\t";

	if (c != '\\')
		return c;

	c = GetChar();
	if (islower(c) && strchr(translation_tab, c))
		return strchr(translation_tab, c)[1];

	return c;
}

//
//
//
void LexicalAnalzyer::skipToEOL(void)
{
	int c;

	// skip to EOL
	while ((c = GetChar()) != '\n');

	// put last character back
	UnGetChar(c);
}

//
//
//
void LexicalAnalzyer::cstyle_comment(void)
{
	int c;

	// skip to EOL
	for (c = GetChar(); c != EOF; c = GetChar())
	{
		if (c == '*')
		{
			if (follow('/', 1, 0))
				return;
		}
	}
}

//
//
//
int LexicalAnalzyer::follow(int expect, int ifyes, int ifno)
{
	int chr;

	chr = GetChar();
	if (chr == expect)
		return ifyes;

	UnGetChar(chr);
	return ifno;
}

//
//
//
int LexicalAnalzyer::getNumber()
{
	int c;
	char buf[DEFAULT_NUM_BUF];
	char *bufptr = buf;
	int base = 10;

	// look for hex numbers
 	c = GetChar();
	if (c == '0' && (follow('X', 1, 0) || follow('x', 1, 0)))
		base = 16;
	else
		UnGetChar(c);

	if (base == 16)
	{
		while (isxdigit(c = GetChar()))
			*bufptr++ = c;
	}
	else
	{
		while (isdigit((c = GetChar())) || c == '.')
			*bufptr++ = c;
	}
	
	// need to put back the last character
	UnGetChar(c);

	// make sure string is asciiz
	*bufptr = '\0';

	// handle floats and ints
	if (!strchr(buf, '.'))
	{
		m_yylval->ival = strtoul(buf, NULL, base);
		return TV_INTVAL;
	}
	else
	{
		m_yylval->fval = (float)atof(buf);
		return TV_FLOATVAL;
	}
}

//
//
//
int LexicalAnalzyer::getCharLiteral()
{
	int c;
	
	c = backslash(GetChar());
	m_yylval->char_val = c;
	c = GetChar();
	if (c != '\'')
		yyerror("missing single quote");

	return TV_CHARVAL;
}

//
//
//
int LexicalAnalzyer::getStringLiteral()
{
	SymbolEntry *sym;
	int c;
	char buf[DEFAULT_TEXT_BUF];
	char *cptr = buf;

	c = GetChar();

	while (c != '"' && cptr < &buf[sizeof(buf)])
	{
		if (c == '\n' || c == EOF)
			yyerror("missing quote");

		// build up our string, translating escape chars
		*cptr++ = backslash(c);
		c = GetChar();
	}

	// make sure its asciiz
	*cptr = '\0';

	sym = m_pParser->lookupSymbol(buf);
	if (!sym)
	{
		sym = m_pParser->installSymbol(buf, stStringLiteral);
		if (!sym)
			yyerror("making symbol table entry");

		sym->srcLine = getLineNumber();
		sym->srcFile = getFile();

		m_pParser->AddNewVar(sym);
	}

	m_yylval->sym = sym;
	return TV_STRING;
}

//
//
//
int LexicalAnalzyer::getKeyword()
{
	return 0;
}

//======================================================================
// Generic Lexical analyzer routine
//======================================================================
int LexicalAnalzyer::yylex()
{
	int chr;
	char buf[DEFAULT_TEXT_BUF];
	char *pBuf = buf;
	SymbolEntry *sym;

yylex01:
	// skip any leading WS
	chr = SkipLeadingWhiteSpace();
	
	// process Unix conf style comments
/*
	if (chr == '#')
	{
		skipToEOL();
		goto yylex01;
	}
*/
	// handle C++ style comments
	if (chr == '/')
	{
		if (follow('/', 1, 0))
		{
			skipToEOL();
			goto yylex01;
		}
	}

	// handle C style comments
	if (chr == '/')
	{
		if (follow('*', 1, 0))
		{
			cstyle_comment();
			goto yylex01;
		}
	}


	// look for a number value
	if (isdigit(chr))
	{
		UnGetChar(chr);
		return getNumber();
	}

	// look for char literals
	if (chr == '\'')
	{
		return getCharLiteral();
	}

	// look for string literals
	if (chr == '"')
	{
		return getStringLiteral();
	}

	// look for keywords or ID
	if (isalpha(chr) || chr == '_')
	{
		// get the token
		do 
		{
			*pBuf++ = ((char)chr);
		} while ((chr = GetChar()) != EOF && isidval(chr));
		
		UnGetChar(chr);
	
		// make sure its asciiz
		*pBuf = 0;

		// search token table for possible match
		TokenTableMap::iterator iterTokens = m_tokenTable.find(buf);
		if (iterTokens != m_tokenTable.end())
			return iterTokens->second;

		// see if symbol is already in symbol table
		sym = m_pParser->lookupSymbol(buf);
		if (!sym)
		{
			sym = m_pParser->installSymbol(buf, stUndef);
			if (!sym)
				yyerror("making finding table entry");

			sym->srcLine = getLineNumber();
			sym->srcFile = getFile();
		}

		m_yylval->sym = sym;

		return TV_ID;
	}

	switch(chr)
	{
	case  '^':  return TV_XOR;

	// TODO - add += and -= like operators
	case  '>':  
		if (TV_GE == follow('=', TV_GE, '>'))
			return TV_GE;
		return follow('>', TV_SHR, '>');

	case  '<':  
		if (TV_LE == follow('=', TV_LE, '<'))
			return TV_LE;
		return follow('<', TV_SHL, '<');

	case  '=':  return follow('=', TV_EQ, '=');

	case  '!':  return follow('=', TV_NE, TV_NOT);
	case  '|':  return follow('|', TV_OR, '|');
	case  '&':  return follow('&', TV_AND, '&');
	case  '+':	return follow('+', TV_INC, '+');
	case  '-':  
		if (TV_DEREF == follow('>', TV_DEREF, '-'))
			return TV_DEREF;

		return follow('-', TV_DEC, '-');
		break;

	// we reached end of current file, but could be a nested include so pop
	// file descriptor stack and try to continue
	case 0:
	case EOF:
		if (PopFile() == EOF)
			return TV_DONE;

		// call ourselves again to get the next token
		return yylex();

	default:
		return chr;
	}
}

