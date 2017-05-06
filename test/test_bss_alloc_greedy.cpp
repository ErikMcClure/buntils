// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/GreedyAlloc.h"
#include "test_alloc.h"

using namespace bss;

TESTDEF::RETPAIR test_bss_ALLOC_ADDITIVE()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<GreedyAlloc, char, 400, 10000>(__testret);
  ENDTEST;
}