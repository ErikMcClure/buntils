// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cTOML.h"

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
  std::chrono::system_clock::time_point date;

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
      GenPair("inlinetest", inlinetest),
      GenPair("date", date)
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
  //TEST(date)
  TEST(o.test2.a == 5);
  TEST(o.test2.test.f == -3.5f);
  TEST(o.nested.Length() == 2);
  TEST(o.nested[0].a == 2);
  TEST(o.nested[1].a == 3);
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
\n\
[[nested]]\n\
a = 3\n\
\n\
[test2.test] \n\
f = -3.5";

  TOMLtest tomltest;
  ParseTOML(tomltest, tomlfile);
  dotest_TOML(tomltest, __testret);

  ENDTEST;
}