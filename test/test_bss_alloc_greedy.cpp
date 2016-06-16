// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test_alloc.h"
#include "bss_alloc_greedy.h"

using namespace bss_util;

TESTDEF::RETPAIR test_bss_ALLOC_ADDITIVE()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<cGreedyAlloc, char, 400, 10000>(__testret);
  ENDTEST;
}