// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Thread.h"
#include "buntils/algo.h"
#include "buntils/HighPrecisionTimer.h"

using namespace bun;

TESTDEF::RETPAIR test_THREAD()
{
  BEGINTEST;
  uint64_t m;
  Semaphore s;

  Thread t([](uint64_t& p, Semaphore& sem) { sem.Wait(); p = HighPrecisionTimer::CloseProfiler(p); }, std::ref(m), std::ref(s));
  TEST(t.join(2) == -1);
  m = HighPrecisionTimer::OpenProfiler();
  s.Notify();
  TEST(t.join(1000) != (size_t)~0);
  //std::cout << "\n" << m << std::endl;
  //while(i > 0)
  //{
  //  //for(int j = bun_RandInt(50000,100000); j > 0; --j) { std::this_thread::sleep_for(0); useless.Update(); }
  //  //timer.Update();
  //  apc.SendSignal(); // This doesn't work on linux
  //} 

  ENDTEST;
}