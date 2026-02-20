// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Variant.h"

using namespace bun;

using VTYPE = Variant<int, bool, Variant<double, Str>, Str>;

int test_VTYPE(VTYPE& v)
{
  switch(v.tag())
  {
  case VTYPE::Type<int>::value: return 0;
  case VTYPE::Type<bool>::value: return 1;
  case VTYPE::Type<Variant<double, Str>>::value: return 2;
  case VTYPE::Type<Str>::value: return 3;
  }
  return -1;
}

TESTDEF::RETPAIR test_VARIANT()
{
  BEGINTEST;
  {
    VTYPE v(true);
    VTYPE v2(Str("string"));
    VTYPE v3(Variant<double, Str>(0.5));

    TEST(v.get<bool>());
    TEST(v2.get<Str>() == "string");
    TEST((v3.get<Variant<double, Str>>().get<double>() == 0.5));
    TEST(test_VTYPE(v) == 1);
    TEST(test_VTYPE(v2) == 3);
    TEST(test_VTYPE(v3) == 2);

    VTYPE v4(v2);
    TEST(v2.get<Str>() == "string");
    TEST(v4.get<Str>() == "string");
    VTYPE v5(std::move(v2));
    TEST(v5.get<Str>() == "string");
    const VTYPE v6(3);
    TEST(v6.get<int>() == 3);
    v2 = v3;
    TEST((v3.get<Variant<double, Str>>().get<double>() == 0.5));
    TEST((v2.get<Variant<double, Str>>().get<double>() == 0.5));
    v = std::move(v4);
    TEST(v.get<Str>() == "string");
    v3 = Str("string");
    TEST(v3.get<Str>() == "string");
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
    DEBUG_CDT<true>::count  = 0;
    using VDEBUG            = Variant<DEBUG_CDT<true>, DEBUG_CDT<false>>;
    // DEBUG_CDT<true> d1;
    DEBUG_CDT<false> d2;
    // const DEBUG_CDT<true> d3;
    const DEBUG_CDT<false> d4;
    // DEBUG_CDT<true> d5;
    DEBUG_CDT<false> d6;
    // const DEBUG_CDT<true> d7;
    const DEBUG_CDT<false> d8;
    // VDEBUG vd1(d1);
    VDEBUG vd2(d2);
    // VDEBUG vd3(std::move(d1));
    VDEBUG vd4(std::move(d2));
    // VDEBUG vd5(d3);
    VDEBUG vd6(d4);
    // VDEBUG vd7(std::move(d3));
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
    VTYPE v3(Str("test"));
    v = std::move(v3);
  }

  TEST(!DEBUG_CDT<false>::count);
  TEST(!DEBUG_CDT<true>::count);

  {
    Variant<double, float, int64_t> ctest(5.9);
    TEST(ctest.convert<int>() == 5);
  }
  ENDTEST;
}