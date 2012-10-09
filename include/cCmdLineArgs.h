// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_CMDLINEARGS_H__BSS__
#define __C_CMDLINEARGS_H__BSS__

#include <vector>
#include "bss_dlldef.h"

// Either stores the standard argc/argv command lines in an array, or (if they are null), parses the win32 command line into tokens
class BSS_DLLEXPORT cCmdLineArgs
{
public:
  cCmdLineArgs(int argc, char** argv);
  ~cCmdLineArgs();
  inline size_t Size() const { return _lines.size(); }
  inline const char* Get(unsigned int index) const { return _lines[index]; }

  inline const char* operator [](unsigned int index) const { return _lines[index]; } 

private:
#pragma warning(push)
#pragma warning(disable:4251)
  std::vector<char*> _lines;
#pragma warning(pop)
  char* _cmdline; // the command line string
  void ParseCmdLine();
};

#endif