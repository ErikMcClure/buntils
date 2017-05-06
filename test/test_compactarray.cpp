// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/CompactArray.h"
#include "test.h"

using namespace bss;

static_assert(sizeof(CompactArray<size_t>) == sizeof(size_t) * 3, "CompactArray is not structured correctly");
static_assert(sizeof(CompactArray<size_t, 1>) == sizeof(size_t) * 3, "CompactArray is not structured correctly");
static_assert(sizeof(CompactArray<size_t, 3>) == sizeof(size_t) * 4, "CompactArray is not structured correctly");

TESTDEF::RETPAIR test_COMPACTARRAY()
{
  BEGINTEST;

  CompactArray<size_t> x;
  TEST(x.begin() != 0);
  TEST(x.Capacity() == 2);
  TEST(x.Length() == 0);
  x.Add(99);
  TEST(x.Front() == 99);
  TEST(x.Back() == 99);
  TEST(x[0] == 99);
  TEST(x.Capacity() == 2);
  TEST(x.Length() == 1);
  x.Insert(98, 1);
  TEST(x[0] == 99);
  TEST(x[1] == 98);
  TEST(x.Front() == 99);
  TEST(x.Back() == 98);
  TEST(x.Capacity() == 2);
  TEST(x.Length() == 2);
  x.RemoveLast();
  TEST(x.Capacity() == 2);
  TEST(x.Length() == 1);
  x.Insert(98, 0);
  TEST(x[0] == 98);
  TEST(x[1] == 99);
  TEST(x.Capacity() == 2);
  TEST(x.Length() == 2);
  x.Insert(97, 1);
  TEST(x[0] == 98);
  TEST(x[1] == 97);
  TEST(x[2] == 99);
  TEST(x.Front() == 98);
  TEST(x.Back() == 99);
  TEST(x.Capacity() > 2);
  TEST(x.Length() == 3);
  x.RemoveLast();
  TEST(x.Length() == 2);

  x.SetLength(5);
  x[0] = 1;
  x[1] = 2;
  x[2] = 3;
  x[3] = 4;
  x[4] = 5;
  x.Add(6);
  TEST(x[5] == 6);
  x.Add(7);
  TEST(x[5] == 6);
  TEST(x[6] == 7);
  x.Add(8);
  TEST(x[7] == 8);
  x.Remove(5);
  TEST(x[5] == 7);
  TEST(x[6] == 8);
  x.Insert(6, 5);
  TEST(x[5] == 6);
  TEST(x[6] == 7);
  CompactArray<int> y;
  y.Add(9);
  y.Add(11);
  
  int zvars[5] = { 4, 3, 2, 1, 5 };
  y.Set(zvars, 3);
  TEST(y.Length() == 3);
  TEST(y[0] == 4);
  TEST(y[1] == 3);
  TEST(y[2] == 2);

  ENDTEST;
}