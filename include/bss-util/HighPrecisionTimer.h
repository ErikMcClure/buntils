// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __HIGHPRECISIONTIMER_H__BSS__
#define __HIGHPRECISIONTIMER_H__BSS__

#include "defines.h"
#include <stdint.h>
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

namespace bss {
  // High precision timer class with nanosecond precision.
  class BSS_DLLEXPORT HighPrecisionTimer
  {
  public:
    // Starts the timer and takes an initial sample. If you want to reset the time or delta to zero later, call ResetTime() or ResetDelta()
    HighPrecisionTimer(const HighPrecisionTimer& copy) = default;
    HighPrecisionTimer();
    // Resamples the timer, updating the current time and setting the delta to the difference between the last time and the current time.
    double Update();
    // Resamples the timer, but warps the resulting increment by the timewarp argument
    double Update(double timewarp);
    // Updates the timer to prime it for the next update, but overrides the delta for this tick with a custom value.
    void Override(uint64_t nsdelta);
    void Override(double delta);
    // Gets the difference in milliseconds between the last call to Update() and the one before it. Does NOT resample the timer.
    inline double GetDelta() const { return _delta; }
    // Gets the delta in nanoseconds
    inline uint64_t GetDeltaNS() const { return _nsDelta; }
    // Gets the current time in milliseconds that has elapsed since the creation of the timer, or a call to ResetTime.
    inline double GetTime() const { return _time; }
    // Gets the current time that has elapsed in nanoseconds, as a precise 64-bit integer.
    inline uint64_t GetTimeNS() const { return _nsTime; }
    // Resets the time to 0 (preserves delta)
    inline void ResetTime() { _time = 0; _nsTime = 0; }
    // Resets the delta to 0, and resamples the timer.
    inline void ResetDelta() 
    {
      _queryTime(&_curTime);
      _delta = 0; 
      _nsDelta = 0; 
    }

    HighPrecisionTimer& operator=(const HighPrecisionTimer&) = default;
    // Converts two nanosecond counts to seconds and returns the difference as a double.
    BSS_FORCEINLINE static double NanosecondDiff(uint64_t now, uint64_t old) { return (now - old) / 1000000000.0; }

    // Starts a profiler call
    BSS_FORCEINLINE static uint64_t OpenProfiler()
    {
      uint64_t ret;
#ifdef BSS_PLATFORM_WIN32
      _queryTime(&ret);
#else
      _queryTime(&ret, BSS_POSIX_CLOCK_PROFILER);
#endif
      return ret;
    }
    // Closes a profiler and returns the number of nanoseconds that elapsed between the open and close function calls.
    BSS_FORCEINLINE static uint64_t CloseProfiler(uint64_t begin)
    {
      uint64_t compare;
#ifdef BSS_PLATFORM_WIN32
      _queryTime(&compare);
      return ((compare - begin) * 1000000000) / _getFrequency(); //convert to nanoseconds
#else
      _queryTime(&compare, BSS_POSIX_CLOCK_PROFILER);
      return compare - begin;
#endif
    }

  protected:
    double _delta; // milliseconds
    double _time; // milliseconds
    uint64_t _curTime;
    uint64_t _nsTime; // total time passed in nanoseconds
    uint64_t _nsDelta; // Delta in nanoseconds;

#ifdef BSS_PLATFORM_WIN32
    static void _queryTime(uint64_t* _pval);
    static uint64_t _getFrequency();
#else
    static void _queryTime(uint64_t* _pval, clockid_t clock = BSS_POSIX_CLOCK);
#endif
  };
}

#endif
