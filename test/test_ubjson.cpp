// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/UBJSON.h"
#include <fstream>

using namespace bun;

struct ubjsontest2
{
  int a;
  Str c;
  double d;

  template<typename Engine>
  void Serialize(Serializer<Engine>& e, const char*)
  {
    e.template EvaluateType<ubjsontest2>(
      GenPair("a", a),
      GenPair("c", c),
      GenPair("d", d)
      );
  }
};

enum TEST_ENUM : char {
  TEST_ENUM_VALUE = 1
};

struct ubjsontest
{
  using VAR = Variant<ubjsontest2, double>;

  TEST_ENUM a;
  short b;
  int c;
  int64_t d;
  uint8_t e;
  uint16_t f;
  uint32_t g;
  uint64_t h;
  ubjsontest2 i;
  float x;
  double y;
  Str p;
  int m[3];
  std::string n[2];
  std::vector<int> u;
  DynArray<bool> v;
  DynArray<Str, size_t, ARRAY_SAFE> w;
  DynArray<ubjsontest2, size_t, ARRAY_SAFE> z;
  std::vector<VAR> var;
  std::tuple<short, Str, double> tuple;

  template<typename Engine>
  void Serialize(Serializer<Engine>& engine, const char*)
  {
    engine.template EvaluateType<ubjsontest>(
      GenPair("a", (char&)a),
      GenPair("b", b),
      GenPair("c", c),
      GenPair("d", d),
      GenPair("e", e),
      GenPair("f", f),
      GenPair("g", g),
      GenPair("h", h),
      GenPair("i", i),
      GenPair("x", x),
      GenPair("y", y),
      GenPair("p", p),
      GenPair("m", m),
      GenPair("n", n),
      GenPair("u", u),
      GenPair("v", v),
      GenPair("w", w),
      GenPair("z", z),
      GenPair("var", var),
      GenPair("tuple", tuple)
      );
  }
};

void VerifyUBJSON(const ubjsontest& t1, const ubjsontest& t2, TESTDEF::RETPAIR& __testret)
{
  TEST(t1.a == t2.a);
  TEST(t1.b == t2.b);
  TEST(t1.c == t2.c);
  TEST(t1.d == t2.d);
  TEST(t1.e == t2.e);
  TEST(t1.f == t2.f);
  TEST(t1.g == t2.g);
  TEST(t1.h == t2.h);
  TEST(t1.i.a == t2.i.a);
  TEST(t1.i.c == t2.i.c);
  TEST(t1.i.d == t2.i.d);
  TEST(t1.x == t2.x);
  TEST(t1.y == t2.y);
  TEST(t1.p == t2.p);
  TEST(t1.m[0] == t2.m[0]);
  TEST(t1.m[1] == t2.m[1]);
  TEST(t1.m[2] == t2.m[2]);
  TEST(t1.n[0] == t2.n[0]);
  TEST(t1.n[1] == t2.n[1]);
  TEST(t1.u.size() == t2.u.size());
  for(size_t i = 0; i < t1.u.size(); ++i)
    TEST(t1.u[i] == t2.u[i]);

  TEST(t1.v.Length() == t2.v.Length());
  for(size_t i = 0; i < t1.v.Length(); ++i)
    TEST(t1.v[i] == t2.v[i]);

  TEST(t1.w.Length() == t2.w.Length());
  for(size_t i = 0; i < t1.w.Length(); ++i)
    TEST(t1.w[i] == t2.w[i]);

  TEST(t1.z.Length() == t2.z.Length());
  for(size_t i = 0; i < t1.z.Length(); ++i)
  {
    TEST(t1.z[i].a == t2.z[i].a);
    TEST(t1.z[i].c == t2.z[i].c);
    TEST(t1.z[i].d == t2.z[i].d);
  }

  TEST(t1.var.size() == t2.var.size());
  for(size_t i = 0; i < t1.var.size(); ++i)
  {
    TEST(t1.var[i].tag() == t2.var[i].tag());
    switch(t1.var[i].tag())
    {
    case ubjsontest::VAR::Type<ubjsontest2>::value:
      TEST(t1.var[i].get<ubjsontest2>().a == t2.var[i].get<ubjsontest2>().a);
      TEST(t1.var[i].get<ubjsontest2>().c == t2.var[i].get<ubjsontest2>().c);
      TEST(t1.var[i].get<ubjsontest2>().d == t2.var[i].get<ubjsontest2>().d);
      break;
    case ubjsontest::VAR::Type<double>::value:
      TEST(t1.var[i].get<double>() == t2.var[i].get<double>());
      break;
    }
  }

  auto[a1, b1, c1] = t1.tuple;
  auto[a2, b2, c2] = t2.tuple;
  TEST(a1 == a2);
  TEST(b1 == b2);
  TEST(c1 == c2);
}

TESTDEF::RETPAIR test_UBJSON()
{
  BEGINTEST;
  ubjsontest t1 = { TEST_ENUM_VALUE, -2, 3, -4, 253, 30000, 7, 8,
    { 9, "foo", 10.0 }, 11.0f, 12.0, "bar",
    { 13, 14, 15 },
    { "fizz", "buzz" },
    { 16, 17, 18, 19, 20 },
    { true, false, true, false, false, true },
    { "stuff", "crap", "things" },
    { { 21, "22", 23.0 }, { 24, "25", 26.0 } },
    { ubjsontest::VAR(ubjsontest2{ 27, "28", 29.0 }), ubjsontest::VAR(30.0) },
    { 31, "32", 33.0 }
  };

  {
    Serializer<UBJSONEngine> s;
    std::fstream fso("out.ubj", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    s.Serialize(t1, fso);
  }

  {
    Serializer<UBJSONEngine> s;
    ubjsontest t2 = { TEST_ENUM_VALUE, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, "", {0,0,0}, {"",""} };
    std::fstream fsi("out.ubj", std::ios_base::in | std::ios_base::binary);
    s.Parse(t2, fsi);
    fsi.close();

    VerifyUBJSON(t1, t2, __testret);
  }

  UBJSONTuple val;
  {
    std::fstream fsi2("out.ubj", std::ios_base::in | std::ios_base::binary);
    Serializer<UBJSONEngine> s;
    s.Parse<UBJSONTuple>(val, fsi2);
  }
  TEST(val.Type == UBJSONTuple::TYPE_OBJECT);
  auto& var1 = val.Object;
  TEST(var1.Length() == 20);
  TEST(var1[0].first == "a");
  TEST(var1[0].second.Type == UBJSONTuple::TYPE_UINT8);
  TEST(var1[0].second.UInt8 == 1);
  TEST(var1[1].first == "b");
  TEST(var1[1].second.Type == UBJSONTuple::TYPE_INT8);
  TEST(var1[1].second.Int8 == -2);
  TEST(var1[2].first == "c");
  TEST(var1[2].second.Type == UBJSONTuple::TYPE_UINT8);
  TEST(var1[2].second.UInt8 == 3);
  TEST(var1[3].first == "d");
  TEST(var1[3].second.Type == UBJSONTuple::TYPE_INT8);
  TEST(var1[3].second.Int8 == -4);
  TEST(var1[4].first == "e");
  TEST(var1[4].second.Type == UBJSONTuple::TYPE_UINT8);
  TEST(var1[4].second.UInt8 == 253);
  TEST(var1[5].first == "f");
  TEST(var1[5].second.Type == UBJSONTuple::TYPE_INT16);
  TEST(var1[5].second.Int16 == 30000);
  TEST(var1[6].first == "g");
  TEST(var1[6].second.Type == UBJSONTuple::TYPE_UINT8);
  TEST(var1[7].first == "h");
  TEST(var1[7].second.Type == UBJSONTuple::TYPE_UINT8);
  TEST(var1[8].first == "i");
  TEST(var1[8].second.Type == UBJSONTuple::TYPE_OBJECT);
  TEST(var1[8].second.Object[0].first == "a");
  TEST(var1[8].second.Object[0].second.UInt8 == 9);
  TEST(var1[8].second.Object[1].first == "c");
  TEST(var1[8].second.Object[1].second.String == Str("foo"));
  TEST(var1[8].second.Object[2].first == "d");
  TEST(var1[8].second.Object[2].second.Double == 10.0);
  TEST(var1[9].first == "x");
  TEST(var1[9].second.Type == UBJSONTuple::TYPE_FLOAT);
  TEST(var1[10].first == "y");
  TEST(var1[10].second.Type == UBJSONTuple::TYPE_DOUBLE);
  TEST(var1[11].first == "p");
  TEST(var1[11].second.Type == UBJSONTuple::TYPE_STRING);
  TEST(var1[12].first == "m");
  TEST(var1[12].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[13].first == "n");
  TEST(var1[13].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[14].first == "u");
  TEST(var1[14].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[15].first == "v");
  TEST(var1[15].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[16].first == "w");
  TEST(var1[16].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[17].first == "z");
  TEST(var1[17].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[18].first == "var");
  TEST(var1[18].second.Type == UBJSONTuple::TYPE_ARRAY);
  TEST(var1[19].first == "tuple");
  TEST(var1[19].second.Type == UBJSONTuple::TYPE_ARRAY);
  
  {
    std::fstream fso2("out2.ubj", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    Serializer<UBJSONEngine> s;
    s.Serialize(val, fso2);
  }

  ubjsontest t3 = { TEST_ENUM_VALUE, 0, 0, 0, 0, 0, 0, 0,{ 0 }, 0, 0, "",{ 0,0,0 },{ "","" } };
  {
    std::fstream fsi3("out2.ubj", std::ios_base::in | std::ios_base::binary);
    Serializer<UBJSONEngine> s;
    s.Parse<ubjsontest>(t3, fsi3);
  }
  VerifyUBJSON(t1, t3, __testret);

  ENDTEST;
}