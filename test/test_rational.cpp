// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Rational.h"

using namespace bun;

TESTDEF::RETPAIR test_RATIONAL()
{
  BEGINTEST;
  Rational<int> tr(1, 10);
  Rational<int> tr2(1, 11);
  Rational<int> tr3(tr + tr2);
  TEST(tr.N() == 1 && tr.D() == 10);
  TEST(tr2.N() == 1 && tr2.D() == 11);
  TEST(tr3.N() == 21 && tr3.D() == 110);
  tr3 = (tr - tr2);
  TEST(tr3.N() == 1 && tr3.D() == 110);
  tr3 = (tr * tr2);
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
  TEST((tr < 3));
  TEST(!(tr > 3));
  TEST(!(tr < tr2));
  TEST((tr > tr2));
  TEST(!(tr == 3));
  TEST((tr != 3));
  TEST(!(tr == tr2));
  TEST((tr != tr2));
  ENDTEST;
}
