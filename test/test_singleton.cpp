// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Singleton.h"
#include "test.h"

using namespace bss;

struct SINGLETEST : Singleton<SINGLETEST>
{
  SINGLETEST() : Singleton<SINGLETEST>(this) {}
};

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
