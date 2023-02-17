// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "test_alloc.h"
#include "buntils/GreedyAlloc.h"
#include "buntils/Thread.h"

using namespace bun;

TESTDEF::RETPAIR test_ALLOC_GREEDY()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<GreedyPolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<GreedyPolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<GreedyPolicy, char, 400, 10000, size_t, size_t>(__testret, 8, 16);
  ENDTEST;
}