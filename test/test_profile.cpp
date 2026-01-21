// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#define BUN_ENABLE_PROFILER
#include "buntils/Profiler.h"
#include "buntils/algo.h"

using namespace bun;

TESTDEF::RETPAIR test_PROFILE()
{
  std::array<uint16_t, 50000> profnums = {};

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
      profnums[bun_RandInt(0, TESTNUM)] += 1;
    }

    auto pr = HighPrecisionTimer::OpenProfiler();
    for(size_t i = 0; i < 100000; ++i)
    {
      PROFILE_BLOCK(outer);
      {
        PROFILE_BLOCK(inner);
        profnums[bun_RandInt(0, TESTNUM)] += 1;
      }
    }
    //std::cout << HighPrecisionTimer::CloseProfiler(pr)/100000.0 << std::endl;

    for(size_t i = 0; i < 100000; ++i)
    {
      PROFILE_BEGIN(beginend);
      profnums[bun_RandInt(0, TESTNUM)] += 1;
      PROFILE_END(beginend);
    }

  }
  PROFILE_OUTPUT("testprofile.txt", 7);
  ENDTEST;
}