#include "parser.h"
//#include <direct.h>

//======================================================================
//
//======================================================================
BaseParser::BaseParser(std::unique<CLexer> theLexer)
{
	m_lexer = theLexer;
	m_iErrorCount = 0;
	m_iWarningCount = 0;
	m_bAllocatedSymbolTable = false;
}

//
BaseParser::~BaseParser()
{
	delete m_lexer;
	m_lexer = NULL;

	if (m_bAllocatedSymbolTable)
	{
		m_pSymbolTable->DumpUnreferencedSymbolsAtCurrentLevel();
		delete m_pSymbolTable;
		m_pSymbolTable = NULL;
	}
}

//
bool BaseParser::Init(std::unique<SymbolTable> pSymbolTable /*= NULL*/)
{
	if (!m_lexer->Init(this, &yylval))
		return false;
	
	m_pSymbolTable = pSymbolTable;

	if (!pSymbolTable)
	{
		m_pSymbolTable = new SymbolTable();
		m_pSymbolTable->Init(8);
		m_bAllocatedSymbolTable = true;
	}
	
	return true;
}

//
void BaseParser::yyerror(const char *fmt, ...)
{
	char buf[512], s[512];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : error near column %d: %s\r\n", m_lexer->getFile(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_iErrorCount++;
	OutputErrorMessage(s);
}

//
void BaseParser::OutputErrorMessage(const char *msg)
{
	m_lexer->yyerror(msg);
}

//
void BaseParser::yywarning(const char *fmt, ...)
{
	char buf[512], s[512];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : warning near column %d: %s\r\n", m_lexer->getFile(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_iWarningCount++;
	OutputWarningMessage(s);
}

//
void BaseParser::OutputWarningMessage(const char *msg)
{
	m_lexer->yywarning(msg);
}

//
void BaseParser::expected(int token)
{
	if (token < 256)
		yyerror("expected to see '%c'", token);
	else
		yyerror("expected to see '%s'", m_lexer->GetLexemeFromToken(token));
}

//
void BaseParser::match(int token)
{
	if (lookahead == token)
		lookahead = m_lexer->yylex();
	else
		expected(token);
}

//
int BaseParser::DoToken(int token)
{
	return 0;
}

//
int BaseParser::Parse(const char *filename)
{
	int rv;
	char szFullPath[_MAX_PATH];
	char workingDir[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_FNAME], ext[_MAX_EXT];
	char oldWorkdingDir[_MAX_PATH];

	assert(filename);

	_fullpath(szFullPath, filename, sizeof(szFullPath));
	_splitpath_s(szFullPath, drive, dir, file, ext);
	_stprintf_s(workingDir, "%s%s", drive, dir);
	_getdcwd(_getdrive(), oldWorkdingDir, sizeof(oldWorkdingDir));
	_chdir(workingDir);

	rv = m_lexer->SetFile(filename);
	if (rv != 0)
	{
		yyerror("Couldn't open file: %s", filename);
		_chdir(oldWorkdingDir);
		return rv;
	}

	lookahead = m_lexer->yylex();
	
	DoToken(lookahead);

	_chdir(oldWorkdingDir);
	return 0;
}

int BaseParser::ParseData(char *textToParse, const char *fileName, LPVOID pUserData)
{
	int rv;

	assert(textToParse);

	rv = m_lexer->SetData(textToParse, fileName, pUserData);
	if (rv != 0)
	{
		yyerror("Couldn't parse text");//open file: %s", filename);
		return rv;
	}

	lookahead = m_lexer->yylex();
	
	DoToken(lookahead);

	return 0;
}
