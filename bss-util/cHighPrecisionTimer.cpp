// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cHighPrecisionTimer.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#endif

using namespace bss_util;

#ifdef BSS_PLATFORM_WIN32
HANDLE hpt_curprocess = GetCurrentProcess(); // Neither the current process nor the CPU frequency can change during program execution, so we just get them here and retrieve them later.
unsigned __int64 hpt_freq;
BOOL hpt_throwaway = QueryPerformanceFrequency((LARGE_INTEGER*)&hpt_freq);
#endif

cHighPrecisionTimer::cHighPrecisionTimer(const cHighPrecisionTimer& copy) : _time(copy._time), _delta(copy._delta), _curTime(copy._curTime)
{
  _construct();
}
cHighPrecisionTimer::cHighPrecisionTimer()
{
  _construct();
  ResetDelta();
  _time = 0;
}

double cHighPrecisionTimer::Update()
{
  unsigned __int64 newTime;
  _querytime(&newTime);
#ifdef BSS_PLATFORM_WIN32
  _delta = ((newTime - _curTime)*1000) / (double)hpt_freq; //We multiply by 1000 BEFORE dividing into a double to maintain precision (since its unlikely the difference between newtime and oldtime is going to be bigger then 9223372036854775
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
  _delta = ((newTime - _curTime)*1000) / (hpt_freq*timewarp);
#else
  _delta = (newTime - _curTime) / (1000000*timewarp);
#endif
  _curTime = newTime;
  _time += _delta;
  return _delta;
}
void cHighPrecisionTimer::Override(double delta)
{
  _querytime(&_curTime);
  _delta=delta;
  _time += _delta;
}

void cHighPrecisionTimer::_construct()
{
#ifdef BSS_PLATFORM_WIN32

  DWORD procmask=_getaffinity();
  HANDLE curthread = GetCurrentThread();
  SetThreadAffinityMask(curthread, 1);
  //we have to do this here before ResetDelta() otherwise ResetDelta will read in the wrong values with _getaffinity()
  SetThreadAffinityMask(curthread, procmask);
#endif
}

#ifdef BSS_PLATFORM_WIN32
void cHighPrecisionTimer::_querytime(unsigned __int64* _pval)
{
  DWORD procmask=_getaffinity();
  HANDLE curthread = GetCurrentThread();
  SetThreadAffinityMask(curthread, 1);
  
  QueryPerformanceCounter((LARGE_INTEGER*)_pval);
  
  SetThreadAffinityMask(curthread, procmask);
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
unsigned long cHighPrecisionTimer::_getaffinity()
{
  DWORD_PTR sysmask;
  DWORD_PTR procmask;
  GetProcessAffinityMask(hpt_curprocess, &procmask, &sysmask);
  return procmask;
}
unsigned __int64 cHighPrecisionTimer::_getfreq() { return hpt_freq; }
#endif
