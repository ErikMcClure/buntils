// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/JSON.h"
#include <fstream>
#include "bss-util/Geometry.h"

using namespace bss;

struct JSONtest2
{
  DynArray<JSONtest2, uint32_t, ARRAY_SAFE> value;
  std::array<int, 2> ia;

  template<typename Engine>
  void Serialize(Serializer<Engine>& s, const char*)
  {
    s.template EvaluateType<JSONtest2>(GenPair("value", value), GenPair("ia", ia));
  }
};

struct JSONtest
{
  int64_t a;
  uint16_t b;
  double c;
  Str test;
  JSONtest2 nested;
  DynArray<uint16_t> foo;
  std::vector<double> bar;
  DynArray<Str, uint16_t, ARRAY_SAFE> foobar;
  DynArray<JSONtest, uint32_t, ARRAY_SAFE> nestarray;
  JSONtest2 nested2;
  bool btrue;
  bool bfalse;
  float fixed[3];
  Hash<int, int> hash;
  Matrix<int, 2, 3> matrix;
  Polygon<int> polygon;
  Triangle<int> triangle;
  CircleSector<int> sector;
  Circle<int> circle;
  Ellipse<int> ellipse;
  std::tuple<short, Str, double> tuple;

  template<typename Engine>
  void Serialize(Serializer<Engine>& s, const char*)
  {
    s.template EvaluateType<JSONtest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c),
      GenPair("test", test),
      GenPair("nested", nested),
      GenPair("foo", foo),
      GenPair("bar", bar),
      GenPair("foobar", foobar),
      GenPair("nestarray", nestarray),
      GenPair("nested2", nested2),
      GenPair("btrue", btrue),
      GenPair("bfalse", bfalse),
      GenPair("fixed", fixed),
      GenPair("hash", hash),
      GenPair("matrix", matrix),
      GenPair("polygon", polygon),
      GenPair("triangle", triangle),
      GenPair("sector", sector),
      GenPair("circle", circle),
      GenPair("ellipse", ellipse),
      GenPair("tuple", tuple)
      );
  }
};

static_assert(internal::serializer::is_serializable<JSONEngine, JSONtest>::value, "object missing Serialize<Engine>(Serializer<Engine>&, const char*) function!");

void dotest_JSON(JSONtest& o, TESTDEF::RETPAIR& __testret)
{
  TEST(o.a == -5);
  TEST(o.b == 342);
  TEST(o.c == 23.7193);
  TEST(o.btrue);
  TEST(!o.bfalse);
  TEST(o.fixed[0] == 0.2f);
  TEST(o.fixed[1] == 23.1f);
  TEST(o.fixed[2] == -3.0f);
  TEST(o.test.length() > 0);
  TEST(o.nested.value.Length() == 2);
  TEST(o.nested.ia[0] == -1);
  TEST(o.nested.ia[1] == 2);
  TEST(o.foo.Length() == 6);
  TEST(o.foo[0] == 5);
  TEST(o.foo[1] == 6);
  TEST(o.foo[2] == 4);
  TEST(o.foo[3] == 2);
  TEST(o.foo[4] == 2);
  TEST(o.foo[5] == 3);
  TEST(o.bar.size() == 5);
  TEST(o.bar[0] == 3.3);
  TEST(o.bar[1] == 1.6543);
  TEST(o.bar[2] == 0.49873);
  TEST(o.bar[3] == 90);
  TEST(o.bar[4] == 4);
  TEST(o.foobar.Length() == 2);
  TEST(o.foobar[0] == "moar");
  TEST(o.foobar[1] == "");
  TEST(o.nestarray.Length() == 2);
  TEST(o.nestarray[1].b == 34);
  TEST(o.nested2.value.Length() == 0);
  TEST(o.hash.Length() == 3);
  TEST(o.hash[40] == 44);
  TEST(o.hash[41] == 45);
  TEST(o.hash[42] == 46);
  TEST(o.matrix.v[0][0] == 1);
  TEST(o.matrix.v[0][1] == 2);
  TEST(o.matrix.v[0][2] == 3);
  TEST(o.matrix.v[1][0] == 4);
  TEST(o.matrix.v[1][1] == 5);
  TEST(o.matrix.v[1][2] == 6);
  TEST(o.polygon[0].x == 1);
  TEST(o.polygon[0].y == 2);
  TEST(o.polygon[1].x == 3);
  TEST(o.polygon[1].y == 4);
  TEST(o.polygon[2].x == 5);
  TEST(o.polygon[2].y == 6);
  TEST(o.triangle.v[0].x == 1);
  TEST(o.triangle.v[0].y == 2);
  TEST(o.triangle.v[1].x == 3);
  TEST(o.triangle.v[1].y == 4);
  TEST(o.triangle.v[2].x == 5);
  TEST(o.triangle.v[2].y == 6);
  TEST(o.sector.v[0] == 1);
  TEST(o.sector.v[1] == 2);
  TEST(o.sector.v[2] == 3);
  TEST(o.sector.v[3] == 4);
  TEST(o.sector.v[4] == 5);
  TEST(o.sector.v[5] == 6);
  TEST(o.sector.x == 1);
  TEST(o.sector.y == 2);
  TEST(o.sector.outer == 3);
  TEST(o.sector.inner == 4);
  TEST(o.sector.min == 5);
  TEST(o.sector.range == 6);
  TEST(o.circle.v[0] == 1);
  TEST(o.circle.v[1] == 2);
  TEST(o.circle.v[2] == 3);
  TEST(o.ellipse.v[0] == 1);
  TEST(o.ellipse.v[1] == 2);
  TEST(o.ellipse.v[2] == 3);
  TEST(o.ellipse.v[3] == 4);
  auto[a, b, c] = o.tuple;
  TEST(a == 1);
  TEST(b == "2");
  TEST(c == 3.0);
}

TESTDEF::RETPAIR test_JSON()
{
  BEGINTEST;
  const char* json = "{ \"a\": -5, \"b\": 342  ,\"c\":23.7193 , \"btrue\": true, \"bfalse\": false, \"fixed\": [0.2, 23.1, -3, 4.0], \"test\":\"\\u01A8string {,};[]\\\"st\\\"'\\\n\\\r\\/\\u0FA8nb\\\"\", \"nested\" : { \"value\": [ { }, { } ], \"ia\":[  -1,2 ] }, \"foo\": [5 ,6, 4,2 ,  2,3,], \"bar\": [3.3,1.6543,0.49873,90, 4], \"foobar\":[\"moar\",\"\"], \"nestarray\": [null, { \"a\":, \"b\":34, }], \"nested2\": null, \"hash\": { \"40\" : 44, \"41\" : 45, \"42\" : 46, }, \"matrix\": [ [1,2,3],[4,5,6] ], \"polygon\": [[1,2],[3,4],[5,6]], \"triangle\": [[1,2],[3,4],[5,6]], \"sector\": [1,2,3,4,5,6], \"circle\": [1,2,3], \"ellipse\": [1,2,3,4], \"tuple\":[1, \"2\", 3.0] }";
  JSONtest o;
  o.btrue = false;
  o.bfalse = true;

  ParseJSON(o, json);
  dotest_JSON(o, __testret);

  WriteJSON(o, "out.json");
  std::fstream fs("out.json");

  JSONtest o2;
  o2.btrue = false;
  o2.bfalse = true;
  ParseJSON(o2, fs);
  dotest_JSON(o2, __testret);
  fs.close();

  WriteJSON(o, "pretty.json", 1);
  std::fstream fs2("pretty.json");

  JSONtest o3;
  o3.btrue = false;
  o3.bfalse = true;
  ParseJSON(o3, fs2);
  dotest_JSON(o3, __testret);
  fs2.close();

  std::fstream fs3("pretty.json");
  JSONValue var;
  ParseJSON(var, fs3);
  fs3.close();

  auto& var1 = var.get<JSONValue::JSONObject>();
  TEST(var1.Length() == 21);
  TEST(var1[0].first == "a");
  TEST(var1[0].second.is<int64_t>());
  TEST(var1[1].first == "b");
  TEST(var1[1].second.is<int64_t>());
  TEST(var1[2].first == "c");
  TEST(var1[2].second.is<double>());
  TEST(var1[3].first == "test");
  TEST(var1[3].second.is<Str>());
  TEST(var1[4].first == "nested");
  TEST(var1[4].second.is<JSONValue::JSONObject>());
  TEST(var1[4].second.get<JSONValue::JSONObject>()[0].first == "value");
  TEST(var1[4].second.get<JSONValue::JSONObject>()[0].second.is<JSONValue::JSONArray>());
  TEST(var1[4].second.get<JSONValue::JSONObject>()[0].second.get<JSONValue::JSONArray>()[0].is<JSONValue::JSONObject>());
  TEST(var1[5].first == "foo");
  TEST(var1[5].second.is<JSONValue::JSONArray>());

  auto& var2 = var1[5].second.get<JSONValue::JSONArray>();
  TEST(var2.Length() == 6);
  TEST(var2[0].get<int64_t>() == 5);
  TEST(var2[1].get<int64_t>() == 6);
  TEST(var2[2].get<int64_t>() == 4);
  TEST(var2[3].get<int64_t>() == 2);
  TEST(var2[4].get<int64_t>() == 2);
  TEST(var2[5].get<int64_t>() == 3);

  TEST(var1[6].first == "bar");
  TEST(var1[6].second.is<JSONValue::JSONArray>());

  auto& var3 = var1[6].second.get<JSONValue::JSONArray>();
  TEST(var3.Length() == 5);
  TEST(var3[0].is<double>());
  TEST(var3[0].get<double>() == 3.3);
  TEST(var3[1].is<double>());
  TEST(var3[1].get<double>() == 1.6543);
  TEST(var3[2].is<double>());
  TEST(var3[2].get<double>() == 0.49873);
  TEST(var3[3].is<int64_t>());
  TEST(var3[3].get<int64_t>() == 90);
  TEST(var3[4].is<int64_t>());
  TEST(var3[4].get<int64_t>() == 4);

  TEST(var1[7].first == "foobar");
  TEST(var1[7].second.is<JSONValue::JSONArray>());

  auto& var4 = var1[7].second.get<JSONValue::JSONArray>();
  TEST(var4.Length() == 2);
  TEST(var4[0].is<Str>());
  TEST(var4[0].get<Str>() == "moar");
  TEST(var4[1].is<Str>());
  TEST(var4[1].get<Str>() == "");

  TEST(var1[8].first == "nestarray");
  TEST(var1[8].second.is<JSONValue::JSONArray>());
  TEST(var1[9].first == "nested2");
  TEST(var1[9].second.is<JSONValue::JSONObject>());
  TEST(var1[10].first == "btrue");
  TEST(var1[10].second.is<bool>());
  TEST(var1[11].first == "bfalse");
  TEST(var1[11].second.is<bool>());
  TEST(var1[12].first == "fixed");
  TEST(var1[12].second.is<JSONValue::JSONArray>());

  WriteJSON<JSONValue>(var, "out2.json", 0);

  JSONtest o4;
  o4.btrue = false;
  o4.bfalse = true;
  std::fstream fs4("out2.json");
  ParseJSON(o4, fs4);
  fs4.close();
  dotest_JSON(o4, __testret);

  Str s; // test to ensure that invalid data does not crash or lock up the parser
  for(int i = (int)strlen(json); i > 0; --i)
  {
    s.assign(json, i);
    ParseJSON(o, s);
    ParseJSON(var, s);
  }
  ENDTEST;
}