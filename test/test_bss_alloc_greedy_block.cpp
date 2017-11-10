// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "test_alloc.h"
#include "bss-util/GreedyBlockAlloc.h"

using namespace bss;

TESTDEF::RETPAIR test_bss_ALLOC_GREEDY_BLOCK()
{
  typedef BSS_ALIGN(16) float Matrix[2];
  BEGINTEST;
  TEST_ALLOC_FUZZER<GreedyBlockPolicy, Matrix, 400, 10000>(__testret);
  ENDTEST;
}