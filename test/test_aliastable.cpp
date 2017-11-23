// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/AliasTable.h"

using namespace bss;

TESTDEF::RETPAIR test_ALIASTABLE()
{
  BEGINTEST;
  const size_t COUNT = 4000000;
  double p[7] = { 0.1,0.2,0.3,0.05,0.05,0.15,0.15 };
  AliasTable<size_t, double> a(p);
  size_t counts[7] = { 0 };
  for(size_t i = 0; i < COUNT; ++i)
    ++counts[a()];
  double real[7] = { 0.0 };
  for(size_t i = 0; i < 7; ++i)
    real[i] = counts[i] / double(COUNT);
  for(size_t i = 0; i < 7; ++i)
    TEST(fCompare(p[i], real[i], (1LL << 45)));
  a(bss_getdefaultengine());
  a.Get(bss_getdefaultengine());

  AliasTable<size_t, double> b;
  b = AliasTable<size_t, double>(p);
  bssFill(counts);
  for(size_t i = 0; i < COUNT; ++i)
    ++counts[b()];
  for(size_t i = 0; i < 7; ++i)
    real[i] = counts[i] / double(COUNT);
  for(size_t i = 0; i < 7; ++i)
    TEST(fCompare(p[i], real[i], (1LL << 46)));

  ENDTEST;
}