// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cCmdLineArgs.h"
#include "bss_win32_includes.h"
#include "bss_deprecated.h"

template<>
cCmdLineArgs<char>::cCmdLineArgs(int argc, char** argv) : m_cmdline(0)
{
  if(!argv || !argc)
  {
    // Save local copy of the command line string, because
    // ParseCmdLine() modifies this string while parsing it.
    PSZ cmdline = GetCommandLineA();
    size_t lng = strlen (cmdline);
    m_cmdline = new char[++lng];
    if (m_cmdline)
    {
        STRCPY(m_cmdline,lng, cmdline);
        ParseCmdLine(); 
    }
  }
  else
  {
    for(int i = 0; i < argc; ++i)
      _lines.push_back(argv[i]);
  }
}

template<>
cCmdLineArgs<wchar_t>::cCmdLineArgs(int argc, char** argv) : m_cmdline(0)
{
  //if(!argv || !argc)
  {
    // Save local copy of the command line string, because
    // ParseCmdLine() modifies this string while parsing it.
    LPWSTR cmdline = GetCommandLineW();
    size_t lng = wcslen (cmdline);
    m_cmdline = new wchar_t[++lng];
    if (m_cmdline)
    {
        WCSCPY(m_cmdline,lng, cmdline);
        ParseCmdLine(); 
    }
  }
  //else
  //{
  //  for(int i = 0; i < argc; ++i)
  //    _lines.push_back(argv[i]);
  //}
}

template<>
cCmdLineArgs<char>::~cCmdLineArgs()
{
  if(m_cmdline) delete [] m_cmdline;
}
template<>
cCmdLineArgs<wchar_t>::~cCmdLineArgs()
{
  if(m_cmdline) delete [] m_cmdline;
}

////////////////////////////////////////////////////////////////////////////////
// Parse m_cmdline into individual tokens, which are delimited by spaces. If a
// token begins with a quote, then that token is terminated by the next quote
// followed immediately by a space or terminator.  This allows tokens to contain
// spaces.
// This input string:     This "is" a ""test"" "of the parsing" alg"o"rithm.
// Produces these tokens: This, is, a, "test", of the parsing, alg"o"rithm
////////////////////////////////////////////////////////////////////////////////
// This function was written by Eric Tetz in 1999 and released without 
// restriction. It has been modified for use in this bss_util implementation.
// http://www.codeguru.com/cpp/w-p/win32/print.php/c1427
////////////////////////////////////////////////////////////////////////////////

template<>
void cCmdLineArgs<char>::ParseCmdLine ()
{
    enum { TERM  = '\0',QUOTE = '\"' };

    bool bInQuotes = false;
    PSZ pargs = m_cmdline;

    while (*pargs)
    {
        while (isspace (*pargs)) pargs++;       // skip leading whitespace
        bInQuotes = (*pargs == QUOTE);  // see if this token is quoted
        if (bInQuotes) pargs++;                 // skip leading quote
        _lines.push_back (pargs);              // store position of current token
        
        // NOTE: Args are normally terminated by whitespace, unless the arg is quoted.  That's why we handle the two cases separately, even though they are very similar.
        if (bInQuotes) // Find next token.
        {
            // find next quote followed by a space or terminator
            while (*pargs && !(*pargs == QUOTE && (isspace (pargs[1]) || pargs[1] == TERM)))
                pargs++;
            if (*pargs)
            {
                *pargs = TERM;  // terminate token
                if (pargs[1])   // if quoted token not followed by a terminator
                    pargs += 2; // advance to next token
            }
        }
        else
        {
            while (*pargs && !isspace (*pargs)) pargs++; // skip to next non-whitespace character
            if (*pargs && isspace (*pargs)) // end of token
            {
               *pargs = TERM;    // terminate token
                pargs++;         // advance to next token or terminator
            }
        }
    }
}

template<>
void cCmdLineArgs<wchar_t>::ParseCmdLine ()
{
    enum { TERM  = L'\0',QUOTE = L'\"' };

    bool bInQuotes = false;
    LPWSTR pargs = m_cmdline;

    while (*pargs)
    {
        while (iswspace (*pargs)) pargs++;       // skip leading whitespace
        bInQuotes = (*pargs == QUOTE);  // see if this token is quoted
        if (bInQuotes) pargs++;                 // skip leading quote
        _lines.push_back (pargs);              // store position of current token
        
        // NOTE: Args are normally terminated by whitespace, unless the arg is quoted.  That's why we handle the two cases separately, even though they are very similar.
        if (bInQuotes) // Find next token.
        {
            // find next quote followed by a space or terminator
            while (*pargs && !(*pargs == QUOTE && (iswspace (pargs[1]) || pargs[1] == TERM))) pargs++;
                
            if (*pargs)
            {
                *pargs = TERM;  // terminate token
                if (pargs[1])   // if quoted token not followed by a terminator
                    pargs += 2; // advance to next token
            }
        }
        else
        {
            while (*pargs && !iswspace (*pargs)) pargs++; // skip to next non-whitespace character
            if (*pargs && iswspace (*pargs)) // end of token
            {
               *pargs = TERM;    // terminate token
                pargs++;         // advance to next token or terminator
            }
        }
    }
}

