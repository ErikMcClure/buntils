// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/lockless.h"

using namespace bun;

TESTDEF::RETPAIR test_LOCKLESS()
{
  BEGINTEST;
  CPU_Barrier();
  alignas(16) bun_PTag<void> a{ .tag = 2 };
  alignas(16) bun_PTag<void> b{ .tag = 1 };
  bun_PTag<void> last{};
  bun_PTag<void> old{};

  TEST(asmcasr(&a, old, old, last) == false);
  TEST(a.tag == 2);
  TEST(asmcasr(&a, b, a, last) == true);
  TEST(a.tag == 1);
  TEST(asmcasr(&a, old, old, last) == false);
  TEST(a.tag == 1);
  TEST(asmcasr(&a, old, b, last) == true);
  TEST(a.tag == 0);

  ENDTEST;
}