// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cThread.h"
#include "bss_algo.h"
#include "cHighPrecisionTimer.h"

using namespace bss_util;

TESTDEF::RETPAIR test_THREAD()
{
  BEGINTEST;
  uint64_t m;
  cThread t([](uint64_t& m) {cThread::Wait(); m = cHighPrecisionTimer::CloseProfiler(m); }, std::ref(m));
  TEST(t.join(2) == -1);
  m = cHighPrecisionTimer::OpenProfiler();
  t.Signal();
  TEST(t.join(1000) != (size_t)-1);
  //std::cout << "\n" << m << std::endl;
  //while(i > 0)
  //{
  //  //for(int j = bssrandint(50000,100000); j > 0; --j) { std::this_thread::sleep_for(0); useless.Update(); }
  //  //timer.Update();
  //  apc.SendSignal(); // This doesn't work on linux
  //} 

  ENDTEST;
}