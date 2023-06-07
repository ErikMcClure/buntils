// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Serializer.h"

using namespace bun;

struct SerializerTest
{
  int a;
  float b;
  const char* c;

  template<typename Engine>
  void Serialize(Serializer<Engine>& e, const char*)
  {
    e.template EvaluateType<SerializerTest>(
      GenPair("a", a),
      GenPair("b", b),
      GenPair("c", c)
      );
  }
};

static_assert(!internal::serializer::is_serializer_array<Hash<int, int>>::value, "is_serializer_array failure");
static_assert(internal::serializer::is_serializer_array<Hash<int>>::value, "is_serializer_array failure");
static_assert(!internal::serializer::is_serializer_array<int>::value, "is_serializer_array failure");
static_assert(internal::serializer::is_serializer_array<DynArray<int>>::value, "is_serializer_array failure");
static_assert(!internal::serializer::is_serializer_array<DynArray<std::pair<int, int>>>::value, "is_serializer_array failure");
static_assert(internal::serializer::is_serializable<EmptyEngine, SerializerTest>::value, "is_serializer failure");
static_assert(!internal::serializer::is_serializable<EmptyEngine, EmptyEngine>::value, "is_serializer failure");
static_assert(!internal::serializer::is_serializable<EmptyEngine, TESTDEF>::value, "is_serializer failure");

TESTDEF::RETPAIR test_Serializer()
{
  BEGINTEST;
  Serializer<EmptyEngine> s;
  SerializerTest test;
  s.Serialize(test, std::cout, "");
  s.Parse(test, std::cin, "");
  ENDTEST;
}