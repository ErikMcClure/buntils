// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cBitField.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_BITFIELD()
{
  BEGINTEST;
  cBitField<uint32_t> t;
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
  ENDTEST;
}
