// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/Thread.h"
#include "bss-util/algo.h"
#include "bss-util/HighPrecisionTimer.h"

using namespace bss;

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
  //  //for(int j = bssRandInt(50000,100000); j > 0; --j) { std::this_thread::sleep_for(0); useless.Update(); }
  //  //timer.Update();
  //  apc.SendSignal(); // This doesn't work on linux
  //} 

  ENDTEST;
}