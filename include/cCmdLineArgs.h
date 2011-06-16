// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_CMDLINEARGS_H__
#define __C_CMDLINEARGS_H__

#include <vector>
#include "bss_dlldef.h"

/* Either stores the standard argc/argv command lines in an array, or (if they are null), parses the win32 command line into tokens */
template<typename C=char>
class cCmdLineArgs
{
public:
  BSS_DLLEXPORT cCmdLineArgs(int argc, char** argv);
  BSS_DLLEXPORT ~cCmdLineArgs();
  inline int Size() const { return _lines.size(); }
  inline const C* Get(unsigned int index) const { return _lines[index]; }

  inline const C* operator [](unsigned int index) const { return _lines[index]; } 

private:
#pragma warning(push)
#pragma warning(disable:4251)
  std::vector<C*> _lines;
#pragma warning(pop)
  C* m_cmdline; // the command line string
  BSS_DLLEXPORT void ParseCmdLine ();
}; // class cCmdLineArgs

typedef cCmdLineArgs<char> cCmdLineArgsA;
typedef cCmdLineArgs<wchar_t> cCmdLineArgsW;

#endif // __C_CMDLINEARGS_H__