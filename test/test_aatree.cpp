// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cAATree.h"
#include "bss_algo.h"

using namespace bss_util;

TESTDEF::RETPAIR test_AA_TREE()
{
  BEGINTEST;

  BlockPolicy<AANODE<int>> fixedaa;
  cAATree<int, CompT<int>, BlockPolicy<AANODE<int>>> aat(&fixedaa);

  xorshift_engine64 e;

  shuffle(testnums, TESTNUM, e);

  //uint64_t prof=cHighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    aat.Insert(testnums[i]);
  //std::cout << cHighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle(testnums, TESTNUM, e);
  //prof=cHighPrecisionTimer::OpenProfiler();
  uint32_t c = 0;
  for(int i = 0; i<TESTNUM; ++i)
    c += (aat.Get(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << cHighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle(testnums, TESTNUM, e);
  //prof=cHighPrecisionTimer::OpenProfiler();
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
  //std::cout << cHighPrecisionTimer::CloseProfiler(prof) << std::endl;

  TEST(aat.GetRoot() == 0);
  aat.Clear();

  c = 0;
  for(int i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c += (aat.Get(testnums[i]) == 0);
  TEST(c == TESTNUM);

  ENDTEST;
}
