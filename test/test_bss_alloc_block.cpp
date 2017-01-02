// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test_alloc.h"
#include "bss_alloc_block.h"

using namespace bss_util;

TESTDEF::RETPAIR test_bss_ALLOC_BLOCK()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<cBlockAlloc<size_t>, size_t, 1, 10000>(__testret);
  ENDTEST;
}