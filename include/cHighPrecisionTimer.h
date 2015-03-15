// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HIGHPRECISIONTIMER_H__BSS__
#define __C_HIGHPRECISIONTIMER_H__BSS__

#include "bss_defines.h"
#ifndef BSS_PLATFORM_WIN32
#include <time.h>
#endif

namespace bss_util
{
  // High precision timer class with nanosecond precision. All values are returned in milliseconds.
  class BSS_DLLEXPORT cHighPrecisionTimer
  {
  public:
    // Starts the timer and takes an initial sample. If you want to reset the time or delta to zero later, call ResetTime() or ResetDelta()
    cHighPrecisionTimer(const cHighPrecisionTimer& copy);
    cHighPrecisionTimer();
    // Resamples the timer, updating the current time and setting the delta to the difference between the last time and the current time.
    double BSS_FASTCALL Update();
    // Resamples the timer, but warps the resulting increment by the timewarp argument
    double BSS_FASTCALL Update(double timewarp);
    // Updates the timer to prime it for the next update, but overrides the delta for this tick with a custom value.
    void BSS_FASTCALL Override(double delta);
    // Gets the difference between the last call to Update() and the one before it. Does NOT resample the timer.
    inline double BSS_FASTCALL GetDelta() const { return _delta; }
    // Gets the current time that has elapsed since the creation of the timer, or a call to ResetTime.
    inline double BSS_FASTCALL GetTime() const { return _time; }
    // Resets the time to 0 (preserves delta)
    inline void BSS_FASTCALL ResetTime() { _time = 0; }
    // Resets the delta to 0, and resamples the timer.
    inline void BSS_FASTCALL ResetDelta() { _querytime(&_curTime); _delta = 0; }

    // Starts a profiler call
    BSS_FORCEINLINE static unsigned __int64 OpenProfiler()
    {
      unsigned __int64 ret;
#ifdef BSS_PLATFORM_WIN32
      _querytime(&ret);
#else
      _querytime(&ret, CLOCK_PROCESS_CPUTIME_ID);
#endif
      return ret;
    }
    // Closes a profiler and returns the number of nanoseconds that elapsed between the open and close function calls.
    BSS_FORCEINLINE static unsigned __int64 BSS_FASTCALL CloseProfiler(unsigned __int64 begin)
    {
      unsigned __int64 compare;
#ifdef BSS_PLATFORM_WIN32
      _querytime(&compare);
      return ((compare-begin)*1000000000)/_getfreq(); //convert to nanoseconds
#else
      _querytime(&compare, CLOCK_PROCESS_CPUTIME_ID);
      return compare-begin;
#endif
    }

  protected:
    double _delta; // milliseconds
    double _time; // milliseconds
    unsigned __int64 _curTime;

#ifdef BSS_PLATFORM_WIN32
    static void BSS_FASTCALL _querytime(unsigned __int64* _pval);
    static unsigned __int64 _getfreq();
#else
    static void BSS_FASTCALL _querytime(unsigned __int64* _pval, clockid_t clock=CLOCK_MONOTONIC_RAW);
#endif
  };
}

#endif
