// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cLinkedList.h"
#include "test.h"

using namespace bss_util;

template<bool L, bool S>
bool cmplist(cLinkedList<int, StandardAllocPolicy<cLLNode<int>>, L, S>& list, const char* nums)
{
  auto cur = list.begin();
  bool r = true;
  while(cur.IsValid() && *nums != 0 && r)
    r = (*(cur++) == (*(nums++) - '0'));
  return r;
}

TESTDEF::RETPAIR test_LINKEDLIST()
{
  BEGINTEST;
  cLinkedList<int, StandardAllocPolicy<cLLNode<int>>, true, true> test;
  cLLNode<int>* llp[5];

  llp[0] = test.Add(1);
  TEST(cmplist(test, "1"));
  llp[1] = test.Add(2);
  TEST(cmplist(test, "12"));
  llp[3] = test.Add(4);
  TEST(cmplist(test, "124"));
  llp[4] = test.Add(5);
  TEST(cmplist(test, "1245"));
  llp[2] = test.Insert(3, llp[3]);
  TEST(cmplist(test, "12345"));
  test.Remove(llp[3]);
  TEST(cmplist(test, "1235"));
  test.Remove(llp[0]);
  TEST(cmplist(test, "235"));
  test.Remove(llp[4]);
  TEST(cmplist(test, "23"));
  test.Insert(0, 0);
  TEST(cmplist(test, "230"));
  TEST(test.Length() == 3);

  cLinkedList<int, StandardAllocPolicy<cLLNode<int>>, false, true> test2;

  llp[0] = test2.Add(1);
  TEST(cmplist(test2, "1"));
  llp[1] = test2.Add(2);
  TEST(cmplist(test2, "21"));
  llp[3] = test2.Add(4);
  TEST(cmplist(test2, "421"));
  llp[4] = test2.Add(5);
  TEST(cmplist(test2, "5421"));
  llp[2] = test2.Insert(3, llp[1]);
  TEST(cmplist(test2, "54321"));
  test2.Remove(llp[3]);
  TEST(cmplist(test2, "5321"));
  test2.Remove(llp[0]);
  TEST(cmplist(test2, "532"));
  test2.Remove(llp[4]);
  TEST(cmplist(test2, "32"));
  test2.Insert(0, 0);
  TEST(cmplist(test2, "032"));
  TEST(test2.Length() == 3);

  cLinkedList<int, StandardAllocPolicy<cLLNode<int>>> test3;

  llp[0] = test3.Add(1);
  TEST(cmplist(test3, "1"));
  llp[1] = test3.Add(2);
  TEST(cmplist(test3, "21"));
  llp[3] = test3.Add(4);
  TEST(cmplist(test3, "421"));
  llp[4] = test3.Add(5);
  TEST(cmplist(test3, "5421"));
  llp[2] = test3.Insert(3, llp[1]);
  TEST(cmplist(test3, "54321"));
  test3.Remove(llp[3]);
  TEST(cmplist(test3, "5321"));
  test3.Remove(llp[0]);
  TEST(cmplist(test3, "532"));
  test3.Remove(llp[4]);
  TEST(cmplist(test3, "32"));
  test3.Insert(0, 0);
  TEST(cmplist(test3, "032"));

  ENDTEST;
}
