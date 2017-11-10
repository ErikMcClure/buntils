// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "test_alloc.h"
#include "bss-util/BlockAllocMT.h"
#include "bss-util/Thread.h"

using namespace bss;

template<class T>
struct MTALLOCWRAP : LocklessBlockPolicy<T> { inline MTALLOCWRAP(size_t init = 8) : LocklessBlockPolicy<T>(init) {} inline void Clear() {} };

typedef void(*ALLOCFN)(TESTDEF::RETPAIR&, MTALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_bss_ALLOC_BLOCK_LOCKLESS()
{
  BEGINTEST;
  TEST_ALLOC_MT<MTALLOCWRAP, size_t, 1, 50000, size_t>(__testret, 10000);
  TEST_ALLOC_MT<MTALLOCWRAP, size_t, 1, 20000>(__testret);
  ENDTEST;
}