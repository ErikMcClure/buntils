// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/TRBtree.h"
#include "buntils/algo.h"
#include "buntils/BlockAlloc.h"
#include <fstream>

using namespace bun;

bool verifytree(TRB_Node<int>* front, uint32_t& same)
{
  same = 0;
  uint32_t pass = 0;
  int last = -1;
  for(TRB_Node<int>* pnode = front; pnode != 0; pnode = pnode->next)
  {
    if(pnode->value == last)
      same += 1;
    if(pnode->value<last)
      pass += 1;
    last = pnode->value;
  }
  return !pass;
}

bool verify_unique_testnums()
{
  Hash<int, char> hash;

  for(size_t i = 0; i<TESTNUM; ++i)
  {
    if(hash.Exists(testnums[i]))
      return false;
    hash.Insert(testnums[i], 1);
  }
  return true;
}

//internal::TRB_NodeBase<TRB_Node<int>> internal::TRB_NodeBase<TRB_Node<int>>::NIL(static_cast<TRB_Node<int>*>(&internal::TRB_NodeBase<TRB_Node<int>>::NIL));
//TRB_Node<int>* internal::TRB_NodeBase<TRB_Node<int>>::pNIL = static_cast<TRB_Node<int>*>(&internal::TRB_NodeBase<TRB_Node<int>>::NIL);

TESTDEF::RETPAIR test_TRBTREE()
{
  BEGINTEST;
  BlockPolicy<TRB_Node<int>> fixedalloc;
  TRBtree<int, std::compare_three_way, PolicyAllocator<TRB_Node<int>, BlockPolicy>> blah(
    PolicyAllocator<TRB_Node<int>, BlockPolicy>{ fixedalloc });

  uint32_t same = 0;
  shuffle_testnums();

  blah.Clear();
  bool valid = true;
  for(size_t i = 0; i < TESTNUM; ++i)
  {
    if(testnums[i] >= TESTNUM)
      std::cout << "BIG: " << i << " = " << testnums[i] << std::endl;
    auto p = blah.Insert(testnums[i]);
    valid  = valid && blah.Validate(p, std::compare_three_way{});
  }

  TEST(valid);
  assert(verifytree(blah.Front(), same));
  TEST(blah.DEBUGVERIFY() >= 0);
  TEST(!blah.Get(-1));
  TEST(!blah.Get(TESTNUM + 1));

    shuffle_testnums();
  int num = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
  {
    auto p = blah.Get(testnums[i]);
    if(p != 0)
      num += (p->value == testnums[i]);
  }
  TEST(num == TESTNUM);

  TEST(blah.Remove(4));
  for(auto i : blah)
    assert(blah.Validate(i, std::compare_three_way{}));
  TEST(blah.GetNear(4)->value == 3);
  TEST(blah.GetNear(4, false)->value == 5);
  blah.Insert(4);
  auto p2 = blah.Get(4);
  TEST(blah.Validate(p2, std::compare_three_way{}));
  p2->value = -4;
  TEST(!blah.Validate(p2, std::compare_three_way{}));
  p2->value = 4;
  TEST(blah.Validate(p2, std::compare_three_way{}));

  shuffle_testnums();
  for(size_t i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);
  shuffle_testnums();
  for(size_t i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);

  TEST(verifytree(blah.Front(), same));
  TEST(blah.DEBUGVERIFY() >= 0);
  TEST(same == (TESTNUM * 2));

  shuffle_testnums();
  num = 0;
  int n2 = 0;
  int n3 = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
  {
    num += (blah.Get(testnums[i]) != 0);
    n2 += blah.Remove(testnums[i]);
    n2 += blah.Remove(testnums[i]);
    n2 += blah.Remove(testnums[i]);
    n3 += (!blah.Get(testnums[i]));
  }
  TEST(num == TESTNUM);
  TEST(n2 == TESTNUM * 3);
  TEST(n3 == TESTNUM);
  TEST(blah.DEBUGVERIFY() >= 0);

  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  ENDTEST;
}
