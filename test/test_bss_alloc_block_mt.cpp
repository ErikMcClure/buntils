// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test_alloc.h"
#include "bss_alloc_block_MT.h"
#include "cThread.h"

using namespace bss_util;

template<class T>
struct MTALLOCWRAP : cLocklessBlockAlloc<T> { inline MTALLOCWRAP(size_t init = 8) : cLocklessBlockAlloc<T>(init) {} inline void Clear() {} };

typedef void(*ALLOCFN)(TESTDEF::RETPAIR&, MTALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_bss_ALLOC_BLOCK_LOCKLESS()
{
  BEGINTEST;
  MTALLOCWRAP<size_t> _alloc(10000);

  const int NUM = 16;
  cThread threads[NUM];
  startflag.store(false);
  for(int i = 0; i < NUM; ++i)
    threads[i] = cThread((ALLOCFN)&TEST_ALLOC_MT<MTALLOCWRAP<size_t>, size_t, 50000>, std::ref(__testret), std::ref(_alloc));
  startflag.store(true);

  for(int i = 0; i < NUM; ++i)
    threads[i].join();

  MTALLOCWRAP<size_t> _alloc2;

  startflag.store(false);
  for(int i = 0; i < NUM; ++i)
    threads[i] = cThread((ALLOCFN)&TEST_ALLOC_MT<MTALLOCWRAP<size_t>, size_t, 10000>, std::ref(__testret), std::ref(_alloc2));
  startflag.store(true);

  for(int i = 0; i < NUM; ++i)
    threads[i].join();

  //std::cout << _alloc2.contention << std::endl;
  //std::cout << _alloc2.grow_contention << std::endl;
  ENDTEST;
}