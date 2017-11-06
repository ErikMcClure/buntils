// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_TEST_ALLOC_H__
#define __BSS_TEST_ALLOC_H__

#include "bss-util/DynArray.h"
#include "bss-util/algo.h"
#include "test.h"

template<class T, typename P, int MAXSIZE, int TRIALS>
void TEST_ALLOC_FUZZER_THREAD(TESTDEF::RETPAIR& __testret, T& _alloc, bss::DynArray<std::tuple<P*, size_t>>& plist)
{
  for(int j = 0; j<5; ++j)
  {
    bool pass = true;
    for(size_t k = 0; k < TRIALS; ++k)
    {
      if(bss::bssRandInt(0, 10)<5 || plist.Length()<3)
      {
        size_t sz = (size_t)bss::bssRandInt(1, MAXSIZE);
        P* test = (P*)_alloc.Alloc(sz);
        for(size_t i = 0; i<sz; ++i) *(char*)(test + i) = (char)(i + 1);
        plist.Add({ test, sz });
      }
      else
      {
        size_t index = (size_t)bss::bssRandInt(0, plist.Length());
        auto& [p, s] = plist[index];
        for(size_t i = 0; i<s; ++i)
        {
          if(((char)(i + 1) != *(char*)(p + i)))
            pass = false;
        }
        bss::bssFillN(p, s);
        _alloc.Dealloc(p);
        std::swap(plist.Back(), plist[index]);
        plist.RemoveLast(); // This technique lets us randomly remove items from the array without having to move large chunks of data by swapping the invalid element with the last one and then removing the last element (which is cheap)
      }
    }
    while(plist.Length())
    {
      auto[p, s] = plist.Back();
      bss::bssFillN(p, s);
      _alloc.Dealloc(p);
      plist.RemoveLast();
    }
    TEST(pass);
    plist.Clear(); // BOY I SHOULD PROBABLY CLEAR THIS BEFORE I PANIC ABOUT INVALID MEMORY ALLOCATIONS, HUH?
    _alloc.Clear();
  }

  bss::CPU_Barrier();
  P* test = (P*)_alloc.Alloc(1);
  bss::CPU_Barrier();
  _alloc.Dealloc(test);
  bss::CPU_Barrier();
  _alloc.Clear();
}

template<class T, typename P, int MAXSIZE, int TRIALS>
void TEST_ALLOC_FUZZER(TESTDEF::RETPAIR& __testret)
{
  bss::DynArray<std::tuple<P*, size_t>> plist;
  for(int k = 0; k<10; ++k)
  {
    T _alloc;
    TEST_ALLOC_FUZZER_THREAD<T, P, MAXSIZE, TRIALS>(__testret, _alloc, plist);
  }
}

template<class T, typename P, int TRIALS>
void TEST_ALLOC_MT(TESTDEF::RETPAIR& pair, T& p)
{
  while(!startflag.load());
  bss::DynArray<std::tuple<P*, size_t>> plist;
  TEST_ALLOC_FUZZER_THREAD<T, P, 1, TRIALS>(pair, p, plist);
}

#endif