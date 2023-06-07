// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __RWLOCK_H__BUN__
#define __RWLOCK_H__BUN__

#include "compiler.h"
#include "lockless.h"
#include <atomic>
#include <assert.h>
#ifdef BUN_DEBUG
#include <thread>
#include <unordered_map>
#endif

namespace bun {
  // Write-preferring Readers-Writer lock, although writer starvation is still theoretically possible if an infinite number of new readers attempt to acquire the lock
  class BUN_COMPILER_DLLEXPORT RWLock {
  public:
    static_assert(ATOMIC_POINTER_LOCK_FREE == 2, "This lock does not function properly on this architecture!");
    inline RWLock() : l(0) // Note: if required, the fetch_or behavior can be emulated by checking a write lock, incrementing the read lock, then checking the write lock again.
    {
      assert(std::atomic_is_lock_free(&l));
      assert(l.load(std::memory_order_relaxed) == 0);
#ifdef BUN_DEBUG
      _debuglock.clear(std::memory_order_release);
#endif
    }
    inline ~RWLock() {}

    // Acquire write lock
    BUN_FORCEINLINE void Lock() noexcept
    {
      //assert(debugEmplace());

      while(asmbts<size_t>((size_t*)&l, WBIT)); // While the returned value includes the flag bit, another writer is performing an operation
      while((l.load(std::memory_order_relaxed)&WMASK) > 0); // Wait for any remaining readers to flush
    }

    // Attempts to acquire the lock, but if another writer already got the lock, aborts the attempt.
    BUN_FORCEINLINE bool AttemptLock() noexcept
    {
      if(asmbts<size_t>((size_t*)&l, WBIT)) // If another writer already has the lock, give up
        return false;

      //assert(debugEmplace());
      while((l.load(std::memory_order_relaxed)&WMASK) > 0); // Wait for any remaining readers to flush
      return true;
    }

    // Attempts to acquire a write lock, but fails if there are any readers or writers
    BUN_FORCEINLINE bool AttemptStrictLock() noexcept
    {
      size_t prev = 0;
      bool r = l.compare_exchange_strong(prev, WFLAG, std::memory_order_release);
      //if(r) assert(debugEmplace());
      return r;
    }

    // Release write lock
    BUN_FORCEINLINE void Unlock() noexcept
    {
      //assert(debugErase() == 1);
      assert(l.load(std::memory_order_relaxed)&WFLAG);
      l.fetch_and(WMASK, std::memory_order_release);
    }

    // Acquire read lock
    BUN_FORCEINLINE void RLock() noexcept
    {
      //assert(debugEmplace());
      while(l.fetch_add(ONE_READER, std::memory_order_acquire)&WFLAG) // Oppurtunistically acquire a read lock and check to see if the writer flag is set
      {
        if(l.fetch_sub(ONE_READER, std::memory_order_release)&WFLAG) // If the writer flag is set, release our lock to let the writer through
          while(l.load(std::memory_order_relaxed)&WFLAG); // Wait until the writer flag is no longer set before looping for another attempt
      }
    }

    // Attempt to acquire read lock, but abort if a writer has locked it.
    BUN_FORCEINLINE bool AttemptRLock() noexcept
    {
      if(l.fetch_add(ONE_READER, std::memory_order_acquire)&WFLAG) // Oppurtunistically acquire a read lock and check to see if the writer flag is set
      {
        l.fetch_sub(ONE_READER, std::memory_order_release);
        return false;
      }

      //assert(debugEmplace());
      return true;
    }
    BUN_FORCEINLINE size_t RUnlock() noexcept
    {
      //assert(debugErase() == 1);
      assert((l.load(std::memory_order_relaxed)&WMASK) > 0);
      return l.fetch_sub(ONE_READER, std::memory_order_release);
    }

    // Upgrades a read lock to a write lock without releasing the read lock. You CANNOT release this write lock using Unlock(), you have to use Downgrade() followed by RUnlock().
    BUN_FORCEINLINE void Upgrade() noexcept
    {
      //assert(debugCount() == 1);
      assert((l.load(std::memory_order_relaxed)&WMASK) > 0);
      while(asmbts<size_t>((size_t*)&l, WBIT)) // Attempt to acquire the write lock
      {
        RUnlock(); // if we fail, we MUST release our own read lock so the other writer can proceed.
        while(l.load(std::memory_order_relaxed)&WFLAG); // Wait until the writer flag is no longer set before looping for another attempt
        l.fetch_add(ONE_READER, std::memory_order_acquire); // Acquire a read lock before our next upgrade attempt - if the attempt fails, we'll release this.
      }
      while((l.load(std::memory_order_relaxed)&WMASK) > ONE_READER); // Only flush to a single read lock, which will be our own
    }

    // Attempts to upgrade a read lock to a write lock, but aborts if an existing writer has already locked it.
    BUN_FORCEINLINE bool AttemptUpgrade() noexcept
    {
      assert((l.load(std::memory_order_relaxed)&WMASK) > 0);
      if(asmbts<size_t>((size_t*)&l, WBIT))
        return false;

      //assert(debugCount() == 1);
      while((l.load(std::memory_order_relaxed)&WMASK) > ONE_READER); // Only flush to a single read lock, which will be our own
      return true;
    }

    // Releases a write lock without releasing the read lock. MUST be called after calling Upgrade()
    BUN_FORCEINLINE void Downgrade() noexcept
    {
      //assert(debugCount() == 1);
      assert(l.load(std::memory_order_relaxed)&WFLAG);
      l.fetch_and(WMASK, std::memory_order_release); // Even though this actually does the same thing as Unlock, we force you to call this function instead so the debug checks are valid.
    }

    BUN_FORCEINLINE size_t ReaderCount() noexcept
    {
      return l.load(std::memory_order_acquire)&WMASK;
    }

    static const size_t WBIT = (sizeof(size_t) << 3) - 1;
    static const size_t WFLAG = (size_t(1) << WBIT);
    static const size_t WMASK = ~WFLAG;
    static const size_t ONE_READER = 1;

    bool IsFree() const { return !l.load(std::memory_order_relaxed); }

  protected:
#pragma warning(push)
#pragma warning(disable:4251)
    std::atomic<size_t> l;

#ifdef BUN_DEBUG
    bool debugEmplace() { while(_debuglock.test_and_set(std::memory_order_acquire)); bool r = _debug.emplace(std::this_thread::get_id(), 0).second; _debuglock.clear(std::memory_order_release); return r; }
    size_t debugErase() { while(_debuglock.test_and_set(std::memory_order_acquire)); size_t r = _debug.erase(std::this_thread::get_id()); _debuglock.clear(std::memory_order_release); return r; }
    size_t debugCount() { while(_debuglock.test_and_set(std::memory_order_acquire)); size_t r = _debug.count(std::this_thread::get_id()); _debuglock.clear(std::memory_order_release); return r; }
    std::unordered_map<std::thread::id, int> _debug;
    std::atomic_flag _debuglock;
#endif
#pragma warning(pop)
  };
}


#endif
