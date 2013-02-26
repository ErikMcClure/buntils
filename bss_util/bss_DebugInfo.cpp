// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "bss_DebugInfo.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#include <psapi.h>
#else
#include <sys/types.h>  // stat()
#include <sys/stat.h>   // stat()
#include <unistd.h>     // readlink()
#endif

bss_DebugInfo::bss_DebugInfo(const bss_DebugInfo& copy) { assert(false); }
//bss_DebugInfo::bss_DebugInfo(const bss_DebugInfo& copy) : cHighPrecisionTimer(copy), bss_Log(copy),_flstart(copy._flstart),
//  _flend(copy._flend), _modpath(copy._modpath)
//{
//  memcpy(_profilers,copy._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
//  memcpy(_flprof,copy._flprof, sizeof(unsigned char)*NUMPROFILERS);
//  _counter = new PROCESS_MEMORY_COUNTERS();
//  *_counter=*copy._counter;
//}
bss_DebugInfo::bss_DebugInfo(bss_DebugInfo&& mov) : cHighPrecisionTimer(std::move(mov)), bss_Log(std::move(mov)), _flstart(mov._flstart),
  _flend(mov._flend), _modpath(std::move(mov._modpath))
{
#ifdef BSS_PLATFORM_WIN32
  _counter = new PROCESS_MEMORY_COUNTERS();
  *_counter=*mov._counter;
#endif
  //_curprocess = GetCurrentProcess();
  memcpy(_profilers,mov._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
  memcpy(_flprof,mov._flprof, sizeof(unsigned char)*NUMPROFILERS);
}

bss_DebugInfo::bss_DebugInfo(std::ostream* log) : cHighPrecisionTimer(), bss_Log(log)
{
#ifdef BSS_PLATFORM_WIN32
  _counter = new PROCESS_MEMORY_COUNTERS();
#endif
  //_curprocess = GetCurrentProcess();
  ClearProfilers();
}

bss_DebugInfo::bss_DebugInfo(const char* logfile, std::ostream* log) : cHighPrecisionTimer(), bss_Log(logfile,log)
{
#ifdef BSS_PLATFORM_WIN32
  _counter = new PROCESS_MEMORY_COUNTERS();
#endif
  //_curprocess = GetCurrentProcess();
  ClearProfilers();
}
#ifdef BSS_PLATFORM_WIN32
bss_DebugInfo::bss_DebugInfo(const wchar_t* logfile, std::ostream* log) : cHighPrecisionTimer(), bss_Log(logfile,log)
{
  _counter = new PROCESS_MEMORY_COUNTERS();
  //_curprocess = GetCurrentProcess();
  ClearProfilers();
}
#endif

bss_DebugInfo::~bss_DebugInfo()
{
#ifdef BSS_PLATFORM_WIN32
  delete _counter;
#endif
  //ClearProfiles();
}

void bss_DebugInfo::ClearProfilers()
{
  memset(_profilers, 0, sizeof(unsigned __int64)*NUMPROFILERS);
  for(unsigned short i = 0; i < NUMPROFILERS; ++i) _flprof[i]=(unsigned char)i; //refill freelist
  _flstart=0;
  _flend=NUMPROFILERS-1;
}

#ifdef BSS_PLATFORM_WIN32
const char* bss_DebugInfo::ModulePath(HMODULE mod)
{
  wchar_t buf[MAX_PATH];
  GetModuleFileNameExW(_curprocess,mod,buf,MAX_PATH);
  _modpath=buf;
  return _modpath;
}

size_t bss_DebugInfo::GetWorkingSet()
{
  return GetProcMemInfo()->WorkingSetSize;
}

const PROCESS_MEMORY_COUNTERS* bss_DebugInfo::GetProcMemInfo()
{
  GetProcessMemoryInfo(_curprocess, _counter, sizeof(PROCESS_MEMORY_COUNTERS));
  return _counter;
}
#else
const char* bss_DebugInfo::ModulePath()
{
  _modpath.resize(2048); // lstat does not work in /proc/, so we just say screw it and hope this buffer is large enough. If it isn't, fuck you.
  int r = readlink("/proc/self/exe", _modpath.UnsafeString(), _modpath.capacity()-1);
  if(r < 0) return 0;
  _modpath.UnsafeString()[r] = '\0';
  return _modpath;
}
#endif

bss_DebugInfo& bss_DebugInfo::operator =(bss_DebugInfo&& right)
{
  memcpy(_profilers,right._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
  memcpy(_flprof,right._flprof, sizeof(unsigned char)*NUMPROFILERS);
  _modpath=std::move(right._modpath);
  _flstart=right._flstart;
  _flend=right._flend;
#ifdef BSS_PLATFORM_WIN32
  *_counter=*right._counter; // Just copying this is a lot simpler than trying to move it.
#endif
  bss_Log::operator=(std::move(right));
  cHighPrecisionTimer::operator=(std::move(right));
  return *this;
}

bss_DebugInfo& bss_DebugInfo::operator =(const bss_DebugInfo& right) { assert(false); return *this; }
//{
//  memcpy(_profilers,right._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
//  memcpy(_flprof,right._flprof, sizeof(unsigned char)*NUMPROFILERS);
//  _modpath=right._modpath;
//  _flstart=right._flstart;
//  _flend=right._flend;
//  *_counter=*right._counter;
//  return *this;
//}