// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/RingAlloc.h"
#include "bss-util/Thread.h"
#include "test_alloc.h"

using namespace bss;

template<class T>
struct MTCIRCALLOCWRAP : RingAlloc<T> { inline MTCIRCALLOCWRAP(size_t init = 8) : RingAlloc<T>(init) {} inline void Clear() {} };

typedef void(*CIRCALLOCFN)(TESTDEF::RETPAIR&, MTCIRCALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_bss_ALLOC_RING()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<RingAlloc<size_t>, size_t, 200, 1000>(__testret);
  MTCIRCALLOCWRAP<size_t> _alloc;

  const int NUM = 16;
  Thread threads[NUM];
  startflag.store(false);
  for(size_t i = 0; i < NUM; ++i)
    threads[i] = Thread((CIRCALLOCFN)&TEST_ALLOC_MT<MTCIRCALLOCWRAP<size_t>, size_t, 1000>, std::ref(__testret), std::ref(_alloc));
  startflag.store(true);

  for(size_t i = 0; i < NUM; ++i)
    threads[i].join();

  ENDTEST;
}
