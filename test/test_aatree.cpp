// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/AATree.h"
#include "test.h"
#include "bss-util/bss_algo.h"
#include <iostream>

using namespace bss;

TESTDEF::RETPAIR test_AA_TREE()
{
  BEGINTEST;

  BlockPolicy<AANODE<int>> fixedaa;
  AATree<int, CompT<int>, BlockPolicy<AANODE<int>>> aat(&fixedaa);

  XorshiftEngine64 e;

  Shuffle(testnums, TESTNUM, e);

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    aat.Insert(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums, TESTNUM, e);
  //prof=HighPrecisionTimer::OpenProfiler();
  uint32_t c = 0;
  for(int i = 0; i<TESTNUM; ++i)
    c += (aat.Get(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums, TESTNUM, e);
  //prof=HighPrecisionTimer::OpenProfiler();
  c = 0;
  for(int i = 0; i<TESTNUM; ++i)
  {
    //if(testnums[i] == 20159)
    //  std::cout << (aat.Get(testnums[i])!=0) << std::endl;

    if(!aat.Remove(testnums[i]))
    {
      std::cout << testnums[i] << std::endl;
    }
    else
    {
      c++;
    }

    //c+=aat.Remove(testnums[i]);
  }
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  TEST(aat.GetRoot() == 0);
  aat.Clear();

  c = 0;
  for(int i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c += (aat.Get(testnums[i]) == 0);
  TEST(c == TESTNUM);

  ENDTEST;
}
