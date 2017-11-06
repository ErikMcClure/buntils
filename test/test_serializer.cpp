// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/Serializer.h"

using namespace bss;

struct SerializerTest
{
  int a;
  float b;
  const char* c;

  template<typename Engine>
  void Serialize(Serializer<Engine>& e)
  {
    e.template EvaluateType<SerializerTest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c)
      );
  }
};

TESTDEF::RETPAIR test_Serializer()
{
  BEGINTEST;
  Serializer<EmptyEngine> s;
  SerializerTest test;
  s.Serialize(test, std::cout, "");
  s.Parse(test, std::cin, "");
  ENDTEST;
}