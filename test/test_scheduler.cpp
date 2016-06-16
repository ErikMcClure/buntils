// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cScheduler.h"
#include <functional>

using namespace bss_util;

TESTDEF::RETPAIR test_SCHEDULER()
{
  BEGINTEST;
  bool ret[3] = { false,false,true };
  cScheduler<std::function<double()>> s;
  s.Add(0.0, [&]()->double { ret[0] = true; return 0.0; });
  s.Add(0.0, [&]()->double { ret[1] = true; return 10000000.0; });
  s.Add(10000000.0, [&]()->double { ret[2] = false; return 0.0; });
  TEST(s.Length() == 3);
  s.Update();
  TEST(ret[0]);
  TEST(ret[1]);
  TEST(ret[2]);
  TEST(s.Length() == 2);
  ENDTEST;
}