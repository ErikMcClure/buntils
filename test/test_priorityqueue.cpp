// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/PriorityQueue.h"

using namespace bun;

TESTDEF::RETPAIR test_PRIORITYQUEUE()
{
  BEGINTEST;
  PriorityQueue<int, Str, CompTInv<int>, uint32_t, ARRAY_SAFE> q;

  q.Push(5, "5");
  q.Push(3, "3");
  q.Push(3, "3");
  q.Push(6, "6");
  q.Push(1, "1");
  q.Push(1, "1");
  q.Push(1, "1");
  q.Push(2, "2");

  TEST(q.Get(1).first == 2);
  TEST(q.Get(2).first == 1);
  TEST(q.Peek().first == 1);
  TEST(q.Peek().second == "1");
  TEST(q.Pop().first == 1);
  TEST(q.Pop().first == 1);
  TEST(q.Pop().first == 1);
  TEST(q.Pop().first == 2);
  TEST(q.Pop().first == 3);
  TEST(q.Pop().first == 3);

  q.Push(1, "1");
  q.Push(2, "2");
  q.Push(4, "4");

  TEST(q.Get(0).first == q.Peek().first)
    q.Discard();
  TEST(!q.Empty());
  TEST(q.Pop().first == 2);
  TEST(q.Pop().first == 4);
  TEST(q.Pop().first == 5);
  TEST(q.Pop().first == 6);
  TEST(q.Empty());

  ENDTEST;
}
