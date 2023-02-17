// Copyright �2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/ArrayCircular.h"

using namespace bun;

TESTDEF::RETPAIR test_ARRAYCIRCULAR()
{
  BEGINTEST;
  ArrayCircular<int> a;
  a.SetCapacity(25);
  TEST(a.Capacity() == 25);
  for(int i = 0; i < 25; ++i)
    a.Push(i);
  TEST(a.Length() == 25);

  {
    int i = 25;
    for(auto v : a)
      TEST(v == --i);
  }
  TEST(a.Pop() == 24);
  TEST(a.Pop() == 23);
  a.Push(987);
  TEST(a.Pop() == 987);
  TEST(a.Length() == 23);
  a.Push(23);
  a.Push(24);
  for(int i = 0; i < 50; ++i)
    TEST(a[i] == (24 - (i % 25)));
  for(int i = 1; i < 50; ++i)
    TEST(a[-i] == ((i - 1) % 25));
  a.SetCapacity(26);
  for(int i = 0; i < 25; ++i)
    TEST(a[i] == (24 - (i % 25)));
  a.Push(25); //This should overwrite 0
  TEST(a[0] == 25);
  TEST(a[-1] == 0);

  {
    int i = 26;
    const auto& ref = a;
    for(auto& v : ref)
      TEST(v == --i);
  }

  //const ArrayCircular<int>& b=a;
  //b[0]=5; // Should cause error

  ENDTEST;
}