#define _CRT_SECURE_NO_WARNINGS

#include "baseparser.h"
#include <assert.h>
#include <stdarg.h>

#ifdef _WIN32
#	include <direct.h>
#else
#	include <libgen.h>
#	include <unistd.h>
#	define _chdir chdir
#endif

//======================================================================
//
//======================================================================
BaseParser::BaseParser(std::unique_ptr<SymbolTable> symbolTable)
{
	m_lexer			= nullptr;
	m_errorCount	= 0;
	m_warningCount	= 0;
	m_pSymbolTable	= std::move(symbolTable);
}

//
BaseParser::~BaseParser()
{
	//m_pSymbolTable->dumpUnreferencedSymbolsAtCurrentLevel();
}

// the parser calls this method to report errors
void BaseParser::yyerror(const Position &pos, const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	sprintf(s, "%s(%d) : error near column %d: %s\r\n", pos.srcFile.c_str(), pos.srcLine, pos.srcColumn, buf);

	m_errorCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yyerror(s);
}

// the parser calls this method to report errors
void BaseParser::yyerror(const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	sprintf(s, "%s(%d) : error near column %d: %s\r\n", m_lexer->getFile().c_str(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_errorCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yyerror(s);
}

// print a warning message
void BaseParser::yywarning(const Position &pos, const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	sprintf(s, "%s(%d) : warning near column %d: %s\r\n", pos.srcFile.c_str(), pos.srcLine, pos.srcColumn, buf);

	m_warningCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yywarning(s);
}

// print a warning message
void BaseParser::yywarning(const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	sprintf(s, "%s(%d) : warning near column %d: %s\r\n", m_lexer->getFile().c_str(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_warningCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yywarning(s);
}

//
void BaseParser::yylog(const char *fmt, ...)
{
	if (!yydebug)
		return;

	char buf[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	puts(buf);
}

//
// show an error on unexpected token
//
void BaseParser::expected(int token)
{
	if (token < 256)
		yyerror("expected to see '%c'", token);
	else
		yyerror("expected to see '%s'", m_lexer->getLexemeFromToken(token));
}

//
// attempt to match the given token
//
int BaseParser::match(int token)
{
	if (lookahead == token)
	{
		lookahead = m_lexer->yylex();
	}
	else
	{
		expected(token);
	}

	return lookahead;
}

//
// this method does nothing and is meant to be overridden in sub-classes 
//
int BaseParser::yyparse()
{
	lookahead = m_lexer->yylex();
	return 0;
}

//
int BaseParser::parseFile (const char *filename)
{
	int rv;

	assert(filename);

#ifdef _WIN32
	char szFullPath[_MAX_PATH];
	char workingDir[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_FNAME], ext[_MAX_EXT];
	char oldWorkingDir[_MAX_PATH];

	_fullpath(szFullPath, filename, sizeof(szFullPath));
	_splitpath_s(szFullPath, drive, dir, file, ext);
	sprintf_s(workingDir, "%s%s", drive, dir);
	_getdcwd(_getdrive(), oldWorkingDir, sizeof(oldWorkingDir));
	_chdir(workingDir);
#else
	char workingDir[PATH_MAX], oldWorkingDir[PATH_MAX];
	strcpy(workingDir, dirname((char*)filename));

	getcwd(oldWorkingDir, PATH_MAX);
	chdir(workingDir);
#endif


	rv = m_lexer->pushFile(filename);
	if (rv != 0)
	{
		yyerror("Couldn't open file: %s", filename);
		_chdir(oldWorkingDir);
		return rv;
	}

	yyparse();

	_chdir(oldWorkingDir);
	return 0;
}

//
int BaseParser::parseData(char *textToParse, const char *fileName, void *pUserData)
{
	int rv;

	assert(textToParse);

	rv = m_lexer->setData(textToParse, fileName, pUserData);
	if (rv != 0)
	{
		yyerror("Couldn't parse text");		//open file: %s", filename);
		return rv;
	}

	// TODO - should return the value from yyparse()?
	yyparse();

	return 0;
}
