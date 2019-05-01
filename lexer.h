#ifndef __LEXER_H
#define __LEXER_H

//
// forward declarations
//
struct SymbolEntry;
class BaseParser;

//======================================================================
//
//======================================================================
#define DEFAULT_NUM_BUF		32
#define DEFAULT_TEXT_BUF	2048

// predefined token values
enum {
	TV_ERROR = 256,
	TV_DONE,

	// numeric values
	TV_INTVAL,
	TV_FLOATVAL,
	TV_CHARVAL,

	// string literal
	TV_STRING,
	
	// identifier
	TV_ID,

	TV_USER
};

//
struct TokenTable
{
	char *lexeme;
	int token;
};

//
union YYSTYPE
{
	// literal values
	int		ival;
	float	fval;
	char	char_val;

	SymbolEntry *sym;	// ID value
	TokenTable *ptt;	// keyword
};

//
//
//
class LexicalAnalzyer
{
protected:
	struct FDNode
	{
		FILE *fdDocument;
		char *pTextData;
		char *filename;
		int column;
		int yylineno;
		void *pUserData;
	};

	int m_iTotalLinesCompiled;
	
	char m_szCurrentSourceLineText[256];
	int m_iCurrentSourceLineIndex;

	// TODO - this should be a std::vector instead!
	FDNode m_fdStack[20];
	int m_iCurrentFD;

	BaseParser *m_pParser;

	YYSTYPE *m_yylval;

	bool m_bCaseInsensitive;

	int (*compare_function)(const char*, const char*);

	struct ltstr
	{
 		bool operator()(const std::string &s1, const std::string &s2) const
		{
			return strcmp(s1.c_str(), s2.c_str()) < 0;
		}
	};

	typedef std::map<std::string, int, ltstr> TokenTableMap;
	TokenTableMap m_tokenTable;

public:
	LexicalAnalzyer(TokenTable *atokenTable, BaseParser *pParser, YYSTYPE *pyylval);
	virtual ~LexicalAnalzyer();

	const char *GetCurrentSourceText() { return m_szCurrentSourceLineText; }
	void ClearCurrentSourceText()		{ m_szCurrentSourceLineText[0] = 0; }

	int SetFile(const char *theFile);
	int SetData(char *theData, const char *fileName, void* pUserData);

	int PopFile();
	virtual void FreeData(void* pUserData);

	const char *getFile() const { return m_fdStack[m_iCurrentFD].filename; }

	// methods to help with lexical processing
	// yylex() will use these to find tokens
	int SkipLeadingWhiteSpace();
	int follow(int expect, int ifyes, int ifno);
	int backslash(int c);

	// comments
	void skipToEOL(void);
	void cstyle_comment(void);

	int getNumber();
	int getStringLiteral();
	int getCharLiteral();
	int getKeyword();
	const char *GetLexemeFromToken(int token);

	int GetChar();
	int UnGetChar(int c);

	void CaseSensitive(bool onoff = true);

	int getColumn()					{ return m_fdStack[m_iCurrentFD].column; }
	int getLineNumber()				{ return m_fdStack[m_iCurrentFD].yylineno; }
	int getTotalLinesCompiled()		{ return m_iTotalLinesCompiled; }

	// functions that may typically be overridden
	virtual int GetToken();
	virtual int yylex();
	virtual void yyerror(const char *s);
	virtual void yywarning(const char *s);
	virtual bool isidval(int c);
	virtual bool iswhitespace(int c);
};

#endif	//#ifndef __LEXER_H
