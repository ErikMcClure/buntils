// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "variant.h"
#include "test.h"

using namespace bss_util;

typedef variant<int, bool, variant<double, cStr>, cStr> VTYPE;

int test_VTYPE(VTYPE& v)
{
  switch(v.tag())
  {
  case VTYPE::Type<int>::value: return 0;
  case VTYPE::Type<bool>::value: return 1;
  case VTYPE::Type<variant<double, cStr>>::value: return 2;
  case VTYPE::Type<cStr>::value: return 3;
  }
  return -1;
}

TESTDEF::RETPAIR test_VARIANT()
{
  BEGINTEST;
  {
    VTYPE v(true);
    VTYPE v2(cStr("string"));
    VTYPE v3(variant<double, cStr>(0.5));

    TEST(v.get<bool>());
    TEST(v2.get<cStr>() == "string");
    TEST((v3.get<variant<double, cStr>>().get<double>() == 0.5));
    TEST(test_VTYPE(v) == 1);
    TEST(test_VTYPE(v2) == 3);
    TEST(test_VTYPE(v3) == 2);

    VTYPE v4(v2);
    TEST(v2.get<cStr>() == "string");
    TEST(v4.get<cStr>() == "string");
    VTYPE v5(std::move(v2));
    TEST(v5.get<cStr>() == "string");
    const VTYPE v6(3);
    TEST(v6.get<int>() == 3);
    v2 = v3;
    TEST((v3.get<variant<double, cStr>>().get<double>() == 0.5));
    TEST((v2.get<variant<double, cStr>>().get<double>() == 0.5));
    v = std::move(v4);
    TEST(v.get<cStr>() == "string");
    v3 = cStr("string");
    TEST(v3.get<cStr>() == "string");
    v2 = v6;
    TEST(v2.get<int>() == 3);
    v4 = 10;
    TEST(v4.get<int>() == 10);
    v5 = -1;
    TEST(v5.get<int>() == -1);
    v4 = false;
    TEST(!v4.get<bool>());

    v3.typeset<int>();
    TEST(v3.is<int>());
    VTYPE v7(v6);
  }

  {
    DEBUG_CDT<false>::count = 0;
    DEBUG_CDT<true>::count = 0;
    typedef variant<DEBUG_CDT<true>, DEBUG_CDT<false>> VDEBUG;
    //DEBUG_CDT<true> d1;
    DEBUG_CDT<false> d2;
    //const DEBUG_CDT<true> d3;
    const DEBUG_CDT<false> d4;
    //DEBUG_CDT<true> d5;
    DEBUG_CDT<false> d6;
    //const DEBUG_CDT<true> d7;
    const DEBUG_CDT<false> d8;
    //VDEBUG vd1(d1);
    VDEBUG vd2(d2);
    //VDEBUG vd3(std::move(d1));
    VDEBUG vd4(std::move(d2));
    //VDEBUG vd5(d3);
    VDEBUG vd6(d4);
    //VDEBUG vd7(std::move(d3));
    VDEBUG vd8(std::move(d4));
    vd2 = d6;
    vd4 = std::move(d6);
    vd6 = d8;
    vd8 = std::move(d8);
  }

  {
    VTYPE v;
  }

  {
    VTYPE v;
    VTYPE v2(32);
    v = v2;
  }

  {
    VTYPE v;
    VTYPE v3(cStr("test"));
    v = std::move(v3);
  }

  TEST(!DEBUG_CDT<false>::count);
  TEST(!DEBUG_CDT<true>::count);

  {
    variant<double, float, int64_t> ctest(5.9);
    TEST(ctest.convert<int>() == 5);
  }
  ENDTEST;
}