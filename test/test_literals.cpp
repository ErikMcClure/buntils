// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/literals.h"
#include "test.h"

using namespace bss;

TESTDEF::RETPAIR test_LITERALS()
{
  BEGINTEST;
  static_assert(std::is_same<decltype(0_sz), size_t>::value, "_sz literal not casting type");
  static_assert(std::is_same<decltype(1_recip), bss::Rational<int64_t>>::value, "_recip literal not casting type");
  TESTSTATIC(0_sz == (size_t)0);
  TESTSTATIC(1_sz == (size_t)1);
  TESTSTATIC(~0_sz == (size_t)~0);
  TEST(5_recip == bss::Rational<int64_t>(1, 5));
  TEST(1_recip == 1LL);
  TESTSTATIC(1_sz == (size_t)1);
  TESTSTATIC(0_deg == 0.0);
  TESTSTATIC(1_deg == (PI / 180.0));
  TESTSTATIC(180_deg == PI);
  TESTSTATIC(360_deg == PI*2.0);
  TESTSTATIC(0_rad == 0.0);
  TESTSTATIC(1_rad == (180.0 / PI));
  ENDTEST;
}