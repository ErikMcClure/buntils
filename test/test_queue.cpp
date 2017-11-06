// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/Queue.h"

using namespace bss;

TESTDEF::RETPAIR test_BSS_QUEUE()
{
  BEGINTEST;
  Queue<int> q(0);
  q.Clear();
  TEST(q.Capacity() == 0);
  TEST(q.Empty());
  q.Push(1);
  TEST(q.Pop() == 1);
  q.Push(2);
  q.Push(3);
  q.Push(4);
  TEST(q.Length() == 3);
  TEST(q.Pop() == 2);
  TEST(q.Length() == 2);
  TEST(q.Pop() == 3);
  TEST(q.Length() == 1);
  q.Push(2);
  q.Push(3);
  q.Push(4);
  TEST(q.Length() == 4);
  TEST(q.Pop() == 4);
  TEST(q.Length() == 3);
  TEST(q.Pop() == 2);
  TEST(q.Length() == 2);
  q.Clear();
  q.Push(5);
  TEST(q.Peek() == 5);
  q.Push(6);
  TEST(q.Peek() == 5);
  q.Discard();
  TEST(q.Length() == 1);
  TEST(q.Peek() == 6);
  Queue<int> q2(3);
  q2.Push(7);
  TEST(q2.Peek() == 7);
  q2 = q;
  TEST(q2.Peek() == 6);
  TEST(q.Peek() == 6);
  q.Push(7);
  q.Push(8);
  q.Push(9);
  TEST(q.Peek() == 6);
  q.Push(10);
  TEST(q.Peek() == 6);
  q.Push(11);
  TEST(q.Peek() == 6);
  q2 = q;
  TEST(q2.Peek() == 6);
  TEST(q2.Pop() == 6);
  TEST(q2.Pop() == 7);
  TEST(q2.Pop() == 8);
  TEST(q2.Pop() == 9);
  TEST(q2.Pop() == 10);
  TEST(q2.Pop() == 11);
  TEST(q2.Empty());

  ENDTEST;
}
