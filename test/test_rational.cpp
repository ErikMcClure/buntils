// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cRational.h"

using namespace bss_util;

TESTDEF::RETPAIR test_RATIONAL()
{
  BEGINTEST;
  cRational<int> tr(1, 10);
  cRational<int> tr2(1, 11);
  cRational<int> tr3(tr + tr2);
  TEST(tr.N() == 1 && tr.D() == 10);
  TEST(tr2.N() == 1 && tr2.D() == 11);
  TEST(tr3.N() == 21 && tr3.D() == 110);
  tr3 = (tr - tr2);
  TEST(tr3.N() == 1 && tr3.D() == 110);
  tr3 = (tr*tr2);
  TEST(tr3.N() == 1 && tr3.D() == 110);
  tr3 = (tr / tr2);
  TEST(tr3.N() == 11 && tr3.D() == 10);
  tr3 = (tr + 3);
  TEST(tr3.N() == 31 && tr3.D() == 10);
  tr3 = (tr - 3);
  TEST(tr3.N() == -29 && tr3.D() == 10);
  tr3 = (tr * 3);
  TEST(tr3.N() == 3 && tr3.D() == 10);
  tr3 = (tr / 3);
  TEST(tr3.N() == 1 && tr3.D() == 30);
  TEST((tr<3));
  TEST(!(tr>3));
  TEST(!(tr<tr2));
  TEST((tr>tr2));
  TEST(!(tr == 3));
  TEST((tr != 3));
  TEST(!(tr == tr2));
  TEST((tr != tr2));
  ENDTEST;
}
