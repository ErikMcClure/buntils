// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/HighPrecisionTimer.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss-util/win32_includes.h"
#endif

using namespace bss;

#ifdef BSS_PLATFORM_WIN32
uint64_t hpt_freq; // The CPU frequency can't change during program execution, so we just get it here and retrieve it later.
BOOL hpt_throwaway = QueryPerformanceFrequency((LARGE_INTEGER*)&hpt_freq);
#endif

HighPrecisionTimer::HighPrecisionTimer() : _time(0), _nsTime(0)
{
  ResetDelta();
}

double HighPrecisionTimer::Update()
{
  uint64_t newTime;
  _queryTime(&newTime);
  if(newTime < _curTime) newTime = _curTime; // Do not allow time to run backwards
#ifdef BSS_PLATFORM_WIN32
  _delta = double((newTime - _curTime) * 1000) / (double)hpt_freq; //We multiply by 1000 BEFORE dividing into a double to maintain precision (since its unlikely the difference between newtime and oldtime is going to be bigger then 9223372036854775
  _nsDelta = ((newTime - _curTime) * 1000000000) / hpt_freq;
#else
  _delta = (newTime - _curTime) / ((double)1000000);
  _nsDelta = newTime - _curTime;
#endif
  _curTime = newTime;
  _nsTime += _nsDelta;
  _time = _nsTime / 1000000.0; // not dividing by 1 billion because this is in milliseconds, not seconds
  return _delta;
}

double HighPrecisionTimer::Update(double timewarp)
{
  if(timewarp == 1.0) return Update();
  uint64_t newTime;
  _queryTime(&newTime);
  if(newTime < _curTime) newTime = _curTime; // Do not allow time to run backwards
#ifdef BSS_PLATFORM_WIN32
  uint64_t warpfreq = (uint64_t)(hpt_freq*timewarp);
  _delta = double((newTime - _curTime) * 1000) / (hpt_freq*timewarp);
  _nsDelta = ((newTime - _curTime) * 1000000000) / warpfreq;
#else
  _delta = (newTime - _curTime) / (1000000 * timewarp);
  _nsDelta = (uint64_t)((newTime - _curTime)*timewarp);
#endif
  _curTime = newTime;
  _nsTime += _nsDelta;
  _time = _nsTime / 1000000.0;
  return _delta;
}
void HighPrecisionTimer::Override(uint64_t nsdelta)
{
  _queryTime(&_curTime);
  _nsDelta = nsdelta;
  _delta = nsdelta / 1000000.0;
  _nsTime += _nsDelta;
  _time = _nsTime / 1000000.0;
}
void HighPrecisionTimer::Override(double delta)
{
  _queryTime(&_curTime);
  _nsDelta = (uint64_t)(delta * 1000000.0);
  _delta = delta;
  _nsTime += _nsDelta;
  _time = _nsTime / 1000000.0;
}

#ifdef BSS_PLATFORM_WIN32
void HighPrecisionTimer::_queryTime(uint64_t* _pval)
{ // The multicore timing glitch that used to happen with QPC calls no longer affects modern windows. See: http://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
  //DWORD procmask=_getaffinity(); 
  //HANDLE curthread = GetCurrentThread();
  //SetThreadAffinityMask(curthread, 1);

  QueryPerformanceCounter((LARGE_INTEGER*)_pval);

  //SetThreadAffinityMask(curthread, procmask);
}
#else
void HighPrecisionTimer::_queryTime(uint64_t* _pval, clockid_t clock)
{
  timespec tspec;
  clock_gettime(clock, &tspec);
  *_pval = (((uint64_t)tspec.tv_sec) * 1000000000) + (uint64_t)tspec.tv_nsec;
}
#endif

#ifdef BSS_PLATFORM_WIN32
uint64_t HighPrecisionTimer::_getFrequency() { return hpt_freq; }
#endif