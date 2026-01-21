// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Singleton.h"

using namespace bun;

#ifdef BUN_COMPILER_MSC
struct SINGLETEST : Singleton<SINGLETEST>
{
  SINGLETEST() {}
  static SINGLETEST* Instance() { return _instance; }
};

template<>
SINGLETEST* Singleton<SINGLETEST>::_instance = 0;

TESTDEF::RETPAIR test_SINGLETON()
{
  BEGINTEST;
  TEST(SINGLETEST::Instance() == 0);
  {
    SINGLETEST test;
    TEST(SINGLETEST::Instance() == &test);
  }
  TEST(SINGLETEST::Instance() == 0);
  SINGLETEST* test2;
  {
    SINGLETEST test;
    TEST(SINGLETEST::Instance() == &test);
    test2 = new SINGLETEST();
    TEST(SINGLETEST::Instance() == test2);
  }
  TEST(SINGLETEST::Instance() == test2);
  delete test2;
  TEST(SINGLETEST::Instance() == 0);
  ENDTEST;
}
#else

TESTDEF::RETPAIR test_SINGLETON()
{
  BEGINTEST;
  ENDTEST;
}

#endif