// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "cHighPrecisionTimer.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#else //BSS_PLATFORM_POSIX
#include <sys/time.h>
#include <unistd.h>
#endif

using namespace bss_util;

cHighPrecisionTimer::cHighPrecisionTimer()
{
  _curprocess = GetCurrentProcess();
  _curthread = GetCurrentThread();
  
  _getaffinity();  
  _curthread = GetCurrentThread();
  SetThreadAffinityMask(_curthread, 1);
  QueryPerformanceFrequency((LARGE_INTEGER*)&_freq);//we have to do this here before ResetDelta() otherwise ResetDelta will read in the wrong values with _getaffinity()
  SetThreadAffinityMask(_curthread, _procmask);
  ResetDelta();
  _time = 0;
}

double cHighPrecisionTimer::Update()
{
  unsigned __int64 newTime;
  _querytime(&newTime);
  _delta = ((newTime - _curTime)*1000) / (double)_freq; //We multiply by 1000 BEFORE dividing into a double to maintain precision (since its unlikely the difference between newtime and oldtime is going to be bigger then 9223372036854775
  _curTime = newTime;
  _time += _delta;
  return _delta;
}

double cHighPrecisionTimer::Update(double timewarp)
{
  unsigned __int64 newTime;
  _querytime(&newTime);
  _delta = ((newTime - _curTime)*1000) / (_freq*timewarp);
  _curTime = newTime;
  _time += _delta;
  return _delta;
}

void cHighPrecisionTimer::_querytime(unsigned __int64* _pval)
{
  _getaffinity();  
  _curthread = GetCurrentThread();
  SetThreadAffinityMask(_curthread, 1);
  
  QueryPerformanceCounter((LARGE_INTEGER*)_pval);
  
  SetThreadAffinityMask(_curthread, _procmask);
}

void cHighPrecisionTimer::_getaffinity()
{
#if _MSC_VER >= 1400 && defined(BSS_CPU_x86_64)
  GetProcessAffinityMask(_curprocess, (PDWORD_PTR)&_procmask, (PDWORD_PTR)&_sysmask);
#else
  GetProcessAffinityMask(_curprocess, &_procmask, &_sysmask);
#endif
}