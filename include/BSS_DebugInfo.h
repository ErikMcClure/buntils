// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DEBUGINFO_H__
#define __BSS_DEBUGINFO_H__

#include "cHighPrecisionTimer.h"
#include "bss_Log.h"

struct HINSTANCE__; //so we can avoid windows.h
struct _PROCESS_MEMORY_COUNTERS; //Include <psapi.h> to read the information

/* An inheritable debug class that exposes process information and provides profiling tools */
typedef class BSS_DLLEXPORT BSS_DebugInfo : public bss_util::cHighPrecisionTimer, public bss_Log
{
public:
  BSS_DebugInfo(const BSS_DebugInfo& copy);
  BSS_DebugInfo(std::ostream* log=0);
  BSS_DebugInfo(const char* logfile, std::ostream* log=0);
  BSS_DebugInfo(const wchar_t* logfile, std::ostream* log=0);
  virtual ~BSS_DebugInfo();
  /* Gets the path of the given module - 0 returns path of calling executable */
  const char* GetModulePath(HINSTANCE__ *mod=0);
  const wchar_t* GetModulePathW(HINSTANCE__ *mod=0);
  /* Gets memory information about the process */
  const _PROCESS_MEMORY_COUNTERS* GetProcMemInfo();
  /* Gets total memory used by process */
  __w64 unsigned long GetWorkingSet();
  /* Starts a profiler and returns the ID. Returns -1 if you have used up all available profiler spaces */
  inline char OpenProfiler()
  {
    if(_flstart==_flend) return -1; //this circular list implementation method leaves one empty cell
    char ret=_flprof[_flstart];
    (++_flstart)%=NUMPROFILERS;

    _querytime(&_profilers[ret]);
    return ret;
  }
  /* Closes a profiler and returns the difference in time in milliseconds as a double */
  inline unsigned __int64 BSS_FASTCALL CloseProfiler(char ID)
  {
    unsigned __int64 compare;
    _querytime(&compare); //done up here to minimize timing inaccuracies
    if(ID>=NUMPROFILERS || !_profilers[ID]) return 0;

    compare -= _profilers[ID];
    _profilers[ID]=0;
    (++_flend)%=NUMPROFILERS; //wrap to NUMPROFILES
    _flprof[_flend]=ID; //add ID to freelist
    return compare;
  }
  /* Clears all profilers */
  void ClearProfilers();

  BSS_DebugInfo& operator =(const BSS_DebugInfo& right);
  static const int PATHBUF = 261;
  static const char NUMPROFILERS = 64;
  static const int DOUBLEPATHBUF=PATHBUF<<1;

protected:
  _PROCESS_MEMORY_COUNTERS* _counter;
  char _modpath[DOUBLEPATHBUF]; //by doubling the path but this lets us use this for both wchar_t and char
  unsigned __int64 _profilers[NUMPROFILERS]; //You can have up to NUMPROFILERS profilers going at once
  unsigned char _flprof[NUMPROFILERS]; //profiler free list (circular buffer)
  unsigned char _flstart; //profiler free list location
  unsigned char _flend;
} BSSDEBUG;

#endif