// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "BSS_DebugInfo.h"
#include "bss_win32_includes.h"
#include <psapi.h>

BSS_DebugInfo::BSS_DebugInfo(const BSS_DebugInfo& copy) : cHighPrecisionTimer(copy), bss_Log(copy),_flstart(copy._flstart),
  _flend(copy._flend), _modpath(copy._modpath)
{
  memcpy(_profilers,copy._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
  memcpy(_flprof,copy._flprof, sizeof(unsigned char)*NUMPROFILERS);
  _counter = new PROCESS_MEMORY_COUNTERS();
  *_counter=*copy._counter;
}

BSS_DebugInfo::BSS_DebugInfo(std::ostream* log) : cHighPrecisionTimer(), bss_Log(log)
{
  ClearProfilers();
  _counter = new PROCESS_MEMORY_COUNTERS();
}

BSS_DebugInfo::BSS_DebugInfo(const char* logfile, std::ostream* log) : cHighPrecisionTimer(), bss_Log(logfile,log)
{
  ClearProfilers();
  _counter = new PROCESS_MEMORY_COUNTERS();
}
BSS_DebugInfo::BSS_DebugInfo(const wchar_t* logfile, std::ostream* log) : cHighPrecisionTimer(), bss_Log(logfile,log)
{
  ClearProfilers();
  _counter = new PROCESS_MEMORY_COUNTERS();
}

BSS_DebugInfo::~BSS_DebugInfo()
{
  delete _counter;
  //ClearProfiles();
}

void BSS_DebugInfo::ClearProfilers()
{
  memset(_profilers, 0, sizeof(unsigned __int64)*NUMPROFILERS);
  for(unsigned short i = 0; i < NUMPROFILERS; ++i) _flprof[i]=(unsigned char)i; //refill freelist
  _flstart=0;
  _flend=NUMPROFILERS-1;
}
const char* BSS_DebugInfo::ModulePath(HMODULE mod)
{
  wchar_t buf[MAX_PATH];
  GetModuleFileNameExW(_curprocess,mod,buf,MAX_PATH);
  _modpath=buf;
  return _modpath;
}

//const wchar_t* BSS_DebugInfo::GetModulePathW(HMODULE mod)
//{
//  GetModuleFileNameExW(_curprocess,mod,(wchar_t*)_modpath,PATHBUF);
//  return (wchar_t*)_modpath;
//}

__w64 size_t BSS_DebugInfo::GetWorkingSet()
{
  return GetProcMemInfo()->WorkingSetSize;
}

const PROCESS_MEMORY_COUNTERS* BSS_DebugInfo::GetProcMemInfo()
{
  GetProcessMemoryInfo(_curprocess, _counter, sizeof(PROCESS_MEMORY_COUNTERS));
  return _counter;
}

BSS_DebugInfo& BSS_DebugInfo::operator =(const BSS_DebugInfo &right)
{
  memcpy(_profilers,right._profilers, sizeof(unsigned __int64)*NUMPROFILERS);
  memcpy(_flprof,right._flprof, sizeof(unsigned char)*NUMPROFILERS);
  _modpath=right._modpath;
  _flstart=right._flstart;
  _flend=right._flend;
  *_counter=*right._counter;
  return *this;
}