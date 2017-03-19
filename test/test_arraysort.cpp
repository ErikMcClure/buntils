// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cArraySort.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_ARRAYSORT()
{
  BEGINTEST;

  DEBUG_CDT<true>::count = 0;

  {
    cArraySort<DEBUG_CDT<true>, CompT<DEBUG_CDT<true>>, uint32_t, CARRAY_SAFE> arrtest;
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

    cArraySort<DEBUG_CDT<true>, CompT<DEBUG_CDT<true>>, uint32_t, CARRAY_SAFE> arrtest2;
    arrtest2.Insert(DEBUG_CDT<true>(7));
    arrtest2.Insert(DEBUG_CDT<true>(8));
    arrtest = arrtest2;
  }
  TEST(!DEBUG_CDT<true>::count)

    cArraySort<int> slicetest;
  int slices[4] = { 0, 1, 2, 3 };
  slicetest = cArraySlice<int>(slices, 4);
  TEST(slicetest.Length() == 4);
  ENDTEST;
}
