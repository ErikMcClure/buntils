// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "test_alloc.h"
#include "bss-util/RingAlloc.h"
#include "bss-util/Thread.h"

using namespace bss;

template<class T>
struct MTCIRCALLOCWRAP : RingAlloc<T> { inline MTCIRCALLOCWRAP(size_t init = 8) : RingAlloc<T>(init) {} inline void Clear() {} };

typedef void(*CIRCALLOCFN)(TESTDEF::RETPAIR&, MTCIRCALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_bss_ALLOC_RING()
{
  BEGINTEST;
  TEST_ALLOC_FUZZER<RingAlloc, size_t, 200, 5000>(__testret);
  TEST_ALLOC_MT<MTCIRCALLOCWRAP, size_t, 200, 10000>(__testret);
  ENDTEST;
}
