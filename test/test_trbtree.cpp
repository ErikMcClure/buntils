// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/TRBtree.h"
#include "bss-util/algo.h"
#include "bss-util/BlockAlloc.h"
#include <fstream>

using namespace bss;

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
void serialize_testnum(const char* file)
{
  std::fstream f;
  f.open(file, std::ios::trunc | std::ios::out);
  for(size_t i = 0; i < TESTNUM; ++i)
    f << testnums[i] << std::endl;
  f.close();
}
void deserialize_testnum(const char* file)
{
  std::fstream f;
  f.open(file, std::ios::in);
  for(size_t i = 0; i < TESTNUM && !f.eof(); ++i)
  {
    f >> testnums[i];
    f.get();
  }
  f.close();
}

bool verify_unique_testnums()
{
  Hash<int, char, false> hash;

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
  TRBtree<int, CompT<int>, PolymorphicAllocator<TRB_Node<int>, BlockPolicy>> blah(&fixedalloc);

  uint32_t same = 0;
  Shuffle(testnums);

  blah.Clear();
  for(size_t i = 0; i < TESTNUM; ++i)
    blah.Insert(testnums[i]);

  assert(verifytree(blah.Front(), same));
  TEST(blah.DEBUGVERIFY() >= 0);
  TEST(!blah.Get(-1));
  TEST(!blah.Get(TESTNUM + 1));

    Shuffle(testnums);
  int num = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
  {
    auto p = blah.Get(testnums[i]);
    if(p != 0)
      num += (p->value == testnums[i]);
  }
  TEST(num == TESTNUM);

  TEST(blah.Remove(4));
  TEST(blah.GetNear(4)->value == 3);
  TEST(blah.GetNear(4, false)->value == 5);
  blah.Insert(4);

  Shuffle(testnums);
  for(size_t i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);
  Shuffle(testnums);
  for(size_t i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);

  TEST(verifytree(blah.Front(), same));
  TEST(blah.DEBUGVERIFY() >= 0);
  TEST(same == (TESTNUM * 2));

  Shuffle(testnums);
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
