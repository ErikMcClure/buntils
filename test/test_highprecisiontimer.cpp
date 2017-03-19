// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cHighPrecisionTimer.h"
#include "test.h"
#include <thread>

using namespace bss_util;

TESTDEF::RETPAIR test_HIGHPRECISIONTIMER()
{
  BEGINTEST;
  cHighPrecisionTimer timer;
  while(timer.GetTime() == 0.0)
    timer.Update();
  double ldelta = timer.GetDelta();
  double ltime = timer.GetTime();
  TEST(ldelta>0.0);
  TEST(ltime>0.0);
  TEST(ldelta<1000.0); //If that took longer than a second, either your CPU choked, or something went terribly wrong.
  TEST(ltime<1000.0);
  timer.Update(std::numeric_limits<double>::infinity());
  TEST(timer.GetDelta() == 0.0);
  TEST(timer.GetTime() == ltime);
  timer.Update(0.5);
  timer.ResetDelta();
  TEST(timer.GetDelta() == 0.0);
  TEST(timer.GetTime()>0.0);
  std::this_thread::sleep_for(std::chrono::duration<uint64_t>::min());
  timer.Update();
  timer.ResetTime();
  TEST(timer.GetDelta()>0.0);
  TEST(timer.GetTime() == 0.0);
  auto prof = cHighPrecisionTimer::OpenProfiler();
  TEST(prof != 0);
  timer.ResetDelta(); // The linux profiler is *too accurate* in that it only measures precisely how much time the process is using. So we can't just put it to sleep.
  while(timer.GetTime()<2.0) timer.Update();

  TEST(cHighPrecisionTimer::CloseProfiler(prof)>1000000); // we should have slept for at least 1 millisecond
  TEST(cHighPrecisionTimer::CloseProfiler(prof)<5000000); // but less than 5 milliseconds
  prof = cHighPrecisionTimer::OpenProfiler();
  TEST(cHighPrecisionTimer::CloseProfiler(prof)<10000);
  ENDTEST;
}