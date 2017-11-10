// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "test_alloc.h"
#include "bss-util/CacheAlloc.h"
#include "bss-util/Thread.h"

using namespace bss;

TESTDEF::RETPAIR test_bss_ALLOC_CACHE()
{
  BEGINTEST;

  {
    StandardAllocator<char, 16> alloc;
    char* p = alloc.allocate(50, 0, 0);
    p = alloc.allocate(60, p, 50);
    alloc.deallocate(p, 60);
  }

  TEST_ALLOC_FUZZER<CachePolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<CachePolicy, char, 400, 10000>(__testret);
  TEST_ALLOC_MT<CachePolicy, char, 400, 10000, size_t, size_t, size_t>(__testret, 300, 8, 16);

  ENDTEST;
}