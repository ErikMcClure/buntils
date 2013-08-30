// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cCmdLineArgs.h"
#include "bss_util_c.h"
#include <ctype.h> // isspace()
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#endif

using namespace bss_util;

cCmdLineArgs::cCmdLineArgs(const char* specify) : _cmdline(specify)
{
  ParseCmdLine();
}

cCmdLineArgs::cCmdLineArgs(int argc, char** argv)
{
  if(!argv || !argc)
  {
#ifdef BSS_PLATFORM_WIN32
    _cmdline=GetCommandLineW(); // Copy command line string because we turn it into a bunch of tokens
    ParseCmdLine();
#endif
  }
  else
  {
    for(int i = 0; i < argc; ++i)
      _lines.Add(argv[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Parse _cmdline into individual tokens, which are delimited by spaces. If a
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

void cCmdLineArgs::ParseCmdLine()
{
  enum { TERM  = '\0',QUOTE = '\"' };

  bool bInQuotes = false;
  char* pargs = _cmdline.UnsafeString();

  while(*pargs)
  {
    while(isspace(*pargs)) pargs++;       // skip leading whitespace
    bInQuotes = (*pargs == QUOTE);  // see if this token is quoted
    if(bInQuotes) pargs++;                 // skip leading quote
    _lines.Add(pargs);              // store position of current token
        
    // NOTE: Args are normally terminated by whitespace, unless the arg is quoted.  That's why we handle the two cases separately, even though they are very similar.
    if(bInQuotes) // Find next token.
    {
      // find next quote followed by a space or terminator
      while(*pargs && !(*pargs == QUOTE && (isspace (pargs[1]) || pargs[1] == TERM)))
        pargs++;
      if(*pargs)
      {
        *pargs = TERM;  // terminate token
        if (pargs[1])   // if quoted token not followed by a terminator
          pargs += 2; // advance to next token
      }
    }
    else
    {
      while(*pargs && !isspace(*pargs)) pargs++; // skip to next non-whitespace character
      if(*pargs && isspace(*pargs)) // end of token
      {
        *pargs = TERM;    // terminate token
        pargs++;         // advance to next token or terminator
      }
    }
  }
}