// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/AliasTable.h"

using namespace bun;

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
  a(bun_getdefaultengine());
  a.Get(bun_getdefaultengine());

  AliasTable<size_t, double> b;
  b = AliasTable<size_t, double>(p);
  bun_Fill(counts);
  for(size_t i = 0; i < COUNT; ++i)
    ++counts[b()];
  for(size_t i = 0; i < 7; ++i)
    real[i] = counts[i] / double(COUNT);
  for(size_t i = 0; i < 7; ++i)
    TEST(fCompare(p[i], real[i], (1LL << 46)));

  ENDTEST;
}