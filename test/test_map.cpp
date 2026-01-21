// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Map.h"
#include <algorithm>
#include <map>

using namespace bun;

TESTDEF::RETPAIR test_MAP()
{
  BEGINTEST;
  Map<int, uint32_t> test;
  test.Clear();
  int ins[] = { 0,5,6,237,289,12,3 };
  int get[] = { 0,3,5,6,12 };
  uint32_t res[] = { 0,6,1,2,5,3,4 };
  uint32_t count = 0;
  TESTARRAY(ins, return test.Insert(ins[i], count++) != -1;);
  std::sort(std::begin(ins), std::end(ins));
  for(size_t i = 0; i < std::ranges::size(get); ++i)
  {
    auto[k, v] = test[test.Get(get[i])];
    TEST(v == res[i]);
  }

  size_t j = 0;
  for(auto[k, v] : test)
  {
    TEST(k == ins[j]);
    TEST(v == res[j]);
    ++j;
  }

  TEST(test.Remove(0) == 0);
  TEST(test.Get(0) == -1);
  TEST(test.size() == (std::ranges::size(ins) - 1));

#ifndef BUN_COMPILER_GCC // Once again, GCC demonstrates its amazing ability to NOT DEFINE ANY FUCKING CONSTRUCTORS
  Map<int, FWDTEST> tst;
  tst.Insert(0, FWDTEST());
  FWDTEST lval;
  tst.Insert(1, lval);
#endif
  ENDTEST;
}
