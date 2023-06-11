// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "test_alloc.h"
#include "buntils/BlockAllocMT.h"
#include "buntils/Thread.h"

using namespace bun;

template<class T>
struct MTALLOCWRAP : LocklessBlockPolicy<T> { inline MTALLOCWRAP(size_t init = 8) : LocklessBlockPolicy<T>(init) {} inline void Clear() {} };

template<class T, size_t MAXSIZE = 32>
class BUN_COMPILER_DLLEXPORT LocklessBlockCollectionPolicy
{
public:
  LocklessBlockCollectionPolicy() = default;
  inline LocklessBlockCollectionPolicy(LocklessBlockCollectionPolicy&&) = default;
  inline T* allocate(size_t num, T* p = nullptr, size_t old = 0) noexcept
  {
    return reinterpret_cast<T*>(_collection.allocate(num * sizeof(T), p, old * sizeof(T)));
  }
  inline void deallocate(T* p, size_t num = 0) noexcept
  {
    _collection.deallocate(p, num * sizeof(T));
  }
  inline void Clear() noexcept
  {
    _collection.Clear();
  }

  LocklessBlockCollectionPolicy& operator=(LocklessBlockCollectionPolicy&&) = default;
protected:
  LocklessBlockCollection<MAXSIZE * sizeof(T)> _collection;
};

typedef void(*ALLOCFN)(TESTDEF::RETPAIR&, MTALLOCWRAP<size_t>&);
TESTDEF::RETPAIR test_ALLOC_BLOCK_LOCKLESS()
{
  BEGINTEST;
  TEST_ALLOC_MT<MTALLOCWRAP, size_t, 1, 50000, size_t>(__testret, 10000);
  TEST_ALLOC_MT<MTALLOCWRAP, size_t, 1, 20000>(__testret);

  {
    LocklessBlockCollection<64> alloc;
    void* p = alloc.allocate(8, 0, 0);
    p = alloc.allocate(9, p, 8);
    p = alloc.allocate(16, p, 9);
    p = alloc.allocate(17, p, 16);
    p = alloc.allocate(32, p, 17);
    p = alloc.allocate(50, p, 32);
    alloc.deallocate(p, 50);
  }

  TEST_ALLOC_FUZZER<LocklessBlockCollectionPolicy, std::byte, 400, 10000>(__testret);
  TEST_ALLOC_MT<LocklessBlockCollectionPolicy, std::byte, 400, 10000>(__testret);
  ENDTEST;
}