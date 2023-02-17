// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/AATree.h"
#include "buntils/algo.h"
#include <iostream>

using namespace bun;

TESTDEF::RETPAIR test_AA_TREE()
{
  BEGINTEST;

  BlockPolicy<AANODE<int>> fixedaa;
  AATree<int, CompT<int>, PolymorphicAllocator<AANODE<int>, BlockPolicy>> aat(&fixedaa);

  XorshiftEngine64 e;

  Shuffle(testnums, TESTNUM, e);

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    aat.Insert(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums, TESTNUM, e);
  //prof=HighPrecisionTimer::OpenProfiler();
  size_t c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
    c += (aat.Get(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums, TESTNUM, e);
  //prof=HighPrecisionTimer::OpenProfiler();
  c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
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
  for(size_t i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c += (aat.Get(testnums[i]) == 0);
  TEST(c == TESTNUM);

  ENDTEST;
}
