// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_TEST_ALLOC_H__
#define __BUN_TEST_ALLOC_H__

#include "buntils/Map.h"
#include "buntils/algo.h"
#include "test.h"
#include "buntils/Thread.h"
#include "buntils/CacheAlloc.h"
#include "buntils/literals.h"

#pragma warning(disable:4101)

template<template <typename> class A, typename T, bool VERIFY>
bool TEST_ALLOC_FUZZER_REMOVE(std::conditional_t<VERIFY, bun::Map<T*, size_t>, bun::DynArray<std::tuple<T*, size_t>>>& plist, A<T>& _alloc, int id)
{
  bool pass = true;
  size_t index = (size_t)bun::bun_RandInt(0, plist.Length());
  auto&[p, s] = plist[index];
  for(size_t i = 0; i<s * sizeof(T); ++i)
  {
    if(reinterpret_cast<uint8_t*>(p)[i] != (uint8_t)((i + id) & 0xFF))
      pass = false;
  }
  bun::bun_FillN(p, s, 0xfd);
  _alloc.deallocate(p, s);
  if constexpr(VERIFY)
    plist.RemoveIndex(index);
  else
  {
    std::swap(plist[index], plist.Back());
    plist.RemoveLast();
  }
  return pass;
}

template<template <typename> class A, typename T, int MAXSIZE, int TRIALS, bool VERIFY>
void TEST_ALLOC_FUZZER_THREAD(TESTDEF::RETPAIR& __testret, A<T>& _alloc, int id)
{
  bool pass = true;
  std::conditional_t<VERIFY, bun::Map<T*, size_t>, bun::DynArray<std::tuple<T*, size_t>>> plist;
  
  while(!startflag.load());
  for(size_t k = 0; k < TRIALS; ++k)
  {
    if(bun::bun_RandInt(0, 10) < 5 || plist.Length() < 3)
    {
      size_t sz = (size_t)bun::bun_RandInt(1, MAXSIZE);
      T* test = _alloc.allocate(sz);
      //if constexpr(std::is_base_of<CacheAlloc, A<T>>::value) _alloc.VERIFY();
      for(size_t i = 0; i < (sz * sizeof(T)); ++i)
        reinterpret_cast<uint8_t*>(test)[i] = (uint8_t)((i + id) & 0xFF);
      //if constexpr(std::is_base_of<CacheAlloc, A<T>>::value) _alloc.VERIFY();

      if constexpr(VERIFY)
      {
        size_t before = plist.GetNear(test, true);
        size_t after = before + 1;
        if(before != ~0_sz)
        {
          auto[p, s] = plist[before];
          if((reinterpret_cast<char*>(p) + (s * sizeof(T))) > reinterpret_cast<char*>(test))
            pass = false;
        }
        if(after < plist.Length())
        {
          auto[p, s] = plist[after];
          if((reinterpret_cast<char*>(test) + (sz * sizeof(T))) > reinterpret_cast<char*>(p))
            pass = false;
        }
        plist.Insert(test, sz);
      }
      else
        plist.Add({ test, sz });
    }
    else
      pass = pass && TEST_ALLOC_FUZZER_REMOVE<A, T, VERIFY>(plist, _alloc, id);
  }
  while(plist.Length())
    pass = pass && TEST_ALLOC_FUZZER_REMOVE<A, T, VERIFY>(plist, _alloc, id);

  TEST(pass);
}

template<template <typename> class A, typename T, int MAXSIZE, int TRIALS, typename... Args>
void TEST_ALLOC_FUZZER(TESTDEF::RETPAIR& __testret, Args... args)
{
  startflag.store(true);
  for(int k = 0; k<10; ++k)
  {
    A<T> _alloc(args...);
    TEST_ALLOC_FUZZER_THREAD<A, T, MAXSIZE, TRIALS, true>(__testret, _alloc, 1);
  }
}

template<template <typename> class A, typename T, int MAXSIZE, int TRIALS, typename... Args>
void TEST_ALLOC_MT(TESTDEF::RETPAIR& __testret, Args... args)
{
  A<T> _alloc(args...);
  int NUM = std::thread::hardware_concurrency();
  VARARRAY(bun::Thread, threads, NUM);
  for(int j = 0; j < 5; ++j)
  {
    startflag.store(false);
    for(int i = 0; i < NUM; ++i)
      threads[i] = bun::Thread(&TEST_ALLOC_FUZZER_THREAD<A, T, MAXSIZE, TRIALS, false>, std::ref(__testret), std::ref(_alloc), i + 1);
    startflag.store(true);

    for(int i = 0; i < NUM; ++i)
      threads[i].join();

    _alloc.Clear();

    bun::CPU_Barrier();
    T* test = (T*)_alloc.allocate(1);
    bun::CPU_Barrier();
    _alloc.deallocate(test, 1);
    bun::CPU_Barrier();
    _alloc.Clear();
  }
}

#endif