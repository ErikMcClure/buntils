// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "test_alloc.h"
#include "buntils/CacheAlloc.h"
#include "buntils/Thread.h"

using namespace bun;

TESTDEF::RETPAIR test_ALLOC_CACHE()
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