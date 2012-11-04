// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DEBUGINFO_H__
#define __BSS_DEBUGINFO_H__

#include "cHighPrecisionTimer.h"
#include "bss_Log.h"
#include "cStr.h"

struct HINSTANCE__; //so we can avoid windows.h
struct _PROCESS_MEMORY_COUNTERS; //Include <psapi.h> to read the information

// An inheritable debug class that exposes process information and provides profiling tools
typedef class BSS_DLLEXPORT bss_DebugInfo : public bss_util::cHighPrecisionTimer, public bss_Log
{
public:
  bss_DebugInfo(bss_DebugInfo&& mov);
  explicit bss_DebugInfo(std::ostream* log=0);
  explicit bss_DebugInfo(const char* logfile, std::ostream* log=0);
  explicit bss_DebugInfo(const wchar_t* logfile, std::ostream* log=0);
  virtual ~bss_DebugInfo();
  // Gets the path of the given module - 0 returns path of calling executable
  const char* ModulePath(HINSTANCE__ *mod=0); //If this is GetModulePath windows' stupid #defines screw it up
  // Gets memory information about the process
  const _PROCESS_MEMORY_COUNTERS* GetProcMemInfo();
  // Gets total memory used by process
  size_t GetWorkingSet();
  // Starts a profiler and returns the ID. Returns -1 if you have used up all available profiler spaces
  inline char OpenProfiler()
  {
    if(_flstart==_flend) return -1; //this circular list implementation method leaves one empty cell
    char ret=_flprof[_flstart];
    (++_flstart)%=NUMPROFILERS;

    _querytime(&_profilers[ret]);
    return ret;
  }
  // Closes a profiler and returns the difference in time in milliseconds as a double
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
  // Clears all profilers
  void ClearProfilers();

  bss_DebugInfo& operator =(bss_DebugInfo&& right);
  static const char NUMPROFILERS = 64;

protected:
  _PROCESS_MEMORY_COUNTERS* _counter;
  cStr _modpath;
  unsigned __int64 _profilers[NUMPROFILERS]; //You can have up to NUMPROFILERS profilers going at once
  char _flprof[NUMPROFILERS]; //profiler free list (circular buffer)
  unsigned char _flstart; //profiler free list location
  unsigned char _flend;
  //void* _curprocess; // We only need this if HighPrecisionTimer doesn't have it

private:
  bss_DebugInfo(const bss_DebugInfo& copy);
  bss_DebugInfo& operator =(const bss_DebugInfo& right);

} BSSDEBUG;

#endif