// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_alloc_ring.h"
#include "cThread.h"
#include "test_alloc.h"

using namespace bss_util;

template<class T>
struct MTCIRCALLOCWRAP : cRingAlloc<T> { inline MTCIRCALLOCWRAP(size_t init = 8) : cRingAlloc<T>(init) {} inline void Clear() {} };

typedef void(*CIRCALLOCFN)(TESTDEF::RETPAIR&, MTCIRCALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_bss_ALLOC_RING()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<cRingAlloc<size_t>, size_t, 200, 1000>(__testret);
  MTCIRCALLOCWRAP<size_t> _alloc;

  const int NUM = 16;
  cThread threads[NUM];
  startflag.store(false);
  for(int i = 0; i < NUM; ++i)
    threads[i] = cThread((CIRCALLOCFN)&TEST_ALLOC_MT<MTCIRCALLOCWRAP<size_t>, size_t, 1000>, std::ref(__testret), std::ref(_alloc));
  startflag.store(true);

  for(int i = 0; i < NUM; ++i)
    threads[i].join();

  ENDTEST;
}
