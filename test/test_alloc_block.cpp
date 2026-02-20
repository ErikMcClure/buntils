// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/BlockAlloc.h"
#include "test_alloc.h"

using namespace bun;

TESTDEF::RETPAIR test_ALLOC_BLOCK()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<BlockPolicy, size_t, 1, 10000>(__testret);
  ENDTEST;
}