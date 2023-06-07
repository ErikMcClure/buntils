// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Array.h"

using namespace bun;

TESTDEF::RETPAIR test_ARRAY()
{
  BEGINTEST;

  Array<int> a(5);
  TEST(a.Capacity() == 5);
  a.Insert(5, 2);
  TEST(a.Capacity() == 6);
  TEST(a[2] == 5);
  a.Remove(1);
  TEST(a[1] == 5);
  a.SetCapacity(10);
  TEST(a[1] == 5);
  TEST(a.Capacity() == 10);

  {
    Array<int> e(0);
    Array<int> b(e);
    b = e;
    e.Insert(5, 0);
    e.Insert(4, 0);
    e.Insert(2, 0);
    e.Insert(3, 1);
    TEST(e.Capacity() == 4);
    int sol[] = { 2,3,4,5 };
    TESTARRAY(sol, return e[i] == sol[i];);
    Array<int> c(0);
    c = e;
    TESTARRAY(sol, return c[i] == sol[i];);
    Array<int> d(0);
    e = d;
    TEST(!e.Capacity());
    e += d;
    TEST(!e.Capacity());
    e = c;
    TESTARRAY(sol, return e[i] == sol[i];);
    e += d;
    TESTARRAY(sol, return e[i] == sol[i];);
    d += c;
    TESTARRAY(sol, return d[i] == sol[i];);
    e += c;
    int sol2[] = { 2,3,4,5,2,3,4,5 };
    TESTARRAY(sol, return e[i] == sol[i];);
  }

  auto f = [](Array<DEBUG_CDT<true>, uint32_t>& arr)->bool {
    for(size_t i = 0; i < arr.Capacity(); ++i)
      if(arr[i]._index != i)
        return false;
    return true;
  };
  auto f2 = [](Array<DEBUG_CDT<true>, uint32_t>& arr, uint32_t s) { for(uint32_t i = s; i < arr.Capacity(); ++i) arr[i]._index = i; };

  assert(!DEBUG_CDT_SAFE::Tracker.Length());
  {
    DEBUG_CDT<true>::count = 0;
    Array<DEBUG_CDT<true>, uint32_t> b(10);
    f2(b, 0);
    b.Remove(5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 5; i < b.Capacity(); ++i) TEST(b[i]._index == (i + 1));
    TEST(b.Capacity() == 9);
    TEST(DEBUG_CDT<true>::count == 9);
    f2(b, 0);
    b.SetCapacity(19);
    f2(b, 9);
    TEST(f(b));
    TEST(DEBUG_CDT<true>::count == 19);
    TEST(b.Capacity() == 19);
    Array<DEBUG_CDT<true>, uint32_t> c(b);
    TEST(f(c));
    TEST(DEBUG_CDT<true>::count == 38);
    b += c;
    for(size_t i = 0; i < 19; ++i) TEST(b[i]._index == i);
    for(size_t i = 19; i < 38; ++i) TEST(b[i]._index == (i - 19));
    TEST(DEBUG_CDT<true>::count == 57);
    b + c;
    f2(b, 0);
    b.Insert(DEBUG_CDT<true>(), 5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 6; i < b.Capacity(); ++i) TEST(b[i]._index == (i - 1));
    TEST(DEBUG_CDT<true>::count == 58);
    b.Insert(DEBUG_CDT<true>(), b.Capacity());
    TEST(DEBUG_CDT<true>::count == 59);
  }
  TEST(!DEBUG_CDT<true>::count);
  TEST(!DEBUG_CDT_SAFE::Tracker.Length());

  auto f3 = [](Array<DEBUG_CDT<false>, uint32_t>& arr)->bool {
    for(size_t i = 0; i < arr.Capacity(); ++i)
      if(arr[i]._index != i)
        return false;
    return true;
  };
  auto f4 = [](Array<DEBUG_CDT<false>, uint32_t>& arr, uint32_t s) { for(uint32_t i = s; i < arr.Capacity(); ++i) arr[i]._index = i; };
  {
    DEBUG_CDT<false>::count = 0;
    Array<DEBUG_CDT<false>, uint32_t> b(10);
    f4(b, 0);
    b.Remove(5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 5; i < b.Capacity(); ++i) TEST(b[i]._index == (i + 1));
    TEST(b.Capacity() == 9);
    TEST(DEBUG_CDT<false>::count == 9);
    f4(b, 0);
    b.SetCapacity(19);
    f4(b, 9);
    TEST(f3(b));
    TEST(DEBUG_CDT<false>::count == 19);
    TEST(b.Capacity() == 19);
    Array<DEBUG_CDT<false>, uint32_t> c(b);
    TEST(f3(c));
    TEST(DEBUG_CDT<false>::count == 38);
    b += c;
    for(size_t i = 0; i < 19; ++i) TEST(b[i]._index == i);
    for(size_t i = 19; i < 38; ++i) TEST(b[i]._index == (i - 19));
    TEST(DEBUG_CDT<false>::count == 57);
    b + c;
    f4(b, 0);
    b.Insert(DEBUG_CDT<false>(), 5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 6; i < b.Capacity(); ++i) TEST(b[i]._index == (i - 1));
    TEST(DEBUG_CDT<false>::count == 58);
    b.Insert(DEBUG_CDT<false>(), b.Capacity());
    TEST(DEBUG_CDT<false>::count == 59);
  }
  TEST(!DEBUG_CDT<false>::count);
  TEST(!DEBUG_CDT_SAFE::Tracker.Length());

  {
    int abc[2][3][4] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    ArrayMultiRef<int, int, int, int> ref((int*)abc, 2, 3, 4);
    auto f6 = [](int z, int y, int x) { return x + 4 * (y + 3 * z); };
    TEST(f6(0, 0, 0) == abc[0][0][0]);
    TEST(f6(1, 2, 3) == abc[1][2][3]);
    TEST(f6(1, 0, 3) == abc[1][0][3]);
    TEST(ref.GetIndice(0, 0, 0) == f6(0, 0, 0));
    TEST(ref.GetIndice(1, 2, 3) == f6(1, 2, 3));
    TEST(ref.GetIndice(1, 0, 3) == f6(1, 0, 3));
    TEST(ref(0, 0, 0) == abc[0][0][0]);
    TEST(ref(1, 2, 3) == abc[1][2][3]);
    TEST(ref(1, 0, 3) == abc[1][0][3]);
    for(int z = 0; z < 2; ++z)
      for(int y = 0; y < 3; ++y)
        for(int x = 0; x < 4; ++x)
          TEST(ref(z, y, x) == abc[z][y][x]);
  }

  VARARRAY(char, vtest, 6);

  int testcounts[] = { 0, 1, 7, 15, 0xFFFF + 1 };
  for(auto i : testcounts)
  {
    {
      DEBUG_CDT<true>::count = 0;
      VARARRAY(DEBUG_CDT<true>, vtest, i); // Note: This leaks memory, but this is a test so we ignore that.
      TEST(DEBUG_CDT<true>::count == i);
    }
    TEST(!DEBUG_CDT<true>::count);
  }

  ENDTEST;
}
