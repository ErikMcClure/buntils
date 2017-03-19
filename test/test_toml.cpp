// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cTOML.h"
#include <fstream>
#include "test.h"

using namespace bss_util;

struct TOMLtest3
{
  float f;

  template<typename Engine>
  void Serialize(cSerializer<Engine>& s)
  {
    s.template EvaluateType<TOMLtest3>(GenPair("f", f));
  }
};
struct TOMLtest2
{
  uint16_t a;
  TOMLtest3 test;

  template<typename Engine>
  void Serialize(cSerializer<Engine>& s)
  {
    s.template EvaluateType<TOMLtest2>(GenPair("a", a), GenPair("test", test));
  }
};

struct TOMLtest
{
  int64_t a;
  uint16_t b;
  double c;
  cStr test;
  TOMLtest2 test2;
  bool btrue;
  bool bfalse;
  cDynArray<double> d;
  int e[3];
  std::vector<cStr> f;
  std::array<bool, 2> g;
  cDynArray<TOMLtest2> nested;
  TOMLtest2 inlinetest;
#ifdef BSS_COMPILER_HAS_TIME_GET
  std::chrono::system_clock::time_point date;
#endif

  template<typename Engine>
  void Serialize(cSerializer<Engine>& s)
  {
    s.template EvaluateType<TOMLtest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c),
      GenPair("test", test),
      GenPair("test2", test2),
      GenPair("btrue", btrue),
      GenPair("bfalse", bfalse),
      GenPair("d", d),
      GenPair("e", e),
      GenPair("f", f),
      GenPair("g", g),
      GenPair("nested", nested),
#ifdef BSS_COMPILER_HAS_TIME_GET
      GenPair("date", date),
#endif
      GenPair("inlinetest", inlinetest)

      );
  }
};

void dotest_TOML(TOMLtest& o, TESTDEF::RETPAIR& __testret)
{
  TEST(o.a == -1);
  TEST(o.b == 2);
  TEST(o.c == 0.28);
  TEST(o.btrue == true);
  TEST(o.bfalse == false);
  TEST(o.d.Length() == 6);
  TEST(o.d[0] == 1e-6);
  TEST(o.d[1] == -2.0);
  TEST(o.d[2] == 0.3);
  TEST(o.d[3] == 0.1);
  TEST(o.d[4] == 1.0);
  TEST(o.d[5] == -2.0);
  TEST(o.e[0] == -2);
  TEST(o.e[1] == 100);
  TEST(o.e[2] == 8);
  TEST(o.f.size() == 4);
  TEST(!strcmp(o.f[0], "first"));
  TEST(!strcmp(o.f[1], "SECOND"));
  TEST(!strcmp(o.f[2], "ThirD"));
  TEST(!strcmp(o.f[3], "fOURTh"));
  TEST(!o.g[0]);
  TEST(o.g[1]);
  TEST(o.inlinetest.a == 6);
  TEST(o.inlinetest.test.f == 123.456f);
#ifdef BSS_COMPILER_HAS_TIME_GET
  time_t time = std::chrono::system_clock::to_time_t(o.date);
  std::tm* t = gmtime(&time);
  TEST(t->tm_year == (2006-1900));
  TEST(t->tm_mon == 0);
  TEST(t->tm_mday == 2);
  TEST(t->tm_hour == 22);
  TEST(t->tm_min == 4);
  TEST(t->tm_sec == 5);
#endif
  TEST(o.test2.a == 5);
  TEST(o.test2.test.f == -3.5f);
  TEST(o.nested.Length() == 2);
  TEST(o.nested[0].a == 2);
  TEST(o.nested[0].test.f == 80.9f);
  TEST(o.nested[1].a == 3);
  TEST(o.nested[1].test.f == -12.21f);
}

TESTDEF::RETPAIR test_TOML()
{
  BEGINTEST;

  const char* tomlfile = " \
a = -1 \n\
b = 2 \n\
c = 0.28 \n\
test = 'test string' \n\
btrue = true \n\
bfalse = false \n\
d = [1e-6, -2.0,.3, 0.1, +1, -2] \n\
e = [-2, 100, \n\
  +8] \n\
f = [\"first\", '''SECOND''', \"\"\"\n\
ThirD\"\"\", 'fOURTh']\n\
g = [false, true]\n\
inlinetest = { a = 6, test = { f = 123.456 } }\n\
date = 2006-01-02T15:04:05-07:00 \n\
\n\
[test2] \n\
a = 5 \n\
\n\
[[nested]]\n\
a = 2\n\
[nested.test]\n\
f = 80.9\n\
\n\
[[nested]]\n\
a = 3\n\
[nested.test]\n\
f = -12.21\n\
\n\
[test2.test] \n\
f = -3.5";

  TOMLtest tomltest;
  ParseTOML(tomltest, tomlfile);
  dotest_TOML(tomltest, __testret);

  WriteTOML(tomltest, "out.toml");

  std::ifstream fs("out.toml", std::ios_base::in | std::ios_base::binary);
  TOMLtest tomltest2;
  ParseTOML(tomltest2, fs);
  dotest_TOML(tomltest2, __testret);

  TOMLValue tomltest3;
  ParseTOML(tomltest3, tomlfile);
  auto& t3 = tomltest3.get<TOMLValue::TOMLTable>();
  TEST(t3["a"]->get<int64_t>() == -1);
  TEST(t3["b"]->get<int64_t>() == 2);
  TEST(t3["c"]->get<double>() == 0.28);
  TEST(t3["btrue"]->get<bool>() == true);
  TEST(t3["bfalse"]->get<bool>() == false);
  auto& t3d = t3["d"]->get<TOMLValue::TOMLArray>();
  TEST(t3d[0].get<double>() == 1e-6);
  TEST(t3d[1].get<int64_t>() == -2);
  TEST(t3d[2].get<double>() == 0.3);
  TEST(t3d[3].get<double>() == 0.1);
  TEST(t3d[4].get<int64_t>() == 1);
  TEST(t3d[5].get<int64_t>() == -2);
  auto& t3e = t3["e"]->get<TOMLValue::TOMLArray>();
  TEST(t3e[0].get<int64_t>() == -2);
  TEST(t3e[1].get<int64_t>() == 100);
  TEST(t3e[2].get<int64_t>() == 8);
  auto& t3f = t3["f"]->get<TOMLValue::TOMLArray>();
  TEST(!strcmp(t3f[0].get<cStr>(), "first"));
  TEST(!strcmp(t3f[1].get<cStr>(), "SECOND"));
  TEST(!strcmp(t3f[2].get<cStr>(), "ThirD"));
  TEST(!strcmp(t3f[3].get<cStr>(), "fOURTh"));
  auto& t3g = t3["g"]->get<TOMLValue::TOMLArray>();
  TEST(!t3g[0].get<bool>());
  TEST(t3g[1].get<bool>());
  auto& t3inline = t3["inlinetest"]->get<TOMLValue::TOMLTable>();
  TEST(t3inline["a"]->get<int64_t>() == 6);
  auto& t3t2 = t3["test2"]->get<TOMLValue::TOMLTable>();
  auto p = t3t2["a"];
  TEST(t3t2["a"]->get<int64_t>() == 5);


  ENDTEST;
}