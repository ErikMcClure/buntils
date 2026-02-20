// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/ArraySort.h"

using namespace bun;

TESTDEF::RETPAIR test_ARRAYSORT()
{
  BEGINTEST;

  DEBUG_CDT<true>::count = 0;

  {
    ArraySort<DEBUG_CDT<true>, std::compare_three_way, uint32_t> arrtest;
    arrtest.Insert(DEBUG_CDT<true>(0));
    arrtest.Insert(DEBUG_CDT<true>(1));
    arrtest.Insert(DEBUG_CDT<true>(2));
    arrtest.Remove(2);
    arrtest.Insert(DEBUG_CDT<true>(3));
    arrtest.Insert(DEBUG_CDT<true>(4));
    arrtest.Insert(DEBUG_CDT<true>(5));
    arrtest.Remove(0);
    arrtest.Insert(DEBUG_CDT<true>(6));
    arrtest.Remove(3);

    TEST(arrtest[0] == 1);
    TEST(arrtest[1] == 3);
    TEST(arrtest[2] == 4);
    TEST(arrtest[3] == 6);

    std::for_each(arrtest.begin(), arrtest.end(), [](DEBUG_CDT<true>& d) { d._index += 1; });
    TEST(arrtest[0] == 2);
    TEST(arrtest[1] == 4);
    TEST(arrtest[2] == 5);
    TEST(arrtest[3] == 7);

    ArraySort<DEBUG_CDT<true>, std::compare_three_way, uint32_t> arrtest2;
    arrtest2.Insert(DEBUG_CDT<true>(7));
    arrtest2.Insert(DEBUG_CDT<true>(8));
    arrtest = arrtest2;
  }
  TEST(!DEBUG_CDT<true>::count)

  ArraySort<int> slicetest;
  int slices[4] = { 0, 1, 2, 3 };
  slicetest     = std::span<int>(slices, 4);
  TEST(slicetest.size() == 4);
  ENDTEST;
}
