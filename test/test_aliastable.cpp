// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cAliasTable.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_ALIASTABLE()
{
  BEGINTEST;
  double p[7] = { 0.1,0.2,0.3,0.05,0.05,0.15,0.15 };
  cAliasTable<uint32_t, double> a(p);
  uint32_t counts[7] = { 0 };
  for(uint32_t i = 0; i < 10000000; ++i)
    ++counts[a()];
  double real[7] = { 0.0 };
  for(uint32_t i = 0; i < 7; ++i)
    real[i] = counts[i] / 10000000.0;
  for(uint32_t i = 0; i < 7; ++i)
    TEST(fcompare(p[i], real[i], (1LL << 45)));

  ENDTEST;
}