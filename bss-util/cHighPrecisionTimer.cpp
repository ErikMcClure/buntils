// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cHighPrecisionTimer.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#endif

using namespace bss_util;

#ifdef BSS_PLATFORM_WIN32
unsigned __int64 hpt_freq; // The CPU frequency can't change during program execution, so we just get it here and retrieve it later.
BOOL hpt_throwaway = QueryPerformanceFrequency((LARGE_INTEGER*)&hpt_freq);
#endif

cHighPrecisionTimer::cHighPrecisionTimer(const cHighPrecisionTimer& copy) : _time(copy._time), _delta(copy._delta), _curTime(copy._curTime)
{
}
cHighPrecisionTimer::cHighPrecisionTimer()
{
  ResetDelta();
  _time = 0;
}

double cHighPrecisionTimer::Update()
{
  unsigned __int64 newTime;
  _querytime(&newTime);
  if(newTime<_curTime) newTime = _curTime; // Do not allow time to run backwards
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
  if(newTime<_curTime) newTime = _curTime; // Do not allow time to run backwards
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

#ifdef BSS_PLATFORM_WIN32
void cHighPrecisionTimer::_querytime(unsigned __int64* _pval)
{ // The multicore timing glitch that used to happen with QPC calls no longer affects modern windows. See: http://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
  //DWORD procmask=_getaffinity(); 
  //HANDLE curthread = GetCurrentThread();
  //SetThreadAffinityMask(curthread, 1);
  
  QueryPerformanceCounter((LARGE_INTEGER*)_pval);
  
  //SetThreadAffinityMask(curthread, procmask);
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
unsigned __int64 cHighPrecisionTimer::_getfreq() { return hpt_freq; }
#endif