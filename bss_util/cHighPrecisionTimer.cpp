// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "cHighPrecisionTimer.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#endif

using namespace bss_util;

cHighPrecisionTimer::cHighPrecisionTimer()
{
#ifdef BSS_PLATFORM_WIN32
  _curprocess = GetCurrentProcess();
  _curthread = GetCurrentThread();
  
  _getaffinity();  
  _curthread = GetCurrentThread();
  SetThreadAffinityMask(_curthread, 1);
  QueryPerformanceFrequency((LARGE_INTEGER*)&_freq);//we have to do this here before ResetDelta() otherwise ResetDelta will read in the wrong values with _getaffinity()
  SetThreadAffinityMask(_curthread, _procmask);
#endif
  ResetDelta();
  _time = 0;
}

double cHighPrecisionTimer::Update()
{
  unsigned __int64 newTime;
  _querytime(&newTime);
#ifdef BSS_PLATFORM_WIN32
  _delta = ((newTime - _curTime)*1000) / (double)_freq; //We multiply by 1000 BEFORE dividing into a double to maintain precision (since its unlikely the difference between newtime and oldtime is going to be bigger then 9223372036854775
#else
  _delta = (newTime - _curTime) / ((double)1000000);
#endif
  _curTime = newTime;
  _time += _delta;
  return _delta;
}

double cHighPrecisionTimer::Update(double timewarp)
{
  unsigned __int64 newTime;
  _querytime(&newTime);
#ifdef BSS_PLATFORM_WIN32
  _delta = ((newTime - _curTime)*1000) / (_freq*timewarp);
#else
  _delta = (newTime - _curTime) / (1000000*timewarp);
#endif
  _curTime = newTime;
  _time += _delta;
  return _delta;
}

#ifdef BSS_PLATFORM_WIN32
void cHighPrecisionTimer::_querytime(unsigned __int64* _pval)
{
  _getaffinity();  
  _curthread = GetCurrentThread();
  SetThreadAffinityMask(_curthread, 1);
  
  QueryPerformanceCounter((LARGE_INTEGER*)_pval);
  
  SetThreadAffinityMask(_curthread, _procmask);
}
#else
void cHighPrecisionTimer::_querytime(unsigned __int64* _pval, clockid_t clock)
{
	timespec tspec;
	clock_gettime(clock, &tspec);
  *_pval = (((unsigned __int64)tspec.tv_sec)*1000000000) + (unsigned __int64)tspec.tv_nsec;
}
#endif

#ifdef BSS_PLATFORM_WIN32
void cHighPrecisionTimer::_getaffinity()
{
#if _MSC_VER >= 1400 && defined(BSS_CPU_x86_64)
  GetProcessAffinityMask(_curprocess, (PDWORD_PTR)&_procmask, (PDWORD_PTR)&_sysmask);
#else
  GetProcessAffinityMask(_curprocess, &_procmask, &_sysmask);
#endif
}
#endif
