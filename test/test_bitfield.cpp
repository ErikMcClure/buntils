// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/BitField.h"
#include "test.h"

using namespace bss;

TESTDEF::RETPAIR test_BITFIELD()
{
  BEGINTEST;
  BitField<uint32_t> t;
  TEST(t == 0);
  t[1] = true;
  TEST(t == 1);
  t[2] = false;
  TEST(t == 1);
  t[3] = t[1];
  TEST(t == 3);
  t[2] = false;
  TEST(t == 1);
  t[4] = true;
  TEST(t == 5);
  t[8] = true;
  TEST(t == 13);
  t = 5;
  TEST(t == 5);
  t += 3;
  TEST(t == 7);
  t -= 1;
  TEST(t == 6);
  t -= 3;
  TEST(t == 4);
  t += 7;
  TEST(t == 7);
  t -= 4;
  TEST(t == 3);
  t.Set(3, 3);
  TEST(t == 3);
  t.Set(5, -1);
  TEST(t == 5);

  const int GROUP1 = 0b1100;
  const int GROUP2 = 0b11;
  BitField<int> g;
  g(GROUP1) = 3;
  TEST(!g(GROUP1));
  TEST(!g.Get());
  TEST(g(GROUP1) == 0);
  g(GROUP1) = (3 << 2);
  TEST(!!g(GROUP1));
  TEST(g(GROUP1) == (3 << 2));
  TEST(g.GetGroup(GROUP1) == (3 << 2));
  g.GetGroup(GROUP1) = (2 << 2);
  TEST(g.Get() == (2 << 2));
  g(GROUP2) = 3;
  TEST(g(GROUP1) == (2 << 2));
  TEST(g(GROUP2) == 3);
  TEST(g.Get() == ((2 << 2) | 3));
  g(GROUP1) = 0;
  TEST(g.Get() == 3);
  ENDTEST;
}
