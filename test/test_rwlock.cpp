// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/RWLock.h"
#include "buntils/Thread.h"

using namespace bun;

TESTDEF::RETPAIR test_RWLOCK()
{
  BEGINTEST;

  RWLock lock;
  lock.Lock();
  lock.Unlock();
  lock.RLock();
  lock.RUnlock();
  lock.RLock();
  lock.Upgrade();
  lock.Downgrade();
  lock.RUnlock();
  TEST(lock.IsFree());

  {
    lock.RLock();
    size_t steps = 0;
    Thread t([&]() {
      lock.RLock();
      ++steps;
      lock.RUnlock();
      ++steps;
    });
    t.join();
    TEST(steps == 2);
    lock.RUnlock();
    TEST(lock.IsFree());
  }

  {
    lock.RLock();
    std::atomic<size_t> steps(0);
    Thread t([&]() {
      steps.fetch_add(1, std::memory_order_relaxed);
      lock.Lock();
      steps.fetch_add(1, std::memory_order_relaxed);
      lock.Unlock();
      steps.fetch_add(1, std::memory_order_relaxed);
    });
    while(steps.load(std::memory_order_relaxed) == 0)
      ;
    TEST(steps.load(std::memory_order_relaxed) == 1);
    lock.RUnlock();
    t.join();
    TEST(steps.load(std::memory_order_relaxed) == 3);
    TEST(lock.IsFree());
  }

  {
    lock.Lock();
    std::atomic<size_t> steps(0);
    Thread t([&]() {
      steps.fetch_add(1, std::memory_order_relaxed);
      lock.RLock();
      steps.fetch_add(1, std::memory_order_relaxed);
      lock.Upgrade();
      lock.Downgrade();
      lock.RUnlock();
      steps.fetch_add(1, std::memory_order_relaxed);
    });
    while(steps.load(std::memory_order_relaxed) == 0)
      ;
    TEST(steps.load(std::memory_order_relaxed) == 1);
    lock.Unlock();
    t.join();
    TEST(steps.load(std::memory_order_relaxed) == 3);
    TEST(lock.IsFree());
  }

  ENDTEST;
}
