// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "cHighPrecisionTimer.h"
#if defined(_WIN32)
#include "bss_win32_includes.h"
#else
#include <sys/time.h>
#include <unistd.h>
static timeval tp;
#endif

using namespace bss_util;

cHighPrecisionTimer::cHighPrecisionTimer()
{
  _curprocess = GetCurrentProcess();
  _curthread = GetCurrentThread();

  _getaffinity();
  SetThreadAffinityMask(_curthread, 1);
  QueryPerformanceFrequency((LARGE_INTEGER*)&_freq);
  SetThreadAffinityMask(_curthread, _procmask); //we have to do this here before ResetDelta() otherwise ResetDelta will read in the wrong values with _getaffinity()
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

double cHighPrecisionTimer::GetDelta() const
{
  return _delta;
}

double cHighPrecisionTimer::GetTime() const
{
  return _time;
}

void cHighPrecisionTimer::ResetTime()
{
  _time = 0;
}

void cHighPrecisionTimer::ResetDelta()
{
  _querytime(&_curTime);
  _delta = 0;
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
#if _MSC_VER >= 1400 && defined (_M_X64)
  GetProcessAffinityMask(_curprocess, (PDWORD_PTR)&_procmask, (PDWORD_PTR)&_sysmask);
#else
  GetProcessAffinityMask(_curprocess, &_procmask, &_sysmask);
#endif
}