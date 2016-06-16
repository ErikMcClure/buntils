// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cSerializer.h"

using namespace bss_util;

struct SerializerTest
{
  int a;
  float b;
  const char* c;

  template<typename Engine>
  void Serialize(cSerializer<Engine>& e)
  {
    e.EvaluateType<SerializerTest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c)
      );
  }
};

TESTDEF::RETPAIR test_Serializer()
{
  BEGINTEST;
  cSerializer<EmptyEngine> s;
  SerializerTest test;
  s.Serialize(test, std::cout, "");
  s.Parse(test, std::cin, "");
  ENDTEST;
}