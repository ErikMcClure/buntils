// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __RWLOCK_H__BSS__
#define __RWLOCK_H__BSS__

#include "bss_compiler.h"
#include <atomic>
#include <assert.h>
#ifdef BSS_DEBUG
#include <thread>
#include <unordered_map>
#endif

namespace bss_util {
  // Write-preferring Readers-Writer lock, although writer starvation is still theoretically possible if an infinite number of new readers attempt to acquire the lock
  class BSS_COMPILER_DLLEXPORT RWLock {
  public:
    inline RWLock() : l(0) {}
    inline ~RWLock() { Lock(); } // Ensures you don't destroy this object while something is trying to read or write to it

    // Acquire write lock
    BSS_FORCEINLINE void Lock() noexcept
    {
      assert(debugemplace());
      while(l.fetch_or(WFLAG, std::memory_order_acquire)&WFLAG); // While the returned value includes the flag bit, another writer is performing an operation
      while((l.load(std::memory_order_relaxed)&WMASK) > 0); // Wait for any remaining readers to flush
    }

    // Release write lock
    BSS_FORCEINLINE void Unlock() noexcept
    { 
      assert(debugerase() == 1);
      assert(l.load(std::memory_order_relaxed)&WFLAG);
      l.fetch_and(WMASK, std::memory_order_release);
    }

    // Acquire read lock
    BSS_FORCEINLINE void RLock() noexcept
    {
      assert(debugemplace());
      while(l.fetch_add(1, std::memory_order_acquire)&WFLAG) //Oppurtunistically acquire a read lock and check to see if the writer flag is set
      {
        if(l.fetch_sub(1, std::memory_order_release)&WFLAG) // If the writer flag is set, release our lock to let the writer through
          while(l.load(std::memory_order_relaxed)&WFLAG); // Wait until the writer flag is no longer set before looping for another attempt
      }
    }

    BSS_FORCEINLINE size_t RUnlock() noexcept
    { 
      assert(debugerase() == 1);
      assert((l.load(std::memory_order_relaxed)&WMASK) > 0);
      return l.fetch_sub(1, std::memory_order_release);
    }

    // Upgrades a read lock to a write lock without releasing the read lock. You CANNOT release this write lock using Unlock(), you have to use Downgrade() followed by RUnlock().
    BSS_FORCEINLINE void Upgrade() noexcept
    {
      assert(debugcount() == 1);
      assert((l.load(std::memory_order_relaxed)&WMASK) > 0);
      while(l.fetch_or(WFLAG, std::memory_order_acquire)&WFLAG); // Acquire the write lock as per normal
      while((l.load(std::memory_order_relaxed)&WMASK) > 1); // Only flush to a single read lock, which will be our own
    }

    // Releases a write lock without releasing the read lock. MUST be called after calling Upgrade()
    BSS_FORCEINLINE void Downgrade() noexcept
    {
      assert(debugcount() == 1);
      assert(l.load(std::memory_order_relaxed)&WFLAG);
      l.fetch_and(WMASK, std::memory_order_release); // Even though this actually does the same thing as Unlock, we force you to call this function instead so the debug checks are valid.
    }

    static const size_t WFLAG = (size_t(1) << ((sizeof(size_t) << 3) - 1));
    static const size_t WMASK = ~WFLAG;

    bool IsFree() const { return !l.load(std::memory_order_relaxed); }

  protected:
    std::atomic<size_t> l;

#ifdef BSS_DEBUG
    bool debugemplace() { while(_debuglock.test_and_set()); bool r = _debug.emplace(std::this_thread::get_id(), 0).second; _debuglock.clear(); return r; }
    size_t debugerase() { while(_debuglock.test_and_set()); size_t r = _debug.erase(std::this_thread::get_id()); _debuglock.clear(); return r; }
    size_t debugcount() { while(_debuglock.test_and_set()); size_t r = _debug.count(std::this_thread::get_id()); _debuglock.clear(); return r; }
    std::unordered_map<std::thread::id, int> _debug;
    std::atomic_flag _debuglock;
#endif
  };
}


#endif
