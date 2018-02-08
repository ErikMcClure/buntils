// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "test_alloc.h"
#include "bss-util/GreedyAlloc.h"
#include "bss-util/Thread.h"

using namespace bss;

TESTDEF::RETPAIR test_bss_ALLOC_GREEDY()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<GreedyPolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<GreedyPolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<GreedyPolicy, char, 400, 10000, size_t, size_t>(__testret, 8, 16);
  ENDTEST;
}