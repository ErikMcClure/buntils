// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss_stack.h"

using namespace bss_util;

TESTDEF::RETPAIR test_BSS_STACK()
{
  BEGINTEST;
  cStack<int> s(0);
  s.Clear();
  s.Push(1);
  TEST(s.Pop() == 1);
  s.Push(2);
  s.Push(3);
  s.Push(4);
  TEST(s.Length() == 3);
  TEST(s.Pop() == 4);
  TEST(s.Length() == 2);
  TEST(s.Pop() == 3);
  TEST(s.Length() == 1);
  s.Clear();
  s.Push(5);
  TEST(s.Peek() == 5);
  TEST(s.Length() == 1);
  cStack<int> s2(3);
  s2.Push(6);
  TEST(s2.Peek() == 6);
  s2 = s;
  TEST(s2.Peek() == 5);
  ENDTEST;
}