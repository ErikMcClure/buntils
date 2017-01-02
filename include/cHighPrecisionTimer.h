// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HIGHPRECISIONTIMER_H__BSS__
#define __C_HIGHPRECISIONTIMER_H__BSS__

#include "bss_defines.h"
#include <cstdint>
#ifndef BSS_PLATFORM_WIN32
#include <time.h>

#ifdef _POSIX_MONOTONIC_CLOCK
#ifdef CLOCK_MONOTONIC_RAW
#define BSS_POSIX_CLOCK CLOCK_MONOTONIC_RAW
#else
#define BSS_POSIX_CLOCK CLOCK_MONOTONIC
#endif
#else
#define BSS_POSIX_CLOCK CLOCK_REALTIME
#endif

#ifdef _POSIX_CPUTIME
#define BSS_POSIX_CLOCK_PROFILER CLOCK_PROCESS_CPUTIME_ID
#else
#define BSS_POSIX_CLOCK_PROFILER BSS_POSIX_CLOCK
#endif
#endif

namespace bss_util
{
  // High precision timer class with nanosecond precision.
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
    void BSS_FASTCALL Override(uint64_t nsdelta);
    void BSS_FASTCALL Override(double delta);
    // Gets the difference in milliseconds between the last call to Update() and the one before it. Does NOT resample the timer.
    inline double BSS_FASTCALL GetDelta() const { return _delta; }
    // Gets the delta in nanoseconds
    inline uint64_t BSS_FASTCALL GetDeltaNS() const { return _nsDelta; }
    // Gets the current time in milliseconds that has elapsed since the creation of the timer, or a call to ResetTime.
    inline double BSS_FASTCALL GetTime() const { return _time; }
    // Gets the current time that has elapsed in nanoseconds, as a precise 64-bit integer.
    inline uint64_t BSS_FASTCALL GetTimeNS() const { return _nsTime; }
    // Resets the time to 0 (preserves delta)
    inline void BSS_FASTCALL ResetTime() { _time = 0; _nsTime = 0; }
    // Resets the delta to 0, and resamples the timer.
    inline void BSS_FASTCALL ResetDelta() { _querytime(&_curTime); _delta = 0; _nsDelta = 0; }

    // Converts two nanosecond counts to seconds and returns the difference as a double.
    BSS_FORCEINLINE static double NanosecondDiff(uint64_t now, uint64_t old) { return (now - old) / 1000000000.0; }

    // Starts a profiler call
    BSS_FORCEINLINE static uint64_t OpenProfiler()
    {
      uint64_t ret;
#ifdef BSS_PLATFORM_WIN32
      _querytime(&ret);
#else
      _querytime(&ret, BSS_POSIX_CLOCK_PROFILER);
#endif
      return ret;
    }
    // Closes a profiler and returns the number of nanoseconds that elapsed between the open and close function calls.
    BSS_FORCEINLINE static uint64_t BSS_FASTCALL CloseProfiler(uint64_t begin)
    {
      uint64_t compare;
#ifdef BSS_PLATFORM_WIN32
      _querytime(&compare);
      return ((compare-begin)*1000000000)/_getfreq(); //convert to nanoseconds
#else
      _querytime(&compare, BSS_POSIX_CLOCK_PROFILER);
      return compare-begin;
#endif
    }

  protected:
    double _delta; // milliseconds
    double _time; // milliseconds
    uint64_t _curTime;
    uint64_t _nsTime; // total time passed in nanoseconds
    uint64_t _nsDelta; // Delta in nanoseconds;

#ifdef BSS_PLATFORM_WIN32
    static void BSS_FASTCALL _querytime(uint64_t* _pval);
    static uint64_t _getfreq();
#else
    static void BSS_FASTCALL _querytime(uint64_t* _pval, clockid_t clock = BSS_POSIX_CLOCK);
#endif
  };
}

#endif
