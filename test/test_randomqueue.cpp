// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/RandomQueue.h"

using namespace bss;

TESTDEF::RETPAIR test_RANDOMQUEUE()
{
  BEGINTEST;
  float rect[4] = { 0,100,1000,2000 };
  PoissonDiskSample<float>(rect, 4.0f, [](float* f)->float { return f[0] + f[1]; });
  ENDTEST;
}