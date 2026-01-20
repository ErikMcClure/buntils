// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "test_alloc.h"
#include "buntils/GreedyBlockAlloc.h"

using namespace bun;

TESTDEF::RETPAIR test_ALLOC_GREEDY_BLOCK()
{
  typedef float Matrix[2];
  BEGINTEST;
  TEST_ALLOC_FUZZER<GreedyBlockPolicy, Matrix, 400, 10000>(__testret);
  ENDTEST;
}