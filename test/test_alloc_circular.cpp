// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "test_alloc.h"
#include "buntils/RingAlloc.h"
#include "buntils/Thread.h"

using namespace bun;

template<class T>
struct MTCIRCALLOCWRAP : RingAlloc<T> { inline MTCIRCALLOCWRAP(size_t init = 8) : RingAlloc<T>(init) {} inline void Clear() {} };

typedef void(*CIRCALLOCFN)(TESTDEF::RETPAIR&, MTCIRCALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_ALLOC_RING()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<RingAlloc, size_t, 200, 5000>(__testret);
  TEST_ALLOC_MT<MTCIRCALLOCWRAP, size_t, 200, 10000>(__testret);
  ENDTEST;
}
