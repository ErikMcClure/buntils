// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_CMDLINEARGS_H__BSS__
#define __C_CMDLINEARGS_H__BSS__

#include "bss_defines.h"
#include "cDynArray.h"
#include "cStr.h"
#include <functional>

namespace bss_util {
  // Either stores the standard argc/argv command lines in an array, or (if they are null), parses the win32 command line into tokens
  class BSS_DLLEXPORT cCmdLineArgs
  {
  public:
    cCmdLineArgs(const cCmdLineArgs& copy);
    explicit cCmdLineArgs(const char* specify);
    cCmdLineArgs(int argc, char** argv);
    template<typename F> //std::function<void(char* const*,int num)>
    inline void Process(F && fn, char divider='-') const
    {
      const char* const* cur=_lines.begin();
      size_t len=1;
      for(size_t i = 1; i<_lines.Length(); ++i)
      {
        if(_lines[i][0]==divider)
        {
          fn(cur,len);
          cur=_lines.begin()+i;
          len=1;
        }
        else
          ++len;
      }
      fn(cur,len);
    }
    inline size_t Length() const { return _lines.Length(); }
    inline const char* Get(unsigned int index) const { return _lines[index]; }
    inline const char* const* GetAll() const { return _lines; }

    inline const char* operator [](unsigned int index) const { return _lines[index]; } 

  private:
    void ParseCmdLine();

    cDynArray<cArraySimple<const char*,size_t>> _lines;
    cStr _cmdline;
  };
}

#endif
