// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#define BSS_ENABLE_PROFILER
#include "profiler.h"
#include "bss_algo.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_PROFILE()
{
  BEGINTEST;
  { // If you don't scope the PROFILE_FUNC, it won't actually return until AFTER PROFILE_OUTPUT gets called.
    PROFILE_FUNC();

    for(int i = 0; i < 100000; ++i)
    {
      CPU_Barrier();
      __PROFILE_STATBLOCK(control, MAKESTRING(control));
      CPU_Barrier();
      __PROFILE_ZONE(control);
      CPU_Barrier();
      testnums[bssrandint(0, TESTNUM)] += 1;
    }

    auto pr = cHighPrecisionTimer::OpenProfiler();
    for(int i = 0; i < 100000; ++i)
    {
      PROFILE_BLOCK(outer);
      {
        PROFILE_BLOCK(inner);
        testnums[bssrandint(0, TESTNUM)] += 1;
      }
    }
    //std::cout << cHighPrecisionTimer::CloseProfiler(pr)/100000.0 << std::endl;

    for(int i = 0; i < 100000; ++i)
    {
      PROFILE_BEGIN(beginend);
      testnums[bssrandint(0, TESTNUM)] += 1;
      PROFILE_END(beginend);
    }

  }
  PROFILE_OUTPUT("testprofile.txt", 7);
  ENDTEST;
}