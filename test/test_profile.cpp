// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#define BSS_ENABLE_PROFILER
#include "bss-util/Profiler.h"
#include "bss-util/algo.h"

using namespace bss;

TESTDEF::RETPAIR test_PROFILE()
{
  BEGINTEST;
  { // If you don't scope the PROFILE_FUNC, it won't actually return until AFTER PROFILE_OUTPUT gets called.
    PROFILE_FUNC();

    for(size_t i = 0; i < 100000; ++i)
    {
      CPU_Barrier();
      __PROFILE_STATBLOCK(control, TXT(control));
      CPU_Barrier();
      __PROFILE_ZONE(control);
      CPU_Barrier();
      testnums[bssRandInt(0, TESTNUM)] += 1;
    }

    auto pr = HighPrecisionTimer::OpenProfiler();
    for(size_t i = 0; i < 100000; ++i)
    {
      PROFILE_BLOCK(outer);
      {
        PROFILE_BLOCK(inner);
        testnums[bssRandInt(0, TESTNUM)] += 1;
      }
    }
    //std::cout << HighPrecisionTimer::CloseProfiler(pr)/100000.0 << std::endl;

    for(size_t i = 0; i < 100000; ++i)
    {
      PROFILE_BEGIN(beginend);
      testnums[bssRandInt(0, TESTNUM)] += 1;
      PROFILE_END(beginend);
    }

  }
  PROFILE_OUTPUT("testprofile.txt", 7);
  ENDTEST;
}