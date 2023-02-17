// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/RandomQueue.h"

using namespace bun;

TESTDEF::RETPAIR test_RANDOMQUEUE()
{
  BEGINTEST;
  float rect[4] = { 0,100,1000,2000 };
  PoissonDiskSample<float>(rect, 4.0f, [](float* f)->float { return f[0] + f[1]; });
  ENDTEST;
}