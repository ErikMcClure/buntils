// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Stack.h"

using namespace bun;

TESTDEF::RETPAIR test_BUN_STACK()
{
  BEGINTEST;
  Stack<int> s(0);
  s.Clear();
  s.Push(1);
  TEST(s.Pop() == 1);
  s.Push(2);
  s.Push(3);
  s.Push(4);
  TEST(s.size() == 3);
  TEST(s.Pop() == 4);
  TEST(s.size() == 2);
  TEST(s.Pop() == 3);
  TEST(s.size() == 1);
  s.Clear();
  s.Push(5);
  TEST(s.Peek() == 5);
  TEST(s.size() == 1);
  Stack<int> s2(3);
  s2.Push(6);
  TEST(s2.Peek() == 6);
  s2 = s;
  TEST(s2.Peek() == 5);
  ENDTEST;
}